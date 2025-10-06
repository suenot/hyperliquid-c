/**
 * @file hyperliquid_trader.c
 * @brief Hyperliquid exchange implementation
 * 
 * Hyperliquid uses Ethereum-style signatures with private keys for authentication.
 * API Endpoints: https://api.hyperliquid.xyz
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <cjson/cJSON.h>

#include "hyperliquid.h"
#include "hl_internal.h"
#include "hl_http.h"
#include "hl_crypto_internal.h"
#include "hl_msgpack.h"
#include "hl_logger.h"

typedef struct {
    char wallet_address[256];        // Main wallet address (0x...)
    char private_key[256];           // API private key for signing
    char base_url[256];              // API base URL
    http_client_t *http_client;      // HTTP client instance
    bool testnet;                    // True if using testnet
} hyperliquid_data_t;

/**
 * @brief Get current timestamp in milliseconds
 */
static uint64_t get_timestamp_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
}

/**
 * @brief Get asset ID for a symbol (hardcoded for now - should use meta endpoint in production)
 */
static uint32_t get_asset_id(const char *symbol) {
    // Hardcoded asset IDs for testnet (from /info meta endpoint)
    // TODO: Cache and fetch dynamically from API
    if (strcmp(symbol, "BTC") == 0) return 3;
    if (strcmp(symbol, "ETH") == 0) return 2;
    if (strcmp(symbol, "SOL") == 0) return 0;
    if (strcmp(symbol, "APT") == 0) return 1;
    if (strcmp(symbol, "ATOM") == 0) return 2;
    
    // Default to BTC if unknown
    return 3;
}

/**
 * @brief Make request to Hyperliquid API
 */
static lv3_error_t hyperliquid_make_request(hyperliquid_data_t *data, 
                                             const char *endpoint,
                                             const char *json_body,
                                             http_response_t *response) {
    if (!data || !endpoint || !response) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    // Create request
    http_request_t request = {0};
    lv3_string_copy(request.method, "POST", sizeof(request.method));
    snprintf(request.url, sizeof(request.url), "%s%s", data->base_url, endpoint);
    
    // Create headers
    char headers[512];
    snprintf(headers, sizeof(headers), "Content-Type: application/json");
    request.headers = headers;
    request.timeout_ms = 30000;
    
    if (json_body && strlen(json_body) > 0) {
        request.body = (char*)json_body;
    }
    
    // Make POST request
    return http_client_post(data->http_client, &request, response);
}

/**
 * @brief Get account information from Hyperliquid
 */
static lv3_error_t hyperliquid_get_account_info(hyperliquid_data_t *data,
                                                 http_response_t *response) {
    // Create request body
    char body[512];
    snprintf(body, sizeof(body),
             "{\"type\":\"clearinghouseState\",\"user\":\"%s\"}",
             data->wallet_address);
    
    return hyperliquid_make_request(data, "/info", body, response);
}

/**
 * @brief Get market price for a symbol on Hyperliquid
 */
static lv3_error_t hyperliquid_get_market_price(exchange_trader_t *trader,
                                                 const char *symbol,
                                                 double *price_out) {
    if (!trader || !symbol || !price_out) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    *price_out = 0.0;
    
    hyperliquid_data_t *data = (hyperliquid_data_t*)trader->exchange_data;
    
    // Remove USDT suffix if present for Hyperliquid
    char trading_symbol[32];
    lv3_string_copy(trading_symbol, symbol, sizeof(trading_symbol));
    char *usdt_pos = strstr(trading_symbol, "USDT");
    if (usdt_pos) {
        *usdt_pos = '\0';
    }
    
    // Request market data
    char body[256];
    snprintf(body, sizeof(body),
             "{\"type\":\"l2Book\",\"coin\":\"%s\"}",
             trading_symbol);
    
    http_response_t response = {0};
    lv3_error_t result = hyperliquid_make_request(data, "/info", body, &response);
    
    if (result != LV3_SUCCESS || response.status_code != 200 || !response.body) {
        http_response_free(&response);
        return LV3_ERROR_NETWORK;
    }
    
    // Parse response
    cJSON *json = cJSON_Parse(response.body);
    if (!json) {
        http_response_free(&response);
        return LV3_ERROR_JSON;
    }
    
    lv3_error_t parse_result = LV3_ERROR_NOT_FOUND;
    
    // Try to extract price from order book
    cJSON *levels = cJSON_GetObjectItem(json, "levels");
    if (cJSON_IsArray(levels) && cJSON_GetArraySize(levels) >= 2) {
        cJSON *bids = cJSON_GetArrayItem(levels, 0);
        cJSON *asks = cJSON_GetArrayItem(levels, 1);
        
        if (cJSON_IsArray(bids) && cJSON_GetArraySize(bids) > 0 &&
            cJSON_IsArray(asks) && cJSON_GetArraySize(asks) > 0) {
            
            cJSON *best_bid = cJSON_GetArrayItem(bids, 0);
            cJSON *best_ask = cJSON_GetArrayItem(asks, 0);
            
            if (cJSON_IsArray(best_bid) && cJSON_IsArray(best_ask)) {
                cJSON *bid_price = cJSON_GetArrayItem(best_bid, 0);
                cJSON *ask_price = cJSON_GetArrayItem(best_ask, 0);
                
                if (cJSON_IsString(bid_price) && cJSON_IsString(ask_price)) {
                    double bid = atof(bid_price->valuestring);
                    double ask = atof(ask_price->valuestring);
                    *price_out = (bid + ask) / 2.0;
                    parse_result = LV3_SUCCESS;
                }
            }
        }
    }
    
    cJSON_Delete(json);
    http_response_free(&response);
    
    return parse_result;
}

/**
 * @brief Calculate quantity for Hyperliquid
 */
static lv3_error_t hyperliquid_calculate_quantity(exchange_trader_t *trader,
                                                   const char *symbol,
                                                   double usdt_amount,
                                                   int leverage,
                                                   char *quantity_out,
                                                   size_t quantity_size) {
    if (!trader || !symbol || !quantity_out) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    // Get current market price
    double price = 0.0;
    lv3_error_t result = hyperliquid_get_market_price(trader, symbol, &price);
    
    if (result != LV3_SUCCESS || price <= 0) {
        // Default fallback
        lv3_string_copy(quantity_out, "0.001", quantity_size);
        return LV3_SUCCESS;
    }
    
    // Calculate quantity: (usdt_amount * leverage) / price
    double calculated_qty = (usdt_amount * leverage) / price;
    snprintf(quantity_out, quantity_size, "%.6f", calculated_qty);
    
    // Remove trailing zeros
    char *end = quantity_out + strlen(quantity_out) - 1;
    while (end > quantity_out && *end == '0') {
        *end = '\0';
        end--;
    }
    if (*end == '.') *end = '\0';
    
    LV3_LOG_DEBUG("Calculated quantity: %s for %s at price $%.6f", 
                  quantity_out, symbol, price);
    
    return LV3_SUCCESS;
}

/**
 * @brief Place limit order on Hyperliquid with EIP-712 signature
 */
static lv3_error_t hyperliquid_place_limit_order(exchange_trader_t *trader,
                                                  const char *symbol,
                                                  const char *side,
                                                  double price,
                                                  double quantity,
                                                  char *order_id_out,
                                                  size_t order_id_size) {
    if (!trader || !symbol || !side || !order_id_out) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    order_id_out[0] = '\0';
    
    hyperliquid_data_t *data = (hyperliquid_data_t*)trader->exchange_data;
    
    // Remove USDT suffix if present
    char trading_symbol[32];
    lv3_string_copy(trading_symbol, symbol, sizeof(trading_symbol));
    char *usdt_pos = strstr(trading_symbol, "USDT");
    if (usdt_pos) {
        *usdt_pos = '\0';
    }
    
    // Get asset ID (simplified - should query from API)
    uint32_t asset_id = get_asset_id(trading_symbol);
    
    // Convert side to boolean (true = buy)
    bool is_buy = (strcmp(side, "Buy") == 0);
    
    // Format price and size as strings with appropriate precision
    char price_str[32], size_str[32];
    snprintf(price_str, sizeof(price_str), "%.8g", price);
    snprintf(size_str, sizeof(size_str), "%.8g", quantity);
    
    // Create order structure
    hl_order_t order = {
        .a = asset_id,
        .b = is_buy,
        .p = price_str,
        .s = size_str,
        .r = false,
        .limit = { .tif = "Gtc" }
    };
    
    // Get current timestamp
    uint64_t nonce = get_timestamp_ms();
    
    // Build action hash (connection_id)
    uint8_t connection_id[32];
    if (hl_build_order_hash(&order, 1, "na", nonce, NULL, connection_id) != 0) {
        LV3_LOG_ERROR("Failed to build order hash");
        return LV3_ERROR_EXCHANGE;
    }
    
    // Sign with EIP-712
    uint8_t signature[65];
    const char *source = data->testnet ? "b" : "a";
    if (eip712_sign_agent("Exchange", 1337, source, connection_id, 
                          data->private_key, signature) != 0) {
        LV3_LOG_ERROR("Failed to sign order");
        return LV3_ERROR_EXCHANGE;
    }
    
    // Convert signature to hex
    char sig_r[67], sig_s[67];
    bytes_to_hex(signature, 32, sig_r, true);
    bytes_to_hex(signature + 32, 32, sig_s, true);
    
    // Build request JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *action = cJSON_CreateObject();
    cJSON *orders_array = cJSON_CreateArray();
    cJSON *order_json = cJSON_CreateObject();
    cJSON *sig_json = cJSON_CreateObject();
    cJSON *order_type = cJSON_CreateObject();
    cJSON *limit_obj = cJSON_CreateObject();
    
    // Build order object
    cJSON_AddNumberToObject(order_json, "a", asset_id);
    cJSON_AddBoolToObject(order_json, "b", is_buy);
    cJSON_AddStringToObject(order_json, "p", price_str);
    cJSON_AddStringToObject(order_json, "s", size_str);
    cJSON_AddBoolToObject(order_json, "r", false);
    
    cJSON_AddStringToObject(limit_obj, "tif", "Gtc");
    cJSON_AddItemToObject(order_type, "limit", limit_obj);
    cJSON_AddItemToObject(order_json, "t", order_type);
    
    // Build action
    cJSON_AddItemToArray(orders_array, order_json);
    cJSON_AddStringToObject(action, "type", "order");
    cJSON_AddItemToObject(action, "orders", orders_array);
    cJSON_AddStringToObject(action, "grouping", "na");
    
    // Build signature object
    cJSON_AddStringToObject(sig_json, "r", sig_r);
    cJSON_AddStringToObject(sig_json, "s", sig_s);
    cJSON_AddNumberToObject(sig_json, "v", signature[64]);
    
    // Build final request (no vaultAddress as per CCXT)
    cJSON_AddItemToObject(root, "action", action);
    cJSON_AddNumberToObject(root, "nonce", nonce);
    cJSON_AddItemToObject(root, "signature", sig_json);
    
    char *body = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    
    if (!body) {
        LV3_LOG_ERROR("Failed to create request JSON");
        return LV3_ERROR_JSON;
    }
    
    LV3_LOG_DEBUG("Placing signed order: %s", body);
    
    // Send request
    http_response_t response = {0};
    lv3_error_t result = hyperliquid_make_request(data, "/exchange", body, &response);
    free(body);
    
    if (result != LV3_SUCCESS || response.status_code != 200 || !response.body) {
        fprintf(stderr, "❌ Failed to place order: HTTP %d\n", response.status_code);
        if (response.body) {
            fprintf(stderr, "API Response: %s\n", response.body);
        }
        http_response_free(&response);
        return LV3_ERROR_EXCHANGE;
    }
    
    // Parse response for order ID
    cJSON *json = cJSON_Parse(response.body);
    if (!json) {
        http_response_free(&response);
        return LV3_ERROR_JSON;
    }
    
    lv3_error_t parse_result = LV3_ERROR_EXCHANGE;
    
    cJSON *status = cJSON_GetObjectItem(json, "status");
    if (cJSON_IsString(status) && strcmp(status->valuestring, "ok") == 0) {
        cJSON *response_obj = cJSON_GetObjectItem(json, "response");
        if (cJSON_IsObject(response_obj)) {
            cJSON *data_obj = cJSON_GetObjectItem(response_obj, "data");
            if (cJSON_IsObject(data_obj)) {
                cJSON *statuses = cJSON_GetObjectItem(data_obj, "statuses");
                if (cJSON_IsArray(statuses) && cJSON_GetArraySize(statuses) > 0) {
                    cJSON *first_status = cJSON_GetArrayItem(statuses, 0);
                    if (cJSON_IsObject(first_status)) {
                        cJSON *resting = cJSON_GetObjectItem(first_status, "resting");
                        if (cJSON_IsObject(resting)) {
                            cJSON *oid = cJSON_GetObjectItem(resting, "oid");
                            if (cJSON_IsNumber(oid)) {
                                snprintf(order_id_out, order_id_size, "%d", oid->valueint);
                                parse_result = LV3_SUCCESS;
                                LV3_LOG_INFO("Order placed successfully: %s", order_id_out);
                            }
                        }
                    }
                }
            }
        }
    } else {
        LV3_LOG_ERROR("Order placement failed: %s", response.body);
    }
    
    cJSON_Delete(json);
    http_response_free(&response);
    
    return parse_result;
}

/**
 * @brief Cancel order on Hyperliquid with EIP-712 signature
 */
static lv3_error_t hyperliquid_cancel_order(exchange_trader_t *trader,
                                             const char *symbol,
                                             const char *order_id) {
    if (!trader || !symbol || !order_id) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    hyperliquid_data_t *data = (hyperliquid_data_t*)trader->exchange_data;
    
    // Remove USDT suffix if present
    char trading_symbol[32];
    lv3_string_copy(trading_symbol, symbol, sizeof(trading_symbol));
    char *usdt_pos = strstr(trading_symbol, "USDT");
    if (usdt_pos) {
        *usdt_pos = '\0';
    }
    
    // Get asset ID
    uint32_t asset_id = get_asset_id(trading_symbol);
    
    // Parse order ID to uint64
    uint64_t oid = strtoull(order_id, NULL, 10);
    
    // Create cancel structure
    hl_cancel_t cancel = {
        .a = asset_id,
        .o = oid
    };
    
    // Get current timestamp
    uint64_t nonce = get_timestamp_ms();
    
    // Build action hash
    uint8_t connection_id[32];
    if (hl_build_cancel_hash(&cancel, 1, nonce, NULL, connection_id) != 0) {
        LV3_LOG_ERROR("Failed to build cancel hash");
        return LV3_ERROR_EXCHANGE;
    }
    
    // Sign with EIP-712
    uint8_t signature[65];
    const char *source = data->testnet ? "b" : "a";
    if (eip712_sign_agent("Exchange", 1337, source, connection_id,
                          data->private_key, signature) != 0) {
        LV3_LOG_ERROR("Failed to sign cancel");
        return LV3_ERROR_EXCHANGE;
    }
    
    // Convert signature to hex
    char sig_r[67], sig_s[67];
    bytes_to_hex(signature, 32, sig_r, true);
    bytes_to_hex(signature + 32, 32, sig_s, true);
    
    // Build request JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *action = cJSON_CreateObject();
    cJSON *cancels_array = cJSON_CreateArray();
    cJSON *cancel_json = cJSON_CreateObject();
    cJSON *sig_json = cJSON_CreateObject();
    
    // Build cancel object
    cJSON_AddNumberToObject(cancel_json, "a", asset_id);
    cJSON_AddNumberToObject(cancel_json, "o", oid);
    
    // Build action
    cJSON_AddItemToArray(cancels_array, cancel_json);
    cJSON_AddStringToObject(action, "type", "cancel");
    cJSON_AddItemToObject(action, "cancels", cancels_array);
    
    // Build signature object
    cJSON_AddStringToObject(sig_json, "r", sig_r);
    cJSON_AddStringToObject(sig_json, "s", sig_s);
    cJSON_AddNumberToObject(sig_json, "v", signature[64]);
    
    // Build final request (no vaultAddress as per CCXT)
    cJSON_AddItemToObject(root, "action", action);
    cJSON_AddNumberToObject(root, "nonce", nonce);
    cJSON_AddItemToObject(root, "signature", sig_json);
    
    char *body = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    
    if (!body) {
        LV3_LOG_ERROR("Failed to create cancel request JSON");
        return LV3_ERROR_JSON;
    }
    
    LV3_LOG_DEBUG("Canceling signed order: %s", body);
    
    // Send request
    http_response_t response = {0};
    lv3_error_t result = hyperliquid_make_request(data, "/exchange", body, &response);
    free(body);
    
    if (result != LV3_SUCCESS || response.status_code != 200) {
        LV3_LOG_ERROR("Failed to cancel order: HTTP %d", response.status_code);
        if (response.body) {
            LV3_LOG_ERROR("Response: %s", response.body);
        }
        http_response_free(&response);
        return LV3_ERROR_EXCHANGE;
    }
    
    // Check response
    lv3_error_t parse_result = LV3_ERROR_EXCHANGE;
    if (response.body) {
        cJSON *json = cJSON_Parse(response.body);
        if (json) {
            cJSON *status = cJSON_GetObjectItem(json, "status");
            if (cJSON_IsString(status) && strcmp(status->valuestring, "ok") == 0) {
                parse_result = LV3_SUCCESS;
                LV3_LOG_INFO("Order canceled successfully: %s", order_id);
            } else {
                LV3_LOG_ERROR("Cancel failed: %s", response.body);
            }
            cJSON_Delete(json);
        }
    }
    
    http_response_free(&response);
    return parse_result;
}

/**
 * @brief Execute trade on Hyperliquid
 */
static lv3_error_t hyperliquid_execute_trade(exchange_trader_t *trader,
                                              const char *symbol,
                                              const char *direction,
                                              double usdt_amount,
                                              int leverage,
                                              double tp_percent __attribute__((unused)),
                                              double sl_percent __attribute__((unused)),
                                              order_result_t *result) {
    if (!trader || !symbol || !direction || !result) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    // hyperliquid_data_t *data = (hyperliquid_data_t*)trader->exchange_data;
    memset(result, 0, sizeof(order_result_t));
    
    // Prepare symbol (remove USDT suffix for Hyperliquid)
    char trading_symbol[32];
    lv3_string_copy(trading_symbol, symbol, sizeof(trading_symbol));
    char *usdt_pos = strstr(trading_symbol, "USDT");
    if (usdt_pos) {
        *usdt_pos = '\0';
    }
    
    LV3_LOG_INFO("Executing %s trade for %s: $%.2f at %dx leverage",
                 direction, trading_symbol, usdt_amount, leverage);
    
    // Get current market price
    double price = 0.0;
    lv3_error_t api_result = hyperliquid_get_market_price(trader, symbol, &price);
    
    if (api_result != LV3_SUCCESS || price <= 0) {
        LV3_LOG_WARNING("Symbol %s not available on Hyperliquid or price not found", 
                       trading_symbol);
        lv3_string_copy(result->error, "Symbol not available on Hyperliquid", 
                       sizeof(result->error));
        snprintf(result->message, sizeof(result->message),
                "New listing %s not yet supported on Hyperliquid", trading_symbol);
        return LV3_SUCCESS; // Not an error, just not available yet
    }
    
    // Calculate quantity
    char quantity[32];
    api_result = hyperliquid_calculate_quantity(trader, symbol, usdt_amount, 
                                                 leverage, quantity, sizeof(quantity));
    if (api_result != LV3_SUCCESS) {
        snprintf(result->error, sizeof(result->error), "Failed to calculate quantity");
        return LV3_SUCCESS;
    }
    
    // Convert side
    const char *side = (strcmp(direction, "long") == 0) ? "Buy" : "Sell";
    
    // Place market order (use limit order close to market price for immediate fill)
    // Hyperliquid doesn't have pure market orders, so we use aggressive limit orders
    double order_price = (strcmp(direction, "long") == 0) ? 
                         price * 1.01 : price * 0.99; // 1% slippage allowance
    
    char order_id[128];
    api_result = hyperliquid_place_limit_order(trader, symbol, side, order_price,
                                                atof(quantity), order_id, sizeof(order_id));
    
    if (api_result != LV3_SUCCESS) {
        snprintf(result->error, sizeof(result->error), "Failed to place order");
        snprintf(result->message, sizeof(result->message), "Trade execution failed");
        return LV3_SUCCESS;
    }
    
    // Success
    result->success = true;
    lv3_string_copy(result->symbol, trading_symbol, sizeof(result->symbol));
    lv3_string_copy(result->side, side, sizeof(result->side));
    lv3_string_copy(result->quantity, quantity, sizeof(result->quantity));
    lv3_string_copy(result->order_id, order_id, sizeof(result->order_id));
    lv3_string_copy(result->message, "Order executed successfully", sizeof(result->message));
    
    LV3_LOG_INFO("✅ Order placed successfully for %s (order_id: %s)", 
                 trading_symbol, order_id);
    
    return LV3_SUCCESS;
}

/**
 * @brief Test Hyperliquid connection
 */
static bool hyperliquid_test_connection(exchange_trader_t *trader) {
    if (!trader) return false;
    
    hyperliquid_data_t *data = (hyperliquid_data_t*)trader->exchange_data;
    
    http_response_t response = {0};
    lv3_error_t result = hyperliquid_get_account_info(data, &response);
    
    bool success = false;
    if (result == LV3_SUCCESS && response.status_code == 200 && response.body) {
        // Check if response is valid JSON
        cJSON *json = cJSON_Parse(response.body);
        if (json) {
            success = true;
            cJSON_Delete(json);
        }
    }
    
    http_response_free(&response);
    LV3_LOG_DEBUG("Hyperliquid connection test: %s", success ? "OK" : "FAILED");
    return success;
}

/**
 * @brief Get account balance from Hyperliquid
 */
static lv3_error_t hyperliquid_get_balance(exchange_trader_t *trader,
                                            char *balance_json_out,
                                            size_t json_size) {
    if (!trader || !balance_json_out) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    hyperliquid_data_t *data = (hyperliquid_data_t*)trader->exchange_data;
    
    // Request user state (includes balance)
    char body[512];
    snprintf(body, sizeof(body),
             "{\"type\":\"clearinghouseState\",\"user\":\"%s\"}",
             data->wallet_address);
    
    http_response_t response = {0};
    lv3_error_t result = hyperliquid_make_request(data, "/info", body, &response);
    
    if (result != LV3_SUCCESS || response.status_code != 200 || !response.body) {
        http_response_free(&response);
        return LV3_ERROR_NETWORK;
    }
    
    // Copy response to output
    if (response.body && strlen(response.body) < json_size) {
        lv3_string_copy(balance_json_out, response.body, json_size);
        result = LV3_SUCCESS;
    } else {
        result = LV3_ERROR_INVALID_PARAMS;
    }
    
    http_response_free(&response);
    return result;
}

/**
 * @brief Get open positions from Hyperliquid
 */
static lv3_error_t hyperliquid_get_positions(exchange_trader_t *trader,
                                              char *positions_json_out,
                                              size_t json_size) {
    if (!trader || !positions_json_out) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    hyperliquid_data_t *data = (hyperliquid_data_t*)trader->exchange_data;
    
    // Request user state (includes positions)
    char body[512];
    snprintf(body, sizeof(body),
             "{\"type\":\"clearinghouseState\",\"user\":\"%s\"}",
             data->wallet_address);
    
    http_response_t response = {0};
    lv3_error_t result = hyperliquid_make_request(data, "/info", body, &response);
    
    if (result != LV3_SUCCESS || response.status_code != 200 || !response.body) {
        http_response_free(&response);
        return LV3_ERROR_NETWORK;
    }
    
    // Parse and extract assetPositions
    cJSON *json = cJSON_Parse(response.body);
    if (!json) {
        http_response_free(&response);
        return LV3_ERROR_JSON;
    }
    
    cJSON *asset_positions = cJSON_GetObjectItem(json, "assetPositions");
    if (asset_positions) {
        char *positions_str = cJSON_PrintUnformatted(asset_positions);
        if (positions_str && strlen(positions_str) < json_size) {
            lv3_string_copy(positions_json_out, positions_str, json_size);
            result = LV3_SUCCESS;
        } else {
            result = LV3_ERROR_INVALID_PARAMS;
        }
        if (positions_str) free(positions_str);
    } else {
        lv3_string_copy(positions_json_out, "[]", json_size);
        result = LV3_SUCCESS;
    }
    
    cJSON_Delete(json);
    http_response_free(&response);
    return result;
}

/**
 * @brief Get trade history from Hyperliquid
 */
static lv3_error_t hyperliquid_get_trade_history(exchange_trader_t *trader,
                                                  char *history_json_out,
                                                  size_t json_size) {
    if (!trader || !history_json_out) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    hyperliquid_data_t *data = (hyperliquid_data_t*)trader->exchange_data;
    
    // Request user fills (trade history)
    char body[512];
    snprintf(body, sizeof(body),
             "{\"type\":\"userFills\",\"user\":\"%s\"}",
             data->wallet_address);
    
    http_response_t response = {0};
    lv3_error_t result = hyperliquid_make_request(data, "/info", body, &response);
    
    if (result != LV3_SUCCESS || response.status_code != 200 || !response.body) {
        http_response_free(&response);
        return LV3_ERROR_NETWORK;
    }
    
    // Copy response to output
    if (response.body && strlen(response.body) < json_size) {
        lv3_string_copy(history_json_out, response.body, json_size);
        result = LV3_SUCCESS;
    } else {
        result = LV3_ERROR_INVALID_PARAMS;
    }
    
    http_response_free(&response);
    return result;
}

/**
 * @brief Create Hyperliquid trader
 */
lv3_error_t hyperliquid_trader_create(const char *wallet_address,
                                       const char *private_key,
                                       bool testnet,
                                       exchange_trader_t **trader_out) {
    if (!wallet_address || !private_key || !trader_out) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    exchange_trader_t *trader = lv3_calloc(1, sizeof(exchange_trader_t));
    if (!trader) return LV3_ERROR_MEMORY;
    
    hyperliquid_data_t *data = lv3_calloc(1, sizeof(hyperliquid_data_t));
    if (!data) {
        lv3_free(trader);
        return LV3_ERROR_MEMORY;
    }
    
    // Initialize data
    lv3_string_copy(data->wallet_address, wallet_address, sizeof(data->wallet_address));
    lv3_string_copy(data->private_key, private_key, sizeof(data->private_key));
    data->testnet = testnet;
    
    if (testnet) {
        lv3_string_copy(data->base_url, "https://api.hyperliquid-testnet.xyz", 
                       sizeof(data->base_url));
    } else {
        lv3_string_copy(data->base_url, "https://api.hyperliquid.xyz", 
                       sizeof(data->base_url));
    }
    
    // Create HTTP client
    lv3_error_t result = http_client_create(NULL, &data->http_client);
    if (result != LV3_SUCCESS) {
        lv3_free(data);
        lv3_free(trader);
        return result;
    }
    
    // Initialize trader
    lv3_string_copy(trader->exchange_name, "hyperliquid", sizeof(trader->exchange_name));
    trader->exchange_data = data;
    trader->execute_trade = hyperliquid_execute_trade;
    trader->test_connection = hyperliquid_test_connection;
    trader->calculate_quantity = hyperliquid_calculate_quantity;
    trader->get_market_price = hyperliquid_get_market_price;
    trader->place_limit_order = hyperliquid_place_limit_order;
    trader->cancel_order = hyperliquid_cancel_order;
    trader->get_balance = hyperliquid_get_balance;
    trader->get_positions = hyperliquid_get_positions;
    trader->get_trade_history = hyperliquid_get_trade_history;
    
    *trader_out = trader;
    LV3_LOG_INFO("Hyperliquid trader created (%s)", testnet ? "testnet" : "mainnet");
    return LV3_SUCCESS;
}

