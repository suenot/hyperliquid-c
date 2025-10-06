/**
 * @file client.c
 * @brief Hyperliquid client implementation
 */

#include "hyperliquid.h"
#include "hl_http.h"
#include "hl_crypto_internal.h"
#include "hl_msgpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/**
 * @brief Opaque client structure
 */
struct hl_client {
    char wallet_address[43];      // 0x + 40 hex + null
    char private_key[65];          // 64 hex + null
    bool testnet;
    http_client_t *http;           // HTTP client handle
    uint32_t timeout_ms;
    pthread_mutex_t mutex;         // Thread safety
    bool debug;
};

// Forward declarations
extern lv3_error_t hyperliquid_trader_create(const char *wallet_address, 
                                              const char *private_key,
                                              bool testnet, 
                                              void **trader_out);
extern void hyperliquid_trader_destroy(void *trader);

hl_client_t* hl_client_create(const char *wallet_address,
                               const char *private_key,
                               bool testnet) {
    if (!wallet_address || !private_key) {
        return NULL;
    }

    // Validate wallet address format
    if (strlen(wallet_address) < 42 ||
        wallet_address[0] != '0' ||
        wallet_address[1] != 'x') {
        return NULL;
    }

    // Validate private key length (allow both 64 and 66 chars for 0x prefix)
    size_t key_len = strlen(private_key);
    if (key_len != 64 && key_len != 66) {
        return NULL;
    }

    // Allocate client
    hl_client_t *client = (hl_client_t*)calloc(1, sizeof(hl_client_t));
    if (!client) {
        return NULL;
    }

    // Copy credentials (strip 0x prefix if present)
    const char* wallet_copy = (strlen(wallet_address) >= 2 && wallet_address[0] == '0' && wallet_address[1] == 'x')
                            ? wallet_address + 2 : wallet_address;
    const char* key_copy = (strlen(private_key) >= 2 && private_key[0] == '0' && private_key[1] == 'x')
                         ? private_key + 2 : private_key;

    strncpy(client->wallet_address, wallet_copy, sizeof(client->wallet_address) - 1);
    strncpy(client->private_key, key_copy, sizeof(client->private_key) - 1);
    client->testnet = testnet;
    client->timeout_ms = 30000; // 30 seconds default
    client->debug = false;
    
    // Initialize mutex
    if (pthread_mutex_init(&client->mutex, NULL) != 0) {
        free(client);
        return NULL;
    }
    
    // Create HTTP client
    client->http = http_client_create();
    if (!client->http) {
        pthread_mutex_destroy(&client->mutex);
        free(client);
        return NULL;
    }
    
    return client;
}

void hl_client_destroy(hl_client_t *client) {
    if (!client) {
        return;
    }
    
    // Destroy HTTP client
    if (client->http) {
        http_client_destroy(client->http);
    }
    
    // Zero out private key before freeing
    memset(client->private_key, 0, sizeof(client->private_key));
    
    // Destroy mutex
    pthread_mutex_destroy(&client->mutex);
    
    // Free client
    free(client);
}

bool hl_test_connection(hl_client_t *client) {
    if (!client) {
        return false;
    }
    
    // Lock mutex
    pthread_mutex_lock(&client->mutex);
    
    // Build URL
    const char *base_url = client->testnet ? 
        "https://api.hyperliquid-testnet.xyz" :
        "https://api.hyperliquid.xyz";
    
    char url[256];
    snprintf(url, sizeof(url), "%s/info", base_url);
    
    // POST request to /info with minimal body
    const char *body = "{\"type\":\"meta\"}";
    const char *headers = "Content-Type: application/json";
    
    http_response_t response = {0};
    lv3_error_t result = http_client_post(client->http, url, body, headers, &response);
    
    bool success = (result == LV3_SUCCESS && response.status_code == 200);
    
    http_response_free(&response);
    
    // Unlock mutex
    pthread_mutex_unlock(&client->mutex);
    
    return success;
}

void hl_set_timeout(hl_client_t *client, uint32_t timeout_ms) {
    if (client) {
        client->timeout_ms = timeout_ms;
    }
}

void hl_set_debug(bool enabled) {
    // Global debug flag (could be per-client in future)
    (void)enabled; // TODO: implement
}

const char* hl_error_string(hl_error_t error) {
    switch (error) {
        case HL_SUCCESS: return "Success";
        case HL_ERROR_INVALID_PARAMS: return "Invalid parameters";
        case HL_ERROR_NETWORK: return "Network error";
        case HL_ERROR_API: return "API error";
        case HL_ERROR_AUTH: return "Authentication error";
        case HL_ERROR_INSUFFICIENT_BALANCE: return "Insufficient balance";
        case HL_ERROR_INVALID_SYMBOL: return "Invalid symbol";
        case HL_ERROR_ORDER_REJECTED: return "Order rejected";
        case HL_ERROR_SIGNATURE: return "Signature error";
        case HL_ERROR_MSGPACK: return "MessagePack error";
        case HL_ERROR_JSON: return "JSON error";
        case HL_ERROR_MEMORY: return "Memory allocation failed";
        case HL_ERROR_TIMEOUT: return "Operation timed out";
        default: return "Unknown error";
    }
}

const char* hl_version(void) {
    return HL_VERSION_STRING;
}

// Internal accessor for other modules
const char* hl_client_get_wallet_address(hl_client_t *client) {
    return client ? client->wallet_address : NULL;
}

const char* hl_client_get_private_key(hl_client_t *client) {
    return client ? client->private_key : NULL;
}

bool hl_client_is_testnet(hl_client_t *client) {
    return client ? client->testnet : false;
}

http_client_t* hl_client_get_http(hl_client_t *client) {
    return client ? client->http : NULL;
}

pthread_mutex_t* hl_client_get_mutex(hl_client_t *client) {
    return client ? &client->mutex : NULL;
}

