/**
 * @file account.c
 * @brief Account & balance management implementation
 */

#include "hl_account.h"
#include "hyperliquid.h"
#include "hl_http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
 * @brief Fetch perpetual account balance
 */
static hl_error_t fetch_perpetual_balance(hl_client_t* client, hl_balance_t* balance) {
    const char* wallet = hl_client_get_wallet_address(client);
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
        balance->account_value = cJSON_GetObjectItem(margin_summary, "accountValue")->valuedouble;
        balance->total_margin_used = cJSON_GetObjectItem(margin_summary, "totalMarginUsed")->valuedouble;
        balance->total_ntl_pos = cJSON_GetObjectItem(margin_summary, "totalNtlPos")->valuedouble;
        balance->total_raw_usd = cJSON_GetObjectItem(margin_summary, "totalRawUsd")->valuedouble;
    }
    
    // Extract cross margin summary
    cJSON* cross_summary = cJSON_GetObjectItem(json, "crossMarginSummary");
    if (cross_summary) {
        balance->cross_account_value = cJSON_GetObjectItem(cross_summary, "accountValue")->valuedouble;
        balance->cross_margin_used = cJSON_GetObjectItem(cross_summary, "totalMarginUsed")->valuedouble;
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
    if (time) {
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
    const char* wallet = hl_client_get_wallet_address(client);
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
 * @brief Fetch all positions
 */
hl_error_t hl_fetch_positions(hl_client_t* client, hl_position_t** positions, size_t* count) {
    if (!client || !positions || !count) {
        return HL_ERROR_INVALID_PARAMS;
    }
    
    // TODO: Implement in Day 3
    *positions = NULL;
    *count = 0;
    
    return HL_ERROR_NOT_IMPLEMENTED;
}

/**
 * @brief Fetch single position
 */
hl_error_t hl_fetch_position(hl_client_t* client, const char* symbol, hl_position_t* position) {
    if (!client || !symbol || !position) {
        return HL_ERROR_INVALID_PARAMS;
    }
    
    // TODO: Implement in Day 3
    return HL_ERROR_NOT_IMPLEMENTED;
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
    
    // TODO: Implement later
    return HL_ERROR_NOT_IMPLEMENTED;
}

