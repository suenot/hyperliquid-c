/**
 * @file trading_api.c
 * @brief Public trading API wrappers for Hyperliquid C SDK
 * 
 * This file implements the new public API by wrapping the working
 * internal trading functions.
 */

#include "hyperliquid.h"
#include "hl_internal.h"
#include "hl_http.h"
#include "hl_crypto_internal.h"
#include "hl_msgpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

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
    
    // Default to 0 (unknown)
    return 0;
}

/**
 * @brief Convert lv3_error_t to hl_error_t
 */
static hl_error_t lv3_to_hl_error(lv3_error_t err) {
    switch (err) {
        case LV3_SUCCESS:
            return HL_SUCCESS;
        case LV3_ERROR_INVALID_PARAMS:
            return HL_ERROR_INVALID_PARAMS;
        case LV3_ERROR_NETWORK:
            return HL_ERROR_NETWORK;
        case LV3_ERROR_EXCHANGE:
            return HL_ERROR_API;
        case LV3_ERROR_JSON:
            return HL_ERROR_JSON;
        case LV3_ERROR_MEMORY:
            return HL_ERROR_MEMORY;
        case LV3_ERROR_TIMEOUT:
            return HL_ERROR_TIMEOUT;
        default:
            return HL_ERROR_API;
    }
}

/**
 * @brief Place limit order on Hyperliquid (new public API)
 */
hl_error_t hl_place_order(hl_client_t* client, 
                          const hl_order_request_t* request, 
                          hl_order_result_t* result) {
    if (!client || !request || !result) {
        return HL_ERROR_INVALID_PARAMS;
    }
    
    // Clear result
    memset(result, 0, sizeof(hl_order_result_t));
    result->order_id = NULL;
    
    // Extract client data using accessors
    const char* wallet = hl_client_get_wallet_address(client);
    const char* key = hl_client_get_private_key(client);
    bool testnet = hl_client_is_testnet(client);
    http_client_t* http = hl_client_get_http(client);
    pthread_mutex_t* mutex = hl_client_get_mutex(client);
    
    if (!wallet || !key || !http || !mutex) {
        snprintf(result->error, sizeof(result->error), "Invalid client state");
        return HL_ERROR_INVALID_PARAMS;
    }
    
    // Lock mutex for thread safety
    pthread_mutex_lock(mutex);
    
    // Get asset ID
    uint32_t asset_id = get_asset_id(request->symbol);
    if (asset_id == 0) {
        pthread_mutex_unlock(mutex);
        snprintf(result->error, sizeof(result->error), "Unknown symbol: %s", request->symbol);
        return HL_ERROR_INVALID_SYMBOL;
    }
    
    // Format price and size as strings
    char price_str[64], size_str[64];
    snprintf(price_str, sizeof(price_str), "%g", request->price);
    snprintf(size_str, sizeof(size_str), "%g", request->quantity);
    
    // Build order structure
    hl_order_t order = {
        .a = asset_id,
        .b = (request->side == HL_SIDE_BUY),
        .p = price_str,
        .s = size_str,
        .r = request->reduce_only,
        .limit = {.tif = "Gtc"} // Good Till Cancel
    };
    
    // Get current timestamp for nonce
    uint64_t nonce = get_timestamp_ms();
    
    // Build action hash
    uint8_t connection_id[32];
    if (hl_build_order_hash(&order, 1, "na", nonce, NULL, connection_id) != 0) {
        pthread_mutex_unlock(mutex);
        snprintf(result->error, sizeof(result->error), "Failed to build order hash");
        return HL_ERROR_SIGNATURE;
    }
    
    // Sign with EIP-712
    uint8_t signature[65];
    const char *source = testnet ? "b" : "a";
    if (eip712_sign_agent("Exchange", 1337, source, connection_id, key, signature) != 0) {
        pthread_mutex_unlock(mutex);
        snprintf(result->error, sizeof(result->error), "Failed to sign order");
        return HL_ERROR_SIGNATURE;
    }
    
    // Convert signature to hex
    char sig_r[67], sig_s[67];
    bytes_to_hex(signature, 32, sig_r, true);
    bytes_to_hex(signature + 32, 32, sig_s, true);
    
    // Build JSON request
    char json_body[4096];
    snprintf(json_body, sizeof(json_body),
             "{\"action\":{\"type\":\"order\",\"orders\":[{\"a\":%u,\"b\":%s,\"p\":\"%s\",\"s\":\"%s\",\"r\":%s,\"t\":{\"limit\":{\"tif\":\"Gtc\"}}}],\"grouping\":\"na\"},"
             "\"nonce\":%llu,"
             "\"signature\":{\"r\":\"%s\",\"s\":\"%s\",\"v\":%d},"
             "\"vaultAddress\":null}",
             asset_id,
             order.b ? "true" : "false",
             order.p,
             order.s,
             order.r ? "true" : "false",
             (unsigned long long)nonce,
             sig_r,
             sig_s,
             signature[64]);
    
    // Build URL
    const char *base_url = testnet ? "https://api.hyperliquid-testnet.xyz" : "https://api.hyperliquid.xyz";
    char url[512];
    snprintf(url, sizeof(url), "%s/exchange", base_url);
    
    // Make POST request
    http_response_t response = {0};
    const char *headers = "Content-Type: application/json";
    lv3_error_t err = http_client_post(http, url, json_body, headers, &response);
    
    pthread_mutex_unlock(mutex);
    
    if (err != LV3_SUCCESS || response.status_code != 200) {
        snprintf(result->error, sizeof(result->error), "HTTP request failed: %d", 
                 response.status_code);
        http_response_free(&response);
        return lv3_to_hl_error(err);
    }
    
    // Parse response JSON
    if (!response.body) {
        http_response_free(&response);
        snprintf(result->error, sizeof(result->error), "Empty response");
        return HL_ERROR_API;
    }
    
    // Simple JSON parsing to extract order ID
    // Response format: {"status":"ok","response":{"data":{"statuses":[{"resting":{"oid":123}}]}}}
    char *oid_str = strstr(response.body, "\"oid\":");
    if (oid_str) {
        oid_str += 6; // Skip "\"oid\":"
        char order_id_buf[64];
        int oid_int = 0;
        if (sscanf(oid_str, "%d", &oid_int) == 1) {
            snprintf(order_id_buf, sizeof(order_id_buf), "%d", oid_int);
            result->order_id = strdup(order_id_buf);
            result->status = HL_ORDER_STATUS_OPEN;
            result->filled_quantity = 0.0;
            result->average_price = 0.0;
        }
    }
    
    http_response_free(&response);
    
    if (!result->order_id) {
        snprintf(result->error, sizeof(result->error), "Failed to parse order ID from response");
        return HL_ERROR_API;
    }
    
    return HL_SUCCESS;
}

/**
 * @brief Cancel order on Hyperliquid (new public API)
 */
hl_error_t hl_cancel_order(hl_client_t* client, 
                           const char* symbol,
                           const char* order_id,
                           hl_cancel_result_t* result) {
    if (!client || !symbol || !order_id || !result) {
        return HL_ERROR_INVALID_PARAMS;
    }
    
    // Clear result
    memset(result, 0, sizeof(hl_cancel_result_t));
    result->cancelled = false;
    
    // Extract client data using accessors
    const char* wallet = hl_client_get_wallet_address(client);
    const char* key = hl_client_get_private_key(client);
    bool testnet = hl_client_is_testnet(client);
    http_client_t* http = hl_client_get_http(client);
    pthread_mutex_t* mutex = hl_client_get_mutex(client);
    
    if (!wallet || !key || !http || !mutex) {
        snprintf(result->error, sizeof(result->error), "Invalid client state");
        return HL_ERROR_INVALID_PARAMS;
    }
    
    // Lock mutex for thread safety
    pthread_mutex_lock(mutex);
    
    // Get asset ID
    uint32_t asset_id = get_asset_id(symbol);
    if (asset_id == 0) {
        pthread_mutex_unlock(mutex);
        snprintf(result->error, sizeof(result->error), "Unknown symbol: %s", symbol);
        return HL_ERROR_INVALID_SYMBOL;
    }
    
    // Parse order ID
    uint64_t oid = strtoull(order_id, NULL, 10);
    
    // Build cancel structure
    hl_cancel_t cancel = {
        .a = asset_id,
        .o = oid
    };
    
    // Get current timestamp
    uint64_t nonce = get_timestamp_ms();
    
    // Build action hash
    uint8_t connection_id[32];
    if (hl_build_cancel_hash(&cancel, 1, nonce, NULL, connection_id) != 0) {
        pthread_mutex_unlock(mutex);
        snprintf(result->error, sizeof(result->error), "Failed to build cancel hash");
        return HL_ERROR_SIGNATURE;
    }
    
    // Sign with EIP-712
    uint8_t signature[65];
    const char *source = testnet ? "b" : "a";
    if (eip712_sign_agent("Exchange", 1337, source, connection_id, key, signature) != 0) {
        pthread_mutex_unlock(mutex);
        snprintf(result->error, sizeof(result->error), "Failed to sign cancel");
        return HL_ERROR_SIGNATURE;
    }
    
    // Convert signature to hex
    char sig_r[67], sig_s[67];
    bytes_to_hex(signature, 32, sig_r, true);
    bytes_to_hex(signature + 32, 32, sig_s, true);
    
    // Build JSON request
    char json_body[2048];
    snprintf(json_body, sizeof(json_body),
             "{\"action\":{\"type\":\"cancel\",\"cancels\":[{\"a\":%u,\"o\":%llu}]},"
             "\"nonce\":%llu,"
             "\"signature\":{\"r\":\"%s\",\"s\":\"%s\",\"v\":%d},"
             "\"vaultAddress\":null}",
             asset_id,
             (unsigned long long)oid,
             (unsigned long long)nonce,
             sig_r,
             sig_s,
             signature[64]);
    
    // Build URL
    const char *base_url = testnet ? "https://api.hyperliquid-testnet.xyz" : "https://api.hyperliquid.xyz";
    char url[512];
    snprintf(url, sizeof(url), "%s/exchange", base_url);
    
    // Make POST request
    http_response_t response = {0};
    const char *headers = "Content-Type: application/json";
    lv3_error_t err = http_client_post(http, url, json_body, headers, &response);
    
    pthread_mutex_unlock(mutex);
    
    if (err != LV3_SUCCESS || response.status_code != 200) {
        snprintf(result->error, sizeof(result->error), "HTTP request failed: %d", 
                 response.status_code);
        http_response_free(&response);
        return lv3_to_hl_error(err);
    }
    
    // Check response
    if (response.body && strstr(response.body, "\"status\":\"ok\"")) {
        result->cancelled = true;
    } else {
        snprintf(result->error, sizeof(result->error), "Cancel failed: %s", 
                 response.body ? response.body : "unknown error");
    }
    
    http_response_free(&response);
    
    return result->cancelled ? HL_SUCCESS : HL_ERROR_API;
}

