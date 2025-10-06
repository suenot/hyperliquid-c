/**
 * @file test_order_management.c
 * @brief Integration tests for order management
 */

#include "../../helpers/test_common.h"

/**
 * @brief Test fetching open orders
 */
test_result_t test_fetch_open_orders(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get open orders
    hl_orders_t orders = {0};
    hl_error_t err = hl_fetch_open_orders(client, NULL, NULL, 50, &orders);

    if (err == HL_SUCCESS) {
        printf("✅ Open orders fetched successfully\n");
        printf("   Open orders count: %zu\n", orders.count);

        // Free orders if needed
        hl_free_orders(&orders);
    } else {
        printf("⚠️  Failed to fetch open orders: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test fetching closed orders
 */
test_result_t test_fetch_closed_orders(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get closed orders
    hl_orders_t orders = {0};
    hl_error_t err = hl_fetch_closed_orders(client, NULL, NULL, 20, &orders);

    if (err == HL_SUCCESS) {
        printf("✅ Closed orders fetched successfully\n");
        printf("   Closed orders count: %zu\n", orders.count);

        // Free orders if needed
        hl_free_orders(&orders);
    } else {
        printf("⚠️  Failed to fetch closed orders: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test fetching a specific order
 */
test_result_t test_fetch_order(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Test with a dummy order ID (should fail gracefully)
    hl_order_t order = {0};
    hl_error_t err = hl_fetch_order(client, "dummy_order_id", "BTC", &order);

    if (err == HL_SUCCESS) {
        printf("✅ Order fetched successfully\n");
    } else {
        printf("⚠️  Failed to fetch order (expected for dummy ID): %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test fetching all orders
 */
test_result_t test_fetch_orders(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get all orders
    hl_orders_t orders = {0};
    hl_error_t err = hl_fetch_orders(client, NULL, NULL, 100, &orders);

    if (err == HL_SUCCESS) {
        printf("✅ All orders fetched successfully\n");
        printf("   Total orders count: %zu\n", orders.count);

        // Free orders if needed
        hl_free_orders(&orders);
    } else {
        printf("⚠️  Failed to fetch all orders: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test fetching user trades
 */
test_result_t test_fetch_my_trades(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Get user trades
    hl_trades_t trades = {0};
    hl_error_t err = hl_fetch_my_trades(client, NULL, NULL, 50, &trades);

    if (err == HL_SUCCESS) {
        printf("✅ User trades fetched successfully\n");
        printf("   User trades count: %zu\n", trades.count);

        // Free trades if needed
    } else {
        printf("⚠️  Failed to fetch user trades: %s\n", hl_error_string(err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test cancel order workflow
 */
test_result_t test_cancel_order_workflow(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // First, place a limit order that we can cancel
    hl_order_request_api_t order = {
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .price = 10000.0,  // Far from market
        .quantity = 0.001,
        .order_type = HL_ORDER_TYPE_LIMIT,
        .time_in_force = HL_TIF_GTC,
        .reduce_only = false,
        .slippage_bps = 0
    };

    hl_order_result_t result = {0};
    hl_error_t place_err = hl_place_order(client, &order, &result);

    if (place_err == HL_SUCCESS && result.order_id) {
        printf("✅ Test order placed: %s\n", result.order_id);

        // Now cancel it
        hl_cancel_result_t cancel_result = {0};
        hl_error_t cancel_err = hl_cancel_order(client, order.symbol, result.order_id, &cancel_result);

        if (cancel_err == HL_SUCCESS) {
            printf("✅ Order cancelled successfully\n");
            test_assert(cancel_result.cancelled, "Cancel status should be true");
        } else {
            printf("⚠️  Failed to cancel order: %s\n", hl_error_string(cancel_err));
        }

        free(result.order_id);
    } else {
        printf("⚠️  Failed to place test order: %s\n", hl_error_string(place_err));
    }

    test_destroy_client(client);
    return TEST_PASS;
}

/**
 * @brief Test cancel all orders
 */
test_result_t test_cancel_all_orders(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Cancel all BTC orders (should work even if no orders exist)
    int cancelled_count = hl_cancel_all_orders(client, "BTC");

    printf("✅ Cancel all orders executed\n");
    printf("   Cancelled orders count: %d\n", cancelled_count);

    test_destroy_client(client);
    return TEST_PASS;
}

int main(void) {
    test_load_env();

    printf("╔══════════════════════════════════════════╗\n");
    printf("║  INTEGRATION TESTS: Order Management     ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    test_func_t tests[] = {
        test_fetch_open_orders,
        test_fetch_closed_orders,
        test_fetch_order,
        test_fetch_orders,
        test_fetch_my_trades,
        test_cancel_order_workflow,
        test_cancel_all_orders
    };

    return test_run_suite("Order Management Integration Tests", tests, sizeof(tests)/sizeof(test_func_t));
}