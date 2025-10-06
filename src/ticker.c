/**
 * @file ticker.c
 * @brief Ticker data implementation
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

// Forward declaration for internal function from markets.c
extern hl_error_t parse_swap_market(cJSON* universe_item, cJSON* context_item, int base_id, hl_market_t* market);

/**
 * @brief Get base URL for client
 */
static const char* get_base_url(hl_client_t* client) {
    return hl_client_is_testnet(client) ?
        "https://api.hyperliquid-testnet.xyz" :
        "https://api.hyperliquid.xyz";
}

/**
 * @brief Create ISO 8601 datetime string from timestamp
 */
static void timestamp_to_datetime(uint64_t timestamp_ms, char* datetime, size_t datetime_size) {
    time_t timestamp_s = (time_t)(timestamp_ms / 1000);
    struct tm* tm_info = gmtime(&timestamp_s);

    if (tm_info) {
        strftime(datetime, datetime_size, "%Y-%m-%dT%H:%M:%S.000Z", tm_info);
    } else {
        datetime[0] = '\0';
    }
}

/**
 * @brief Parse ticker data from market context
 */
static hl_error_t parse_ticker_from_market(const hl_market_t* market, hl_ticker_t* ticker) {
    if (!market || !ticker) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Copy symbol
    strncpy(ticker->symbol, market->symbol, sizeof(ticker->symbol) - 1);

    // Set prices
    ticker->last_price = market->mark_price;  // Use mark price as last price
    ticker->close = market->mark_price;       // Same for close

    // For bid/ask we don't have direct data, so we'll use mark price +/- small offset
    // In real implementation, this would come from order book data
    if (market->mark_price > 0) {
        double spread = market->mark_price * 0.0001; // 0.01% spread
        ticker->bid = market->mark_price - spread;
        ticker->ask = market->mark_price + spread;
    }

    // Previous close - we don't have this data
    ticker->previous_close = 0.0; // Not available

    // Volume - we have day volume
    ticker->volume_24h = market->day_volume;
    ticker->quote_volume = market->day_volume;

    // 24h high/low - not available in market data
    ticker->high_24h = 0.0;
    ticker->low_24h = 0.0;
    ticker->change_24h = 0.0;

    // Timestamps - current time
    ticker->timestamp = (uint64_t)time(NULL) * 1000;
    timestamp_to_datetime(ticker->timestamp, ticker->datetime, sizeof(ticker->datetime));

    // Additional data for swaps
    ticker->mark_price = market->mark_price;
    ticker->oracle_price = market->oracle_price;
    ticker->funding_rate = market->funding_rate;
    ticker->open_interest = market->open_interest;

    return HL_SUCCESS;
}

/**
 * @brief Fetch fresh market data from API
 */
static hl_error_t fetch_fresh_market_data(hl_client_t* client, hl_markets_t* markets) {
    http_client_t* http = (http_client_t*)hl_client_get_http(client);
    if (!http) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Build request for both swap and spot markets
    char body[512];
    snprintf(body, sizeof(body), "{\"type\":\"metaAndAssetCtxs\"}");

    const char* base_url = get_base_url(client);
    char url[256];
    snprintf(url, sizeof(url), "%s/info", base_url);

    // Make request
    http_response_t response = {0};
    lv3_error_t err = http_client_post(http, url, body, "Content-Type: application/json", &response);

    if (err != LV3_SUCCESS) {
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

    if (!cJSON_IsArray(json) || cJSON_GetArraySize(json) < 2) {
        cJSON_Delete(json);
        return HL_ERROR_PARSE;
    }

    // Get meta and contexts
    cJSON* meta = cJSON_GetArrayItem(json, 0);
    cJSON* contexts = cJSON_GetArrayItem(json, 1);

    if (!meta || !contexts || !cJSON_IsArray(contexts)) {
        cJSON_Delete(json);
        return HL_ERROR_PARSE;
    }

    cJSON* universe = cJSON_GetObjectItem(meta, "universe");
    if (!universe || !cJSON_IsArray(universe)) {
        cJSON_Delete(json);
        return HL_ERROR_PARSE;
    }

    int swap_count = cJSON_GetArraySize(universe);
    if (swap_count == 0) {
        cJSON_Delete(json);
        return HL_SUCCESS;
    }

    // Allocate markets array
    hl_market_t* markets_array = calloc(swap_count, sizeof(hl_market_t));
    if (!markets_array) {
        cJSON_Delete(json);
        return HL_ERROR_MEMORY;
    }

    // Parse each market
    int valid_markets = 0;
    for (int i = 0; i < swap_count; i++) {
        cJSON* universe_item = cJSON_GetArrayItem(universe, i);
        cJSON* context_item = cJSON_GetArrayItem(contexts, i);

        if (universe_item && context_item) {
            hl_error_t parse_err = parse_swap_market(universe_item, context_item, i, &markets_array[valid_markets]);
            if (parse_err == HL_SUCCESS) {
                valid_markets++;
            }
        }
    }

    markets->markets = markets_array;
    markets->count = valid_markets;

    cJSON_Delete(json);
    return HL_SUCCESS;
}

/**
 * @brief Get ticker for a single symbol
 */
hl_error_t hl_get_ticker(hl_client_t* client, const char* symbol, hl_ticker_t* ticker) {
    if (!client || !symbol || !ticker) {
        return HL_ERROR_INVALID_PARAMS;
    }

    memset(ticker, 0, sizeof(hl_ticker_t));

    // Fetch fresh market data
    hl_markets_t markets = {0};
    hl_error_t err = fetch_fresh_market_data(client, &markets);
    if (err != HL_SUCCESS) {
        return err;
    }

    // Find the requested symbol
    for (size_t i = 0; i < markets.count; i++) {
        if (strcmp(markets.markets[i].symbol, symbol) == 0) {
            err = parse_ticker_from_market(&markets.markets[i], ticker);
            hl_markets_free(&markets);
            return err;
        }
    }

    hl_markets_free(&markets);
    return HL_ERROR_NOT_FOUND;
}
