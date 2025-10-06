/**
 * @file trading_bot.c
 * @brief Complete Hyperliquid trading bot example
 *
 * This example demonstrates:
 * - Client initialization and connection testing
 * - Account balance monitoring
 * - Market data fetching
 * - Order placement and management
 * - WebSocket real-time data streaming
 * - Error handling and cleanup
 */

#include "hyperliquid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

// Global variables for signal handling
volatile sig_atomic_t running = 1;
hl_client_t* g_client = NULL;
char* g_active_order_id = NULL;

/**
 * @brief Signal handler for graceful shutdown
 */
void signal_handler(int signum) {
    printf("\n🛑 Received signal %d, shutting down...\n", signum);
    running = 0;
}

/**
 * @brief WebSocket ticker callback
 */
void on_ticker_update(void* data, void* user_data) {
    static int update_count = 0;
    update_count++;
    printf("📊 Ticker update #%d\n", update_count);

    // In real implementation, parse the ticker data
    // hl_ticker_t* ticker = (hl_ticker_t*)data;
}

/**
 * @brief WebSocket order callback
 */
void on_order_update(void* data, void* user_data) {
    printf("📋 Order update received\n");
    // Handle order status changes
}

/**
 * @brief Display account balance
 */
void display_balance(hl_client_t* client) {
    printf("\n💰 Account Balance:\n");

    hl_balances_t balances = {0};
    hl_error_t err = hl_fetch_balance(client, &balances);

    if (err == HL_SUCCESS) {
        printf("┌────────────┬────────────┬────────────┬────────────┐\n");
        printf("│ Asset      │ Free       │ Used       │ Total      │\n");
        printf("├────────────┼────────────┼────────────┼────────────┤\n");

        for (size_t i = 0; i < balances.count; i++) {
            hl_balance_t* bal = &balances.balances[i];
            printf("│ %-10s │ %10.6f │ %10.6f │ %10.6f │\n",
                   bal->asset, bal->free, bal->used, bal->total);
        }

        printf("└────────────┴────────────┴────────────┴────────────┘\n");
        hl_free_balances(&balances);
    } else {
        printf("❌ Failed to fetch balance: %d\n", err);
    }
}

/**
 * @brief Display market information
 */
void display_markets(hl_client_t* client) {
    printf("\n📈 Available Markets:\n");

    hl_markets_t markets = {0};
    hl_error_t err = hl_fetch_markets(client, &markets);

    if (err == HL_SUCCESS) {
        printf("Found %zu markets:\n", markets.count);

        // Display first 10 markets
        for (size_t i = 0; i < markets.count && i < 10; i++) {
            hl_market_t* market = &markets.markets[i];
            printf("  • %s (%s)\n", market->symbol, market->type);
        }

        if (markets.count > 10) {
            printf("  ... and %zu more\n", markets.count - 10);
        }

        hl_markets_free(&markets);
    } else {
        printf("❌ Failed to fetch markets: %d\n", err);
    }
}

/**
 * @brief Display current ticker for a symbol
 */
void display_ticker(hl_client_t* client, const char* symbol) {
    printf("\n📊 Current %s Ticker:\n", symbol);

    hl_ticker_t ticker = {0};
    hl_error_t err = hl_fetch_ticker(client, symbol, &ticker);

    if (err == HL_SUCCESS) {
        printf("Price: $%.2f\n", ticker.last_price);
        printf("Bid:   $%.2f\n", ticker.bid);
        printf("Ask:   $%.2f\n", ticker.ask);
        printf("Volume: %.4f\n", ticker.volume);
    } else {
        printf("❌ Failed to fetch ticker: %d\n", err);
    }
}

/**
 * @brief Place a test limit order
 */
void place_test_order(hl_client_t* client) {
    printf("\n🎯 Placing Test Order:\n");

    // Get current ticker price first
    hl_ticker_t ticker = {0};
    hl_error_t ticker_err = hl_fetch_ticker(client, "BTC/USDC:USDC", &ticker);

    if (ticker_err != HL_SUCCESS) {
        printf("❌ Cannot place order: failed to get ticker\n");
        return;
    }

    // Place limit order 5% below current price (buy)
    double order_price = ticker.last_price * 0.95;

    hl_order_request_t request = {
        .symbol = "BTC/USDC:USDC",
        .type = "limit",
        .side = "buy",
        .amount = "0.001",  // Small amount for testing
        .price = NULL       // Will set below
    };

    // Convert price to string
    char price_str[32];
    snprintf(price_str, sizeof(price_str), "%.2f", order_price);
    request.price = price_str;

    printf("Symbol: %s\n", request.symbol);
    printf("Side: %s\n", request.side);
    printf("Type: %s\n", request.type);
    printf("Amount: %s\n", request.amount);
    printf("Price: $%s (5%% below market)\n", request.price);

    hl_order_result_t result = {0};
    hl_error_t err = hl_create_order(client, &request, &result);

    if (err == HL_SUCCESS) {
        printf("✅ Order placed successfully!\n");
        printf("Order ID: %s\n", result.order_id);
        printf("Status: %s\n", result.status);

        // Store order ID for potential cancellation
        g_active_order_id = strdup(result.order_id);
        free(result.order_id);
    } else {
        printf("❌ Failed to place order: %d\n", err);
    }
}

/**
 * @brief Cancel active order if exists
 */
void cancel_active_order(hl_client_t* client) {
    if (!g_active_order_id) {
        printf("\n❌ No active order to cancel\n");
        return;
    }

    printf("\n🔄 Cancelling Order: %s\n", g_active_order_id);

    hl_cancel_result_t result = {0};
    hl_error_t err = hl_cancel_order(client, "BTC/USDC:USDC", g_active_order_id, &result);

    if (err == HL_SUCCESS) {
        printf("✅ Order cancelled successfully!\n");
        printf("Status: %s\n", result.status);
    } else {
        printf("❌ Failed to cancel order: %d\n", err);
    }

    free(g_active_order_id);
    g_active_order_id = NULL;
}

/**
 * @brief Display open orders
 */
void display_open_orders(hl_client_t* client) {
    printf("\n📋 Open Orders:\n");

    hl_orders_t orders = {0};
    hl_error_t err = hl_fetch_open_orders(client, NULL, NULL, 10, &orders);

    if (err == HL_SUCCESS) {
        if (orders.count == 0) {
            printf("No open orders\n");
        } else {
            printf("Found %zu open orders:\n", orders.count);

            for (size_t i = 0; i < orders.count; i++) {
                hl_order_t* order = &orders.orders[i];
                printf("  • %s: %s %s %.6f @ $%.2f (%s)\n",
                       order->id, order->side, order->symbol,
                       order->amount, order->price, order->status);
            }
        }

        hl_free_orders(&orders);
    } else {
        printf("❌ Failed to fetch open orders: %d\n", err);
    }
}

/**
 * @brief Setup WebSocket subscriptions
 */
void setup_websocket(hl_client_t* client) {
    printf("\n🔌 Setting up WebSocket subscriptions...\n");

    // Initialize WebSocket
    if (!hl_ws_init_client(client, true)) {
        printf("❌ Failed to initialize WebSocket\n");
        return;
    }

    printf("✅ WebSocket initialized\n");

    // Subscribe to BTC ticker
    const char* ticker_sub = hl_watch_ticker(client, "BTC/USDC:USDC",
                                           on_ticker_update, NULL);
    if (ticker_sub) {
        printf("✅ Subscribed to BTC/USDC:USDC ticker: %s\n", ticker_sub);
    }

    // Subscribe to order updates
    const char* order_sub = hl_watch_orders(client, NULL, on_order_update, NULL);
    if (order_sub) {
        printf("✅ Subscribed to order updates: %s\n", order_sub);
    }
}

/**
 * @brief Main bot loop
 */
void bot_loop(hl_client_t* client) {
    int cycle = 0;

    printf("\n🤖 Starting trading bot loop...\n");
    printf("Press Ctrl+C to stop\n\n");

    while (running) {
        cycle++;

        printf("=== Cycle %d ===\n", cycle);

        // Display balance every 10 cycles
        if (cycle % 10 == 1) {
            display_balance(client);
        }

        // Display open orders
        display_open_orders(client);

        // Display current BTC price
        display_ticker(client, "BTC/USDC:USDC");

        printf("⏳ Waiting 10 seconds...\n\n");
        sleep(10);
    }
}

int main(int argc, char* argv[]) {
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("🚀 Hyperliquid C SDK Trading Bot Demo\n");
    printf("=====================================\n\n");

    // Create client
    printf("🔧 Creating client...\n");
    g_client = hl_client_create(NULL);
    if (!g_client) {
        printf("❌ Failed to create client\n");
        return 1;
    }

    // Test connection
    printf("🔗 Testing connection...\n");
    if (hl_test_connection(g_client) != HL_SUCCESS) {
        printf("❌ Connection test failed\n");
        hl_client_destroy(g_client);
        return 1;
    }
    printf("✅ Connected to Hyperliquid API\n\n");

    // Display initial information
    display_balance(g_client);
    display_markets(g_client);
    display_ticker(g_client, "BTC/USDC:USDC");

    // Interactive menu
    printf("\n🎮 Trading Bot Menu:\n");
    printf("1. Place test limit order\n");
    printf("2. Cancel active order\n");
    printf("3. Start WebSocket streaming\n");
    printf("4. Start automated bot loop\n");
    printf("5. Exit\n");
    printf("\nChoose an option (1-5): ");

    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("❌ Invalid input\n");
        hl_client_destroy(g_client);
        return 1;
    }

    switch (choice) {
        case 1:
            place_test_order(g_client);
            display_open_orders(g_client);
            break;

        case 2:
            cancel_active_order(g_client);
            break;

        case 3:
            setup_websocket(g_client);
            printf("\n🎯 WebSocket streaming active!\n");
            printf("Press Ctrl+C to stop...\n");

            // Simple event loop
            while (running) {
                sleep(1);
            }

            hl_ws_cleanup_client(g_client);
            break;

        case 4:
            bot_loop(g_client);
            break;

        case 5:
        default:
            printf("👋 Goodbye!\n");
            break;
    }

    // Cleanup
    if (g_active_order_id) {
        free(g_active_order_id);
    }

    hl_client_destroy(g_client);
    printf("✅ Cleanup complete\n");

    return 0;
}
