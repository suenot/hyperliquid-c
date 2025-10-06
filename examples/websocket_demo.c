/**
 * @file websocket_demo.c
 * @brief WebSocket API demonstration
 */

#include "hyperliquid.h"
#include "hl_ws_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Global client for demo
hl_client_t* g_client = NULL;

/**
 * @brief Ticker update callback
 */
void on_ticker_update(void* data, void* user_data) {
    printf("📊 TICKER UPDATE: Received market data\n");
    // In real implementation, data would be parsed ticker structure
}

/**
 * @brief Order book update callback
 */
void on_orderbook_update(void* data, void* user_data) {
    printf("📈 ORDER BOOK UPDATE: Bids/Asks changed\n");
    // In real implementation, data would be parsed orderbook structure
}

/**
 * @brief User order update callback
 */
void on_order_update(void* data, void* user_data) {
    printf("📋 ORDER UPDATE: Your order status changed\n");
    // In real implementation, data would be parsed order structure
}

/**
 * @brief User trade update callback
 */
void on_trade_update(void* data, void* user_data) {
    printf("💰 TRADE UPDATE: New trade executed\n");
    // In real implementation, data would be parsed trade structure
}

/**
 * @brief Demo function showing WebSocket subscriptions
 */
void demo_websocket_subscriptions(void) {
    printf("🚀 Starting WebSocket Demo...\n\n");

    // Create client
    g_client = hl_client_create(NULL);
    if (!g_client) {
        printf("❌ Failed to create client\n");
        return;
    }

    printf("✅ Client created\n");

    // Initialize WebSocket (testnet)
    if (!hl_ws_init_client(g_client, true)) {
        printf("❌ Failed to initialize WebSocket\n");
        hl_client_destroy(g_client);
        return;
    }

    printf("✅ WebSocket initialized (testnet)\n\n");

    // Subscribe to ticker updates
    printf("📊 Subscribing to BTC/USDC:USDC ticker...\n");
    const char* ticker_sub = hl_watch_ticker(g_client, "BTC/USDC:USDC", on_ticker_update, NULL);
    if (ticker_sub) {
        printf("✅ Ticker subscription created: %s\n", ticker_sub);
    } else {
        printf("❌ Ticker subscription failed\n");
    }

    // Subscribe to order book updates
    printf("📈 Subscribing to ETH/USDC:USDC order book...\n");
    const char* ob_sub = hl_watch_order_book(g_client, "ETH/USDC:USDC", 20, on_orderbook_update, NULL);
    if (ob_sub) {
        printf("✅ Order book subscription created: %s\n", ob_sub);
    } else {
        printf("❌ Order book subscription failed\n");
    }

    // Subscribe to user orders (requires authentication)
    printf("📋 Subscribing to user order updates...\n");
    const char* order_sub = hl_watch_orders(g_client, NULL, on_order_update, NULL);
    if (order_sub) {
        printf("✅ Order subscription created: %s\n", order_sub);
    } else {
        printf("❌ Order subscription failed\n");
    }

    // Subscribe to user trades (requires authentication)
    printf("💰 Subscribing to user trade updates...\n");
    const char* trade_sub = hl_watch_my_trades(g_client, NULL, on_trade_update, NULL);
    if (trade_sub) {
        printf("✅ Trade subscription created: %s\n", trade_sub);
    } else {
        printf("❌ Trade subscription failed\n");
    }

    printf("\n🎯 Active Subscriptions:\n");
    printf("   • %s: BTC/USDC:USDC ticker\n", ticker_sub ? ticker_sub : "NONE");
    printf("   • %s: ETH/USDC:USDC order book\n", ob_sub ? ob_sub : "NONE");
    printf("   • %s: User orders\n", order_sub ? order_sub : "NONE");
    printf("   • %s: User trades\n", trade_sub ? trade_sub : "NONE");

    // Simulate running for a few seconds (in real app, this would be event loop)
    printf("\n⏳ Listening for updates (5 seconds)...\n");
    sleep(5);

    // Unsubscribe from ticker
    if (ticker_sub) {
        printf("🔇 Unsubscribing from ticker...\n");
        if (hl_unwatch(g_client, ticker_sub)) {
            printf("✅ Successfully unsubscribed from ticker\n");
        }
    }

    printf("\n🧹 Cleaning up...\n");
    hl_ws_cleanup_client(g_client);
    hl_client_destroy(g_client);

    printf("✅ Demo completed!\n");
}

/**
 * @brief Demo showing WebSocket client standalone usage
 */
void demo_ws_client_standalone(void) {
    printf("🔧 WebSocket Client Standalone Demo...\n\n");

    // Create WebSocket client
    hl_ws_config_t config;
    hl_ws_config_default(&config, true); // testnet

    printf("🌐 Connecting to: %s\n", config.url);

    hl_ws_client_t* ws = hl_ws_client_create(&config);
    if (!ws) {
        printf("❌ Failed to create WebSocket client\n");
        return;
    }

    printf("✅ WebSocket client created\n");

    // Set callbacks
    hl_ws_client_set_message_callback(ws, NULL, NULL); // No callbacks for demo
    hl_ws_client_set_error_callback(ws, NULL, NULL);

    // Try to connect (will fail in framework mode, but shows API)
    printf("🔌 Attempting connection...\n");
    if (hl_ws_client_connect(ws)) {
        printf("✅ Connection successful\n");

        // Send a test message
        const char* test_msg = "{\"method\":\"ping\"}";
        if (hl_ws_client_send_text(ws, test_msg)) {
            printf("📤 Test message sent\n");
        }

        // Disconnect
        hl_ws_client_disconnect(ws);
        printf("🔌 Disconnected\n");
    } else {
        printf("⚠️  Connection failed (expected in framework mode)\n");
    }

    hl_ws_client_destroy(ws);
    printf("✅ Standalone demo completed!\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║         HYPERLIQUID C SDK - WEBSOCKET DEMO                 ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    // Demo 1: Client with WebSocket subscriptions
    demo_websocket_subscriptions();

    printf("\n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("\n");

    // Demo 2: Standalone WebSocket client
    demo_ws_client_standalone();

    printf("\n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("🎉 WebSocket Framework Demo Complete!\n");
    printf("📊 This demonstrates the subscription-based real-time API\n");
    printf("🔧 For production use, integrate with a real WebSocket library\n");
    printf("════════════════════════════════════════════════════════════════\n");

    return 0;
}
