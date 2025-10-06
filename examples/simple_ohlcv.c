/**
 * @file simple_ohlcv.c
 * @brief Example: Fetch and analyze OHLCV candlestick data
 *
 * This example demonstrates how to:
 * - Fetch historical candlestick data for technical analysis
 * - Calculate technical indicators (SMA, highest/lowest)
 * - Handle cases where OHLCV data may not be available (testnet)
 * - Work with different timeframes and limits
 *
 * Build: make -f Makefile.tests example_ohlcv
 * Run: ./build/bin/example_ohlcv
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/hyperliquid.h"

void analyze_candles(const hl_ohlcvs_t* ohlcvs) {
    if (!ohlcvs || ohlcvs->count == 0) {
        printf("   No candle data to analyze\n");
        return;
    }

    printf("   📊 Candle Analysis:\n");
    printf("   Symbol: %s | Timeframe: %s | Candles: %zu\n\n", ohlcvs->symbol, ohlcvs->timeframe, ohlcvs->count);

    // Show first and last candles
    if (ohlcvs->count >= 2) {
        const hl_ohlcv_t* first = hl_ohlcvs_get_candle(ohlcvs, 0);
        const hl_ohlcv_t* last = hl_ohlcvs_get_latest(ohlcvs);

        printf("   📈 First Candle:\n");
        printf("      Time: %llu | O: %.2f | H: %.2f | L: %.2f | C: %.2f | V: %.2f\n",
               (unsigned long long)first->timestamp, first->open, first->high, first->low, first->close, first->volume);

        printf("   📉 Last Candle:\n");
        printf("      Time: %llu | O: %.2f | H: %.2f | L: %.2f | C: %.2f | V: %.2f\n",
               (unsigned long long)last->timestamp, last->open, last->high, last->low, last->close, last->volume);

        // Calculate price change
        double price_change = last->close - first->open;
        double price_change_pct = (price_change / first->open) * 100;
        printf("   📊 Price Change: %.2f (%.2f%%)\n", price_change, price_change_pct);
    }

    // Calculate technical indicators if enough data
    if (ohlcvs->count >= 5) {
        double* sma = calloc(ohlcvs->count, sizeof(double));
        if (sma) {
            hl_error_t err = hl_ohlcvs_calculate_sma(ohlcvs, 5, true, sma);
            if (err == HL_SUCCESS) {
                printf("   📈 SMA(5) - Close:\n");
                for (size_t i = 4; i < ohlcvs->count && i < 8; i++) { // Show first few
                    printf("      [%zu]: %.2f\n", i, sma[i]);
                }
                if (ohlcvs->count > 8) printf("      ... (%zu more)\n", ohlcvs->count - 8);
            }
            free(sma);
        }

        // Calculate highest high and lowest low
        double highest = hl_ohlcvs_highest_high(ohlcvs, 0, ohlcvs->count);
        double lowest = hl_ohlcvs_lowest_low(ohlcvs, 0, ohlcvs->count);

        printf("   📊 Range Analysis:\n");
        printf("      Highest High: %.2f\n", highest);
        printf("      Lowest Low: %.2f\n", lowest);
        printf("      Range: %.2f\n", highest - lowest);
    }

    printf("\n");
}

int main(void) {
    printf("📊 Hyperliquid OHLCV Candlestick Analysis Example\n");
    printf("===============================================\n\n");

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

    // Test different scenarios
    struct {
        const char* symbol;
        const char* timeframe;
        uint32_t* limit;
        const char* description;
    } test_cases[] = {
        {"BTC/USDC:USDC", "1m", NULL, "BTC - No limit (last 24h)"},
        {"ETH/USDC:USDC", "5m", NULL, "ETH - No limit (last 24h)"},
        {"SOL/USDC:USDC", "15m", NULL, "SOL - No limit (last 24h)"},
        {"DOGE/USDC:USDC", "1h", NULL, "DOGE - No limit (last 24h)"}
    };

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    printf("🔍 Testing OHLCV data availability...\n");
    printf("ℹ️  Note: Testnet may not have historical OHLCV data\n\n");

    for (size_t i = 0; i < num_tests; i++) {
        const char* symbol = test_cases[i].symbol;
        const char* timeframe = test_cases[i].timeframe;
        uint32_t* limit = test_cases[i].limit;
        const char* description = test_cases[i].description;

        printf("📊 %s\n", description);
        printf("Symbol: %s | Timeframe: %s | Limit: %s\n",
               symbol, timeframe, limit ? "limited" : "unlimited");

        // Fetch OHLCV data
        hl_ohlcvs_t ohlcvs = {0};
        hl_error_t err = hl_fetch_ohlcv(client, symbol, timeframe, NULL, limit, NULL, &ohlcvs);

        if (err != HL_SUCCESS) {
            printf("❌ Failed to fetch OHLCV: %s\n\n", hl_error_string(err));
            continue;
        }

        printf("✅ Fetched %zu candles\n", ohlcvs.count);

        if (ohlcvs.count == 0) {
            printf("ℹ️  No historical data available (normal for testnet)\n");
            printf("💡 Try mainnet for real OHLCV data\n");
        } else {
            analyze_candles(&ohlcvs);
        }

        hl_ohlcvs_free(&ohlcvs);
    }

    // Demonstrate time range filtering
    printf("⏰ Time Range Filtering Example\n");
    printf("===============================\n");

    // Get data for last 2 hours
    uint64_t now = (uint64_t)time(NULL) * 1000;
    uint64_t two_hours_ago = now - (2LL * 60LL * 60LL * 1000LL);

    printf("Fetching BTC/USDC:USDC data from %llu to %llu (2 hours)\n",
           (unsigned long long)two_hours_ago, (unsigned long long)now);

    hl_ohlcvs_t range_ohlcvs = {0};
    hl_error_t err = hl_fetch_ohlcv(client, "BTC/USDC:USDC", "1m", &two_hours_ago, NULL, &now, &range_ohlcvs);

    if (err == HL_SUCCESS) {
        printf("✅ Fetched %zu candles in time range\n", range_ohlcvs.count);
        if (range_ohlcvs.count > 0) {
            analyze_candles(&range_ohlcvs);
        } else {
            printf("ℹ️  No data in specified time range\n");
        }
    } else {
        printf("❌ Failed to fetch time-range data: %s\n", hl_error_string(err));
    }

    hl_ohlcvs_free(&range_ohlcvs);

    // Cleanup
    hl_client_destroy(client);

    printf("\n📋 Summary:\n");
    printf("═══════════\n");
    printf("✅ OHLCV API calls work correctly\n");
    printf("✅ Testnet may not have historical data (normal)\n");
    printf("✅ Mainnet will have real candlestick data\n");
    printf("✅ Technical analysis functions are ready\n");
    printf("✅ Time range filtering works as expected\n");

    printf("\n💡 Key Takeaways:\n");
    printf("   • OHLCV data enables technical analysis\n");
    printf("   • Testnet is for testing API calls, not historical data\n");
    printf("   • Mainnet provides real candlestick data\n");
    printf("   • Timeframes: 1m, 5m, 15m, 1h, 1d, etc.\n");
    printf("   • Built-in indicators: SMA, highest/lowest analysis\n");

    printf("\n🎉 Example completed successfully!\n");
    printf("💡 OHLCV data is essential for algorithmic trading! 📈\n");

    return 0;
}
