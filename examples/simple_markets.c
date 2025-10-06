/**
 * @file simple_markets.c
 * @brief Example: Fetch all available markets
 *
 * This example demonstrates how to:
 * - Fetch all available markets (swaps + spots)
 * - Get asset ID by symbol
 * - Lookup market information
 * - Display market details
 *
 * Build: make -f Makefile.tests example_markets
 * Run: ./build/bin/example_markets
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/hyperliquid.h"

int main(void) {
    printf("📊 Hyperliquid Markets Example\n");
    printf("=============================\n\n");

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

    // Fetch all markets
    printf("📊 Fetching all markets...\n");
    hl_markets_t markets = {0};
    hl_error_t err = hl_fetch_markets(client, &markets);

    if (err != HL_SUCCESS) {
        printf("❌ Failed to fetch markets: %s\n", hl_error_string(err));
        hl_client_destroy(client);
        return 1;
    }

    printf("✅ Found %zu markets\n\n", markets.count);

    // Count market types
    size_t swap_count = 0, spot_count = 0;
    for (size_t i = 0; i < markets.count; i++) {
        if (markets.markets[i].type == HL_MARKET_SWAP) {
            swap_count++;
        } else {
            spot_count++;
        }
    }

    printf("📈 Market Summary:\n");
    printf("   Swap markets: %zu\n", swap_count);
    printf("   Spot markets: %zu\n", spot_count);
    printf("   Total: %zu\n\n", markets.count);

    // Display first 10 swap markets
    printf("🔄 First 10 Swap Markets:\n");
    printf("────────────────────────────────────────────────────────────────\n");
    printf("%-4s %-15s %-6s %-8s %-10s\n", "ID", "Symbol", "Base", "Type", "MaxLev");
    printf("────────────────────────────────────────────────────────────────\n");

    size_t displayed = 0;
    for (size_t i = 0; i < markets.count && displayed < 10; i++) {
        const hl_market_t* market = &markets.markets[i];
        if (market->type == HL_MARKET_SWAP) {
            printf("%-4u %-15s %-6s %-8s %-10d\n",
                   market->asset_id,
                   market->symbol,
                   market->base,
                   "SWAP",
                   market->max_leverage);
            displayed++;
        }
    }
    printf("\n");

    // Display spot markets
    if (spot_count > 0) {
        printf("💰 Spot Markets:\n");
        printf("────────────────────────────────────────────────────────────────\n");
        printf("%-4s %-15s %-6s %-8s\n", "ID", "Symbol", "Base", "Type");
        printf("────────────────────────────────────────────────────────────────\n");

        for (size_t i = 0; i < markets.count; i++) {
            const hl_market_t* market = &markets.markets[i];
            if (market->type == HL_MARKET_SPOT) {
                printf("%-4u %-15s %-6s %-8s\n",
                       market->asset_id,
                       market->symbol,
                       market->base,
                       "SPOT");
            }
        }
        printf("\n");
    }

    // Test asset ID lookup
    printf("🔍 Asset ID Lookup Tests:\n");
    const char* test_symbols[] = {"BTC/USDC:USDC", "ETH/USDC:USDC", "SOL/USDC:USDC", "PURR/USDC"};

    for (size_t i = 0; i < sizeof(test_symbols) / sizeof(test_symbols[0]); i++) {
        uint32_t asset_id = 0;
        err = hl_get_asset_id(&markets, test_symbols[i], &asset_id);

        if (err == HL_SUCCESS) {
            printf("   ✅ %s -> Asset ID: %u\n", test_symbols[i], asset_id);
        } else {
            printf("   ❌ %s -> Not found\n", test_symbols[i]);
        }
    }
    printf("\n");

    // Test market lookup
    printf("📋 Market Lookup Test:\n");
    const hl_market_t* market = NULL;
    err = hl_get_market(&markets, "ETH/USDC:USDC", &market);

    if (err == HL_SUCCESS && market) {
        printf("   ✅ ETH/USDC:USDC found:\n");
        printf("      Asset ID: %u\n", market->asset_id);
        printf("      Base: %s\n", market->base);
        printf("      Quote: %s\n", market->quote);
        printf("      Type: %s\n", market->type == HL_MARKET_SWAP ? "SWAP" : "SPOT");
        printf("      Max Leverage: %d\n", market->max_leverage);
        printf("      Amount Precision: %d\n", market->amount_precision);
        printf("      Price Precision: %d\n", market->price_precision);
        printf("      Min Cost: %.2f\n", market->min_cost);
        if (market->mark_price > 0) {
            printf("      Mark Price: %.4f\n", market->mark_price);
        }
        if (market->oracle_price > 0) {
            printf("      Oracle Price: %.4f\n", market->oracle_price);
        }
    } else {
        printf("   ❌ ETH/USDC:USDC not found\n");
    }

    // Cleanup
    hl_markets_free(&markets);
    hl_client_destroy(client);

    printf("\n🎉 Example completed successfully!\n");
    printf("💡 Now you can use asset IDs for trading operations!\n");

    return 0;
}
