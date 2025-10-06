/**
 * @file markets.c
 * @brief Market data and symbol mapping implementation
 */

#include "hyperliquid.h"
#include "hl_http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
 * @brief Parse single swap market from JSON
 */
hl_error_t parse_swap_market(cJSON* universe_item, cJSON* context_item, int base_id, hl_market_t* market) {
    if (!universe_item || !context_item || !market) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Parse universe data (static info)
    cJSON* name = cJSON_GetObjectItem(universe_item, "name");
    cJSON* max_leverage = cJSON_GetObjectItem(universe_item, "maxLeverage");
    cJSON* sz_decimals = cJSON_GetObjectItem(universe_item, "szDecimals");

    if (!name || !cJSON_IsString(name)) {
        return HL_ERROR_PARSE;
    }

    // Set basic info
    strncpy(market->base, name->valuestring, sizeof(market->base) - 1);
    strcpy(market->quote, "USDC");
    strcpy(market->settle, "USDC");

    // Create symbol: "BASE/USDC:USDC"
    snprintf(market->symbol, sizeof(market->symbol), "%s/USDC:USDC", market->base);

    // Set asset ID
    market->asset_id = (uint32_t)base_id;
    snprintf(market->base_id, sizeof(market->base_id), "%d", base_id);

    // Set market type
    market->type = HL_MARKET_SWAP;
    market->active = true; // Assume active unless specified

    // Set precision
    if (sz_decimals && sz_decimals->valuestring) {
        market->amount_precision = atoi(sz_decimals->valuestring);
    } else {
        market->amount_precision = 4;
    }
    market->price_precision = 6; // Default for crypto

    // Set limits
    if (max_leverage && max_leverage->valuestring) {
        market->max_leverage = atoi(max_leverage->valuestring);
    } else {
        market->max_leverage = 50;
    }
    market->min_cost = 10.0; // Default minimum cost

    // Parse context data (dynamic info)
    cJSON* mark_px = cJSON_GetObjectItem(context_item, "markPx");
    cJSON* oracle_px = cJSON_GetObjectItem(context_item, "oraclePx");
    cJSON* funding = cJSON_GetObjectItem(context_item, "funding");
    cJSON* day_ntl_vlm = cJSON_GetObjectItem(context_item, "dayNtlVlm");
    cJSON* open_interest = cJSON_GetObjectItem(context_item, "openInterest");

    // Set prices
    if (mark_px) {
        market->mark_price = cJSON_IsString(mark_px) ?
            atof(mark_px->valuestring) : mark_px->valuedouble;
    }

    if (oracle_px) {
        market->oracle_price = cJSON_IsString(oracle_px) ?
            atof(oracle_px->valuestring) : oracle_px->valuedouble;
    }

    if (funding) {
        market->funding_rate = cJSON_IsString(funding) ?
            atof(funding->valuestring) : funding->valuedouble;
    }

    // Set volumes
    if (day_ntl_vlm) {
        market->day_volume = cJSON_IsString(day_ntl_vlm) ?
            atof(day_ntl_vlm->valuestring) : day_ntl_vlm->valuedouble;
    }

    if (open_interest) {
        market->open_interest = cJSON_IsString(open_interest) ?
            atof(open_interest->valuestring) : open_interest->valuedouble;
    }

    return HL_SUCCESS;
}

/**
 * @brief Fetch perpetual swap markets
 */
static hl_error_t fetch_swap_markets(hl_client_t* client, hl_markets_t* markets) {
    http_client_t* http = (http_client_t*)hl_client_get_http(client);
    if (!http) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Build request for swap markets
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
 * @brief Parse single spot market from JSON
 */
static hl_error_t parse_spot_market(cJSON* universe_item, cJSON* context_item, cJSON* tokens, hl_market_t* market) {
    if (!universe_item || !context_item || !tokens || !market) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Parse universe data
    cJSON* name = cJSON_GetObjectItem(universe_item, "name");
    cJSON* tokens_pos = cJSON_GetObjectItem(universe_item, "tokens");
    cJSON* index = cJSON_GetObjectItem(universe_item, "index");

    if (!name || !tokens_pos || !cJSON_IsArray(tokens_pos) || cJSON_GetArraySize(tokens_pos) < 2) {
        return HL_ERROR_PARSE;
    }

    // Extract base and quote from name (e.g., "PURR/USDC")
    const char* market_name = cJSON_GetStringValue(name);
    if (!market_name) {
        return HL_ERROR_PARSE;
    }

    char* slash = strchr(market_name, '/');
    if (!slash) {
        return HL_ERROR_PARSE;
    }

    size_t base_len = slash - market_name;
    if (base_len >= sizeof(market->base)) {
        return HL_ERROR_PARSE;
    }

    memcpy(market->base, market_name, base_len);
    market->base[base_len] = '\0';
    strcpy(market->quote, slash + 1);
    strcpy(market->settle, market->quote); // For spot, settle = quote

    // Create symbol: "BASE/QUOTE"
    snprintf(market->symbol, sizeof(market->symbol), "%s/%s", market->base, market->quote);

    // Set asset ID from index
    if (index) {
        market->asset_id = (uint32_t)cJSON_GetNumberValue(index);
        snprintf(market->base_id, sizeof(market->base_id), "%d", market->asset_id);
    }

    // Set market type
    market->type = HL_MARKET_SPOT;
    market->active = true;

    // Get token info for precision
    int base_token_idx = cJSON_GetArrayItem(tokens_pos, 0)->valueint;
    int quote_token_idx = cJSON_GetArrayItem(tokens_pos, 1)->valueint;

    cJSON* base_token = cJSON_GetArrayItem(tokens, base_token_idx);
    cJSON* quote_token = cJSON_GetArrayItem(tokens, quote_token_idx);

    if (base_token) {
        cJSON* sz_decimals = cJSON_GetObjectItem(base_token, "szDecimals");
        if (sz_decimals) {
            market->amount_precision = (int)cJSON_GetNumberValue(sz_decimals);
        }
    }

    market->price_precision = 8; // Default for spot
    market->max_leverage = 1; // Spot markets have no leverage
    market->min_cost = 1.0; // Lower minimum for spot

    // Parse context data (prices)
    cJSON* mark_px = cJSON_GetObjectItem(context_item, "markPx");
    cJSON* day_ntl_vlm = cJSON_GetObjectItem(context_item, "dayNtlVlm");

    if (mark_px) {
        market->mark_price = cJSON_IsString(mark_px) ?
            atof(mark_px->valuestring) : mark_px->valuedouble;
    }

    if (day_ntl_vlm) {
        market->day_volume = cJSON_IsString(day_ntl_vlm) ?
            atof(day_ntl_vlm->valuestring) : day_ntl_vlm->valuedouble;
    }

    return HL_SUCCESS;
}

/**
 * @brief Fetch spot markets
 */
static hl_error_t fetch_spot_markets(hl_client_t* client, hl_markets_t* markets) {
    http_client_t* http = (http_client_t*)hl_client_get_http(client);
    if (!http) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Build request for spot markets
    char body[512];
    snprintf(body, sizeof(body), "{\"type\":\"spotMetaAndAssetCtxs\"}");

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

    if (!json || !cJSON_IsArray(json) || cJSON_GetArraySize(json) < 2) {
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
    cJSON* tokens = cJSON_GetObjectItem(meta, "tokens");

    if (!universe || !tokens || !cJSON_IsArray(universe) || !cJSON_IsArray(tokens)) {
        cJSON_Delete(json);
        return HL_ERROR_PARSE;
    }

    int spot_count = cJSON_GetArraySize(universe);
    if (spot_count == 0) {
        cJSON_Delete(json);
        return HL_SUCCESS;
    }

    // Allocate markets array (extend existing)
    size_t existing_count = markets->count;
    hl_market_t* markets_array = realloc(markets->markets, (existing_count + spot_count) * sizeof(hl_market_t));
    if (!markets_array) {
        cJSON_Delete(json);
        return HL_ERROR_MEMORY;
    }

    markets->markets = markets_array;

    // Parse each market
    int valid_markets = 0;
    for (int i = 0; i < spot_count; i++) {
        cJSON* universe_item = cJSON_GetArrayItem(universe, i);
        cJSON* context_item = cJSON_GetArrayItem(contexts, i);

        if (universe_item && context_item) {
            hl_error_t parse_err = parse_spot_market(universe_item, context_item, tokens,
                                                   &markets_array[existing_count + valid_markets]);
            if (parse_err == HL_SUCCESS) {
                valid_markets++;
            }
        }
    }

    markets->count = existing_count + valid_markets;

    cJSON_Delete(json);
    return HL_SUCCESS;
}

/**
 * @brief Fetch all available markets
 */
hl_error_t hl_fetch_markets(hl_client_t* client, hl_markets_t* markets) {
    if (!client || !markets) {
        return HL_ERROR_INVALID_PARAMS;
    }

    memset(markets, 0, sizeof(hl_markets_t));

    // Fetch swap markets first
    hl_error_t err = fetch_swap_markets(client, markets);
    if (err != HL_SUCCESS) {
        hl_markets_free(markets);
        return err;
    }

    // Then fetch spot markets
    err = fetch_spot_markets(client, markets);
    if (err != HL_SUCCESS) {
        hl_markets_free(markets);
        return err;
    }

    return HL_SUCCESS;
}

/**
 * @brief Free markets collection
 */
void hl_markets_free(hl_markets_t* markets) {
    if (markets && markets->markets) {
        free(markets->markets);
        markets->markets = NULL;
        markets->count = 0;
    }
}

/**
 * @brief Get asset ID by symbol
 */
hl_error_t hl_get_asset_id(const hl_markets_t* markets, const char* symbol, uint32_t* asset_id) {
    if (!markets || !symbol || !asset_id) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Extract coin from symbol (e.g., "BTC/USDC:USDC" -> "BTC")
    const char* coin = symbol;
    const char* slash = strchr(symbol, '/');
    if (slash) {
        // Temporary buffer for coin extraction
        static char coin_buf[32];
        size_t coin_len = slash - symbol;
        if (coin_len >= sizeof(coin_buf)) {
            return HL_ERROR_INVALID_PARAMS;
        }
        memcpy(coin_buf, symbol, coin_len);
        coin_buf[coin_len] = '\0';
        coin = coin_buf;
    }

    // Find market by coin
    for (size_t i = 0; i < markets->count; i++) {
        if (strcmp(markets->markets[i].base, coin) == 0 &&
            markets->markets[i].type == HL_MARKET_SWAP) {
            *asset_id = markets->markets[i].asset_id;
            return HL_SUCCESS;
        }
    }

    return HL_ERROR_NOT_FOUND;
}

/**
 * @brief Get market info by symbol
 */
hl_error_t hl_get_market(const hl_markets_t* markets, const char* symbol, const hl_market_t** market) {
    if (!markets || !symbol || !market) {
        return HL_ERROR_INVALID_PARAMS;
    }

    for (size_t i = 0; i < markets->count; i++) {
        if (strcmp(markets->markets[i].symbol, symbol) == 0) {
            *market = &markets->markets[i];
            return HL_SUCCESS;
        }
    }

    return HL_ERROR_NOT_FOUND;
}

/**
 * @brief Get market info by asset ID
 */
hl_error_t hl_get_market_by_id(const hl_markets_t* markets, uint32_t asset_id, const hl_market_t** market) {
    if (!markets || !market) {
        return HL_ERROR_INVALID_PARAMS;
    }

    for (size_t i = 0; i < markets->count; i++) {
        if (markets->markets[i].asset_id == asset_id) {
            *market = &markets->markets[i];
            return HL_SUCCESS;
        }
    }

    return HL_ERROR_NOT_FOUND;
}

// Forward declarations for open interest
typedef struct hl_open_interest hl_open_interest_t;

struct hl_open_interest {
    char symbol[32];            /**< trading symbol */
    double open_interest;       /**< current open interest */
    char timestamp[32];         /**< timestamp */
    char datetime[32];          /**< ISO datetime */
    char info[512];             /**< raw exchange data */
};

typedef struct {
    hl_open_interest_t* interests;
    size_t count;
} hl_open_interests_t;

/**
 * @brief Fetch open interests for symbols
 */
hl_error_t hl_fetch_open_interests(hl_client_t* client,
                                  const char** symbols,
                                  size_t symbols_count,
                                  hl_open_interests_t* interests) {
    if (!client || !interests) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Clear output
    memset(interests, 0, sizeof(hl_open_interests_t));

    // Use metaAndAssetCtxs endpoint to get asset contexts with open interest
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

    // Allocate interests array
    interests->interests = calloc(ctxs_size, sizeof(hl_open_interest_t));
    if (!interests->interests) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_MEMORY;
    }

    // Parse each open interest
    size_t valid_interests = 0;
    for (int i = 0; i < ctxs_size && valid_interests < ctxs_size; i++) {
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

        // Check if we should include this symbol
        bool include_symbol = true;
        if (symbols && symbols_count > 0) {
            include_symbol = false;
            char expected_symbol[64];
            snprintf(expected_symbol, sizeof(expected_symbol), "%s/USDC:USDC", name->valuestring);

            for (size_t j = 0; j < symbols_count; j++) {
                if (strcmp(symbols[j], expected_symbol) == 0) {
                    include_symbol = true;
                    break;
                }
            }
        }

        if (!include_symbol) {
            continue;
        }

        hl_open_interest_t* interest = &interests->interests[valid_interests];

        // Set symbol
        snprintf(interest->symbol, sizeof(interest->symbol), "%s/USDC:USDC", name->valuestring);

        // Extract open interest from context
        cJSON* open_interest = cJSON_GetObjectItem(ctx_item, "openInterest");

        if (open_interest) {
            if (cJSON_IsString(open_interest)) {
                interest->open_interest = atof(open_interest->valuestring);
            } else if (cJSON_IsNumber(open_interest)) {
                interest->open_interest = open_interest->valuedouble;
            }
        }

        // Set timestamps
        hl_current_timestamp(interest->timestamp, sizeof(interest->timestamp));
        strcpy(interest->datetime, interest->timestamp); // TODO: format properly

        // Store raw context info
        char* ctx_str = cJSON_PrintUnformatted(ctx_item);
        if (ctx_str) {
            strncpy(interest->info, ctx_str, sizeof(interest->info) - 1);
            cJSON_free(ctx_str);
        }

        valid_interests++;
    }

    interests->count = valid_interests;

    cJSON_Delete(json);
    http_response_free(response);

    return HL_SUCCESS;
}

/**
 * @brief Fetch open interest for single symbol
 */
hl_error_t hl_fetch_open_interest(hl_client_t* client,
                                 const char* symbol,
                                 hl_open_interest_t* interest) {
    if (!client || !symbol || !interest) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Clear output
    memset(interest, 0, sizeof(hl_open_interest_t));

    // Use fetch_open_interests with single symbol
    hl_open_interests_t interests = {0};
    const char* symbols[] = {symbol};

    hl_error_t err = hl_fetch_open_interests(client, symbols, 1, &interests);
    if (err != HL_SUCCESS) {
        return err;
    }

    if (interests.count == 0) {
        return HL_ERROR_NOT_FOUND;
    }

    // Copy first result
    *interest = interests.interests[0];
    free(interests.interests);

    return HL_SUCCESS;
}

/**
 * @brief Free open interests array
 */
void hl_free_open_interests(hl_open_interests_t* interests) {
    if (interests && interests->interests) {
        free(interests->interests);
        interests->interests = NULL;
        interests->count = 0;
    }
}
