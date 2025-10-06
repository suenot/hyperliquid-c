/**
 * @file market_extended.c
 * @brief Extended market data functions implementation
 */

#include "hyperliquid.h"
#include "hl_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Fetch multiple tickers
 */
hl_error_t hl_fetch_tickers(hl_client_t* client,
                           const char** symbols,
                           size_t symbols_count,
                           hl_tickers_t* tickers) {
    if (!client || !tickers) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Clear output
    memset(tickers, 0, sizeof(hl_tickers_t));

    // If no symbols specified, fetch all mids
    char request_body[1024];
    if (symbols_count == 0 || !symbols) {
        strcpy(request_body, "{\"type\":\"allMids\"}");
    } else {
        // For specific symbols, we need to build request
        // For now, fallback to allMids and filter
        strcpy(request_body, "{\"type\":\"allMids\"}");
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

    // Response is an array of [coin, mid_price] pairs
    if (!cJSON_IsArray(json)) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    int array_size = cJSON_GetArraySize(json);
    if (array_size == 0) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_SUCCESS;
    }

    // Allocate tickers array
    size_t max_tickers = (symbols_count > 0) ? symbols_count : array_size;
    tickers->tickers = calloc(max_tickers, sizeof(hl_ticker_t));
    if (!tickers->tickers) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_MEMORY;
    }

    // Parse tickers
    size_t valid_tickers = 0;
    for (int i = 0; i < array_size && valid_tickers < max_tickers; i++) {
        cJSON* item = cJSON_GetArrayItem(json, i);
        if (!cJSON_IsArray(item) || cJSON_GetArraySize(item) != 2) {
            continue;
        }

        cJSON* coin = cJSON_GetArrayItem(item, 0);
        cJSON* mid_price = cJSON_GetArrayItem(item, 1);

        if (!coin || !cJSON_IsString(coin) || !mid_price) {
            continue;
        }

        // Check if we want this symbol
        bool want_symbol = (symbols_count == 0);
        if (!want_symbol && symbols) {
            for (size_t j = 0; j < symbols_count; j++) {
                if (symbols[j] && strcmp(symbols[j], coin->valuestring) == 0) {
                    want_symbol = true;
                    break;
                }
            }
        }

        if (!want_symbol) continue;

        // Create ticker
        hl_ticker_t* ticker = &tickers->tickers[valid_tickers];

        // Set symbol
        snprintf(ticker->symbol, sizeof(ticker->symbol), "%s/USDC:USDC", coin->valuestring);

        // Set last price (mid)
        if (cJSON_IsString(mid_price)) {
            ticker->last = atof(mid_price->valuestring);
        } else if (cJSON_IsNumber(mid_price)) {
            ticker->last = mid_price->valuedouble;
        }

        // Set other prices (same as last for mids)
        ticker->bid = ticker->last;
        ticker->ask = ticker->last;
        ticker->close = ticker->last;

        // Set current timestamp
        hl_current_timestamp(ticker->timestamp, sizeof(ticker->timestamp));
        strcpy(ticker->datetime, ticker->timestamp); // TODO: format properly

        valid_tickers++;
    }

    tickers->count = valid_tickers;

    cJSON_Delete(json);
    http_response_free(response);

    return HL_SUCCESS;
}
