/**
 * @file simple_positions.c
 * @brief Example: Fetch account positions
 *
 * This example demonstrates how to:
 * - Fetch all open positions
 * - Fetch a single position by symbol
 * - Get trading fees for a symbol
 * - Handle positions that don't exist
 *
 * Build: make -f Makefile.tests example_positions
 * Run: ./build/bin/example_positions
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/hyperliquid.h"

int main(void) {
    printf("📊 Hyperliquid Positions Example\n");
    printf("================================\n\n");

    // Create client (testnet)
    hl_client_t* client = hl_client_create(
        "0xAAF96800a2609604c64620df4B5280694E1D812d",
        "0x2ebd552aed6ceb534b27f229e2cb62799b34d6a074c2f6b8b450d44b31445af8",
        true  // testnet
    );

    if (!client) {
        printf("❌ Failed to create client\n");
        return 1;
    }

    printf("✅ Client created successfully\n\n");

    // Fetch all positions
    printf("📊 Fetching all positions...\n");
    hl_position_t* positions = NULL;
    size_t count = 0;
    hl_error_t err = hl_fetch_positions(client, &positions, &count);

    if (err != HL_SUCCESS) {
        printf("❌ Failed to fetch positions: %s\n", hl_error_string(err));
        hl_client_destroy(client);
        return 1;
    }

    printf("✅ Found %zu positions\n", count);

    if (count > 0) {
        for (size_t i = 0; i < count; i++) {
            printf("\nPosition %zu:\n", i + 1);
            printf("  Symbol: %s\n", positions[i].symbol);
            printf("  Coin: %s\n", positions[i].coin);
            printf("  Side: %s\n", positions[i].side == HL_POSITION_LONG ? "LONG" : "SHORT");
            printf("  Size: %.6f\n", positions[i].size);
            printf("  Entry Price: $%.2f\n", positions[i].entry_price);
            printf("  Current Value: $%.2f\n", positions[i].position_value);
            printf("  Unrealized PnL: $%.6f\n", positions[i].unrealized_pnl);
            printf("  Leverage: %dx\n", positions[i].leverage);
            printf("  Margin Mode: %s\n", positions[i].is_isolated ? "Isolated" : "Cross");
            printf("  Liquidation Price: $%.2f\n", positions[i].liquidation_price);
        }
    } else {
        printf("ℹ️  No open positions (normal for test accounts)\n");
    }

    printf("\n");

    // Test fetching single position
    printf("🎯 Testing single position fetch...\n");
    const char* test_symbols[] = {"BTC/USDC:USDC", "ETH/USDC:USDC"};

    for (size_t i = 0; i < sizeof(test_symbols) / sizeof(test_symbols[0]); i++) {
        printf("  Looking for %s...\n", test_symbols[i]);

        hl_position_t position = {0};
        err = hl_fetch_position(client, test_symbols[i], &position);

        if (err == HL_SUCCESS) {
            printf("    ✅ Found position!\n");
            printf("      Size: %.6f %s\n", position.size,
                   position.side == HL_POSITION_LONG ? "LONG" : "SHORT");
            printf("      Entry: $%.2f\n", position.entry_price);
            printf("      PnL: $%.6f\n", position.unrealized_pnl);
        } else if (err == HL_ERROR_NOT_FOUND) {
            printf("    ℹ️  No position found\n");
        } else {
            printf("    ❌ Error: %s\n", hl_error_string(err));
        }
    }

    printf("\n");

    // Get trading fees
    printf("💰 Fetching trading fees...\n");
    hl_trading_fee_t fee = {0};
    err = hl_fetch_trading_fee(client, "BTC/USDC:USDC", &fee);

    if (err != HL_SUCCESS) {
        printf("❌ Failed to fetch trading fees: %s\n", hl_error_string(err));
    } else {
        printf("✅ BTC/USDC:USDC fees:\n");
        printf("   Maker Fee: %.4f%% %s\n", fee.maker_fee * 100,
               fee.maker_fee < 0 ? "(rebate)" : "");
        printf("   Taker Fee: %.4f%%\n", fee.taker_fee * 100);
    }

    // Cleanup
    if (positions) {
        hl_free_positions(positions, count);
    }
    hl_client_destroy(client);

    printf("\n🎉 Example completed successfully!\n");

    return 0;
}
