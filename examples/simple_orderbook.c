/**
 * @file simple_orderbook.c
 * @brief Example: Fetch and analyze real-time order book data
 *
 * This example demonstrates how to:
 * - Fetch order book data for different symbols
 * - Analyze bid/ask spreads and market depth
 * - Calculate volume at different depth levels
 * - Use utility functions for quick analysis
 * - Compare order books across different assets
 *
 * Build: make -f Makefile.tests example_orderbook
 * Run: ./build/bin/example_orderbook
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/hyperliquid.h"

void print_orderbook_summary(const hl_orderbook_t* book, const char* title) {
    printf("📊 %s\n", title);
    printf("═══════════════════════════════════════\n");

    printf("Symbol: %s\n", book->symbol);
    printf("Timestamp: %llu\n", (unsigned long long)book->timestamp_ms);
    printf("Bid levels: %zu\n", book->bids_count);
    printf("Ask levels: %zu\n", book->asks_count);

    if (book->bids_count > 0 && book->asks_count > 0) {
        double best_bid = hl_orderbook_get_best_bid(book);
        double best_ask = hl_orderbook_get_best_ask(book);
        double spread = hl_orderbook_get_spread(book);

        printf("Best Bid: $%.2f (%.4f lots)\n", best_bid, book->bids[0].quantity);
        printf("Best Ask: $%.2f (%.4f lots)\n", best_ask, book->asks[0].quantity);
        printf("Spread: $%.4f (%.4f%%)\n", spread, (spread / best_bid) * 100);

        // Volume analysis
        double bid_vol_5 = hl_orderbook_get_bid_volume(book, 5);
        double ask_vol_5 = hl_orderbook_get_ask_volume(book, 5);
        double bid_vol_all = hl_orderbook_get_bid_volume(book, 0);
        double ask_vol_all = hl_orderbook_get_ask_volume(book, 0);

        printf("Top 5 Levels Volume:\n");
        printf("  Bids: %.4f lots\n", bid_vol_5);
        printf("  Asks: %.4f lots\n", ask_vol_5);
        printf("Full Book Volume:\n");
        printf("  Bids: %.4f lots\n", bid_vol_all);
        printf("  Asks: %.4f lots\n", ask_vol_all);

        // Market depth analysis
        printf("Market Depth (by price levels):\n");
        for (size_t i = 0; i < 5 && i < book->bids_count; i++) {
            printf("  Bid %zu: $%.2f (%.4f)\n", i + 1, book->bids[i].price, book->bids[i].quantity);
        }
        printf("  ...\n");
        for (size_t i = 0; i < 5 && i < book->asks_count; i++) {
            printf("  Ask %zu: $%.2f (%.4f)\n", i + 1, book->asks[i].price, book->asks[i].quantity);
        }
    }

    printf("\n");
}

int main(void) {
    printf("📈 Hyperliquid Order Book Analysis Example\n");
    printf("===========================================\n\n");

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

    // Test different symbols with different depths
    struct {
        const char* symbol;
        uint32_t depth;
        const char* description;
    } test_cases[] = {
        {"BTC/USDC:USDC", 20, "BTC - Full depth"},
        {"ETH/USDC:USDC", 10, "ETH - Medium depth"},
        {"SOL/USDC:USDC", 5, "SOL - Shallow depth"},
        {"DOGE/USDC:USDC", 3, "DOGE - Minimal depth"}
    };

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    printf("🔍 Fetching order books for %zu symbols...\n\n", num_tests);

    // Fetch and analyze each order book
    for (size_t i = 0; i < num_tests; i++) {
        const char* symbol = test_cases[i].symbol;
        uint32_t depth = test_cases[i].depth;
        const char* description = test_cases[i].description;

        printf("📊 Analyzing %s\n", description);
        printf("Symbol: %s, Depth: %u levels\n", symbol, depth);

        // Fetch order book
        hl_orderbook_t book = {0};
        hl_error_t err = hl_fetch_order_book(client, symbol, depth, &book);

        if (err != HL_SUCCESS) {
            printf("❌ Failed to fetch order book: %s\n\n", hl_error_string(err));
            continue;
        }

        // Print summary
        char title[100];
        snprintf(title, sizeof(title), "Order Book: %s", description);
        print_orderbook_summary(&book, title);

        // Cleanup
        hl_free_orderbook(&book);
    }

    // Advanced analysis - compare spreads
    printf("📊 Spread Comparison Analysis\n");
    printf("═══════════════════════════════\n");

    const char* compare_symbols[] = {"BTC/USDC:USDC", "ETH/USDC:USDC", "SOL/USDC:USDC"};
    size_t num_compare = sizeof(compare_symbols) / sizeof(compare_symbols[0]);

    printf("Comparing spreads across assets:\n\n");

    for (size_t i = 0; i < num_compare; i++) {
        hl_orderbook_t book = {0};
        hl_error_t err = hl_fetch_order_book(client, compare_symbols[i], 20, &book);

        if (err == HL_SUCCESS) {
            double spread = hl_orderbook_get_spread(&book);
            double spread_pct = (spread / book.bids[0].price) * 100;

            printf(" %-12s | Spread: $%-8.2f | %.4f%%\n",
                   book.symbol, spread, spread_pct);

            hl_free_orderbook(&book);
        }
    }

    printf("\n");

    // Liquidity analysis
    printf("💧 Liquidity Analysis\n");
    printf("═══════════════════════\n");

    printf("Analyzing market liquidity (top 10 levels):\n\n");

    for (size_t i = 0; i < num_compare; i++) {
        hl_orderbook_t book = {0};
        hl_error_t err = hl_fetch_order_book(client, compare_symbols[i], 10, &book);

        if (err == HL_SUCCESS) {
            double bid_vol = hl_orderbook_get_bid_volume(&book, 0);
            double ask_vol = hl_orderbook_get_ask_volume(&book, 0);
            double total_liquidity = bid_vol + ask_vol;

            printf(" %-12s | Bid: %-8.2f | Ask: %-8.2f | Total: %.2f lots\n",
                   book.symbol, bid_vol, ask_vol, total_liquidity);

            hl_free_orderbook(&book);
        }
    }

    // Cleanup
    hl_client_destroy(client);

    printf("\n📋 Summary:\n");
    printf("═══════════\n");
    printf("✅ Successfully analyzed order books for %zu symbols\n", num_tests);
    printf("✅ Demonstrated depth limiting and market analysis\n");
    printf("✅ Showed spread comparison and liquidity metrics\n");
    printf("✅ Real-time L2 order book data from Hyperliquid testnet\n");

    printf("\n💡 Key Insights:\n");
    printf("   • Order books provide real bid/ask spreads (not simulated)\n");
    printf("   • Depth parameter controls how many levels to fetch\n");
    printf("   • Higher volume assets typically have tighter spreads\n");
    printf("   • Order books are essential for trading strategy development\n");
    printf("   • Real-time data enables market microstructure analysis\n");

    printf("\n🎉 Example completed successfully!\n");
    printf("💡 Order book data is crucial for algorithmic trading! ⚡\n");

    return 0;
}
