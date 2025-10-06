/**
 * @file websocket.c
 * @brief WebSocket API implementation
 */

#include "hyperliquid.h"
#include "hl_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <uuid/uuid.h>

// Internal client extension for WebSocket
typedef struct {
    hl_ws_client_t* ws_client;          /**< WebSocket client */
    hl_ws_subscription_t* subscriptions; /**< Active subscriptions */
    size_t subscription_count;          /**< Number of subscriptions */
    size_t subscription_capacity;       /**< Subscription array capacity */
} hl_client_ws_extension_t;

/**
 * @brief Generate unique subscription ID
 */
static void generate_subscription_id(char* buffer, size_t size) {
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse_lower(uuid, buffer);
}

/**
 * @brief Add subscription to client
 */
static const char* add_subscription(hl_client_t* client, const char* channel,
                                   const char* symbol, hl_ws_data_callback_t callback,
                                   void* user_data) {
    if (!client) return NULL;

    hl_client_ws_extension_t* ws_ext = (hl_client_ws_extension_t*)client->ws_extension;
    if (!ws_ext) return NULL;

    // Expand subscription array if needed
    if (ws_ext->subscription_count >= ws_ext->subscription_capacity) {
        size_t new_capacity = ws_ext->subscription_capacity * 2;
        if (new_capacity == 0) new_capacity = 8;

        hl_ws_subscription_t* new_subs = realloc(ws_ext->subscriptions,
                                                new_capacity * sizeof(hl_ws_subscription_t));
        if (!new_subs) return NULL;

        ws_ext->subscriptions = new_subs;
        ws_ext->subscription_capacity = new_capacity;
    }

    // Create subscription
    hl_ws_subscription_t* sub = &ws_ext->subscriptions[ws_ext->subscription_count];
    generate_subscription_id(sub->subscription_id, sizeof(sub->subscription_id));
    strncpy(sub->channel, channel, sizeof(sub->channel) - 1);
    if (symbol) {
        strncpy(sub->symbol, symbol, sizeof(sub->symbol) - 1);
    }
    sub->callback = callback;
    sub->user_data = user_data;
    sub->active = true;

    ws_ext->subscription_count++;

    return sub->subscription_id;
}

/**
 * @brief WebSocket message handler
 */
static void ws_message_handler(const char* message, size_t size, void* user_data) {
    hl_client_t* client = (hl_client_t*)user_data;
    if (!client) return;

    // Parse WebSocket message and dispatch to appropriate callbacks
    // This is a placeholder - real implementation would parse JSON
    printf("WS MESSAGE: %.*s\n", (int)size, message);
}

/**
 * @brief WebSocket error handler
 */
static void ws_error_handler(const char* error, void* user_data) {
    hl_client_t* client = (hl_client_t*)user_data;
    if (!client) return;

    printf("WS ERROR: %s\n", error);
}

/**
 * @brief WebSocket connect handler
 */
static void ws_connect_handler(void* user_data) {
    hl_client_t* client = (hl_client_t*)user_data;
    if (!client) return;

    printf("WS CONNECTED\n");
}

/**
 * @brief Initialize WebSocket for client
 */
bool hl_ws_init_client(hl_client_t* client, bool testnet) {
    if (!client) return false;

    // Create WebSocket extension
    hl_client_ws_extension_t* ws_ext = calloc(1, sizeof(hl_client_ws_extension_t));
    if (!ws_ext) return false;

    // Create WebSocket client
    hl_ws_config_t config;
    hl_ws_config_default(&config, testnet);

    ws_ext->ws_client = hl_ws_client_create(&config);
    if (!ws_ext->ws_client) {
        free(ws_ext);
        return false;
    }

    // Set callbacks
    hl_ws_client_set_message_callback(ws_ext->ws_client, ws_message_handler, client);
    hl_ws_client_set_error_callback(ws_ext->ws_client, ws_error_handler, client);
    hl_ws_client_set_connect_callback(ws_ext->ws_client, ws_connect_handler, client);

    // Store extension in client (this assumes client has a ws_extension field)
    client->ws_extension = ws_ext;

    return true;
}

/**
 * @brief Cleanup WebSocket for client
 */
void hl_ws_cleanup_client(hl_client_t* client) {
    if (!client || !client->ws_extension) return;

    hl_client_ws_extension_t* ws_ext = (hl_client_ws_extension_t*)client->ws_extension;

    // Disconnect and destroy WebSocket client
    if (ws_ext->ws_client) {
        hl_ws_client_disconnect(ws_ext->ws_client);
        hl_ws_client_destroy(ws_ext->ws_client);
    }

    // Free subscriptions
    free(ws_ext->subscriptions);
    free(ws_ext);

    client->ws_extension = NULL;
}

/**
 * @brief Watch ticker updates
 */
const char* hl_watch_ticker(hl_client_t* client, const char* symbol,
                           hl_ws_data_callback_t callback, void* user_data) {
    if (!client || !symbol || !callback) return NULL;

    hl_client_ws_extension_t* ws_ext = (hl_client_ws_extension_t*)client->ws_extension;
    if (!ws_ext) return NULL;

    // Connect if not connected
    if (!hl_ws_client_is_connected(ws_ext->ws_client)) {
        if (!hl_ws_client_connect(ws_ext->ws_client)) {
            return NULL;
        }
    }

    // Subscribe to ticker channel
    char subscription_msg[256];
    snprintf(subscription_msg, sizeof(subscription_msg),
             "{\"method\":\"subscribe\",\"subscription\":{\"type\":\"ticker\",\"coin\":\"%s\"}}",
             symbol);

    if (!hl_ws_client_send_text(ws_ext->ws_client, subscription_msg)) {
        return NULL;
    }

    return add_subscription(client, "ticker", symbol, callback, user_data);
}

/**
 * @brief Watch multiple tickers
 */
const char* hl_watch_tickers(hl_client_t* client, const char** symbols, size_t symbols_count,
                            hl_ws_data_callback_t callback, void* user_data) {
    if (!client || !callback) return NULL;

    // For simplicity, subscribe to all tickers
    return hl_watch_ticker(client, "*", callback, user_data);
}

/**
 * @brief Watch order book updates
 */
const char* hl_watch_order_book(hl_client_t* client, const char* symbol, uint32_t depth,
                               hl_ws_data_callback_t callback, void* user_data) {
    if (!client || !symbol || !callback) return NULL;

    hl_client_ws_extension_t* ws_ext = (hl_client_ws_extension_t*)client->ws_extension;
    if (!ws_ext) return NULL;

    // Connect if not connected
    if (!hl_ws_client_is_connected(ws_ext->ws_client)) {
        if (!hl_ws_client_connect(ws_ext->ws_client)) {
            return NULL;
        }
    }

    // Subscribe to order book channel
    char subscription_msg[256];
    snprintf(subscription_msg, sizeof(subscription_msg),
             "{\"method\":\"subscribe\",\"subscription\":{\"type\":\"l2Book\",\"coin\":\"%s\"}}",
             symbol);

    if (!hl_ws_client_send_text(ws_ext->ws_client, subscription_msg)) {
        return NULL;
    }

    return add_subscription(client, "l2Book", symbol, callback, user_data);
}

/**
 * @brief Watch OHLCV updates
 */
const char* hl_watch_ohlcv(hl_client_t* client, const char* symbol, const char* timeframe,
                          hl_ws_data_callback_t callback, void* user_data) {
    if (!client || !symbol || !timeframe || !callback) return NULL;

    hl_client_ws_extension_t* ws_ext = (hl_client_ws_extension_t*)client->ws_extension;
    if (!ws_ext) return NULL;

    // Connect if not connected
    if (!hl_ws_client_is_connected(ws_ext->ws_client)) {
        if (!hl_ws_client_connect(ws_ext->ws_client)) {
            return NULL;
        }
    }

    // Subscribe to OHLCV channel (placeholder - Hyperliquid may not have this)
    char subscription_msg[256];
    snprintf(subscription_msg, sizeof(subscription_msg),
             "{\"method\":\"subscribe\",\"subscription\":{\"type\":\"candle\",\"coin\":\"%s\",\"interval\":\"%s\"}}",
             symbol, timeframe);

    if (!hl_ws_client_send_text(ws_ext->ws_client, subscription_msg)) {
        return NULL;
    }

    return add_subscription(client, "candle", symbol, callback, user_data);
}

/**
 * @brief Watch trades
 */
const char* hl_watch_trades(hl_client_t* client, const char* symbol,
                           hl_ws_data_callback_t callback, void* user_data) {
    if (!client || !symbol || !callback) return NULL;

    hl_client_ws_extension_t* ws_ext = (hl_client_ws_extension_t*)client->ws_extension;
    if (!ws_ext) return NULL;

    // Connect if not connected
    if (!hl_ws_client_is_connected(ws_ext->ws_client)) {
        if (!hl_ws_client_connect(ws_ext->ws_client)) {
            return NULL;
        }
    }

    // Subscribe to trades channel
    char subscription_msg[256];
    snprintf(subscription_msg, sizeof(subscription_msg),
             "{\"method\":\"subscribe\",\"subscription\":{\"type\":\"trades\",\"coin\":\"%s\"}}",
             symbol);

    if (!hl_ws_client_send_text(ws_ext->ws_client, subscription_msg)) {
        return NULL;
    }

    return add_subscription(client, "trades", symbol, callback, user_data);
}

/**
 * @brief Watch user orders
 */
const char* hl_watch_orders(hl_client_t* client, const char* symbol,
                           hl_ws_data_callback_t callback, void* user_data) {
    if (!client || !callback) return NULL;

    hl_client_ws_extension_t* ws_ext = (hl_client_ws_extension_t*)client->ws_extension;
    if (!ws_ext) return NULL;

    // Connect if not connected
    if (!hl_ws_client_is_connected(ws_ext->ws_client)) {
        if (!hl_ws_client_connect(ws_ext->ws_client)) {
            return NULL;
        }
    }

    // Subscribe to user orders channel
    const char* user_address = hl_client_get_wallet_address_old(client);
    if (!user_address) return NULL;

    char subscription_msg[256];
    snprintf(subscription_msg, sizeof(subscription_msg),
             "{\"method\":\"subscribe\",\"subscription\":{\"type\":\"orderUpdates\",\"user\":\"%s\"}}",
             user_address);

    if (!hl_ws_client_send_text(ws_ext->ws_client, subscription_msg)) {
        return NULL;
    }

    return add_subscription(client, "orderUpdates", symbol, callback, user_data);
}

/**
 * @brief Watch user trades
 */
const char* hl_watch_my_trades(hl_client_t* client, const char* symbol,
                              hl_ws_data_callback_t callback, void* user_data) {
    if (!client || !callback) return NULL;

    hl_client_ws_extension_t* ws_ext = (hl_client_ws_extension_t*)client->ws_extension;
    if (!ws_ext) return NULL;

    // Connect if not connected
    if (!hl_ws_client_is_connected(ws_ext->ws_client)) {
        if (!hl_ws_client_connect(ws_ext->ws_client)) {
            return NULL;
        }
    }

    // Subscribe to user fills channel
    const char* user_address = hl_client_get_wallet_address_old(client);
    if (!user_address) return NULL;

    char subscription_msg[256];
    snprintf(subscription_msg, sizeof(subscription_msg),
             "{\"method\":\"subscribe\",\"subscription\":{\"type\":\"userFills\",\"user\":\"%s\"}}",
             user_address);

    if (!hl_ws_client_send_text(ws_ext->ws_client, subscription_msg)) {
        return NULL;
    }

    return add_subscription(client, "userFills", symbol, callback, user_data);
}

/**
 * @brief Unwatch subscription
 */
bool hl_unwatch(hl_client_t* client, const char* subscription_id) {
    if (!client || !subscription_id || !client->ws_extension) return false;

    hl_client_ws_extension_t* ws_ext = (hl_client_ws_extension_t*)client->ws_extension;

    // Find and deactivate subscription
    for (size_t i = 0; i < ws_ext->subscription_count; i++) {
        if (strcmp(ws_ext->subscriptions[i].subscription_id, subscription_id) == 0) {
            ws_ext->subscriptions[i].active = false;

            // Send unsubscribe message
            char unsubscribe_msg[256];
            snprintf(unsubscribe_msg, sizeof(unsubscribe_msg),
                     "{\"method\":\"unsubscribe\",\"subscription\":{\"type\":\"%s\"}}",
                     ws_ext->subscriptions[i].channel);

            hl_ws_client_send_text(ws_ext->ws_client, unsubscribe_msg);
            return true;
        }
    }

    return false;
}

/**
 * @brief Create order via WebSocket (placeholder)
 */
hl_error_t hl_create_order_ws(hl_client_t* client,
                             const hl_order_request_t* request,
                             hl_order_result_t* result) {
    // Placeholder - would implement WebSocket order creation
    // Similar to REST API but over WebSocket
    return HL_ERROR_NOT_IMPLEMENTED;
}

/**
 * @brief Cancel order via WebSocket (placeholder)
 */
hl_error_t hl_cancel_order_ws(hl_client_t* client,
                             const char* symbol,
                             const char* order_id,
                             hl_cancel_result_t* result) {
    // Placeholder - would implement WebSocket order cancellation
    return HL_ERROR_NOT_IMPLEMENTED;
}
