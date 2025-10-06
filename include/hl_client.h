/**
 * @file hl_client.h
 * @brief CCXT-compatible client interface
 */

#ifndef HL_CLIENT_H
#define HL_CLIENT_H

#include "hl_exchange.h"
#include "hl_types.h"
#include <stdbool.h>

// Forward declarations
typedef struct hl_client hl_client_t;
typedef struct hl_options hl_options_t;
typedef struct hl_http_client hl_http_client_t;
typedef struct hl_ws_client hl_ws_client_t;

// Callback types
typedef void (*hl_callback_t)(void* data, size_t size);
typedef void (*hl_error_callback_t)(int error, const char* message);

/**
 * @brief Client options
 */
struct hl_options {
    bool testnet;                   /**< use testnet */
    int timeout;                    /**< request timeout in ms */
    int rate_limit;                 /**< rate limit between requests */
    bool enable_cache;              /**< enable market data caching */
    char user_agent[256];           /**< user agent string */
};

/**
 * @brief Main Hyperliquid client (CCXT compatible)
 */
struct hl_client {
    // Exchange configuration
    const hl_exchange_config_t* config;

    // Authentication
    char api_key[128];
    char secret[128];
    char wallet_address[64];

    // HTTP/WebSocket clients
    hl_http_client_t* http_client;
    hl_ws_client_t* ws_client;

    // WebSocket extensions
    void* ws_extension;             /**< WebSocket subscription management */

    // State
    hl_markets_t* markets;          /**< cached markets */
    hl_balances_t* balances;        /**< cached balances */

    // Options
    hl_options_t options;

    // Callbacks
    hl_error_callback_t on_error;
    hl_callback_t on_message;

    // Internal state
    void* internal;                 /**< internal client data */
};

/**
 * @brief Create new Hyperliquid client
 * @param wallet_address Wallet address (required for trading)
 * @param private_key Private key (required for trading)
 * @param testnet Use testnet
 * @return Pointer to client or NULL on error
 */
hl_client_t* hl_client_new(const char* wallet_address, const char* private_key, bool testnet);

/**
 * @brief Free client resources
 * @param client Client to free
 */
void hl_client_free(hl_client_t* client);

/**
 * @brief Get client testnet status
 * @param client Client instance
 * @return true if testnet
 */
bool hl_client_is_testnet(const hl_client_t* client);

/**
 * @brief Get wallet address
 * @param client Client instance
 * @return Wallet address
 */
const char* hl_client_get_wallet_address(const hl_client_t* client);

/**
 * @brief Get private key
 * @param client Client instance
 * @return Private key
 */
const char* hl_client_get_private_key(const hl_client_t* client);

/**
 * @brief Get HTTP client
 * @param client Client instance
 * @return HTTP client
 */
hl_http_client_t* hl_client_get_http_client(hl_client_t* client);

/**
 * @brief Get WebSocket client
 * @param client Client instance
 * @return WebSocket client
 */
hl_ws_client_t* hl_client_get_ws_client(hl_client_t* client);

/**
 * @brief Set error callback
 * @param client Client instance
 * @param callback Error callback function
 */
void hl_client_set_error_callback(hl_client_t* client, hl_error_callback_t callback);

/**
 * @brief Set message callback
 * @param client Client instance
 * @param callback Message callback function
 */
void hl_client_set_message_callback(hl_client_t* client, hl_callback_t callback);

/**
 * @brief Test connection to exchange
 * @param client Client instance
 * @return true if connection successful
 */
bool hl_client_test_connection(hl_client_t* client);

/**
 * @brief Load markets from exchange
 * @param client Client instance
 * @return error code
 */
hl_error_t hl_client_load_markets(hl_client_t* client);

/**
 * @brief Get cached markets
 * @param client Client instance
 * @return Markets or NULL if not loaded
 */
const hl_markets_t* hl_client_get_markets(const hl_client_t* client);

/**
 * @brief Get market by symbol
 * @param client Client instance
 * @param symbol Trading symbol
 * @return Market or NULL if not found
 */
const hl_market_t* hl_client_get_market(const hl_client_t* client, const char* symbol);

/**
 * @brief Check if exchange has specific capability
 * @param client Client instance
 * @param capability Capability name
 * @return true if supported
 */
bool hl_client_has_capability(const hl_client_t* client, const char* capability);

// Legacy compatibility functions
/**
 * @brief Legacy: Get wallet address accessor
 * @param client Client instance
 * @return Wallet address
 */
static inline const char* hl_client_get_wallet(const hl_client_t* client) {
    return hl_client_get_wallet_address(client);
}

/**
 * @brief Legacy: Get private key accessor
 * @param client Client instance
 * @return Private key
 */
static inline const char* hl_client_get_key(const hl_client_t* client) {
    return hl_client_get_private_key(client);
}

#endif // HL_CLIENT_H
