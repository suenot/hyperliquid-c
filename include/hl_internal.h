/**
 * @file hl_internal.h
 * @brief Internal APIs for Hyperliquid SDK modules
 * 
 * This header is NOT part of the public API.
 * It provides internal functions for communication between modules.
 */

#ifndef HL_INTERNAL_H
#define HL_INTERNAL_H

#include "hyperliquid.h"
#include "hl_http.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// Internal client accessors (used by trading/account/market modules)
const char* hl_client_get_wallet_address(hl_client_t *client);
const char* hl_client_get_private_key(hl_client_t *client);
bool hl_client_is_testnet(hl_client_t *client);
http_client_t* hl_client_get_http(hl_client_t *client);
pthread_mutex_t* hl_client_get_mutex(hl_client_t *client);

// Error code mapping (hl_error_t to lv3_error_t)
static inline int hl_to_lv3_error(lv3_error_t lv3_err) {
    switch (lv3_err) {
        case LV3_SUCCESS: return HL_SUCCESS;
        case LV3_ERROR_INVALID_PARAMS: return HL_ERROR_INVALID_PARAMS;
        case LV3_ERROR_NETWORK: return HL_ERROR_NETWORK;
        case LV3_ERROR_HTTP: return HL_ERROR_NETWORK;
        case LV3_ERROR_JSON: return HL_ERROR_JSON;
        case LV3_ERROR_EXCHANGE: return HL_ERROR_API;
        case LV3_ERROR_MEMORY: return HL_ERROR_MEMORY;
        default: return HL_ERROR_API;
    }
}

#ifdef __cplusplus
}
#endif

#endif // HL_INTERNAL_H

