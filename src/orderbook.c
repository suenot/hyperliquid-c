/**
 * @file orderbook.c
 * @brief Order book implementation
 */

#include "hyperliquid.h"
#include "hl_http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cjson/cJSON.h>

// Internal client accessor (from client.c)
extern const char* hl_client_get_wallet_address(hl_client_t* client);
extern bool hl_client_is_testnet(hl_client_t* client);
extern void* hl_client_get_http(hl_client_t* client);

/**
 * @brief Get base URL for client
 */
static const char* get_base_url(hl_client_t* client) {
    return hl_client_is_testnet(client) ?
        "https://api.hyperliquid-testnet.xyz" :
        "https://api.hyperliquid.xyz";
}

/**
 * @brief Parse single order book level from JSON
 */
static hl_error_t parse_orderbook_level(cJSON* level_json, hl_book_level_t* level) {
    if (!level_json || !level) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Parse price (px field)
    cJSON* px_json = cJSON_GetObjectItem(level_json, "px");
    if (!px_json || !cJSON_IsString(px_json)) {
        return HL_ERROR_PARSE;
    }
    level->price = atof(px_json->valuestring);

    // Parse quantity (sz field)
    cJSON* sz_json = cJSON_GetObjectItem(level_json, "sz");
    if (!sz_json || !cJSON_IsString(sz_json)) {
        return HL_ERROR_PARSE;
    }
    level->quantity = atof(sz_json->valuestring);

    return HL_SUCCESS;
}

/**
 * @brief Parse order book levels array
 */
static hl_error_t parse_orderbook_levels(cJSON* levels_json, hl_book_level_t** levels, size_t* count, uint32_t max_depth) {
    if (!levels_json || !levels || !count) {
        return HL_ERROR_INVALID_PARAMS;
    }

    if (!cJSON_IsArray(levels_json)) {
        return HL_ERROR_PARSE;
    }

    size_t num_levels = cJSON_GetArraySize(levels_json);
    if (num_levels == 0) {
        *levels = NULL;
        *count = 0;
        return HL_SUCCESS;
    }

    // Limit depth if specified
    if (max_depth > 0 && num_levels > max_depth) {
        num_levels = max_depth;
    }

    // Allocate memory for levels
    hl_book_level_t* levels_array = calloc(num_levels, sizeof(hl_book_level_t));
    if (!levels_array) {
        return HL_ERROR_MEMORY;
    }

    // Parse each level
    size_t valid_levels = 0;
    for (size_t i = 0; i < num_levels; i++) {
        cJSON* level_json = cJSON_GetArrayItem(levels_json, i);
        if (level_json) {
            hl_error_t err = parse_orderbook_level(level_json, &levels_array[valid_levels]);
            if (err == HL_SUCCESS) {
                valid_levels++;
            }
        }
    }

    *levels = levels_array;
    *count = valid_levels;
    return HL_SUCCESS;
}

/**
 * @brief Fetch order book from API
 */
hl_error_t hl_fetch_order_book(hl_client_t* client, const char* symbol, uint32_t depth, hl_orderbook_t* book) {
    if (!client || !symbol || !book) {
        return HL_ERROR_INVALID_PARAMS;
    }

    memset(book, 0, sizeof(hl_orderbook_t));

    // Get markets data first
    hl_markets_t markets = {0};
    hl_error_t err = hl_fetch_markets(client, &markets);
    if (err != HL_SUCCESS) {
        return err;
    }

    // Get asset ID for the symbol
    uint32_t asset_id;
    err = hl_get_asset_id(&markets, symbol, &asset_id);
    if (err != HL_SUCCESS) {
        hl_markets_free(&markets);
        return err;
    }

    // Determine if it's a swap or spot market
    const hl_market_t* market_info;
    err = hl_get_market(&markets, symbol, &market_info);
    if (err != HL_SUCCESS) {
        hl_markets_free(&markets);
        return err;
    }

    // Prepare request
    http_client_t* http = (http_client_t*)hl_client_get_http(client);
    if (!http) {
        return HL_ERROR_INVALID_PARAMS;
    }

    const char* base_url = get_base_url(client);
    char url[256];
    snprintf(url, sizeof(url), "%s/info", base_url);

    // Build request body
    char body[256];
    if (market_info->type == HL_MARKET_SWAP) {
        // For swaps, use coin name (baseName)
        snprintf(body, sizeof(body), "{\"type\":\"l2Book\",\"coin\":\"%s\"}", market_info->base);
    } else {
        // For spots, use asset ID
        snprintf(body, sizeof(body), "{\"type\":\"l2Book\",\"coin\":\"%u\"}", asset_id);
    }

    // Make request
    http_response_t response = {0};
    lv3_error_t http_err = http_client_post(http, url, body, "Content-Type: application/json", &response);

    if (http_err != LV3_SUCCESS) {
        http_response_free(&response);
        return HL_ERROR_NETWORK;
    }

    if (response.status_code != 200) {
        http_response_free(&response);
        return HL_ERROR_API;
    }

    // Parse response
    cJSON* json = cJSON_Parse(response.body);
    http_response_free(&response);

    if (!json) {
        return HL_ERROR_PARSE;
    }

    // Extract levels array
    cJSON* levels_json = cJSON_GetObjectItem(json, "levels");
    if (!levels_json || !cJSON_IsArray(levels_json) || cJSON_GetArraySize(levels_json) < 2) {
        cJSON_Delete(json);
        return HL_ERROR_PARSE;
    }

    // Copy symbol
    strncpy(book->symbol, symbol, sizeof(book->symbol) - 1);

    // Parse bids (index 0)
    cJSON* bids_json = cJSON_GetArrayItem(levels_json, 0);
    err = parse_orderbook_levels(bids_json, &book->bids, &book->bids_count, depth);
    if (err != HL_SUCCESS) {
        hl_free_orderbook(book);
        cJSON_Delete(json);
        hl_markets_free(&markets);
        return err;
    }

    // Parse asks (index 1)
    cJSON* asks_json = cJSON_GetArrayItem(levels_json, 1);
    err = parse_orderbook_levels(asks_json, &book->asks, &book->asks_count, depth);
    if (err != HL_SUCCESS) {
        hl_free_orderbook(book);
        cJSON_Delete(json);
        hl_markets_free(&markets);
        return err;
    }

    // Parse timestamp
    cJSON* time_json = cJSON_GetObjectItem(json, "time");
    if (time_json && cJSON_IsString(time_json)) {
        book->timestamp_ms = strtoull(time_json->valuestring, NULL, 10);
    } else {
        book->timestamp_ms = (uint64_t)time(NULL) * 1000; // Fallback to current time
    }

    cJSON_Delete(json);
    hl_markets_free(&markets);
    return HL_SUCCESS;
}

/**
 * @brief Free order book memory
 */
void hl_free_orderbook(hl_orderbook_t* book) {
    if (book) {
        if (book->bids) {
            free(book->bids);
            book->bids = NULL;
        }
        if (book->asks) {
            free(book->asks);
            book->asks = NULL;
        }
        book->bids_count = 0;
        book->asks_count = 0;
        book->timestamp_ms = 0;
        book->symbol[0] = '\0';
    }
}

/**
 * @brief Get best bid price from order book
 */
double hl_orderbook_get_best_bid(const hl_orderbook_t* book) {
    if (!book || !book->bids || book->bids_count == 0) {
        return 0.0;
    }
    return book->bids[0].price; // Bids are sorted high to low
}

/**
 * @brief Get best ask price from order book
 */
double hl_orderbook_get_best_ask(const hl_orderbook_t* book) {
    if (!book || !book->asks || book->asks_count == 0) {
        return 0.0;
    }
    return book->asks[0].price; // Asks are sorted low to high
}

/**
 * @brief Get spread (ask - bid) from order book
 */
double hl_orderbook_get_spread(const hl_orderbook_t* book) {
    double bid = hl_orderbook_get_best_bid(book);
    double ask = hl_orderbook_get_best_ask(book);

    if (bid > 0.0 && ask > 0.0) {
        return ask - bid;
    }
    return 0.0;
}

/**
 * @brief Get bid volume at specified depth
 */
double hl_orderbook_get_bid_volume(const hl_orderbook_t* book, uint32_t depth) {
    if (!book || !book->bids) {
        return 0.0;
    }

    size_t levels_to_sum = (depth == 0) ? book->bids_count : (size_t)depth;
    if (levels_to_sum > book->bids_count) {
        levels_to_sum = book->bids_count;
    }

    double total_volume = 0.0;
    for (size_t i = 0; i < levels_to_sum; i++) {
        total_volume += book->bids[i].quantity;
    }

    return total_volume;
}

/**
 * @brief Get ask volume at specified depth
 */
double hl_orderbook_get_ask_volume(const hl_orderbook_t* book, uint32_t depth) {
    if (!book || !book->asks) {
        return 0.0;
    }

    size_t levels_to_sum = (depth == 0) ? book->asks_count : (size_t)depth;
    if (levels_to_sum > book->asks_count) {
        levels_to_sum = book->asks_count;
    }

    double total_volume = 0.0;
    for (size_t i = 0; i < levels_to_sum; i++) {
        total_volume += book->asks[i].quantity;
    }

    return total_volume;
}

/**
 * @brief Legacy wrapper for hl_get_orderbook
 */
hl_error_t hl_get_orderbook(hl_client_t* client, const char* symbol, uint32_t depth, hl_orderbook_t* book) {
    return hl_fetch_order_book(client, symbol, depth, book);
}
