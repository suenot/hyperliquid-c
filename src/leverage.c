/**
 * @file leverage.c
 * @brief Leverage and margin management functions
 */

#include "hyperliquid.h"
#include "hl_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Set leverage for symbol
 */
hl_error_t hl_set_leverage(hl_client_t* client,
                          int leverage,
                          const char* symbol) {
    if (!client || leverage < 1 || leverage > 50) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Prepare request
    char request_body[2048];
    const char* wallet_address = hl_client_get_wallet_address_old(client);
    const char* private_key = hl_client_get_private_key_old(client);
    bool testnet = hl_client_is_testnet_old(client);

    if (!wallet_address || !private_key) {
        return HL_ERROR_AUTH;
    }

    // Get asset ID if symbol is specified
    uint32_t asset_id = 0;
    if (symbol) {
        // Extract coin name from symbol (e.g., "BTC/USDC:USDC" -> "BTC")
        char coin[32] = {0};
        const char* separator = strchr(symbol, '/');
        if (separator) {
            size_t coin_len = separator - symbol;
            if (coin_len < sizeof(coin)) {
                memcpy(coin, symbol, coin_len);
            }
        } else {
            // Assume symbol is just the coin name
            strncpy(coin, symbol, sizeof(coin) - 1);
        }

        // Get asset ID from coin name
        // For simplicity, use a basic mapping (in production, this would query the API)
        if (strcmp(coin, "BTC") == 0) asset_id = 1;
        else if (strcmp(coin, "ETH") == 0) asset_id = 2;
        else if (strcmp(coin, "SOL") == 0) asset_id = 3;
        else if (strcmp(coin, "USDC") == 0) asset_id = 0;
        else {
            // Try to get from markets (simplified approach)
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
    }

    // Create leverage action
    char action_json[1024];
    if (symbol && asset_id > 0) {
        // Set leverage for specific asset
        snprintf(action_json, sizeof(action_json),
                 "[{\"type\":\"updateLeverage\",\"leverage\":%d,\"asset\":%u}]",
                 leverage, asset_id);
    } else {
        // Set default leverage
        snprintf(action_json, sizeof(action_json),
                 "[{\"type\":\"updateLeverage\",\"leverage\":%d}]",
                 leverage);
    }

    // Build exchange request
    unsigned long long nonce = get_timestamp_ms();
    char exchange_request[1536];
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
        // Parse error response
        if (strstr(response->body, "error")) {
            result = HL_ERROR_API;
        } else {
            result = HL_ERROR_NETWORK;
        }
    } else if (strstr(response->body, "\"status\":\"ok\"") == NULL) {
        result = HL_ERROR_API;
    }

    http_response_free(response);
    return result;
}
