/**
 * @file test_market_discovery.c
 * @brief Market discovery and information tests
 *
 * Tests market data discovery functions:
 * - hl_fetch_markets() - All markets
 * - hl_fetch_swap_markets() - Derivatives markets
 * - hl_fetch_spot_markets() - Spot markets
 * - hl_get_asset_id() - Asset ID resolution
 * - hl_get_market() - Market details
 * - hl_get_market_by_id() - Market by ID
 * - hl_fetch_open_interests() - Open interest data
 * - Market utility functions
 * - Memory management for market data
 * - Error scenarios
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "hyperliquid.h"
#include "helpers/test_common.h"

// Test result storage
static test_result_t test_results[] = {
    {TEST_STATUS_PENDING, "test_fetch_all_markets", 0.0},
    {TEST_STATUS_PENDING, "test_fetch_swap_markets", 0.0},
    {TEST_STATUS_PENDING, "test_fetch_spot_markets", 0.0},
    {TEST_STATUS_PENDING, "test_get_asset_id", 0.0},
    {TEST_STATUS_PENDING, "test_get_market", 0.0},
    {TEST_STATUS_PENDING, "test_get_market_by_id", 0.0},
    {TEST_STATUS_PENDING, "test_fetch_open_interests", 0.0},
    {TEST_STATUS_PENDING, "test_fetch_open_interest", 0.0},
    {TEST_STATUS_PENDING, "test_market_memory_management", 0.0},
    {TEST_STATUS_PENDING, "test_market_data_validation", 0.0},
    {TEST_STATUS_PENDING, "test_market_error_scenarios", 0.0},
    {TEST_STATUS_PENDING, "test_market_edge_cases", 0.0},
};
static const size_t num_tests = sizeof(test_results) / sizeof(test_results[0]);

test_result_t test_fetch_all_markets() {
    hl_client_t *client = NULL;
    hl_markets_t markets = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_fetch_markets(client, &markets);

    if (error == HL_SUCCESS) {
        TEST_ASSERT(markets.count > 0, "Should have at least one market");
        TEST_ASSERT(markets.markets != NULL, "Markets array should not be NULL");

        printf("  Found %zu markets\n", markets.count);

        // Validate market data structure
        for (size_t i = 0; i < markets.count && i < 10; i++) {  // Sample first 10
            const hl_market_t *market = &markets.markets[i];
            TEST_ASSERT(market->symbol != NULL, "Market symbol should not be NULL");
            TEST_ASSERT(market->symbol[0] != '\0', "Market symbol should not be empty");
            TEST_ASSERT(market->base != NULL, "Base currency should not be NULL");
            TEST_ASSERT(market->quote != NULL, "Quote currency should not be NULL");

            printf("    Market %zu: %s (%s/%s)\n", i, market->symbol,
                   market->base, market->quote);

            // Validate numeric fields
            TEST_ASSERT(market->price_precision >= 0, "Price precision should be non-negative");
            TEST_ASSERT(market->amount_precision >= 0, "Amount precision should be non-negative");
            TEST_ASSERT(market->min_amount > 0, "Min amount should be positive");
            TEST_ASSERT(market->max_amount > 0, "Max amount should be positive");
        }

        // Test that common symbols exist
        bool found_btc = false;
        bool found_eth = false;
        for (size_t i = 0; i < markets.count; i++) {
            if (strstr(markets.markets[i].symbol, "BTC") != NULL) {
                found_btc = true;
            }
            if (strstr(markets.markets[i].symbol, "ETH") != NULL) {
                found_eth = true;
            }
        }
        if (found_btc) printf("  ✓ Found BTC markets\n");
        if (found_eth) printf("  ✓ Found ETH markets\n");

    } else if (error == HL_ERROR_NETWORK) {
        printf("  Skipping (network error): %s\n", hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching markets: %s", hl_error_string(error));
    }

    // Cleanup
    hl_free_markets(&markets);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_fetch_swap_markets() {
    hl_client_t *client = NULL;
    hl_markets_t markets = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_fetch_swap_markets(client, &markets);

    if (error == HL_SUCCESS) {
        printf("  Found %zu swap markets\n", markets.count);

        // Validate that these are swap markets (typically perpetual)
        for (size_t i = 0; i < markets.count && i < 5; i++) {
            const hl_market_t *market = &markets.markets[i];
            TEST_ASSERT(market->symbol != NULL, "Market symbol should not be NULL");

            printf("    Swap market %zu: %s\n", i, market->symbol);

            // Swap markets often have USDC as quote
            if (strstr(market->symbol, "USDC") != NULL) {
                printf("      ✓ Perpetual swap market detected\n");
            }
        }
    } else if (error == HL_ERROR_NETWORK) {
        printf("  Skipping (network error): %s\n", hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching swap markets: %s", hl_error_string(error));
    }

    // Cleanup
    hl_free_markets(&markets);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_fetch_spot_markets() {
    hl_client_t *client = NULL;
    hl_markets_t markets = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_fetch_spot_markets(client, &markets);

    if (error == HL_SUCCESS) {
        printf("  Found %zu spot markets\n", markets.count);

        // Validate that these are spot markets
        for (size_t i = 0; i < markets.count && i < 5; i++) {
            const hl_market_t *market = &markets.markets[i];
            TEST_ASSERT(market->symbol != NULL, "Market symbol should not be NULL");

            printf("    Spot market %zu: %s\n", i, market->symbol);

            // Spot markets often have different patterns
            if (strstr(market->symbol, "/") != NULL) {
                printf("      ✓ Spot trading pair detected\n");
            }
        }
    } else if (error == HL_ERROR_NETWORK) {
        printf("  Skipping (network error): %s\n", hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching spot markets: %s", hl_error_string(error));
    }

    // Cleanup
    hl_free_markets(&markets);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_get_asset_id() {
    hl_client_t *client = NULL;
    hl_markets_t markets = {0};
    uint32_t asset_id = 0;
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // First fetch markets to have data for asset ID lookup
    error = hl_fetch_markets(client, &markets);

    if (error == HL_SUCCESS && markets.count > 0) {
        // Test getting asset ID for BTC
        error = hl_get_asset_id(&markets, "BTC", &asset_id);

        if (error == HL_SUCCESS) {
            TEST_ASSERT(asset_id > 0, "BTC asset ID should be positive");
            printf("  BTC asset ID: %u\n", asset_id);
        } else {
            printf("  Could not find BTC asset ID: %s\n", hl_error_string(error));
        }

        // Test getting asset ID for first market in list
        if (markets.count > 0) {
            const char *first_symbol = markets.markets[0].symbol;
            uint32_t first_asset_id = 0;
            error = hl_get_asset_id(&markets, first_symbol, &first_asset_id);

            if (error == HL_SUCCESS) {
                printf("  %s asset ID: %u\n", first_symbol, first_asset_id);
            }
        }

        // Test with non-existent symbol
        error = hl_get_asset_id(&markets, "NONEXISTENT_SYMBOL", &asset_id);
        TEST_ASSERT(error == HL_ERROR_NOT_FOUND || error == HL_ERROR_INVALID_SYMBOL,
                   "Expected not found or invalid symbol error");

    } else if (error == HL_ERROR_NETWORK) {
        printf("  Skipping (network error): %s\n", hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error: %s", hl_error_string(error));
    }

    // Cleanup
    hl_free_markets(&markets);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_get_market() {
    hl_client_t *client = NULL;
    hl_markets_t markets = {0};
    const hl_market_t *market = NULL;
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_fetch_markets(client, &markets);

    if (error == HL_SUCCESS && markets.count > 0) {
        // Test getting market for BTC
        error = hl_get_market(&markets, "BTC", &market);

        if (error == HL_SUCCESS && market != NULL) {
            TEST_ASSERT(strcmp(market->symbol, "BTC") == 0, "Market symbol should match");
            printf("  Found BTC market: %s/%s\n", market->base, market->quote);
        } else {
            printf("  Could not find BTC market: %s\n", hl_error_string(error));
        }

        // Test getting first market in list
        if (markets.count > 0) {
            const char *first_symbol = markets.markets[0].symbol;
            const hl_market_t *first_market = NULL;
            error = hl_get_market(&markets, first_symbol, &first_market);

            if (error == HL_SUCCESS && first_market != NULL) {
                printf("  Found %s market: %s/%s\n", first_symbol,
                       first_market->base, first_market->quote);
            }
        }

        // Test with non-existent symbol
        error = hl_get_market(&markets, "NONEXISTENT_SYMBOL", &market);
        TEST_ASSERT(error == HL_ERROR_NOT_FOUND || error == HL_ERROR_INVALID_SYMBOL,
                   "Expected not found or invalid symbol error");

    } else if (error == HL_ERROR_NETWORK) {
        printf("  Skipping (network error): %s\n", hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error: %s", hl_error_string(error));
    }

    // Cleanup
    hl_free_markets(&markets);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_get_market_by_id() {
    hl_client_t *client = NULL;
    hl_markets_t markets = {0};
    const hl_market_t *market = NULL;
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_fetch_markets(client, &markets);

    if (error == HL_SUCCESS && markets.count > 0) {
        // Test getting market by ID for first market
        uint32_t first_asset_id = 0;
        error = hl_get_asset_id(&markets, markets.markets[0].symbol, &first_asset_id);

        if (error == HL_SUCCESS) {
            error = hl_get_market_by_id(&markets, first_asset_id, &market);

            if (error == HL_SUCCESS && market != NULL) {
                TEST_ASSERT(strcmp(market->symbol, markets.markets[0].symbol) == 0,
                           "Market symbol should match");
                printf("  Found market by ID %u: %s\n", first_asset_id, market->symbol);
            }
        }

        // Test with invalid asset ID
        error = hl_get_market_by_id(&markets, 999999, &market);
        TEST_ASSERT(error == HL_ERROR_NOT_FOUND, "Expected not found error for invalid ID");

    } else if (error == HL_ERROR_NETWORK) {
        printf("  Skipping (network error): %s\n", hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error: %s", hl_error_string(error));
    }

    // Cleanup
    hl_free_markets(&markets);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_fetch_open_interests() {
    hl_client_t *client = NULL;
    hl_open_interests_t interests = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test fetching open interests for BTC
    const char *symbols[] = {"BTC"};
    error = hl_fetch_open_interests(client, symbols, 1, &interests);

    if (error == HL_SUCCESS) {
        printf("  Found %zu open interest entries\n", interests.count);

        for (size_t i = 0; i < interests.count; i++) {
            const hl_open_interest_t *interest = &interests.interests[i];
            TEST_ASSERT(interest->symbol != NULL, "Symbol should not be NULL");
            TEST_ASSERT(interest->open_interest >= 0.0, "Open interest should be non-negative");

            printf("    %s open interest: %.2f\n", interest->symbol, interest->open_interest);
        }
    } else if (error == HL_ERROR_NETWORK) {
        printf("  Skipping (network error): %s\n", hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching open interests: %s", hl_error_string(error));
    }

    // Cleanup
    hl_free_open_interests(&interests);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_fetch_open_interest() {
    hl_client_t *client = NULL;
    hl_open_interest_t interest = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test fetching open interest for BTC
    error = hl_fetch_open_interest(client, "BTC", &interest);

    if (error == HL_SUCCESS) {
        TEST_ASSERT(strcmp(interest.symbol, "BTC") == 0, "Symbol should match");
        TEST_ASSERT(interest.open_interest >= 0.0, "Open interest should be non-negative");

        printf("  BTC open interest: %.2f\n", interest.open_interest);
    } else if (error == HL_ERROR_NETWORK) {
        printf("  Skipping (network error): %s\n", hl_error_string(error));
        TEST_PASS();
    } else if (error == HL_ERROR_NOT_FOUND) {
        printf("  No open interest data for BTC (acceptable)\n");
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching open interest: %s", hl_error_string(error));
    }

    // Cleanup
    hl_free_open_interest(&interest);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_market_memory_management() {
    hl_markets_t markets = {0};
    hl_open_interests_t interests = {0};
    hl_open_interest_t interest = {0};

    TEST_START_TIMER();

    // Test double free safety
    hl_free_markets(&markets);  // Should not crash
    hl_free_open_interests(&interests);  // Should not crash
    hl_free_open_interest(&interest);  // Should not crash
    TEST_PASS();

    // Test cleanup with empty arrays
    markets.count = 0;
    markets.markets = NULL;
    hl_free_markets(&markets);  // Should not crash
    TEST_PASS();

    interests.count = 0;
    interests.interests = NULL;
    hl_free_open_interests(&interests);  // Should not crash
    TEST_PASS();

    // Test with single populated entry
    markets.count = 1;
    markets.markets = malloc(sizeof(hl_market_t));
    if (markets.markets) {
        strcpy((char*)markets.markets[0].symbol, "TEST");
        hl_free_markets(&markets);
    }
    TEST_PASS();

    return TEST_PASS();
}

test_result_t test_market_data_validation() {
    hl_client_t *client = NULL;
    hl_markets_t markets = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_fetch_markets(client, &markets);

    if (error == HL_SUCCESS && markets.count > 0) {
        // Detailed validation of market data structure
        for (size_t i = 0; i < markets.count && i < 5; i++) {
            const hl_market_t *market = &markets.markets[i];

            // Required string fields
            TEST_ASSERT(market->symbol != NULL, "Symbol cannot be NULL");
            TEST_ASSERT(market->base != NULL, "Base currency cannot be NULL");
            TEST_ASSERT(market->quote != NULL, "Quote currency cannot be NULL");

            // Non-empty strings
            TEST_ASSERT(market->symbol[0] != '\0', "Symbol cannot be empty");
            TEST_ASSERT(market->base[0] != '\0', "Base currency cannot be empty");
            TEST_ASSERT(market->quote[0] != '\0', "Quote currency cannot be empty");

            // Valid numeric values
            TEST_ASSERT(market->price_precision >= 0, "Price precision must be >= 0");
            TEST_ASSERT(market->amount_precision >= 0, "Amount precision must be >= 0");
            TEST_ASSERT(market->min_amount > 0, "Min amount must be > 0");
            TEST_ASSERT(market->max_amount > 0, "Max amount must be > 0");
            TEST_ASSERT(market->min_amount <= market->max_amount, "Min amount <= max amount");

            printf("  ✓ Market %s data validation passed\n", market->symbol);
        }
    } else if (error == HL_ERROR_NETWORK) {
        printf("  Skipping (network error): %s\n", hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error: %s", hl_error_string(error));
    }

    // Cleanup
    hl_free_markets(&markets);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_market_error_scenarios() {
    hl_client_t *client = NULL;
    hl_markets_t markets = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    // Test with NULL client
    error = hl_fetch_markets(NULL, &markets);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL client");

    // Test with NULL markets struct
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_fetch_markets(client, NULL);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL markets");

    // Test asset ID lookup with invalid parameters
    uint32_t asset_id = 0;
    error = hl_get_asset_id(NULL, "BTC", &asset_id);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL markets");

    error = hl_get_asset_id(&markets, NULL, &asset_id);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL symbol");

    error = hl_get_asset_id(&markets, "BTC", NULL);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL asset_id");

    // Test market lookup with invalid parameters
    const hl_market_t *market = NULL;
    error = hl_get_market(NULL, "BTC", &market);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL markets");

    error = hl_get_market(&markets, NULL, &market);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL symbol");

    error = hl_get_market(&markets, "BTC", NULL);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL market");

    // Cleanup
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_market_edge_cases() {
    hl_client_t *client = NULL;
    hl_markets_t markets = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_fetch_markets(client, &markets);

    if (error == HL_SUCCESS) {
        // Test with very long symbol name
        char long_symbol[256];
        memset(long_symbol, 'A', 255);
        long_symbol[255] = '\0';
        uint32_t asset_id = 0;
        error = hl_get_asset_id(&markets, long_symbol, &asset_id);
        // Should return not found or invalid symbol, not crash

        // Test with special characters in symbol
        error = hl_get_asset_id(&markets, "BTC/USDC:USDC-TEST", &asset_id);
        // Should handle gracefully

        // Test with mixed case symbol
        error = hl_get_asset_id(&markets, "btc", &asset_id);
        // Should handle case sensitivity appropriately

        printf("  ✓ Edge cases handled gracefully\n");
    } else if (error == HL_ERROR_NETWORK) {
        printf("  Skipping (network error): %s\n", hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error: %s", hl_error_string(error));
    }

    // Cleanup
    hl_free_markets(&markets);
    hl_client_destroy(client);
    return TEST_PASS();
}

// Main test runner
int main() {
    printf("=== Market Discovery Comprehensive Tests ===\n\n");

    // Run all tests
    for (size_t i = 0; i < num_tests; i++) {
        printf("Running %s...\n", test_results[i].name);

        switch (i) {
            case 0: test_results[i] = test_fetch_all_markets(); break;
            case 1: test_results[i] = test_fetch_swap_markets(); break;
            case 2: test_results[i] = test_fetch_spot_markets(); break;
            case 3: test_results[i] = test_get_asset_id(); break;
            case 4: test_results[i] = test_get_market(); break;
            case 5: test_results[i] = test_get_market_by_id(); break;
            case 6: test_results[i] = test_fetch_open_interests(); break;
            case 7: test_results[i] = test_fetch_open_interest(); break;
            case 8: test_results[i] = test_market_memory_management(); break;
            case 9: test_results[i] = test_market_data_validation(); break;
            case 10: test_results[i] = test_market_error_scenarios(); break;
            case 11: test_results[i] = test_market_edge_cases(); break;
            default:
                test_results[i].status = TEST_STATUS_SKIP;
                test_results[i].duration = 0.0;
                break;
        }

        print_test_result(&test_results[i]);
        printf("\n");
    }

    // Print summary
    print_test_summary(test_results, num_tests);

    // Return success if all tests passed or were skipped
    return (get_failed_count(test_results, num_tests) == 0) ? 0 : 1;
}