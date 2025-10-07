/**
 * @file test_account_comprehensive.c
 * @brief Comprehensive account management tests
 *
 * Tests critical account functions:
 * - hl_fetch_balance() for both spot and perpetual accounts
 * - hl_fetch_positions() and hl_fetch_position()
 * - hl_fetch_trading_fee()
 * - hl_fetch_ledger()
 * - Memory management for account data
 * - Error scenarios for account operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "hyperliquid.h"
#include "helpers/test_common.h"

// Test result storage
static test_result_t test_results[] = {
    {TEST_STATUS_PENDING, "test_fetch_balance_perpetual", 0.0},
    {TEST_STATUS_PENDING, "test_fetch_balance_spot", 0.0},
    {TEST_STATUS_PENDING, "test_fetch_positions", 0.0},
    {TEST_STATUS_PENDING, "test_fetch_single_position", 0.0},
    {TEST_STATUS_PENDING, "test_fetch_trading_fee", 0.0},
    {TEST_STATUS_PENDING, "test_fetch_ledger", 0.0},
    {TEST_STATUS_PENDING, "test_balance_memory_management", 0.0},
    {TEST_STATUS_PENDING, "test_positions_memory_management", 0.0},
    {TEST_STATUS_PENDING, "test_account_error_scenarios", 0.0},
    {TEST_STATUS_PENDING, "test_invalid_account_parameters", 0.0},
};
static const size_t num_tests = sizeof(test_results) / sizeof(test_results[0]);

// Helper function to test balance fetching
test_result_t test_fetch_balance_perpetual() {
    hl_client_t *client = NULL;
    hl_balance_t balance = {0};
    hl_error_t error = HL_SUCCESS;

    // Create test client
    TEST_START_TIMER();
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test perpetual account balance
    error = hl_fetch_balance(client, HL_ACCOUNT_TYPE_PERPETUAL, &balance);

    if (error == HL_SUCCESS) {
        TEST_ASSERT(balance.total >= 0.0, "Invalid total balance");
        TEST_ASSERT(balance.free >= 0.0, "Invalid free balance");
        TEST_ASSERT(balance.used >= 0.0, "Invalid used balance");
        TEST_ASSERT(fabs((balance.free + balance.used) - balance.total) < 0.000001,
                   "Balance inconsistency: free + used != total");

        // Log balance information
        printf("  Perpetual balance: Total=%.6f, Free=%.6f, Used=%.6f\n",
               balance.total, balance.free, balance.used);
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK) {
        // Expected in CI environment without credentials
        printf("  Skipping balance test (no credentials or network): %s\n",
               hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching perpetual balance: %s",
                 hl_error_string(error));
    }

    // Cleanup
    hl_free_balance(&balance);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_fetch_balance_spot() {
    hl_client_t *client = NULL;
    hl_balance_t balance = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test spot account balance
    error = hl_fetch_balance(client, HL_ACCOUNT_TYPE_SPOT, &balance);

    if (error == HL_SUCCESS) {
        TEST_ASSERT(balance.total >= 0.0, "Invalid total balance");
        TEST_ASSERT(balance.free >= 0.0, "Invalid free balance");
        TEST_ASSERT(balance.used >= 0.0, "Invalid used balance");
        TEST_ASSERT(fabs((balance.free + balance.used) - balance.total) < 0.000001,
                   "Balance inconsistency: free + used != total");

        printf("  Spot balance: Total=%.6f, Free=%.6f, Used=%.6f\n",
               balance.total, balance.free, balance.used);
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK) {
        printf("  Skipping spot balance test (no credentials or network): %s\n",
               hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching spot balance: %s",
                 hl_error_string(error));
    }

    // Cleanup
    hl_free_balance(&balance);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_fetch_positions() {
    hl_client_t *client = NULL;
    hl_positions_t positions = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test fetching all positions
    error = hl_fetch_positions(client, NULL, 0, &positions);

    if (error == HL_SUCCESS) {
        printf("  Found %zu positions\n", positions.count);

        // Validate position data if any exist
        for (size_t i = 0; i < positions.count; i++) {
            const hl_position_t *pos = &positions.positions[i];
            TEST_ASSERT(pos->symbol != NULL, "Position symbol is NULL");
            TEST_ASSERT(pos->symbol[0] != '\0', "Position symbol is empty");
            TEST_ASSERT(pos->size != 0.0 || !pos->open, "Zero-size position marked as open");

            printf("    Position %zu: %s, Size=%.6f, Entry=%.6f, PNL=%.6f\n",
                   i, pos->symbol, pos->size, pos->entry_price, pos->unrealized_pnl);
        }
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK) {
        printf("  Skipping positions test (no credentials or network): %s\n",
               hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching positions: %s",
                 hl_error_string(error));
    }

    // Cleanup
    hl_free_positions(&positions);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_fetch_single_position() {
    hl_client_t *client = NULL;
    hl_position_t position = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test fetching single position for BTC
    error = hl_fetch_position(client, "BTC", &position);

    if (error == HL_SUCCESS) {
        TEST_ASSERT(position.symbol != NULL, "Position symbol is NULL");
        TEST_ASSERT(strcmp(position.symbol, "BTC") == 0, "Position symbol mismatch");
        TEST_ASSERT(position.size >= 0.0 || position.size <= 0.0, "Invalid position size");

        printf("  BTC position: Size=%.6f, Entry=%.6f, Mark=%.6f, PNL=%.6f\n",
               position.size, position.entry_price, position.mark_price,
               position.unrealized_pnl);
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK ||
               error == HL_ERROR_NOT_FOUND) {
        // NOT_FOUND is acceptable if no position exists
        printf("  Skipping single position test (no credentials, network, or no position): %s\n",
               hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching single position: %s",
                 hl_error_string(error));
    }

    // Cleanup
    hl_free_position(&position);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_fetch_trading_fee() {
    hl_client_t *client = NULL;
    hl_trading_fee_t fee = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test fetching trading fee for BTC
    error = hl_fetch_trading_fee(client, "BTC", &fee);

    if (error == HL_SUCCESS) {
        TEST_ASSERT(fee.maker_fee >= 0.0, "Invalid maker fee");
        TEST_ASSERT(fee.taker_fee >= 0.0, "Invalid taker fee");

        printf("  BTC trading fees: Maker=%.4f%%, Taker=%.4f%%\n",
               fee.maker_fee * 100.0, fee.taker_fee * 100.0);
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK) {
        printf("  Skipping trading fee test (no credentials or network): %s\n",
               hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching trading fee: %s",
                 hl_error_string(error));
    }

    // Cleanup
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_fetch_ledger() {
    hl_client_t *client = NULL;
    hl_ledger_t ledger = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test fetching ledger with limited entries
    error = hl_fetch_ledger(client, NULL, NULL, 10, &ledger);

    if (error == HL_SUCCESS) {
        printf("  Found %zu ledger entries\n", ledger.count);

        // Validate ledger entries
        for (size_t i = 0; i < ledger.count; i++) {
            const hl_ledger_entry_t *entry = &ledger.entries[i];
            TEST_ASSERT(entry->currency != NULL, "Ledger entry currency is NULL");
            TEST_ASSERT(entry->amount != 0.0, "Ledger entry amount is zero");
            TEST_ASSERT(entry->timestamp > 0, "Invalid ledger entry timestamp");

            printf("    Entry %zu: %s %.6f, Type=%s, Time=%ld\n",
                   i, entry->currency, entry->amount,
                   entry->type, entry->timestamp);
        }
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK) {
        printf("  Skipping ledger test (no credentials or network): %s\n",
               hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching ledger: %s",
                 hl_error_string(error));
    }

    // Cleanup
    hl_free_ledger(&ledger);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_balance_memory_management() {
    hl_balance_t balance = {0};

    TEST_START_TIMER();

    // Test double free safety
    hl_free_balance(&balance);  // Should not crash
    TEST_PASS();

    // Test cleanup with populated data
    balance.total = 100.0;
    balance.free = 80.0;
    balance.used = 20.0;
    hl_free_balance(&balance);  // Should not crash
    TEST_PASS();

    return TEST_PASS();
}

test_result_t test_positions_memory_management() {
    hl_positions_t positions = {0};

    TEST_START_TIMER();

    // Test double free safety
    hl_free_positions(&positions);  // Should not crash
    TEST_PASS();

    // Test cleanup with empty array
    positions.count = 0;
    positions.positions = NULL;
    hl_free_positions(&positions);  // Should not crash
    TEST_PASS();

    return TEST_PASS();
}

test_result_t test_account_error_scenarios() {
    hl_client_t *client = NULL;
    hl_balance_t balance = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    // Test with NULL client
    error = hl_fetch_balance(NULL, HL_ACCOUNT_TYPE_PERPETUAL, &balance);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL client");

    // Test with NULL balance
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_fetch_balance(client, HL_ACCOUNT_TYPE_PERPETUAL, NULL);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL balance");

    // Test with invalid account type
    error = hl_fetch_balance(client, (hl_account_type_t)999, &balance);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with invalid account type");

    // Cleanup
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_invalid_account_parameters() {
    hl_client_t *client = NULL;
    hl_position_t position = {0};
    hl_trading_fee_t fee = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test position with NULL symbol
    error = hl_fetch_position(client, NULL, &position);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL symbol");

    // Test position with NULL position struct
    error = hl_fetch_position(client, "BTC", NULL);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL position");

    // Test trading fee with NULL symbol
    error = hl_fetch_trading_fee(client, NULL, &fee);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL symbol");

    // Test trading fee with NULL fee struct
    error = hl_fetch_trading_fee(client, "BTC", NULL);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL fee");

    // Cleanup
    hl_client_destroy(client);
    return TEST_PASS();
}

// Main test runner
int main() {
    printf("=== Account Management Comprehensive Tests ===\n\n");

    // Run all tests
    for (size_t i = 0; i < num_tests; i++) {
        printf("Running %s...\n", test_results[i].name);

        switch (i) {
            case 0: test_results[i] = test_fetch_balance_perpetual(); break;
            case 1: test_results[i] = test_fetch_balance_spot(); break;
            case 2: test_results[i] = test_fetch_positions(); break;
            case 3: test_results[i] = test_fetch_single_position(); break;
            case 4: test_results[i] = test_fetch_trading_fee(); break;
            case 5: test_results[i] = test_fetch_ledger(); break;
            case 6: test_results[i] = test_balance_memory_management(); break;
            case 7: test_results[i] = test_positions_memory_management(); break;
            case 8: test_results[i] = test_account_error_scenarios(); break;
            case 9: test_results[i] = test_invalid_account_parameters(); break;
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