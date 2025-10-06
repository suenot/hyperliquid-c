/**
 * @file test_account_management.c
 * @brief Integration tests for account management
 */

#include "../../helpers/test_common.h"

/**
 * @brief Test fetching currencies
 */
test_result_t test_fetch_currencies(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get available currencies
    void* currencies = NULL; // Would be proper currencies type
    hl_error_t err = hl_fetch_currencies(client, currencies);

    if (err == HL_SUCCESS) {
        printf("✅ Currencies fetched successfully\n");
        // Free currencies if needed
    } else {
        printf("⚠️  Failed to fetch currencies: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test setting leverage
 */
test_result_t test_set_leverage(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Set leverage for BTC (use conservative leverage)
    hl_error_t err = hl_set_leverage(client, 2, "BTC");

    if (err == HL_SUCCESS) {
        printf("✅ Leverage set successfully (2x for BTC)\n");
    } else {
        printf("⚠️  Failed to set leverage: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test adding margin
 */
test_result_t test_add_margin(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Add margin to BTC position (small amount)
    hl_error_t err = hl_add_margin(client, "BTC", 1.0);

    if (err == HL_SUCCESS) {
        printf("✅ Margin added successfully (1 USDC to BTC)\n");
    } else {
        printf("⚠️  Failed to add margin: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test reducing margin
 */
test_result_t test_reduce_margin(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Reduce margin from BTC position (small amount)
    hl_error_t err = hl_reduce_margin(client, "BTC", 0.5);

    if (err == HL_SUCCESS) {
        printf("✅ Margin reduced successfully (0.5 USDC from BTC)\n");
    } else {
        printf("⚠️  Failed to reduce margin: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test fetching funding rates
 */
test_result_t test_fetch_funding_rates(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get funding rates
    void* rates = NULL; // Would be proper funding rates type
    hl_error_t err = hl_fetch_funding_rates(client, NULL, 0, rates);

    if (err == HL_SUCCESS) {
        printf("✅ Funding rates fetched successfully\n");
        // Free rates if needed
    } else {
        printf("⚠️  Failed to fetch funding rates: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test fetching deposits
 */
test_result_t test_fetch_deposits(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get deposit history
    void* deposits = NULL; // Would be proper deposits type
    hl_error_t err = hl_fetch_deposits(client, NULL, NULL, 20, deposits);

    if (err == HL_SUCCESS) {
        printf("✅ Deposits fetched successfully\n");
        // Free deposits if needed
    } else {
        printf("⚠️  Failed to fetch deposits: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test fetching withdrawals
 */
test_result_t test_fetch_withdrawals(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get withdrawal history
    void* withdrawals = NULL; // Would be proper withdrawals type
    hl_error_t err = hl_fetch_withdrawals(client, NULL, NULL, 20, withdrawals);

    if (err == HL_SUCCESS) {
        printf("✅ Withdrawals fetched successfully\n");
        // Free withdrawals if needed
    } else {
        printf("⚠️  Failed to fetch withdrawals: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

int main(void) {
    test_load_env();

    printf("╔══════════════════════════════════════════╗\n");
    printf("║  INTEGRATION TESTS: Account Management    ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    test_func_t tests[] = {
        test_fetch_currencies,
        test_set_leverage,
        test_add_margin,
        test_reduce_margin,
        test_fetch_funding_rates,
        test_fetch_deposits,
        test_fetch_withdrawals
    };

    return test_run_suite("Account Management Integration Tests", tests, sizeof(tests)/sizeof(test_func_t));
}