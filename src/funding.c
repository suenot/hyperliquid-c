/**
 * @file funding.c
 * @brief Funding rates and related functions implementation
 */

#include "hyperliquid.h"
#include "hl_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Forward declarations
typedef struct hl_funding_rate hl_funding_rate_t;

struct hl_funding_rate {
    char symbol[32];                /**< trading symbol */
    char timestamp[32];             /**< timestamp */
    char datetime[32];              /**< ISO 8601 datetime */
    double funding_rate;            /**< current funding rate */
    double mark_price;              /**< mark price */
    double index_price;             /**< index price */
    double open_interest;           /**< open interest */
    double premium;                 /**< premium */
    char info[1024];                /**< raw exchange data */
};

typedef struct {
    hl_funding_rate_t* rates;
    size_t count;
} hl_funding_rates_t;

/**
 * @brief Fetch funding rates for all symbols
 */
hl_error_t hl_fetch_funding_rates(hl_client_t* client, hl_funding_rates_t* rates) {
    if (!client || !rates) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Clear output
    memset(rates, 0, sizeof(hl_funding_rates_t));

    // Prepare request - use metaAndAssetCtxs endpoint
    char request_body[256] = "{\"type\":\"metaAndAssetCtxs\"}";

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

    // Response is an array: [meta, assetCtxs]
    if (!cJSON_IsArray(json) || cJSON_GetArraySize(json) != 2) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    // Get meta and asset contexts
    cJSON* meta = cJSON_GetArrayItem(json, 0);
    cJSON* asset_ctxs = cJSON_GetArrayItem(json, 1);

    if (!meta || !cJSON_IsObject(meta) || !asset_ctxs || !cJSON_IsArray(asset_ctxs)) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    // Get universe from meta
    cJSON* universe = cJSON_GetObjectItem(meta, "universe");
    if (!universe || !cJSON_IsArray(universe)) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    int universe_size = cJSON_GetArraySize(universe);
    int ctxs_size = cJSON_GetArraySize(asset_ctxs);

    if (universe_size != ctxs_size) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    // Allocate funding rates array
    rates->rates = calloc(ctxs_size, sizeof(hl_funding_rate_t));
    if (!rates->rates) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_MEMORY;
    }

    // Parse each funding rate
    size_t valid_rates = 0;
    for (int i = 0; i < ctxs_size && valid_rates < ctxs_size; i++) {
        cJSON* universe_item = cJSON_GetArrayItem(universe, i);
        cJSON* ctx_item = cJSON_GetArrayItem(asset_ctxs, i);

        if (!universe_item || !cJSON_IsObject(universe_item) ||
            !ctx_item || !cJSON_IsObject(ctx_item)) {
            continue;
        }

        // Get coin name from universe
        cJSON* name = cJSON_GetObjectItem(universe_item, "name");
        if (!name || !cJSON_IsString(name)) {
            continue;
        }

        hl_funding_rate_t* rate = &rates->rates[valid_rates];

        // Set symbol
        snprintf(rate->symbol, sizeof(rate->symbol), "%s/USDC:USDC", name->valuestring);

        // Extract funding data from context
        cJSON* funding = cJSON_GetObjectItem(ctx_item, "funding");
        cJSON* mark_px = cJSON_GetObjectItem(ctx_item, "markPx");
        cJSON* oracle_px = cJSON_GetObjectItem(ctx_item, "oraclePx");
        cJSON* open_interest = cJSON_GetObjectItem(ctx_item, "openInterest");
        cJSON* premium = cJSON_GetObjectItem(ctx_item, "premium");

        // Set funding rate
        if (funding) {
            if (cJSON_IsString(funding)) {
                rate->funding_rate = atof(funding->valuestring);
            } else if (cJSON_IsNumber(funding)) {
                rate->funding_rate = funding->valuedouble;
            }
        }

        // Set mark price
        if (mark_px) {
            if (cJSON_IsString(mark_px)) {
                rate->mark_price = atof(mark_px->valuestring);
            } else if (cJSON_IsNumber(mark_px)) {
                rate->mark_price = mark_px->valuedouble;
            }
        }

        // Set index price (oracle)
        if (oracle_px) {
            if (cJSON_IsString(oracle_px)) {
                rate->index_price = atof(oracle_px->valuestring);
            } else if (cJSON_IsNumber(oracle_px)) {
                rate->index_price = oracle_px->valuedouble;
            }
        }

        // Set open interest
        if (open_interest) {
            if (cJSON_IsString(open_interest)) {
                rate->open_interest = atof(open_interest->valuestring);
            } else if (cJSON_IsNumber(open_interest)) {
                rate->open_interest = open_interest->valuedouble;
            }
        }

        // Set premium
        if (premium) {
            if (cJSON_IsString(premium)) {
                rate->premium = atof(premium->valuestring);
            } else if (cJSON_IsNumber(premium)) {
                rate->premium = premium->valuedouble;
            }
        }

        // Set timestamps
        hl_current_timestamp(rate->timestamp, sizeof(rate->timestamp));
        strcpy(rate->datetime, rate->timestamp); // TODO: format properly

        // Store raw context info
        char* ctx_str = cJSON_PrintUnformatted(ctx_item);
        if (ctx_str) {
            strncpy(rate->info, ctx_str, sizeof(rate->info) - 1);
            cJSON_free(ctx_str);
        }

        valid_rates++;
    }

    rates->count = valid_rates;

    cJSON_Delete(json);
    http_response_free(response);

    return HL_SUCCESS;
}

/**
 * @brief Free funding rates array
 */
void hl_free_funding_rates(hl_funding_rates_t* rates) {
    if (rates && rates->rates) {
        free(rates->rates);
        rates->rates = NULL;
        rates->count = 0;
    }
}
