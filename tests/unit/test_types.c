/**
 * @file test_types.c
 * @brief Unit tests for data types and structures
 */

#include "../helpers/test_common.h"

/**
 * @brief Test order creation and validation
 */
test_result_t test_order_types(void) {
    // Test order request creation
    hl_order_request_api_t order = {
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .price = 50000.0,
        .quantity = 0.001,
        .order_type = HL_ORDER_TYPE_LIMIT,
        .time_in_force = HL_TIF_GTC,
        .reduce_only = false,
        .slippage_bps = 10
    };

    test_assert(order.side == HL_SIDE_BUY, "Order side");
    test_assert(order.price == 50000.0, "Order price");
    test_assert(order.quantity == 0.001, "Order quantity");
    test_assert(order.order_type == HL_ORDER_TYPE_LIMIT, "Order type");
    test_assert(order.time_in_force == HL_TIF_GTC, "Time in force");
    test_assert(!order.reduce_only, "Reduce only flag");
    test_assert(order.slippage_bps == 10, "Slippage BPS");

    printf("✅ Order types test passed\n");
    return TEST_PASS;
}

/**
 * @brief Test order result structure
 */
test_result_t test_order_result_types(void) {
    hl_order_result_t result = {0};

    // Initialize result
    result.order_id = NULL;
    result.status = HL_ORDER_STATUS_OPEN;
    result.filled_quantity = 0.0;
    result.average_price = 0.0;
    strcpy(result.error, "");

    test_assert(result.status == HL_ORDER_STATUS_OPEN, "Order status");
    test_assert(result.filled_quantity == 0.0, "Filled quantity");
    test_assert(result.average_price == 0.0, "Average price");
    test_assert(strlen(result.error) == 0, "Error message empty");

    printf("✅ Order result types test passed\n");
    return TEST_PASS;
}

/**
 * @brief Test cancel result structure
 */
test_result_t test_cancel_result_types(void) {
    hl_cancel_result_t result = {0};

    // Initialize result
    result.cancelled = true;
    strcpy(result.error, "");

    test_assert(result.cancelled, "Cancel status");
    test_assert(strlen(result.error) == 0, "Error message empty");

    printf("✅ Cancel result types test passed\n");
    return TEST_PASS;
}

/**
 * @brief Test enum values
 */
test_result_t test_enum_values(void) {
    // Test side enum
    test_assert(HL_SIDE_BUY == 0, "HL_SIDE_BUY value");
    test_assert(HL_SIDE_SELL == 1, "HL_SIDE_SELL value");

    // Test order type enum
    test_assert(HL_ORDER_TYPE_LIMIT == 0, "HL_ORDER_TYPE_LIMIT value");
    test_assert(HL_ORDER_TYPE_MARKET == 1, "HL_ORDER_TYPE_MARKET value");

    // Test time in force enum
    test_assert(HL_TIF_GTC == 0, "HL_TIF_GTC value");
    test_assert(HL_TIF_IOC == 1, "HL_TIF_IOC value");
    test_assert(HL_TIF_ALO == 2, "HL_TIF_ALO value");

    // Test order status enum
    test_assert(HL_ORDER_STATUS_OPEN == 0, "HL_ORDER_STATUS_OPEN value");
    test_assert(HL_ORDER_STATUS_FILLED == 1, "HL_ORDER_STATUS_FILLED value");
    test_assert(HL_ORDER_STATUS_PARTIALLY_FILLED == 2, "HL_ORDER_STATUS_PARTIALLY_FILLED value");
    test_assert(HL_ORDER_STATUS_CANCELLED == 3, "HL_ORDER_STATUS_CANCELLED value");
    test_assert(HL_ORDER_STATUS_REJECTED == 4, "HL_ORDER_STATUS_REJECTED value");

    printf("✅ Enum values test passed\n");
    return TEST_PASS;
}

/**
 * @brief Test market order creation
 */
test_result_t test_market_order_types(void) {
    hl_order_request_api_t market_order = {
        .symbol = "ETH",
        .side = HL_SIDE_SELL,
        .price = 0.0,  // Market orders have 0 price
        .quantity = 1.0,
        .order_type = HL_ORDER_TYPE_MARKET,
        .time_in_force = HL_TIF_IOC,
        .reduce_only = false,
        .slippage_bps = 50
    };

    test_assert(market_order.order_type == HL_ORDER_TYPE_MARKET, "Market order type");
    test_assert(market_order.price == 0.0, "Market order price is 0");
    test_assert(market_order.time_in_force == HL_TIF_IOC, "Market order uses IOC");
    test_assert(market_order.slippage_bps == 50, "Slippage for market order");

    printf("✅ Market order types test passed\n");
    return TEST_PASS;
}

/**
 * @brief Test reduce-only order
 */
test_result_t test_reduce_only_order_types(void) {
    hl_order_request_api_t reduce_order = {
        .symbol = "SOL",
        .side = HL_SIDE_SELL,
        .price = 150.0,
        .quantity = 10.0,
        .order_type = HL_ORDER_TYPE_LIMIT,
        .time_in_force = HL_TIF_GTC,
        .reduce_only = true,
        .slippage_bps = 0
    };

    test_assert(reduce_order.reduce_only, "Reduce only flag set");
    test_assert(reduce_order.side == HL_SIDE_SELL, "Reduce only is sell order");
    test_assert(reduce_order.slippage_bps == 0, "No slippage for limit order");

    printf("✅ Reduce only order types test passed\n");
    return TEST_PASS;
}

int main(void) {
    test_load_env();

    printf("╔══════════════════════════════════════════╗\n");
    printf("║  UNIT TESTS: Data Types & Structures      ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    test_func_t tests[] = {
        test_order_types,
        test_order_result_types,
        test_cancel_result_types,
        test_enum_values,
        test_market_order_types,
        test_reduce_only_order_types
    };

    return test_run_suite("Types Unit Tests", tests, sizeof(tests)/sizeof(test_func_t));
}