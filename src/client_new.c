/**
 * @file client_new.c
 * @brief CCXT-compatible client implementation
 */

#include "hl_client.h"
#include "hl_internal.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

// Forward declarations for internal functions
static hl_http_client_t* http_client_create_internal(void);
static void http_client_free_internal(hl_http_client_t* client);
static hl_ws_client_t* ws_client_create_internal(void);
static void ws_client_free_internal(hl_ws_client_t* client);

hl_client_t* hl_client_new(const char* wallet_address, const char* private_key, bool testnet) {
    if (!wallet_address || !private_key) {
        return NULL;
    }

    hl_client_t* client = calloc(1, sizeof(hl_client_t));
    if (!client) {
        return NULL;
    }

    // Initialize configuration
    client->config = hl_exchange_describe();
    if (!client->config) {
        free(client);
        return NULL;
    }

    // Copy authentication data
    strncpy(client->api_key, wallet_address, sizeof(client->api_key) - 1);
    strncpy(client->secret, private_key, sizeof(client->secret) - 1);
    strncpy(client->wallet_address, wallet_address, sizeof(client->wallet_address) - 1);

    // Initialize options
    client->options.testnet = testnet;
    client->options.timeout = 30000; // 30 seconds
    client->options.rate_limit = client->config->rate_limit;
    client->options.enable_cache = true;
    strcpy(client->options.user_agent, "Hyperliquid-C-SDK/1.0.0");

    // Create HTTP client
    client->http_client = http_client_create_internal();
    if (!client->http_client) {
        free(client);
        return NULL;
    }

    // Create WebSocket client (optional for now)
    client->ws_client = NULL; // Will be created on demand

    // Initialize markets cache
    client->markets = NULL;

    // Initialize balances cache
    client->balances = NULL;

    // Initialize callbacks
    client->on_error = NULL;
    client->on_message = NULL;

    return client;
}

void hl_client_free(hl_client_t* client) {
    if (!client) return;

    // Free HTTP client
    if (client->http_client) {
        http_client_free_internal(client->http_client);
    }

    // Free WebSocket client
    if (client->ws_client) {
        ws_client_free_internal(client->ws_client);
    }

    // Free markets cache
    if (client->markets) {
        if (client->markets->markets) {
            for (size_t i = 0; i < client->markets->count; i++) {
                hl_market_free(&client->markets->markets[i]);
            }
            free(client->markets->markets);
        }
        free(client->markets);
    }

    // Free balances cache
    if (client->balances) {
        if (client->balances->balances) {
            for (size_t i = 0; i < client->balances->count; i++) {
                hl_balance_free(&client->balances->balances[i]);
            }
            free(client->balances->balances);
        }
        free(client->balances);
    }

    free(client);
}

bool hl_client_is_testnet(const hl_client_t* client) {
    return client ? client->options.testnet : false;
}

const char* hl_client_get_wallet_address(const hl_client_t* client) {
    return client ? client->wallet_address : NULL;
}

const char* hl_client_get_private_key(const hl_client_t* client) {
    return client ? client->secret : NULL;
}

hl_http_client_t* hl_client_get_http_client(hl_client_t* client) {
    return client ? client->http_client : NULL;
}

hl_ws_client_t* hl_client_get_ws_client(hl_client_t* client) {
    if (!client) return NULL;

    // Create WebSocket client on demand
    if (!client->ws_client && hl_client_has_capability(client, "ws")) {
        client->ws_client = ws_client_create_internal();
    }

    return client->ws_client;
}

void hl_client_set_error_callback(hl_client_t* client, hl_error_callback_t callback) {
    if (client) {
        client->on_error = callback;
    }
}

void hl_client_set_message_callback(hl_client_t* client, hl_callback_t callback) {
    if (client) {
        client->on_message = callback;
    }
}

bool hl_client_test_connection(hl_client_t* client) {
    if (!client || !client->http_client) return false;

    // TODO: Implement connection test using new client
    return true; // Placeholder
}

hl_error_t hl_client_load_markets(hl_client_t* client) {
    if (!client) return -1;

    // Free existing markets
    if (client->markets) {
        if (client->markets->markets) {
            for (size_t i = 0; i < client->markets->count; i++) {
                hl_market_free(&client->markets->markets[i]);
            }
            free(client->markets->markets);
        }
        free(client->markets);
        client->markets = NULL;
    }

    // Load markets using existing function
    hl_markets_t markets = {0};
    hl_error_t err = hl_fetch_markets(client, &markets);

    if (err == 0) {
        client->markets = malloc(sizeof(hl_markets_t));
        if (client->markets) {
            *client->markets = markets;
        }
    }

    return err;
}

const hl_markets_t* hl_client_get_markets(const hl_client_t* client) {
    return client ? client->markets : NULL;
}

const hl_market_t* hl_client_get_market(const hl_client_t* client, const char* symbol) {
    if (!client || !client->markets || !symbol) return NULL;

    for (size_t i = 0; i < client->markets->count; i++) {
        if (strcmp(client->markets->markets[i].symbol, symbol) == 0) {
            return &client->markets->markets[i];
        }
    }

    return NULL;
}

bool hl_client_has_capability(const hl_client_t* client, const char* capability) {
    return client && client->config ?
           hl_exchange_has(client->config, capability) : false;
}

// Internal HTTP client wrapper (for now using existing implementation)
static hl_http_client_t* http_client_create_internal(void) {
    // For now, return a dummy pointer - we'll integrate with existing HTTP client
    return (hl_http_client_t*)malloc(1); // Placeholder
}

static void http_client_free_internal(hl_http_client_t* client) {
    if (client) free(client);
}

// Internal WebSocket client wrapper
static hl_ws_client_t* ws_client_create_internal(void) {
    // WebSocket support not implemented yet
    return NULL;
}

static void ws_client_free_internal(hl_ws_client_t* client) {
    // WebSocket cleanup not implemented yet
}

// Legacy compatibility - include old client functions
extern hl_client_t* hl_client_create(const char* wallet, const char* key, bool testnet);
extern void hl_client_destroy(hl_client_t* client);
extern bool hl_test_connection(hl_client_t* client);

// Wrapper functions for backward compatibility
hl_client_t* hl_client_create(const char* wallet, const char* key, bool testnet) {
    return hl_client_new(wallet, key, testnet);
}

void hl_client_destroy(hl_client_t* client) {
    hl_client_free(client);
}

bool hl_test_connection(hl_client_t* client) {
    return hl_client_test_connection(client);
}
