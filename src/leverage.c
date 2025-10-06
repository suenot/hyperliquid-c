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

    // Create leverage action
    char action_json[1024];
    if (symbol) {
        // Set leverage for specific symbol
        snprintf(action_json, sizeof(action_json),
                 "[{\"type\":\"updateLeverage\",\"leverage\":%d,\"asset\":%u}]",
                 leverage, 0); // TODO: get asset ID from symbol
    } else {
        // Set default leverage
        snprintf(action_json, sizeof(action_json),
                 "[{\"type\":\"updateLeverage\",\"leverage\":%d}]",
                 leverage);
    }

    // Build exchange request
    unsigned long long nonce = get_timestamp_ms();
    snprintf(request_body, sizeof(request_body),
             "{\"action\":{\"type\":\"batchModify\",\"modifies\":%s},\"nonce\":%llu,\"signature\":null}",
             action_json, nonce);

    // TODO: Add EIP-712 signing
    // For now, return not implemented
    return HL_ERROR_NOT_IMPLEMENTED;
}
