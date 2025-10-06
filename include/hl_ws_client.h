/**
 * @file hl_ws_client.h
 * @brief WebSocket client for real-time Hyperliquid data
 */

#ifndef HL_WS_CLIENT_H
#define HL_WS_CLIENT_H

#include <stdbool.h>
#include <stdint.h>

// Forward declarations
typedef struct hl_ws_client hl_ws_client_t;
typedef struct hl_ws_config hl_ws_config_t;
typedef void (*hl_ws_message_callback_t)(const char* message, size_t size, void* user_data);
typedef void (*hl_ws_error_callback_t)(const char* error, void* user_data);
typedef void (*hl_ws_connect_callback_t)(void* user_data);

/**
 * @brief WebSocket client configuration
 */
struct hl_ws_config {
    const char* url;                    /**< WebSocket URL */
    int reconnect_delay_ms;             /**< Delay between reconnection attempts */
    int ping_interval_ms;               /**< Ping interval */
    int timeout_ms;                     /**< Connection timeout */
    bool auto_reconnect;                /**< Auto reconnect on disconnect */
    int max_reconnect_attempts;         /**< Maximum reconnection attempts */
};

/**
 * @brief WebSocket client instance
 */
struct hl_ws_client {
    void* internal;                     /**< Internal WebSocket implementation */
    hl_ws_config_t config;              /**< Client configuration */

    // Callbacks
    hl_ws_message_callback_t on_message;
    hl_ws_error_callback_t on_error;
    hl_ws_connect_callback_t on_connect;
    void* user_data;                    /**< User data for callbacks */

    // State
    bool connected;                     /**< Connection status */
    bool running;                       /**< Client running status */
};

/**
 * @brief Create WebSocket client
 * @param config Client configuration
 * @return WebSocket client instance or NULL on error
 */
hl_ws_client_t* hl_ws_client_create(const hl_ws_config_t* config);

/**
 * @brief Destroy WebSocket client
 * @param client Client instance
 */
void hl_ws_client_destroy(hl_ws_client_t* client);

/**
 * @brief Connect to WebSocket server
 * @param client Client instance
 * @return true on success
 */
bool hl_ws_client_connect(hl_ws_client_t* client);

/**
 * @brief Disconnect from WebSocket server
 * @param client Client instance
 */
void hl_ws_client_disconnect(hl_ws_client_t* client);

/**
 * @brief Send message to WebSocket server
 * @param client Client instance
 * @param message Message to send
 * @param size Message size
 * @return true on success
 */
bool hl_ws_client_send(hl_ws_client_t* client, const char* message, size_t size);

/**
 * @brief Send text message to WebSocket server
 * @param client Client instance
 * @param message Null-terminated string
 * @return true on success
 */
bool hl_ws_client_send_text(hl_ws_client_t* client, const char* message);

/**
 * @brief Check if client is connected
 * @param client Client instance
 * @return true if connected
 */
bool hl_ws_client_is_connected(const hl_ws_client_t* client);

/**
 * @brief Set message callback
 * @param client Client instance
 * @param callback Message callback
 * @param user_data User data for callback
 */
void hl_ws_client_set_message_callback(hl_ws_client_t* client,
                                      hl_ws_message_callback_t callback,
                                      void* user_data);

/**
 * @brief Set error callback
 * @param client Client instance
 * @param callback Error callback
 * @param user_data User data for callback
 */
void hl_ws_client_set_error_callback(hl_ws_client_t* client,
                                    hl_ws_error_callback_t callback,
                                    void* user_data);

/**
 * @brief Set connect callback
 * @param client Client instance
 * @param callback Connect callback
 * @param user_data User data for callback
 */
void hl_ws_client_set_connect_callback(hl_ws_client_t* client,
                                      hl_ws_connect_callback_t callback,
                                      void* user_data);

/**
 * @brief Get default WebSocket configuration
 * @param config Output configuration
 * @param testnet Use testnet
 */
void hl_ws_config_default(hl_ws_config_t* config, bool testnet);

#endif // HL_WS_CLIENT_H
