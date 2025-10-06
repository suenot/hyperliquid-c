/**
 * @file ohlcv.c
 * @brief OHLCV candlestick data implementation
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
 * @brief Parse single OHLCV candle from JSON
 */
static hl_error_t parse_ohlcv_candle(cJSON* candle_json, hl_ohlcv_t* candle) {
    if (!candle_json || !candle) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Parse timestamp (t field)
    cJSON* t_json = cJSON_GetObjectItem(candle_json, "t");
    if (!t_json || !cJSON_IsString(t_json)) {
        return HL_ERROR_PARSE;
    }
    candle->timestamp = strtoull(t_json->valuestring, NULL, 10);

    // Parse open price (o field)
    cJSON* o_json = cJSON_GetObjectItem(candle_json, "o");
    if (!o_json || !cJSON_IsString(o_json)) {
        return HL_ERROR_PARSE;
    }
    candle->open = atof(o_json->valuestring);

    // Parse high price (h field)
    cJSON* h_json = cJSON_GetObjectItem(candle_json, "h");
    if (!h_json || !cJSON_IsString(h_json)) {
        return HL_ERROR_PARSE;
    }
    candle->high = atof(h_json->valuestring);

    // Parse low price (l field)
    cJSON* l_json = cJSON_GetObjectItem(candle_json, "l");
    if (!l_json || !cJSON_IsString(l_json)) {
        return HL_ERROR_PARSE;
    }
    candle->low = atof(l_json->valuestring);

    // Parse close price (c field)
    cJSON* c_json = cJSON_GetObjectItem(candle_json, "c");
    if (!c_json || !cJSON_IsString(c_json)) {
        return HL_ERROR_PARSE;
    }
    candle->close = atof(c_json->valuestring);

    // Parse volume (v field)
    cJSON* v_json = cJSON_GetObjectItem(candle_json, "v");
    if (!v_json || !cJSON_IsString(v_json)) {
        return HL_ERROR_PARSE;
    }
    candle->volume = atof(v_json->valuestring);

    return HL_SUCCESS;
}

/**
 * @brief Validate timeframe string
 */
static hl_error_t validate_timeframe(const char* timeframe) {
    if (!timeframe) {
        return HL_ERROR_INVALID_PARAMS;
    }

    const char* valid_timeframes[] = {
        "1m", "3m", "5m", "15m", "30m",
        "1h", "2h", "4h", "8h", "12h",
        "1d", "3d", "1w", "1M"
    };

    size_t num_valid = sizeof(valid_timeframes) / sizeof(valid_timeframes[0]);
    for (size_t i = 0; i < num_valid; i++) {
        if (strcmp(timeframe, valid_timeframes[i]) == 0) {
            return HL_SUCCESS;
        }
    }

    return HL_ERROR_INVALID_PARAMS;
}

/**
 * @brief Calculate start time based on limit and timeframe
 */
static uint64_t calculate_start_time(uint64_t end_time, uint32_t limit, const char* timeframe) {
    if (limit == 0) {
        // Return a reasonable default (30 days ago)
        return end_time - (30LL * 24LL * 60LL * 60LL * 1000LL);
    }

    // Convert timeframe to milliseconds
    uint64_t timeframe_ms = 0;
    if (strcmp(timeframe, "1m") == 0) timeframe_ms = 60 * 1000;
    else if (strcmp(timeframe, "3m") == 0) timeframe_ms = 3 * 60 * 1000;
    else if (strcmp(timeframe, "5m") == 0) timeframe_ms = 5 * 60 * 1000;
    else if (strcmp(timeframe, "15m") == 0) timeframe_ms = 15 * 60 * 1000;
    else if (strcmp(timeframe, "30m") == 0) timeframe_ms = 30 * 60 * 1000;
    else if (strcmp(timeframe, "1h") == 0) timeframe_ms = 60 * 60 * 1000;
    else if (strcmp(timeframe, "2h") == 0) timeframe_ms = 2 * 60 * 60 * 1000;
    else if (strcmp(timeframe, "4h") == 0) timeframe_ms = 4 * 60 * 60 * 1000;
    else if (strcmp(timeframe, "8h") == 0) timeframe_ms = 8 * 60 * 60 * 1000;
    else if (strcmp(timeframe, "12h") == 0) timeframe_ms = 12 * 60 * 60 * 1000;
    else if (strcmp(timeframe, "1d") == 0) timeframe_ms = 24 * 60 * 60 * 1000;
    else if (strcmp(timeframe, "3d") == 0) timeframe_ms = 3 * 24 * 60 * 60 * 1000;
    else if (strcmp(timeframe, "1w") == 0) timeframe_ms = 7 * 24 * 60 * 60 * 1000;
    else if (strcmp(timeframe, "1M") == 0) timeframe_ms = 30LL * 24LL * 60LL * 60LL * 1000LL;

    return end_time - (uint64_t)limit * timeframe_ms;
}

/**
 * @brief Fetch OHLCV data from API
 */
hl_error_t hl_fetch_ohlcv(hl_client_t* client, const char* symbol, const char* timeframe,
                         uint64_t* since, uint32_t* limit, uint64_t* until,
                         hl_ohlcvs_t* ohlcvs) {
    if (!client || !symbol || !timeframe || !ohlcvs) {
        return HL_ERROR_INVALID_PARAMS;
    }

    memset(ohlcvs, 0, sizeof(hl_ohlcvs_t));

    // Validate timeframe
    hl_error_t err = validate_timeframe(timeframe);
    if (err != HL_SUCCESS) {
        return err;
    }

    // Get markets data first
    hl_markets_t markets = {0};
    err = hl_fetch_markets(client, &markets);
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
        hl_markets_free(&markets);
        return HL_ERROR_INVALID_PARAMS;
    }

    const char* base_url = get_base_url(client);
    char url[256];
    snprintf(url, sizeof(url), "%s/info", base_url);

    // Calculate timestamps
    uint64_t end_time = until ? *until : (uint64_t)time(NULL) * 1000;
    uint64_t start_time = since ? *since : 0;

    // If since is not provided but limit is, calculate start_time
    if (!since && limit && *limit > 0) {
        start_time = calculate_start_time(end_time, *limit, timeframe);
    } else if (!since && (!limit || *limit == 0)) {
        // No limit specified, get last 24 hours of data
        start_time = end_time - (24LL * 60LL * 60LL * 1000LL);
    }

    // Build request body
    char body[512];
    if (market_info->type == HL_MARKET_SWAP) {
        // For swaps, use coin name (baseName)
        snprintf(body, sizeof(body),
                "{\"type\":\"candleSnapshot\",\"req\":{\"coin\":\"%s\",\"interval\":\"%s\",\"startTime\":%llu,\"endTime\":%llu}}",
                market_info->base, timeframe, start_time, end_time);
    } else {
        // For spots, use asset ID
        snprintf(body, sizeof(body),
                "{\"type\":\"candleSnapshot\",\"req\":{\"coin\":\"%u\",\"interval\":\"%s\",\"startTime\":%llu,\"endTime\":%llu}}",
                asset_id, timeframe, start_time, end_time);
    }

    // Make request
    http_response_t response = {0};
    lv3_error_t http_err = http_client_post(http, url, body, "Content-Type: application/json", &response);

    if (http_err != LV3_SUCCESS) {
        http_response_free(&response);
        hl_markets_free(&markets);
        return HL_ERROR_NETWORK;
    }

    if (response.status_code != 200) {
        http_response_free(&response);
        hl_markets_free(&markets);
        return HL_ERROR_API;
    }

    // Parse response
    cJSON* json = cJSON_Parse(response.body);
    http_response_free(&response);

    if (!json) {
        hl_markets_free(&markets);
        return HL_ERROR_PARSE;
    }

    if (!cJSON_IsArray(json)) {
        cJSON_Delete(json);
        hl_markets_free(&markets);
        return HL_ERROR_PARSE;
    }

    size_t num_candles = cJSON_GetArraySize(json);
    if (num_candles == 0) {
        cJSON_Delete(json);
        hl_markets_free(&markets);
        return HL_SUCCESS;
    }

    // Limit candles if specified
    if (limit && *limit > 0 && num_candles > *limit) {
        num_candles = *limit;
    }

    // Allocate memory for candles
    hl_ohlcv_t* candles = calloc(num_candles, sizeof(hl_ohlcv_t));
    if (!candles) {
        cJSON_Delete(json);
        hl_markets_free(&markets);
        return HL_ERROR_MEMORY;
    }

    // Parse each candle
    size_t valid_candles = 0;
    for (size_t i = 0; i < num_candles; i++) {
        cJSON* candle_json = cJSON_GetArrayItem(json, i);
        if (candle_json) {
            hl_error_t parse_err = parse_ohlcv_candle(candle_json, &candles[valid_candles]);
            if (parse_err == HL_SUCCESS) {
                valid_candles++;
            }
        }
    }

    // Copy metadata
    strncpy(ohlcvs->symbol, symbol, sizeof(ohlcvs->symbol) - 1);
    strncpy(ohlcvs->timeframe, timeframe, sizeof(ohlcvs->timeframe) - 1);
    ohlcvs->candles = candles;
    ohlcvs->count = valid_candles;

    cJSON_Delete(json);
    hl_markets_free(&markets);
    return HL_SUCCESS;
}

/**
 * @brief Free OHLCV data memory
 */
void hl_ohlcvs_free(hl_ohlcvs_t* ohlcvs) {
    if (ohlcvs) {
        if (ohlcvs->candles) {
            free(ohlcvs->candles);
            ohlcvs->candles = NULL;
        }
        ohlcvs->count = 0;
        ohlcvs->symbol[0] = '\0';
        ohlcvs->timeframe[0] = '\0';
    }
}

/**
 * @brief Get latest candle from OHLCV data
 */
const hl_ohlcv_t* hl_ohlcvs_get_latest(const hl_ohlcvs_t* ohlcvs) {
    if (!ohlcvs || !ohlcvs->candles || ohlcvs->count == 0) {
        return NULL;
    }
    return &ohlcvs->candles[ohlcvs->count - 1];
}

/**
 * @brief Calculate simple moving average
 */
hl_error_t hl_ohlcvs_calculate_sma(const hl_ohlcvs_t* ohlcvs, size_t period,
                                  bool use_close, double* sma) {
    if (!ohlcvs || !sma || period == 0 || ohlcvs->count < period) {
        return HL_ERROR_INVALID_PARAMS;
    }

    for (size_t i = 0; i < ohlcvs->count; i++) {
        if (i < period - 1) {
            sma[i] = 0.0; // Not enough data
            continue;
        }

        double sum = 0.0;
        for (size_t j = 0; j < period; j++) {
            size_t idx = i - (period - 1) + j;
            if (use_close) {
                sum += ohlcvs->candles[idx].close;
            } else {
                sum += (ohlcvs->candles[idx].high + ohlcvs->candles[idx].low) / 2.0;
            }
        }
        sma[i] = sum / period;
    }

    return HL_SUCCESS;
}

/**
 * @brief Find highest high in period
 */
double hl_ohlcvs_highest_high(const hl_ohlcvs_t* ohlcvs, size_t start_index, size_t count) {
    if (!ohlcvs || !ohlcvs->candles || start_index >= ohlcvs->count || count == 0) {
        return 0.0;
    }

    size_t end_index = start_index + count;
    if (end_index > ohlcvs->count) {
        end_index = ohlcvs->count;
    }

    double highest = 0.0;
    for (size_t i = start_index; i < end_index; i++) {
        if (ohlcvs->candles[i].high > highest) {
            highest = ohlcvs->candles[i].high;
        }
    }

    return highest;
}

/**
 * @brief Find lowest low in period
 */
double hl_ohlcvs_lowest_low(const hl_ohlcvs_t* ohlcvs, size_t start_index, size_t count) {
    if (!ohlcvs || !ohlcvs->candles || start_index >= ohlcvs->count || count == 0) {
        return 0.0;
    }

    size_t end_index = start_index + count;
    if (end_index > ohlcvs->count) {
        end_index = ohlcvs->count;
    }

    double lowest = ohlcvs->candles[start_index].low;
    for (size_t i = start_index + 1; i < end_index; i++) {
        if (ohlcvs->candles[i].low < lowest) {
            lowest = ohlcvs->candles[i].low;
        }
    }

    return lowest;
}

/**
 * @brief Get candle by index
 */
const hl_ohlcv_t* hl_ohlcvs_get_candle(const hl_ohlcvs_t* ohlcvs, size_t index) {
    if (!ohlcvs || !ohlcvs->candles || index >= ohlcvs->count) {
        return NULL;
    }
    return &ohlcvs->candles[index];
}
