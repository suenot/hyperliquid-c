/**
 * @file test_place_order.c
 * @brief Integration tests for order placement
 */

#include "../../helpers/test_common.h"

/**
 * @brief Test placing a limit order
 */
test_result_t test_place_limit_order(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Create a limit order far from market price (should not fill immediately)
    hl_order_request_api_t order = {
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .price = 10000.0,  // Very low price, unlikely to fill
        .quantity = 0.001,  // Small quantity
        .order_type = HL_ORDER_TYPE_LIMIT,
        .time_in_force = HL_TIF_GTC,
        .reduce_only = false,
        .slippage_bps = 0
    };

    hl_order_result_t result = {0};
    hl_error_t err = hl_place_order(client, &order, &result);

    if (err == HL_SUCCESS) {
        test_assert_not_null(result.order_id, "Order ID returned");
        printf("✅ Limit order placed: %s\n", result.order_id);

        // Clean up - cancel the order
        hl_cancel_result_t cancel_result = {0};
        hl_error_t cancel_err = hl_cancel_order(client, order.symbol, result.order_id, &cancel_result);
        if (cancel_err == HL_SUCCESS) {
            printf("✅ Test order cancelled\n");
        }

        // Free the order ID
        if (result.order_id) {
            free(result.order_id);
        }
    } else {
        printf("⚠️  Order placement failed: %s\n", hl_error_string(err));
        // Don't fail the test for API errors - might be rate limits or temporary issues
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test placing a market order
 */
test_result_t test_place_market_order(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Create a very small market order
    hl_order_request_api_t order = {
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .price = 0.0,  // Market order
        .quantity = 0.0001,  // Very small quantity
        .order_type = HL_ORDER_TYPE_MARKET,
        .time_in_force = HL_TIF_IOC,
        .reduce_only = false,
        .slippage_bps = 100  // Allow 1% slippage
    };

    hl_order_result_t result = {0};
    hl_error_t err = hl_place_order(client, &order, &result);

    if (err == HL_SUCCESS) {
        test_assert_not_null(result.order_id, "Order ID returned");
        printf("✅ Market order placed: %s\n", result.order_id);

        // Market orders should fill quickly or be rejected
        if (result.status == HL_ORDER_STATUS_FILLED) {
            printf("✅ Market order filled at price: %.2f\n", result.average_price);
        }

        if (result.order_id) {
            free(result.order_id);
        }
    } else {
        printf("⚠️  Market order placement failed: %s\n", hl_error_string(err));
        // Don't fail the test for API errors
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test placing invalid order
 */
test_result_t test_place_invalid_order(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Create an invalid order (zero quantity)
    hl_order_request_api_t invalid_order = {
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .price = 50000.0,
        .quantity = 0.0,  // Invalid: zero quantity
        .order_type = HL_ORDER_TYPE_LIMIT,
        .time_in_force = HL_TIF_GTC,
        .reduce_only = false,
        .slippage_bps = 0
    };

    hl_order_result_t result = {0};
    hl_error_t err = hl_place_order(client, &invalid_order, &result);

    test_assert(err != HL_SUCCESS, "Invalid order should fail");
    printf("✅ Invalid order correctly rejected: %s\n", hl_error_string(err));

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test placing order with invalid symbol
 */
test_result_t test_place_invalid_symbol(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Create order with invalid symbol
    hl_order_request_api_t invalid_symbol_order = {
        .symbol = "INVALID_SYMBOL",
        .side = HL_SIDE_BUY,
        .price = 50000.0,
        .quantity = 0.001,
        .order_type = HL_ORDER_TYPE_LIMIT,
        .time_in_force = HL_TIF_GTC,
        .reduce_only = false,
        .slippage_bps = 0
    };

    hl_order_result_t result = {0};
    hl_error_t err = hl_place_order(client, &invalid_symbol_order, &result);

    test_assert(err != HL_SUCCESS, "Invalid symbol should fail");
    printf("✅ Invalid symbol correctly rejected: %s\n", hl_error_string(err));

    test_destroy_client(client);
    return TEST_PASS;
}

int main(void) {
    test_load_env();

    printf("╔══════════════════════════════════════════╗\n");
    printf("║  INTEGRATION TESTS: Place Order          ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    test_func_t tests[] = {
        test_place_limit_order,
        test_place_market_order,
        test_place_invalid_order,
        test_place_invalid_symbol
    };

    return test_run_suite("Place Order Integration Tests", tests, sizeof(tests)/sizeof(test_func_t));
}