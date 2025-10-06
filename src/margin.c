/**
 * @file margin.c
 * @brief Margin management functions for positions
 */

#include "hyperliquid.h"
#include "hl_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Add margin to a position
 */
hl_error_t hl_add_margin(hl_client_t* client,
                        const char* symbol,
                        double amount) {
    if (!client || !symbol || amount <= 0) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Prepare request
    char request_body[2048];
    const char* wallet_address = hl_client_get_wallet_address_old(client);
    const char* private_key = hl_client_get_private_key_old(client);

    if (!wallet_address || !private_key) {
        return HL_ERROR_AUTH;
    }

    // Get asset ID from symbol
    uint32_t asset_id = 0;
    char coin[32] = {0};
    const char* separator = strchr(symbol, '/');
    if (separator) {
        size_t coin_len = separator - symbol;
        if (coin_len < sizeof(coin)) {
            memcpy(coin, symbol, coin_len);
        }
    } else {
        strncpy(coin, symbol, sizeof(coin) - 1);
    }

    // Basic asset mapping
    if (strcmp(coin, "BTC") == 0) asset_id = 1;
    else if (strcmp(coin, "ETH") == 0) asset_id = 2;
    else if (strcmp(coin, "SOL") == 0) asset_id = 3;
    else {
        // Try to get from markets
        hl_markets_t markets = {0};
        if (hl_fetch_markets(client, &markets) == HL_SUCCESS) {
            for (size_t i = 0; i < markets.count; i++) {
                if (strcmp(markets.markets[i].base, coin) == 0) {
                    asset_id = markets.markets[i].asset_id;
                    break;
                }
            }
            hl_markets_free(&markets);
        }
    }

    if (asset_id == 0) {
        return HL_ERROR_INVALID_PARAMS; // Asset not found
    }

    // Create margin action
    char action_json[256];
    snprintf(action_json, sizeof(action_json),
             "[{\"type\":\"updateMargin\",\"asset\":%u,\"margin\":%.8f}]",
             asset_id, amount);

    // Build exchange request
    unsigned long long nonce = get_timestamp_ms();
    char exchange_request[1024];
    snprintf(exchange_request, sizeof(exchange_request),
             "{\"action\":{\"type\":\"batchModify\",\"modifies\":%s},\"nonce\":%llu}",
             action_json, nonce);

    // Create EIP-712 signature
    char signature[132] = {0};
    if (hl_build_exchange_signature(exchange_request, private_key, signature, sizeof(signature)) != HL_SUCCESS) {
        return HL_ERROR_SIGNATURE;
    }

    // Build final request with signature
    snprintf(request_body, sizeof(request_body),
             "%s,\"signature\":\"%s\"}", exchange_request, signature);

    // Make request
    http_response_t* response = NULL;
    lv3_error_t err = http_client_post_old(client, "/exchange", request_body, NULL, &response);

    if (err != LV3_SUCCESS || !response) {
        if (response) {
            http_response_free(response);
        }
        return HL_ERROR_NETWORK;
    }

    // Check response
    hl_error_t result = HL_SUCCESS;
    if (response->status_code != 200) {
        result = HL_ERROR_API;
    } else if (strstr(response->body, "\"status\":\"ok\"") == NULL) {
        result = HL_ERROR_API;
    }

    http_response_free(response);
    return result;
}

/**
 * @brief Reduce margin from a position
 */
hl_error_t hl_reduce_margin(hl_client_t* client,
                           const char* symbol,
                           double amount) {
    if (!client || !symbol || amount <= 0) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // For reduce margin, we use negative amount
    return hl_add_margin(client, symbol, -amount);
}
