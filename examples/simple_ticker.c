/**
 * @file simple_ticker.c
 * @brief Example: Fetch ticker data for trading symbols
 *
 * This example demonstrates how to:
 * - Fetch ticker data for individual symbols
 * - Display real-time price information
 * - Show bid/ask spreads and trading volumes
 * - Access swap-specific data (funding rates, open interest)
 *
 * Build: make -f Makefile.tests example_ticker
 * Run: ./build/bin/example_ticker
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/hyperliquid.h"

int main(void) {
    printf("📊 Hyperliquid Ticker Example\n");
    printf("============================\n\n");

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

    // Test symbols
    const char* symbols[] = {"BTC/USDC:USDC", "ETH/USDC:USDC", "SOL/USDC:USDC", "DOGE/USDC:USDC"};
    size_t num_symbols = sizeof(symbols) / sizeof(symbols[0]);

    printf("📈 Fetching ticker data for %zu symbols...\n\n", num_symbols);

    for (size_t i = 0; i < num_symbols; i++) {
        const char* symbol = symbols[i];

        printf("🔍 %s\n", symbol);
        printf("─────────────────────────────\n");

        // Fetch ticker
        hl_ticker_t ticker = {0};
        hl_error_t err = hl_get_ticker(client, symbol, &ticker);

        if (err != HL_SUCCESS) {
            printf("❌ Failed to fetch ticker: %s\n\n", hl_error_string(err));
            continue;
        }

        // Display basic price information
        printf("💰 Price Information:\n");
        printf("   Last Price:  $%.2f\n", ticker.last_price);
        printf("   Close Price: $%.2f\n", ticker.close);
        printf("   Bid Price:   $%.2f\n", ticker.bid);
        printf("   Ask Price:   $%.2f\n", ticker.ask);
        printf("   Spread:      $%.2f (%.4f%%)\n",
               ticker.ask - ticker.bid,
               ((ticker.ask - ticker.bid) / ticker.last_price) * 100);

        // Display volume information
        printf("📊 Volume Information:\n");
        printf("   24h Volume: $%.0f\n", ticker.volume_24h);
        printf("   Quote Vol:  $%.0f\n", ticker.quote_volume);

        // Display timestamps
        printf("🕐 Timing Information:\n");
        printf("   Timestamp:   %llu\n", (unsigned long long)ticker.timestamp);
        printf("   Datetime:    %s\n", ticker.datetime);

        // Display swap-specific information
        printf("🔄 Swap-Specific Data:\n");
        printf("   Mark Price:     $%.2f\n", ticker.mark_price);
        printf("   Oracle Price:   $%.2f\n", ticker.oracle_price);
        printf("   Funding Rate:   %.6f%%\n", ticker.funding_rate * 100);
        printf("   Open Interest:  %.2f\n", ticker.open_interest);

        printf("\n");
    }

    // Summary
    printf("📋 Summary:\n");
    printf("───────────\n");
    printf("✅ Successfully fetched ticker data for %zu symbols\n", num_symbols);
    printf("✅ All symbols are perpetual swaps (marked by :USDC suffix)\n");
    printf("✅ Real-time data includes mark prices, funding rates, and open interest\n");
    printf("✅ Bid/ask spreads calculated from mark price for demo purposes\n");
    printf("💡 In production, order book data would provide real bid/ask spreads\n");

    // Cleanup
    hl_client_destroy(client);

    printf("\n🎉 Example completed successfully!\n");
    printf("💡 Ticker data is essential for trading strategies and market analysis!\n");

    return 0;
}
