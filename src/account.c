/**
 * @file account.c
 * @brief Account & balance management implementation
 */

#include "hyperliquid.h"
#include "hl_http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cjson/cJSON.h>

// Internal client accessor (from client.c)
extern const char* hl_client_get_wallet_address_old(hl_client_t* client);
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
 * @brief Fetch perpetual account balance
 */
static hl_error_t fetch_perpetual_balance(hl_client_t* client, hl_balance_t* balance) {
    const char* wallet = hl_client_get_wallet_address_old(client);
    if (!wallet) {
        return HL_ERROR_INVALID_PARAMS;
    }

    http_client_t* http = (http_client_t*)hl_client_get_http(client);
    if (!http) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Build request
    char body[512];
    snprintf(body, sizeof(body),
             "{\"type\":\"clearinghouseState\",\"user\":\"%s\"}",
             wallet);

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

    // Extract margin summary
    cJSON* margin_summary = cJSON_GetObjectItem(json, "marginSummary");
    if (margin_summary) {
        cJSON* account_value = cJSON_GetObjectItem(margin_summary, "accountValue");
        if (account_value) {
            balance->account_value = cJSON_IsString(account_value) ?
                atof(account_value->valuestring) : account_value->valuedouble;
        }

        cJSON* total_margin = cJSON_GetObjectItem(margin_summary, "totalMarginUsed");
        if (total_margin) {
            balance->total_margin_used = cJSON_IsString(total_margin) ?
                atof(total_margin->valuestring) : total_margin->valuedouble;
        }

        cJSON* total_ntl = cJSON_GetObjectItem(margin_summary, "totalNtlPos");
        if (total_ntl) {
            balance->total_ntl_pos = cJSON_IsString(total_ntl) ?
                atof(total_ntl->valuestring) : total_ntl->valuedouble;
        }

        cJSON* total_raw = cJSON_GetObjectItem(margin_summary, "totalRawUsd");
        if (total_raw) {
            balance->total_raw_usd = cJSON_IsString(total_raw) ?
                atof(total_raw->valuestring) : total_raw->valuedouble;
        }
    }

    // Extract cross margin summary
    cJSON* cross_summary = cJSON_GetObjectItem(json, "crossMarginSummary");
    if (cross_summary) {
        cJSON* cross_value = cJSON_GetObjectItem(cross_summary, "accountValue");
        if (cross_value) {
            balance->cross_account_value = cJSON_IsString(cross_value) ?
                atof(cross_value->valuestring) : cross_value->valuedouble;
        }

        cJSON* cross_margin = cJSON_GetObjectItem(cross_summary, "totalMarginUsed");
        if (cross_margin) {
            balance->cross_margin_used = cJSON_IsString(cross_margin) ?
                atof(cross_margin->valuestring) : cross_margin->valuedouble;
        }
    }

    // Extract withdrawable
    cJSON* withdrawable = cJSON_GetObjectItem(json, "withdrawable");
    if (withdrawable) {
        balance->withdrawable = cJSON_IsString(withdrawable) ?
            atof(withdrawable->valuestring) : withdrawable->valuedouble;
    }

    // Extract cross maintenance margin
    cJSON* cross_maint = cJSON_GetObjectItem(json, "crossMaintenanceMarginUsed");
    if (cross_maint) {
        balance->cross_maintenance_margin_used = cJSON_IsString(cross_maint) ?
            atof(cross_maint->valuestring) : cross_maint->valuedouble;
    }

    // Extract timestamp
    cJSON* time = cJSON_GetObjectItem(json, "time");
    if (time && time->valuestring) {
        balance->timestamp = (uint64_t)atoll(time->valuestring);
    }

    balance->type = HL_ACCOUNT_PERPETUAL;
    balance->spot_balances = NULL;
    balance->spot_balance_count = 0;

    cJSON_Delete(json);

    return HL_SUCCESS;
}

/**
 * @brief Fetch spot account balance
 */
static hl_error_t fetch_spot_balance(hl_client_t* client, hl_balance_t* balance) {
    const char* wallet = hl_client_get_wallet_address_old(client);
    if (!wallet) {
        return HL_ERROR_INVALID_PARAMS;
    }
    
    http_client_t* http = (http_client_t*)hl_client_get_http(client);
    if (!http) {
        return HL_ERROR_INVALID_PARAMS;
    }
    
    // Build request
    char body[512];
    snprintf(body, sizeof(body),
             "{\"type\":\"spotClearinghouseState\",\"user\":\"%s\"}",
             wallet);
    
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
    
    // Extract balances array
    cJSON* balances_array = cJSON_GetObjectItem(json, "balances");
    if (!balances_array || !cJSON_IsArray(balances_array)) {
        cJSON_Delete(json);
        return HL_ERROR_PARSE;
    }
    
    int count = cJSON_GetArraySize(balances_array);
    if (count == 0) {
        balance->spot_balances = NULL;
        balance->spot_balance_count = 0;
        balance->type = HL_ACCOUNT_SPOT;
        cJSON_Delete(json);
        return HL_SUCCESS;
    }
    
    // Allocate spot balances
    hl_spot_balance_t* spot_balances = calloc(count, sizeof(hl_spot_balance_t));
    if (!spot_balances) {
        cJSON_Delete(json);
        return HL_ERROR_MEMORY;
    }
    
    // Parse each balance
    for (int i = 0; i < count; i++) {
        cJSON* item = cJSON_GetArrayItem(balances_array, i);
        if (!item) continue;
        
        cJSON* coin = cJSON_GetObjectItem(item, "coin");
        cJSON* total = cJSON_GetObjectItem(item, "total");
        cJSON* hold = cJSON_GetObjectItem(item, "hold");
        
        if (coin && total && hold) {
            strncpy(spot_balances[i].coin, coin->valuestring, sizeof(spot_balances[i].coin) - 1);
            spot_balances[i].total = cJSON_IsString(total) ? atof(total->valuestring) : total->valuedouble;
            spot_balances[i].hold = cJSON_IsString(hold) ? atof(hold->valuestring) : hold->valuedouble;
            spot_balances[i].available = spot_balances[i].total - spot_balances[i].hold;
        }
    }
    
    balance->type = HL_ACCOUNT_SPOT;
    balance->spot_balances = spot_balances;
    balance->spot_balance_count = count;
    
    // Reset perpetual fields
    balance->account_value = 0;
    balance->total_margin_used = 0;
    balance->withdrawable = 0;
    
    cJSON_Delete(json);
    
    return HL_SUCCESS;
}

/**
 * @brief Fetch account balance
 */
hl_error_t hl_fetch_balance(hl_client_t* client, hl_account_type_t type, hl_balance_t* balance) {
    if (!client || !balance) {
        return HL_ERROR_INVALID_PARAMS;
    }

    memset(balance, 0, sizeof(hl_balance_t));

    if (type == HL_ACCOUNT_PERPETUAL) {
        return fetch_perpetual_balance(client, balance);
    } else {
        return fetch_spot_balance(client, balance);
    }
}

/**
 * @brief Free spot balances
 */
void hl_free_spot_balances(hl_spot_balance_t* balances, size_t count) {
    (void)count;
    if (balances) {
        free(balances);
    }
}

/**
 * @brief Parse single position from JSON
 */
static hl_error_t parse_position(cJSON* position_data, hl_position_t* position) {
    if (!position_data || !position) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Get the position object
    cJSON* pos = cJSON_GetObjectItem(position_data, "position");
    if (!pos) {
        return HL_ERROR_PARSE;
    }

    // Parse coin/symbol
    cJSON* coin = cJSON_GetObjectItem(pos, "coin");
    if (!coin || !cJSON_IsString(coin)) {
        return HL_ERROR_PARSE;
    }

    // For now, create symbol as "COIN/USDC:USDC"
    // TODO: Use proper market mapping when fetch_markets is implemented
    char symbol[64];
    snprintf(symbol, sizeof(symbol), "%s/USDC:USDC", coin->valuestring);
    strncpy(position->symbol, symbol, sizeof(position->symbol) - 1);

    // Parse size and determine side
    cJSON* szi = cJSON_GetObjectItem(pos, "szi");
    if (szi) {
        position->size = cJSON_IsString(szi) ? atof(szi->valuestring) : szi->valuedouble;
        position->side = (position->size > 0) ? HL_POSITION_LONG : HL_POSITION_SHORT;
        position->size = fabs(position->size); // Absolute value
    }

    // Parse entry price
    cJSON* entry_px = cJSON_GetObjectItem(pos, "entryPx");
    if (entry_px) {
        position->entry_price = cJSON_IsString(entry_px) ?
            atof(entry_px->valuestring) : entry_px->valuedouble;
    }

    // Parse liquidation price
    cJSON* liq_px = cJSON_GetObjectItem(pos, "liquidationPx");
    if (liq_px) {
        position->liquidation_price = cJSON_IsString(liq_px) ?
            atof(liq_px->valuestring) : liq_px->valuedouble;
    }

    // Parse unrealized PnL
    cJSON* unrealized_pnl = cJSON_GetObjectItem(pos, "unrealizedPnl");
    if (unrealized_pnl) {
        position->unrealized_pnl = cJSON_IsString(unrealized_pnl) ?
            atof(unrealized_pnl->valuestring) : unrealized_pnl->valuedouble;
    }

    // Parse margin used
    cJSON* margin_used = cJSON_GetObjectItem(pos, "marginUsed");
    if (margin_used) {
        position->margin_used = cJSON_IsString(margin_used) ?
            atof(margin_used->valuestring) : margin_used->valuedouble;
    }

    // Parse position value
    cJSON* position_value = cJSON_GetObjectItem(pos, "positionValue");
    if (position_value) {
        position->position_value = cJSON_IsString(position_value) ?
            atof(position_value->valuestring) : position_value->valuedouble;
    }

    // Parse return on equity
    cJSON* roe = cJSON_GetObjectItem(pos, "returnOnEquity");
    if (roe) {
        position->return_on_equity = cJSON_IsString(roe) ?
            atof(roe->valuestring) : roe->valuedouble;
    }

    // Parse leverage
    cJSON* leverage_obj = cJSON_GetObjectItem(pos, "leverage");
    if (leverage_obj && cJSON_IsObject(leverage_obj)) {
        cJSON* leverage_value = cJSON_GetObjectItem(leverage_obj, "value");
        if (leverage_value) {
            position->leverage = cJSON_IsString(leverage_value) ?
                atoi(leverage_value->valuestring) : leverage_value->valueint;
        }

        cJSON* leverage_type = cJSON_GetObjectItem(leverage_obj, "type");
        if (leverage_type && cJSON_IsString(leverage_type)) {
            position->is_isolated = (strcmp(leverage_type->valuestring, "isolated") == 0);
        }
    }

    // Parse max leverage
    cJSON* max_leverage = cJSON_GetObjectItem(pos, "maxLeverage");
    if (max_leverage) {
        position->max_leverage = cJSON_IsString(max_leverage) ?
            atoi(max_leverage->valuestring) : max_leverage->valueint;
    }

    // Parse cumulative funding
    cJSON* cum_funding = cJSON_GetObjectItem(pos, "cumFunding");
    if (cum_funding && cJSON_IsObject(cum_funding)) {
        cJSON* all_time = cJSON_GetObjectItem(cum_funding, "allTime");
        if (all_time) {
            position->cum_funding_all_time = cJSON_IsString(all_time) ?
                atof(all_time->valuestring) : all_time->valuedouble;
        }

        cJSON* since_open = cJSON_GetObjectItem(cum_funding, "sinceOpen");
        if (since_open) {
            position->cum_funding_since_open = cJSON_IsString(since_open) ?
                atof(since_open->valuestring) : since_open->valuedouble;
        }

        cJSON* since_change = cJSON_GetObjectItem(cum_funding, "sinceChange");
        if (since_change) {
            position->cum_funding_since_change = cJSON_IsString(since_change) ?
                atof(since_change->valuestring) : since_change->valuedouble;
        }
    }

    // Set coin symbol
    strncpy(position->coin, coin->valuestring, sizeof(position->coin) - 1);

    return HL_SUCCESS;
}

/**
 * @brief Fetch all positions
 */
hl_error_t hl_fetch_positions(hl_client_t* client, hl_position_t** positions, size_t* count) {
    if (!client || !positions || !count) {
        return HL_ERROR_INVALID_PARAMS;
    }

    const char* wallet = hl_client_get_wallet_address_old(client);
    if (!wallet) {
        return HL_ERROR_INVALID_PARAMS;
    }

    http_client_t* http = (http_client_t*)hl_client_get_http(client);
    if (!http) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Build request - same as balance but we parse assetPositions
    char body[512];
    snprintf(body, sizeof(body),
             "{\"type\":\"clearinghouseState\",\"user\":\"%s\"}",
             wallet);

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

    // Get assetPositions array
    cJSON* asset_positions = cJSON_GetObjectItem(json, "assetPositions");
    if (!asset_positions || !cJSON_IsArray(asset_positions)) {
        cJSON_Delete(json);
        *positions = NULL;
        *count = 0;
        return HL_SUCCESS; // No positions is not an error
    }

    int positions_count = cJSON_GetArraySize(asset_positions);
    if (positions_count == 0) {
        cJSON_Delete(json);
        *positions = NULL;
        *count = 0;
        return HL_SUCCESS;
    }

    // Allocate positions array
    hl_position_t* positions_array = calloc(positions_count, sizeof(hl_position_t));
    if (!positions_array) {
        cJSON_Delete(json);
        return HL_ERROR_MEMORY;
    }

    // Parse each position
    int valid_positions = 0;
    for (int i = 0; i < positions_count; i++) {
        cJSON* pos_data = cJSON_GetArrayItem(asset_positions, i);
        if (!pos_data) continue;

        hl_error_t parse_err = parse_position(pos_data, &positions_array[valid_positions]);
        if (parse_err == HL_SUCCESS) {
            valid_positions++;
        }
        // Skip invalid positions but continue parsing others
    }

    cJSON_Delete(json);

    *positions = positions_array;
    *count = valid_positions;

    return HL_SUCCESS;
}

/**
 * @brief Fetch single position
 */
hl_error_t hl_fetch_position(hl_client_t* client, const char* symbol, hl_position_t* position) {
    if (!client || !symbol || !position) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Extract coin from symbol (e.g., "ETH/USDC:USDC" -> "ETH")
    const char* coin = symbol;
    const char* slash = strchr(symbol, '/');
    if (slash) {
        // Copy coin to temporary buffer
        char coin_buf[32];
        size_t coin_len = slash - symbol;
        if (coin_len >= sizeof(coin_buf)) {
            return HL_ERROR_INVALID_PARAMS;
        }
        memcpy(coin_buf, symbol, coin_len);
        coin_buf[coin_len] = '\0';
        coin = coin_buf;
    }

    // Fetch all positions and find the one we need
    hl_position_t* all_positions = NULL;
    size_t count = 0;

    hl_error_t err = hl_fetch_positions(client, &all_positions, &count);
    if (err != HL_SUCCESS) {
        return err;
    }

    // Find position by coin
    for (size_t i = 0; i < count; i++) {
        if (strcmp(all_positions[i].coin, coin) == 0) {
            // Found the position, copy it
            memcpy(position, &all_positions[i], sizeof(hl_position_t));
            hl_free_positions(all_positions, count);
            return HL_SUCCESS;
        }
    }

    // Position not found
    hl_free_positions(all_positions, count);
    return HL_ERROR_NOT_FOUND;
}

/**
 * @brief Free positions array
 */
void hl_free_positions(hl_position_t* positions, size_t count) {
    (void)count;
    if (positions) {
        free(positions);
    }
}

/**
 * @brief Fetch trading fee
 */
hl_error_t hl_fetch_trading_fee(hl_client_t* client, const char* symbol, hl_trading_fee_t* fee) {
    if (!client || !symbol || !fee) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // For now, return default fees
    // TODO: Implement real fee fetching when API supports it
    strncpy(fee->symbol, symbol, sizeof(fee->symbol) - 1);
    fee->maker_fee = -0.0002; // -0.02%
    fee->taker_fee = 0.0006;  // 0.06%

    return HL_SUCCESS;
}

