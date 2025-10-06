/**
 * @file trades.c
 * @brief Trade history functions implementation
 */

#include "hyperliquid.h"
#include "hl_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Forward declarations
static hl_error_t parse_trade_from_json(cJSON* trade_json, hl_trade_t* trade);

/**
 * @brief Fetch user trades (my trades)
 */
hl_error_t hl_fetch_my_trades(hl_client_t* client,
                             const char* symbol,
                             const char* since,
                             uint32_t limit,
                             hl_trades_t* trades) {
    if (!client || !trades) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Clear output
    memset(trades, 0, sizeof(hl_trades_t));

    // Prepare request
    char request_body[1024];
    const char* user_address = hl_client_get_wallet_address_old(client);

    if (!user_address) {
        return HL_ERROR_AUTH;
    }

    // Use userFills method
    snprintf(request_body, sizeof(request_body),
             "{\"type\":\"userFills\",\"user\":\"%s\"}", user_address);

    // Make request
    http_response_t* response = NULL;
    lv3_error_t err = http_client_post_old(client, "/info", request_body, NULL, &response);

    if (err != LV3_SUCCESS || !response || response->status_code != 200) {
        if (response) {
            http_response_free(response);
        }
        return HL_ERROR_NETWORK;
    }

    // Parse JSON response
    cJSON* json = cJSON_Parse(response->body);
    if (!json) {
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    // Response should be an array of trades
    if (!cJSON_IsArray(json)) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    int array_size = cJSON_GetArraySize(json);
    if (array_size == 0) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_SUCCESS; // No trades
    }

    // Allocate trades array
    trades->trades = calloc(array_size, sizeof(hl_trade_t));
    if (!trades->trades) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_MEMORY;
    }

    // Parse each trade
    size_t valid_trades = 0;
    cJSON* trade_json = NULL;
    cJSON_ArrayForEach(trade_json, json) {
        if (valid_trades >= array_size) break;

        hl_error_t parse_err = parse_trade_from_json(trade_json, &trades->trades[valid_trades]);
        if (parse_err == HL_SUCCESS) {
            valid_trades++;
        }
    }

    trades->count = valid_trades;

    cJSON_Delete(json);
    http_response_free(response);

    return HL_SUCCESS;
}

/**
 * @brief Fetch public trades
 */
hl_error_t hl_fetch_trades(hl_client_t* client,
                          const char* symbol,
                          const char* since,
                          uint32_t limit,
                          hl_trades_t* trades) {
    if (!client || !symbol || !trades) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Clear output
    memset(trades, 0, sizeof(hl_trades_t));

    // Prepare request - use recentTrades endpoint
    char request_body[1024];

    if (limit > 0) {
        snprintf(request_body, sizeof(request_body),
                 "{\"type\":\"recentTrades\",\"coin\":\"%s\",\"limit\":%u}",
                 symbol, limit);
    } else {
        snprintf(request_body, sizeof(request_body),
                 "{\"type\":\"recentTrades\",\"coin\":\"%s\"}",
                 symbol);
    }

    // Make request
    http_response_t* response = NULL;
    lv3_error_t err = http_client_post_old(client, "/info", request_body, NULL, &response);

    if (err != LV3_SUCCESS || !response || response->status_code != 200) {
        if (response) {
            http_response_free(response);
        }
        return HL_ERROR_NETWORK;
    }

    // Parse JSON response
    cJSON* json = cJSON_Parse(response->body);
    if (!json) {
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    // Response should be an array of trades
    if (!cJSON_IsArray(json)) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    int array_size = cJSON_GetArraySize(json);
    if (array_size == 0) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_SUCCESS; // No trades
    }

    // Allocate trades array
    trades->trades = calloc(array_size, sizeof(hl_trade_t));
    if (!trades->trades) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_MEMORY;
    }

    // Parse each trade
    size_t valid_trades = 0;
    cJSON* trade_json = NULL;
    cJSON_ArrayForEach(trade_json, json) {
        if (valid_trades >= array_size) break;

        hl_error_t parse_err = parse_trade_from_json(trade_json, &trades->trades[valid_trades]);
        if (parse_err == HL_SUCCESS) {
            // Set symbol if not set
            if (strlen(trades->trades[valid_trades].symbol) == 0) {
                strncpy(trades->trades[valid_trades].symbol, symbol, sizeof(trades->trades[valid_trades].symbol) - 1);
            }
            valid_trades++;
        }
    }

    trades->count = valid_trades;

    cJSON_Delete(json);
    http_response_free(response);

    return HL_SUCCESS;
}

/**
 * @brief Parse trade from JSON
 */
static hl_error_t parse_trade_from_json(cJSON* trade_json, hl_trade_t* trade) {
    if (!trade_json || !trade) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Extract fields based on Hyperliquid API response format
    cJSON* coin = cJSON_GetObjectItem(trade_json, "coin");
    cJSON* side = cJSON_GetObjectItem(trade_json, "side");
    cJSON* px = cJSON_GetObjectItem(trade_json, "px");
    cJSON* sz = cJSON_GetObjectItem(trade_json, "sz");
    cJSON* time = cJSON_GetObjectItem(trade_json, "time");
    cJSON* hash = cJSON_GetObjectItem(trade_json, "hash");

    // For user fills, different field names
    if (!px) px = cJSON_GetObjectItem(trade_json, "price");
    if (!sz) sz = cJSON_GetObjectItem(trade_json, "size");
    if (!time) time = cJSON_GetObjectItem(trade_json, "timestamp");

    if (!coin || !cJSON_IsString(coin) || !side || !cJSON_IsString(side)) {
        return HL_ERROR_JSON;
    }

    // Set symbol (coin)
    if (coin->valuestring) {
        strncpy(trade->symbol, coin->valuestring, sizeof(trade->symbol) - 1);
    }

    // Set side
    if (strcmp(side->valuestring, "B") == 0) {
        strcpy(trade->side, "buy");
    } else if (strcmp(side->valuestring, "A") == 0 || strcmp(side->valuestring, "Sell") == 0) {
        strcpy(trade->side, "sell");
    }

    // Set type
    strcpy(trade->type, "limit"); // Most trades are limit orders

    // Set price
    if (px) {
        if (cJSON_IsString(px)) {
            trade->price = atof(px->valuestring);
        } else if (cJSON_IsNumber(px)) {
            trade->price = px->valuedouble;
        }
    }

    // Set amount
    if (sz) {
        if (cJSON_IsString(sz)) {
            trade->amount = atof(sz->valuestring);
        } else if (cJSON_IsNumber(sz)) {
            trade->amount = sz->valuedouble;
        }
    }

    // Calculate cost
    trade->cost = trade->price * trade->amount;

    // Set timestamps
    if (time) {
        if (cJSON_IsNumber(time)) {
            unsigned long long ts = (unsigned long long)time->valuedouble;
            snprintf(trade->timestamp, sizeof(trade->timestamp), "%llu", ts);
            snprintf(trade->datetime, sizeof(trade->datetime), "%llu", ts); // TODO: format properly
        } else if (cJSON_IsString(time)) {
            strncpy(trade->timestamp, time->valuestring, sizeof(trade->timestamp) - 1);
            strncpy(trade->datetime, time->valuestring, sizeof(trade->datetime) - 1);
        }
    }

    // Set trade ID
    if (hash && cJSON_IsString(hash)) {
        strncpy(trade->id, hash->valuestring, sizeof(trade->id) - 1);
    }

    return HL_SUCCESS;
}
