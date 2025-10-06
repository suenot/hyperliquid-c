/**
 * @file currencies.c
 * @brief Currency and funding related functions implementation
 */

#include "hyperliquid.h"
#include "hl_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Forward declarations
typedef struct hl_currency hl_currency_t;

struct hl_currency {
    char id[32];                    /**< currency id */
    char name[32];                  /**< currency name */
    char code[16];                  /**< currency code */
    int precision;                  /**< price precision */
    bool active;                    /**< currency is active */
    bool deposit;                   /**< deposit enabled */
    bool withdraw;                  /**< withdraw enabled */
    char type[16];                  /**< currency type */
    char info[1024];                /**< raw exchange data */
};

typedef struct {
    hl_currency_t* currencies;
    size_t count;
} hl_currencies_t;

/**
 * @brief Fetch all available currencies
 */
hl_error_t hl_fetch_currencies(hl_client_t* client, hl_currencies_t* currencies) {
    if (!client || !currencies) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Clear output
    memset(currencies, 0, sizeof(hl_currencies_t));

    // Prepare request - use meta endpoint
    char request_body[256] = "{\"type\":\"meta\"}";

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

    // Response is an object with universe array
    cJSON* universe = cJSON_GetObjectItem(json, "universe");
    if (!universe || !cJSON_IsArray(universe)) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    int array_size = cJSON_GetArraySize(universe);
    if (array_size == 0) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_SUCCESS;
    }

    // Allocate currencies array
    currencies->currencies = calloc(array_size, sizeof(hl_currency_t));
    if (!currencies->currencies) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_MEMORY;
    }

    // Parse currencies
    size_t valid_currencies = 0;
    for (int i = 0; i < array_size && valid_currencies < array_size; i++) {
        cJSON* currency_json = cJSON_GetArrayItem(universe, i);
        if (!currency_json || !cJSON_IsObject(currency_json)) {
            continue;
        }

        hl_currency_t* currency = &currencies->currencies[valid_currencies];

        // Extract fields
        cJSON* name = cJSON_GetObjectItem(currency_json, "name");
        cJSON* sz_decimals = cJSON_GetObjectItem(currency_json, "szDecimals");

        if (name && cJSON_IsString(name)) {
            // Set ID (index)
            snprintf(currency->id, sizeof(currency->id), "%d", i);

            // Set name and code (same for crypto)
            strncpy(currency->name, name->valuestring, sizeof(currency->name) - 1);
            strncpy(currency->code, name->valuestring, sizeof(currency->code) - 1);

            // Set precision
            if (sz_decimals && cJSON_IsNumber(sz_decimals)) {
                currency->precision = (int)sz_decimals->valuedouble;
            } else {
                currency->precision = 8; // default
            }

            // Set other properties
            currency->active = true;
            currency->deposit = true;  // Assume available
            currency->withdraw = true; // Assume available
            strcpy(currency->type, "crypto");

            // Store raw info (simplified)
            char info_buf[256];
            snprintf(info_buf, sizeof(info_buf), "{\"name\":\"%s\",\"szDecimals\":%d}",
                    currency->name, currency->precision);
            strncpy(currency->info, info_buf, sizeof(currency->info) - 1);

            valid_currencies++;
        }
    }

    currencies->count = valid_currencies;

    cJSON_Delete(json);
    http_response_free(response);

    return HL_SUCCESS;
}

/**
 * @brief Free currencies array
 */
void hl_free_currencies(hl_currencies_t* currencies) {
    if (currencies && currencies->currencies) {
        free(currencies->currencies);
        currencies->currencies = NULL;
        currencies->count = 0;
    }
}
