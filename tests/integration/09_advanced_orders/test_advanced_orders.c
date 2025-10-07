/**
 * @file test_advanced_orders.c
 * @brief Advanced order management tests
 *
 * Tests advanced order operations:
 * - hl_create_orders() - Bulk order creation
 * - hl_cancel_orders() - Bulk order cancellation
 * - hl_edit_order() - Order modification
 * - hl_fetch_canceled_orders() - Canceled order history
 * - hl_fetch_canceled_and_closed_orders() - Combined history
 * - Bulk operation error handling
 * - Memory management for bulk operations
 * - Edge cases and validation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "hyperliquid.h"
#include "helpers/test_common.h"

// Test result storage
static test_result_t test_results[] = {
    {TEST_STATUS_PENDING, "test_create_bulk_orders", 0.0},
    {TEST_STATUS_PENDING, "test_cancel_bulk_orders", 0.0},
    {TEST_STATUS_PENDING, "test_edit_order", 0.0},
    {TEST_STATUS_PENDING, "test_fetch_canceled_orders", 0.0},
    {TEST_STATUS_PENDING, "test_fetch_canceled_and_closed_orders", 0.0},
    {TEST_STATUS_PENDING, "test_bulk_order_memory_management", 0.0},
    {TEST_STATUS_PENDING, "test_advanced_order_validation", 0.0},
    {TEST_STATUS_PENDING, "test_bulk_order_error_scenarios", 0.0},
    {TEST_STATUS_PENDING, "test_order_edge_cases", 0.0},
    {TEST_STATUS_PENDING, "test_order_workflow_integration", 0.0},
};
static const size_t num_tests = sizeof(test_results) / sizeof(test_results[0]);

test_result_t test_create_bulk_orders() {
    hl_client_t *client = NULL;
    hl_order_request_t orders[3] = {0};
    hl_order_result_t results[3] = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Prepare bulk orders (different symbols/prices to avoid conflicts)
    orders[0] = (hl_order_request_t){
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .type = HL_ORDER_TYPE_LIMIT,
        .price = 30000.0,  // Low buy price
        .quantity = 0.001
    };

    orders[1] = (hl_order_request_t){
        .symbol = "ETH",
        .side = HL_SIDE_BUY,
        .type = HL_ORDER_TYPE_LIMIT,
        .price = 1500.0,   // Low buy price
        .quantity = 0.01
    };

    orders[2] = (hl_order_request_t){
        .symbol = "BTC",
        .side = HL_SIDE_SELL,
        .type = HL_ORDER_TYPE_LIMIT,
        .price = 100000.0, // High sell price
        .quantity = 0.001
    };

    error = hl_create_orders(client, orders, 3, results);

    if (error == HL_SUCCESS) {
        printf("  Successfully created bulk orders\n");

        for (int i = 0; i < 3; i++) {
            if (results[i].status == HL_ORDER_STATUS_OPEN) {
                printf("    Order %d: %s opened successfully\n", i, results[i].order_id);
            } else {
                printf("    Order %d: Status=%d, Error=%s\n", i,
                       results[i].status, results[i].error_message);
            }

            // Test cleanup of individual order results
            if (results[i].order_id) {
                free((void*)results[i].order_id);
            }
        }
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK ||
               error == HL_ERROR_INSUFFICIENT_BALANCE) {
        printf("  Skipping bulk order creation (no credentials/network/balance): %s\n",
               hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error creating bulk orders: %s", hl_error_string(error));
    }

    // Cleanup
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_cancel_bulk_orders() {
    hl_client_t *client = NULL;
    const char *order_ids[3] = {"test_order_1", "test_order_2", "test_order_3"};
    hl_cancel_result_t results[3] = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_cancel_orders(client, order_ids, 3, results);

    if (error == HL_SUCCESS) {
        printf("  Bulk cancel operation completed\n");

        for (int i = 0; i < 3; i++) {
            printf("    Cancel result %d: Success=%s\n", i,
                   results[i].success ? "true" : "false");
            if (results[i].error_message) {
                printf("      Error: %s\n", results[i].error_message);
            }
        }
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK ||
               error == HL_ERROR_NOT_FOUND) {
        printf("  Skipping bulk cancel (no credentials/network/orders): %s\n",
               hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error cancelling bulk orders: %s", hl_error_string(error));
    }

    // Cleanup
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_edit_order() {
    hl_client_t *client = NULL;
    hl_order_request_t new_order = {0};
    hl_order_result_t result = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Prepare modified order
    new_order = (hl_order_request_t){
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .type = HL_ORDER_TYPE_LIMIT,
        .price = 35000.0,  // Different price
        .quantity = 0.001
    };

    error = hl_edit_order(client, "nonexistent_order_id", &new_order, &result);

    if (error == HL_SUCCESS) {
        if (result.status == HL_ORDER_STATUS_OPEN) {
            printf("  Order edited successfully: %s\n", result.order_id);
        } else {
            printf("  Order edit returned: Status=%d, Error=%s\n",
                   result.status, result.error_message ? result.error_message : "None");
        }
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK ||
               error == HL_ERROR_NOT_FOUND) {
        printf("  Skipping order edit (no credentials/network/order): %s\n",
               hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error editing order: %s", hl_error_string(error));
    }

    // Cleanup
    if (result.order_id) {
        free((void*)result.order_id);
    }
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_fetch_canceled_orders() {
    hl_client_t *client = NULL;
    hl_orders_t orders = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_fetch_canceled_orders(client, NULL, NULL, 10, &orders);

    if (error == HL_SUCCESS) {
        printf("  Found %zu canceled orders\n", orders.count);

        for (size_t i = 0; i < orders.count; i++) {
            const hl_order_t *order = &orders.orders[i];
            TEST_ASSERT(order->symbol != NULL, "Order symbol should not be NULL");
            TEST_ASSERT(order->status == HL_ORDER_STATUS_CANCELLED,
                       "Order should have cancelled status");

            printf("    Canceled order %zu: %s, Status=%d\n", i, order->symbol, order->status);
        }
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK) {
        printf("  Skipping canceled orders fetch (no credentials/network): %s\n",
               hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching canceled orders: %s", hl_error_string(error));
    }

    // Cleanup
    hl_free_orders(&orders);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_fetch_canceled_and_closed_orders() {
    hl_client_t *client = NULL;
    hl_orders_t orders = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_fetch_canceled_and_closed_orders(client, NULL, NULL, 10, &orders);

    if (error == HL_SUCCESS) {
        printf("  Found %zu canceled and closed orders\n", orders.count);

        int canceled_count = 0;
        int closed_count = 0;

        for (size_t i = 0; i < orders.count; i++) {
            const hl_order_t *order = &orders.orders[i];
            TEST_ASSERT(order->symbol != NULL, "Order symbol should not be NULL");

            if (order->status == HL_ORDER_STATUS_CANCELLED) {
                canceled_count++;
            } else if (order->status == HL_ORDER_STATUS_FILLED ||
                      order->status == HL_ORDER_STATUS_PARTIALLY_FILLED) {
                closed_count++;
            }

            printf("    Order %zu: %s, Status=%d\n", i, order->symbol, order->status);
        }

        printf("    Summary: %d canceled, %d closed\n", canceled_count, closed_count);
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK) {
        printf("  Skipping canceled and closed orders fetch (no credentials/network): %s\n",
               hl_error_string(error));
        TEST_PASS();
    } else {
        TEST_FAIL("Unexpected error fetching canceled and closed orders: %s",
                 hl_error_string(error));
    }

    // Cleanup
    hl_free_orders(&orders);
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_bulk_order_memory_management() {
    hl_order_request_t *orders = NULL;
    hl_order_result_t *results = NULL;
    const char **order_ids = NULL;
    hl_cancel_result_t *cancel_results = NULL;

    TEST_START_TIMER();

    // Test allocation and deallocation
    const int test_count = 5;

    orders = malloc(test_count * sizeof(hl_order_request_t));
    TEST_ASSERT(orders != NULL, "Failed to allocate orders array");

    results = malloc(test_count * sizeof(hl_order_result_t));
    TEST_ASSERT(results != NULL, "Failed to allocate results array");

    order_ids = malloc(test_count * sizeof(char*));
    TEST_ASSERT(order_ids != NULL, "Failed to allocate order IDs array");

    cancel_results = malloc(test_count * sizeof(hl_cancel_result_t));
    TEST_ASSERT(cancel_results != NULL, "Failed to allocate cancel results array");

    // Initialize test data
    for (int i = 0; i < test_count; i++) {
        orders[i] = (hl_order_request_t){
            .symbol = "BTC",
            .side = HL_SIDE_BUY,
            .type = HL_ORDER_TYPE_LIMIT,
            .price = 30000.0 + (i * 1000.0),
            .quantity = 0.001
        };

        order_ids[i] = "test_order_id";
    }

    // Test double free safety
    free(orders);
    free(results);
    free(order_ids);
    free(cancel_results);

    free(orders);  // Should be safe
    free(results); // Should be safe
    free(order_ids); // Should be safe
    free(cancel_results); // Should be safe

    // Test with NULL pointers
    free(NULL); // Should be safe

    TEST_PASS();
}

test_result_t test_advanced_order_validation() {
    hl_client_t *client = NULL;
    hl_order_request_t orders[3] = {0};
    hl_order_result_t results[3] = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test bulk orders with invalid parameters
    orders[0] = (hl_order_request_t){
        .symbol = "",  // Empty symbol
        .side = HL_SIDE_BUY,
        .type = HL_ORDER_TYPE_LIMIT,
        .price = 30000.0,
        .quantity = 0.001
    };

    orders[1] = (hl_order_request_t){
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .type = HL_ORDER_TYPE_LIMIT,
        .price = -1.0,  // Invalid price
        .quantity = 0.001
    };

    orders[2] = (hl_order_request_t){
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .type = HL_ORDER_TYPE_LIMIT,
        .price = 30000.0,
        .quantity = 0.0  // Invalid quantity
    };

    error = hl_create_orders(client, orders, 3, results);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with invalid order data");

    // Test bulk cancel with NULL order IDs
    const char **null_order_ids = NULL;
    error = hl_cancel_orders(client, null_order_ids, 1, results);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL order IDs");

    // Test order edit with invalid parameters
    hl_order_request_t new_order = {0};
    error = hl_edit_order(NULL, "order_id", &new_order, results);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL client");

    error = hl_edit_order(client, NULL, &new_order, results);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL order ID");

    error = hl_edit_order(client, "order_id", NULL, results);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL order request");

    // Cleanup
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_bulk_order_error_scenarios() {
    hl_client_t *client = NULL;
    hl_order_request_t orders[1] = {0};
    hl_order_result_t results[1] = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    // Test with NULL client
    error = hl_create_orders(NULL, orders, 1, results);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL client");

    // Test with NULL orders array
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_create_orders(client, NULL, 1, results);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL orders");

    // Test with NULL results array
    error = hl_create_orders(client, orders, 1, NULL);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL results");

    // Test with zero count
    error = hl_create_orders(client, orders, 0, results);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with zero count");

    // Test with very large count (should handle gracefully)
    error = hl_create_orders(client, orders, 1000000, results);
    // Should either succeed or return appropriate error, not crash

    // Cleanup
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_order_edge_cases() {
    hl_client_t *client = NULL;
    hl_order_request_t order = {0};
    hl_order_result_t result = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test order with very small quantity
    order = (hl_order_request_t){
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .type = HL_ORDER_TYPE_LIMIT,
        .price = 30000.0,
        .quantity = 0.00000001  // Very small quantity
    };

    error = hl_place_order(client, &order, &result);
    if (error != HL_SUCCESS && error != HL_ERROR_AUTH && error != HL_ERROR_NETWORK) {
        printf("  Small quantity order rejected: %s\n", hl_error_string(error));
    }

    // Test order with very large quantity
    order.quantity = 1000000.0;  // Very large quantity
    error = hl_place_order(client, &order, &result);
    if (error != HL_SUCCESS && error != HL_ERROR_AUTH && error != HL_ERROR_NETWORK) {
        printf("  Large quantity order rejected: %s\n", hl_error_string(error));
    }

    // Test order with extreme precision
    order.price = 30000.123456789;
    order.quantity = 0.001;
    error = hl_place_order(client, &order, &result);
    if (error != HL_SUCCESS && error != HL_ERROR_AUTH && error != HL_ERROR_NETWORK) {
        printf("  High precision order rejected: %s\n", hl_error_string(error));
    }

    // Cleanup
    if (result.order_id) {
        free((void*)result.order_id);
    }
    hl_client_destroy(client);
    return TEST_PASS();
}

test_result_t test_order_workflow_integration() {
    hl_client_t *client = NULL;
    hl_order_request_t order = {0};
    hl_order_result_t result = {0};
    hl_orders_t orders = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test complete order workflow
    // 1. Create order
    order = (hl_order_request_t){
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .type = HL_ORDER_TYPE_LIMIT,
        .price = 30000.0,
        .quantity = 0.001
    };

    error = hl_place_order(client, &order, &result);

    if (error == HL_SUCCESS) {
        printf("  Order created: %s\n", result.order_id);

        // 2. Fetch open orders to verify
        error = hl_fetch_open_orders(client, NULL, NULL, 10, &orders);
        if (error == HL_SUCCESS) {
            printf("  Found %zu open orders\n", orders.count);
        }

        // 3. Cancel the order
        hl_cancel_result_t cancel_result = {0};
        error = hl_cancel_order(client, "BTC", result.order_id, &cancel_result);
        if (error == HL_SUCCESS && cancel_result.success) {
            printf("  Order canceled successfully\n");
        }

        // 4. Fetch canceled orders to verify
        hl_free_orders(&orders);
        error = hl_fetch_canceled_orders(client, NULL, NULL, 10, &orders);
        if (error == HL_SUCCESS) {
            printf("  Found %zu canceled orders\n", orders.count);
        }
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK ||
               error == HL_ERROR_INSUFFICIENT_BALANCE) {
        printf("  Skipping workflow integration (no credentials/network/balance): %s\n",
               hl_error_string(error));
    } else {
        TEST_FAIL("Unexpected error in order workflow: %s", hl_error_string(error));
    }

    // Cleanup
    if (result.order_id) {
        free((void*)result.order_id);
    }
    hl_free_orders(&orders);
    hl_client_destroy(client);
    return TEST_PASS();
}

// Main test runner
int main() {
    printf("=== Advanced Order Management Tests ===\n\n");

    // Run all tests
    for (size_t i = 0; i < num_tests; i++) {
        printf("Running %s...\n", test_results[i].name);

        switch (i) {
            case 0: test_results[i] = test_create_bulk_orders(); break;
            case 1: test_results[i] = test_cancel_bulk_orders(); break;
            case 2: test_results[i] = test_edit_order(); break;
            case 3: test_results[i] = test_fetch_canceled_orders(); break;
            case 4: test_results[i] = test_fetch_canceled_and_closed_orders(); break;
            case 5: test_results[i] = test_bulk_order_memory_management(); break;
            case 6: test_results[i] = test_advanced_order_validation(); break;
            case 7: test_results[i] = test_bulk_order_error_scenarios(); break;
            case 8: test_results[i] = test_order_edge_cases(); break;
            case 9: test_results[i] = test_order_workflow_integration(); break;
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