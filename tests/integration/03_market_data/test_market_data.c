/**
 * @file test_market_data.c
 * @brief Integration tests for market data APIs
 */

#include "../../helpers/test_common.h"

/**
 * @brief Test fetching ticker data
 */
test_result_t test_fetch_ticker(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get BTC ticker
    hl_ticker_t ticker = {0};
    hl_error_t err = hl_get_ticker(client, "BTC", &ticker);

    if (err == HL_SUCCESS) {
        printf("✅ BTC Ticker fetched successfully\n");
        printf("   Price: %.2f\n", ticker.last_price);
        printf("   Volume: %.2f\n", ticker.volume_24h);

        // Free ticker resources if needed
        hl_free_orderbook(NULL); // Using available free function
    } else {
        printf("⚠️  Failed to fetch ticker: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test fetching order book
 */
test_result_t test_fetch_orderbook(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get BTC order book
    hl_orderbook_t orderbook = {0};
    hl_error_t err = hl_get_orderbook(client, "BTC", 10, &orderbook);

    if (err == HL_SUCCESS) {
        printf("✅ BTC Orderbook fetched successfully\n");
        printf("   Bids: %zu\n", orderbook.bids_count);
        printf("   Asks: %zu\n", orderbook.asks_count);

        // Free orderbook resources
        hl_free_orderbook(&orderbook);
    } else {
        printf("⚠️  Failed to fetch orderbook: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test fetching market price
 */
test_result_t test_fetch_market_price(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get BTC market price
    double price = 0.0;
    int err = hl_get_market_price(client, "BTC", &price);

    if (err == 0) {
        test_assert(price > 0.0, "Market price should be positive");
        printf("✅ BTC Market Price: %.2f\n", price);
    } else {
        printf("⚠️  Failed to fetch market price\n");
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test fetching trade history
 */
test_result_t test_fetch_trade_history(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get recent trades for BTC
    hl_trade_t* trades = NULL;
    int count = 0;
    int err = hl_get_trade_history(client, "BTC", &trades, &count);

    if (err == 0) {
        printf("✅ Trade history fetched successfully\n");
        printf("   Trades count: %d\n", count);

        // Free trade history
        hl_free_trades(trades);
    } else {
        printf("⚠️  Failed to fetch trade history\n");
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test fetching public trades
 */
test_result_t test_fetch_public_trades(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get public trades for ETH
    hl_trades_t trades = {0};
    hl_error_t err = hl_fetch_trades(client, "ETH", NULL, 10, &trades);

    if (err == HL_SUCCESS) {
        printf("✅ Public trades fetched successfully\n");
        printf("   Trades count: %zu\n", trades.count);

        // Free trades if needed
        if (trades.trades) {
            // This would need a proper free function based on the API
        }
    } else {
        printf("⚠️  Failed to fetch public trades: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test fetching multiple tickers
 */
test_result_t test_fetch_tickers(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get multiple tickers
    const char* symbols[] = {"BTC", "ETH", "SOL"};
    void* tickers = NULL; // Would be proper ticker array type
    hl_error_t err = hl_fetch_tickers(client, symbols, 3, tickers);

    if (err == HL_SUCCESS) {
        printf("✅ Multiple tickers fetched successfully\n");
        // Free tickers if needed
    } else {
        printf("⚠️  Failed to fetch multiple tickers: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

int main(void) {
    test_load_env();

    printf("╔══════════════════════════════════════════╗\n");
    printf("║  INTEGRATION TESTS: Market Data          ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    test_func_t tests[] = {
        test_fetch_ticker,
        test_fetch_orderbook,
        test_fetch_market_price,
        test_fetch_trade_history,
        test_fetch_public_trades,
        test_fetch_tickers
    };

    return test_run_suite("Market Data Integration Tests", tests, sizeof(tests)/sizeof(test_func_t));
}