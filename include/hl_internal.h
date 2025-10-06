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
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for internal structures
typedef struct exchange_trader exchange_trader_t;

// Exchange trader structure (for bridging old code)
struct exchange_trader {
    char exchange_name[32];
    void *exchange_data;
    
    // Function pointers (old API - for internal use only)
    lv3_error_t (*execute_trade)(exchange_trader_t *trader, const char *symbol,
                                 const char *direction, double usdt_amount,
                                 int leverage, double tp_percent, double sl_percent,
                                 void *result);
    bool (*test_connection)(exchange_trader_t *trader);
    lv3_error_t (*calculate_quantity)(exchange_trader_t *trader, const char *symbol,
                                      double price, double usdt_amount, int leverage,
                                      char *quantity_out, size_t quantity_size);
    lv3_error_t (*get_market_price)(exchange_trader_t *trader, const char *symbol, double *price_out);
    lv3_error_t (*place_limit_order)(exchange_trader_t *trader, const char *symbol,
                                     bool is_buy, double price, double quantity,
                                     char *order_id_out, size_t order_id_size);
    lv3_error_t (*cancel_order)(exchange_trader_t *trader, const char *symbol, const char *order_id);
    lv3_error_t (*get_balance)(exchange_trader_t *trader, double *balance_out);
    lv3_error_t (*get_positions)(exchange_trader_t *trader, char **positions_json_out);
    lv3_error_t (*get_trade_history)(exchange_trader_t *trader, const char *symbol, char **history_json_out);
};

// Internal client accessors (used by trading/account/market modules)
const char* hl_client_get_wallet_address_old(hl_client_t *client);
const char* hl_client_get_private_key_old(hl_client_t *client);
bool hl_client_is_testnet_old(hl_client_t *client);
http_client_t* hl_client_get_http_old(hl_client_t *client);
pthread_mutex_t* hl_client_get_mutex_old(hl_client_t *client);

// Utility functions
static inline void lv3_string_copy(char *dest, const char *src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) return;
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

// Memory management wrappers
static inline void* lv3_calloc(size_t count, size_t size) {
    return calloc(count, size);
}

static inline void* lv3_realloc(void* ptr, size_t size) {
    return realloc(ptr, size);
}

static inline void lv3_free(void* ptr) {
    free(ptr);
}

// Time utilities
static inline int64_t lv3_current_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000;
}

// Compatibility wrappers for old http_client functions
static inline lv3_error_t http_client_get_old(http_client_t *client, const http_request_t *req, http_response_t *resp) {
    return http_client_get(client, req->url, resp);
}

static inline lv3_error_t http_client_post_old(http_client_t *client, const http_request_t *req, http_response_t *resp) {
    return http_client_post(client, req->url, req->body, req->headers, resp);
}

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

