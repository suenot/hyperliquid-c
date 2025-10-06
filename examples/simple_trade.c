/**
 * @file simple_trade.c
 * @brief Simple trading example for Hyperliquid C SDK
 * 
 * Demonstrates:
 * - Client initialization
 * - Connection testing
 * - Balance querying
 * - Order placement
 * - Order cancellation
 */

#include "hyperliquid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    printf("=== Hyperliquid C SDK - Simple Trading Example ===\n\n");
    
    // Get credentials from environment
    const char *wallet_address = getenv("HYPERLIQUID_WALLET_ADDRESS");
    const char *private_key = getenv("HYPERLIQUID_PRIVATE_KEY");
    const char *testnet_env = getenv("HYPERLIQUID_TESTNET");
    
    if (!wallet_address || !private_key) {
        fprintf(stderr, "Error: Set HYPERLIQUID_WALLET_ADDRESS and HYPERLIQUID_PRIVATE_KEY\n");
        fprintf(stderr, "Example:\n");
        fprintf(stderr, "  export HYPERLIQUID_WALLET_ADDRESS=\"0x...\"\n");
        fprintf(stderr, "  export HYPERLIQUID_PRIVATE_KEY=\"...\"\n");
        fprintf(stderr, "  export HYPERLIQUID_TESTNET=\"true\"  # optional, default true\n");
        return 1;
    }
    
    bool testnet = true;
    if (testnet_env && (strcmp(testnet_env, "false") == 0 || strcmp(testnet_env, "0") == 0)) {
        testnet = false;
    }
    
    printf("Network: %s\n", testnet ? "TESTNET" : "MAINNET");
    printf("Wallet: %s\n\n", wallet_address);
    
    // Create client
    hl_client_t *client = hl_client_create(wallet_address, private_key, testnet);
    if (!client) {
        fprintf(stderr, "Failed to create client\n");
        return 1;
    }
    printf("✅ Client created\n");
    
    // Test connection
    printf("Testing connection...\n");
    if (!hl_test_connection(client)) {
        fprintf(stderr, "❌ Connection failed\n");
        hl_client_destroy(client);
        return 1;
    }
    printf("✅ Connection OK\n\n");
    
    // Get balance
    printf("Fetching account balance...\n");
    hl_balance_t balance;
    if (hl_get_balance(client, &balance) == HL_SUCCESS) {
        printf("✅ Balance retrieved:\n");
        printf("   Account Value: %.2f USDC\n", balance.account_value);
        printf("   Withdrawable:  %.2f USDC\n", balance.withdrawable);
        printf("   Margin Used:   %.2f USDC\n", balance.total_margin_used);
        printf("   Total Pos:     %.2f USDC\n\n", balance.total_ntl_pos);
    } else {
        fprintf(stderr, "⚠️  Failed to get balance\n\n");
    }
    
    // Get current BTC price
    printf("Fetching BTC market price...\n");
    double market_price = 0.0;
    if (hl_get_market_price(client, "BTC", &market_price) == HL_SUCCESS) {
        printf("✅ BTC price: $%.2f\n\n", market_price);
    } else {
        fprintf(stderr, "⚠️  Failed to get market price\n\n");
        market_price = 95000.0; // fallback
    }
    
    // Place a limit order 20% below market (safe limit order)
    double order_price = market_price * 0.80;
    double order_quantity = 0.001;
    
    printf("Placing BUY limit order:\n");
    printf("   Symbol:   BTC\n");
    printf("   Price:    $%.2f (20%% below market)\n", order_price);
    printf("   Quantity: %.4f BTC\n", order_quantity);
    printf("   Value:    $%.2f\n", order_price * order_quantity);
    
    hl_order_request_t order = {
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .price = order_price,
        .quantity = order_quantity,
        .order_type = HL_ORDER_TYPE_LIMIT,
        .time_in_force = HL_TIF_GTC,
        .reduce_only = false,
        .slippage_bps = 0
    };
    
    hl_order_result_t result;
    int ret = hl_place_order(client, &order, &result);
    
    if (ret == HL_SUCCESS) {
        printf("✅ Order placed successfully!\n");
        printf("   Order ID: %llu\n", result.order_id);
        printf("   Status:   %s\n", 
               result.status == HL_ORDER_STATUS_OPEN ? "OPEN" : "FILLED");
        
        if (result.filled_quantity > 0) {
            printf("   Filled:   %.4f BTC @ $%.2f\n", 
                   result.filled_quantity, result.avg_fill_price);
        }
        
        // If order is open, cancel it
        if (result.status == HL_ORDER_STATUS_OPEN && result.order_id > 0) {
            printf("\nCanceling order %llu...\n", result.order_id);
            
            if (hl_cancel_order(client, "BTC", result.order_id) == HL_SUCCESS) {
                printf("✅ Order canceled successfully\n");
            } else {
                fprintf(stderr, "⚠️  Failed to cancel order\n");
            }
        }
    } else {
        fprintf(stderr, "❌ Failed to place order: %s\n", hl_error_string(ret));
        if (strlen(result.error) > 0) {
            fprintf(stderr, "   API Error: %s\n", result.error);
        }
    }
    
    // Get open positions
    printf("\nFetching open positions...\n");
    hl_position_t *positions = NULL;
    size_t pos_count = 0;
    if (hl_get_positions(client, &positions, &pos_count) == HL_SUCCESS) {
        if (pos_count > 0) {
            printf("✅ Open positions (%zu):\n", pos_count);
            for (size_t i = 0; i < pos_count; i++) {
                printf("   %s: %.4f @ $%.2f (PnL: $%.2f)\n",
                       positions[i].symbol,
                       positions[i].size,
                       positions[i].entry_price,
                       positions[i].unrealized_pnl);
            }
            hl_free_positions(positions);
        } else {
            printf("   No open positions\n");
        }
    } else {
        fprintf(stderr, "⚠️  Failed to get positions\n");
    }
    
    // Cleanup
    printf("\n");
    hl_client_destroy(client);
    printf("✅ Client destroyed\n");
    
    printf("\n=== Example Complete ===\n");
    return 0;
}

