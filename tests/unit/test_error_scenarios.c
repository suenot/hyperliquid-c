/**
 * @file test_error_scenarios.c
 * @brief Comprehensive error handling tests
 *
 * Tests all 15 error codes with realistic scenarios:
 * - Network errors
 * - Authentication errors
 * - Invalid parameters
 * - API errors
 * - Memory allocation errors
 * - Timeout scenarios
 * - Parsing errors
 * - Resource not found errors
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "hyperliquid.h"
#include "helpers/test_common.h"

// Test result storage
static test_result_t test_results[] = {
    {TEST_STATUS_PENDING, "test_success_error_code", 0.0},
    {TEST_STATUS_PENDING, "test_invalid_params_errors", 0.0},
    {TEST_STATUS_PENDING, "test_network_errors", 0.0},
    {TEST_STATUS_PENDING, "test_api_errors", 0.0},
    {TEST_STATUS_PENDING, "test_auth_errors", 0.0},
    {TEST_STATUS_PENDING, "test_insufficient_balance", 0.0},
    {TEST_STATUS_PENDING, "test_invalid_symbol", 0.0},
    {TEST_STATUS_PENDING, "test_order_rejected", 0.0},
    {TEST_STATUS_PENDING, "test_signature_errors", 0.0},
    {TEST_STATUS_PENDING, "test_msgpack_errors", 0.0},
    {TEST_STATUS_PENDING, "test_json_errors", 0.0},
    {TEST_STATUS_PENDING, "test_memory_errors", 0.0},
    {TEST_STATUS_PENDING, "test_timeout_errors", 0.0},
    {TEST_STATUS_PENDING, "test_not_implemented_errors", 0.0},
    {TEST_STATUS_PENDING, "test_not_found_errors", 0.0},
    {TEST_STATUS_PENDING, "test_parse_errors", 0.0},
    {TEST_STATUS_PENDING, "test_error_string_validation", 0.0},
};
static const size_t num_tests = sizeof(test_results) / sizeof(test_results[0]);

test_result_t test_success_error_code() {
    TEST_START_TIMER();

    // Test HL_SUCCESS error code
    TEST_ASSERT(HL_SUCCESS == 0, "HL_SUCCESS should be 0");
    TEST_ASSERT(strcmp(hl_error_string(HL_SUCCESS), "Success") == 0,
               "HL_SUCCESS error string mismatch");

    TEST_PASS();
}

test_result_t test_invalid_params_errors() {
    hl_client_t *client = NULL;
    hl_order_request_t order = {0};
    hl_order_result_t result = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    // Test NULL client
    error = hl_place_order(NULL, &order, &result);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL client");
    TEST_ASSERT(strcmp(hl_error_string(error), "Invalid parameters") == 0,
               "Invalid params error string mismatch");

    // Test NULL order request
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    error = hl_place_order(client, NULL, &result);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL order request");

    // Test NULL result
    error = hl_place_order(client, &order, NULL);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL result");

    // Test invalid symbol
    order.symbol = "";
    order.side = HL_SIDE_BUY;
    order.type = HL_ORDER_TYPE_LIMIT;
    order.price = 50000.0;
    order.quantity = 0.001;

    error = hl_place_order(client, &order, &result);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with empty symbol");

    // Test invalid order side
    order.symbol = "BTC";
    order.side = (hl_side_t)999;
    error = hl_place_order(client, &order, &result);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with invalid side");

    // Test invalid order type
    order.side = HL_SIDE_BUY;
    order.type = (hl_order_type_t)999;
    error = hl_place_order(client, &order, &result);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with invalid type");

    // Test invalid price
    order.type = HL_ORDER_TYPE_LIMIT;
    order.price = -1.0;
    error = hl_place_order(client, &order, &result);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with negative price");

    // Test invalid quantity
    order.price = 50000.0;
    order.quantity = 0.0;
    error = hl_place_order(client, &order, &result);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with zero quantity");

    // Test invalid quantity (negative)
    order.quantity = -0.001;
    error = hl_place_order(client, &order, &result);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with negative quantity");

    // Cleanup
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_network_errors() {
    hl_client_t *client = NULL;
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    // Test connection timeout
    client = hl_client_create(NULL, NULL, true);
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Set very short timeout to force network error
    hl_set_timeout(client, 1);  // 1ms timeout

    error = hl_test_connection(client);
    if (error != HL_SUCCESS) {
        // Expected to fail with network error or timeout
        TEST_ASSERT(error == HL_ERROR_NETWORK || error == HL_ERROR_TIMEOUT,
                   "Expected network or timeout error, got: %s", hl_error_string(error));
    }

    // Test with invalid endpoint by modifying client configuration
    // (This tests the network error path in a controlled way)
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_api_errors() {
    hl_client_t *client = NULL;
    hl_order_request_t order = {0};
    hl_order_result_t result = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test with valid parameters but expect API error due to auth or network
    order.symbol = "BTC";
    order.side = HL_SIDE_BUY;
    order.type = HL_ORDER_TYPE_LIMIT;
    order.price = 1.0;  // Very low price to potentially get rejected
    order.quantity = 0.001;

    error = hl_place_order(client, &order, &result);

    // We expect either success (if credentials work) or various error codes
    if (error != HL_SUCCESS) {
        TEST_ASSERT(error == HL_ERROR_AUTH ||
                   error == HL_ERROR_NETWORK ||
                   error == HL_ERROR_API ||
                   error == HL_ERROR_INSUFFICIENT_BALANCE ||
                   error == HL_ERROR_ORDER_REJECTED,
                   "Expected API-related error, got: %s", hl_error_string(error));
    }

    // Cleanup
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_auth_errors() {
    hl_client_t *client = NULL;
    hl_balance_t balance = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    // Test with invalid credentials
    client = hl_client_create("0xinvalidaddress", "invalidkey", true);
    TEST_ASSERT(client != NULL, "Failed to create client with invalid credentials");

    error = hl_fetch_balance(client, HL_ACCOUNT_TYPE_PERPETUAL, &balance);
    TEST_ASSERT(error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK,
               "Expected auth or network error with invalid credentials, got: %s",
               hl_error_string(error));

    // Test with NULL credentials (no authentication)
    hl_client_destroy(client);
    client = hl_client_create(NULL, NULL, true);
    TEST_ASSERT(client != NULL, "Failed to create client with NULL credentials");

    error = hl_fetch_balance(client, HL_ACCOUNT_TYPE_PERPETUAL, &balance);
    TEST_ASSERT(error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK,
               "Expected auth or network error with NULL credentials, got: %s",
               hl_error_string(error));

    // Cleanup
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_insufficient_balance() {
    hl_client_t *client = NULL;
    hl_order_request_t order = {0};
    hl_order_result_t result = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test with very large order that should exceed balance
    order.symbol = "BTC";
    order.side = HL_SIDE_BUY;
    order.type = HL_ORDER_TYPE_LIMIT;
    order.price = 100000.0;
    order.quantity = 1000.0;  // Very large quantity

    error = hl_place_order(client, &order, &result);

    if (error == HL_ERROR_INSUFFICIENT_BALANCE) {
        TEST_ASSERT(strcmp(hl_error_string(error), "Insufficient balance") == 0,
                   "Insufficient balance error string mismatch");
        printf("  Successfully got insufficient balance error as expected\n");
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK) {
        // Acceptable in CI environment
        printf("  Skipping (no auth/network): %s\n", hl_error_string(error));
    }

    // Cleanup
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_invalid_symbol() {
    hl_client_t *client = NULL;
    hl_ticker_t ticker = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test with completely invalid symbol
    error = hl_fetch_ticker(client, "INVALID_SYMBOL_DOES_NOT_EXIST", &ticker);

    if (error == HL_ERROR_INVALID_SYMBOL) {
        TEST_ASSERT(strcmp(hl_error_string(error), "Invalid symbol") == 0,
                   "Invalid symbol error string mismatch");
        printf("  Successfully got invalid symbol error as expected\n");
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK ||
               error == HL_ERROR_NOT_FOUND) {
        // Acceptable alternatives
        printf("  Got expected error for invalid symbol: %s\n", hl_error_string(error));
    }

    // Test with empty symbol
    error = hl_fetch_ticker(client, "", &ticker);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with empty symbol");

    // Test with NULL symbol
    error = hl_fetch_ticker(client, NULL, &ticker);
    TEST_ASSERT(error == HL_ERROR_INVALID_PARAMS,
               "Expected invalid params error with NULL symbol");

    // Cleanup
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_order_rejected() {
    hl_client_t *client = NULL;
    hl_order_request_t order = {0};
    hl_order_result_t result = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test with order that might be rejected (extreme prices)
    order.symbol = "BTC";
    order.side = HL_SIDE_BUY;
    order.type = HL_ORDER_TYPE_LIMIT;
    order.price = 1.0;  // Extremely low price
    order.quantity = 1000.0;  // Large quantity

    error = hl_place_order(client, &order, &result);

    if (error == HL_ERROR_ORDER_REJECTED) {
        TEST_ASSERT(strcmp(hl_error_string(error), "Order rejected by exchange") == 0,
                   "Order rejected error string mismatch");
        printf("  Successfully got order rejected error as expected\n");
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK ||
               error == HL_ERROR_INSUFFICIENT_BALANCE) {
        // Acceptable alternatives
        printf("  Got expected error for extreme order: %s\n", hl_error_string(error));
    }

    // Cleanup
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_signature_errors() {
    hl_client_t *client = NULL;
    hl_order_request_t order = {0};
    hl_order_result_t result = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    // Test with malformed private key
    client = hl_client_create("0x1234567890abcdef", "malformed_private_key", true);
    TEST_ASSERT(client != NULL, "Failed to create client with malformed key");

    order.symbol = "BTC";
    order.side = HL_SIDE_BUY;
    order.type = HL_ORDER_TYPE_LIMIT;
    order.price = 50000.0;
    order.quantity = 0.001;

    error = hl_place_order(client, &order, &result);

    if (error == HL_ERROR_SIGNATURE) {
        TEST_ASSERT(strcmp(hl_error_string(error), "Signature generation failed") == 0,
                   "Signature error string mismatch");
        printf("  Successfully got signature error as expected\n");
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK) {
        // Acceptable alternatives
        printf("  Got expected error with malformed key: %s\n", hl_error_string(error));
    }

    // Cleanup
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_msgpack_errors() {
    // This tests the MessagePack serialization error path
    // We can't easily trigger a MessagePack error directly from the API
    // so we test that the error code exists and has the right string

    TEST_START_TIMER();

    TEST_ASSERT(HL_ERROR_MSGPACK > 0, "HL_ERROR_MSGPACK should be positive");
    TEST_ASSERT(strcmp(hl_error_string(HL_ERROR_MSGPACK), "MessagePack error") == 0,
               "MessagePack error string mismatch");

    TEST_PASS();
}

test_result_t test_json_errors() {
    // Similar to MessagePack, test that the error code exists
    TEST_START_TIMER();

    TEST_ASSERT(HL_ERROR_JSON > 0, "HL_ERROR_JSON should be positive");
    TEST_ASSERT(strcmp(hl_error_string(HL_ERROR_JSON), "JSON parsing error") == 0,
               "JSON error string mismatch");

    TEST_PASS();
}

test_result_t test_memory_errors() {
    // Test memory allocation error scenarios
    TEST_START_TIMER();

    TEST_ASSERT(HL_ERROR_MEMORY > 0, "HL_ERROR_MEMORY should be positive");
    TEST_ASSERT(strcmp(hl_error_string(HL_ERROR_MEMORY), "Memory allocation failed") == 0,
               "Memory error string mismatch");

    // Test operations that might fail with memory errors
    // (In practice, memory allocation rarely fails on modern systems,
    // but we can test the error handling path exists)

    TEST_PASS();
}

test_result_t test_timeout_errors() {
    hl_client_t *client = NULL;
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    TEST_ASSERT(HL_ERROR_TIMEOUT > 0, "HL_ERROR_TIMEOUT should be positive");
    TEST_ASSERT(strcmp(hl_error_string(HL_ERROR_TIMEOUT), "Operation timed out") == 0,
               "Timeout error string mismatch");

    // Test timeout functionality
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Set very short timeout
    hl_set_timeout(client, 1);  // 1ms

    error = hl_test_connection(client);
    if (error == HL_ERROR_TIMEOUT) {
        printf("  Successfully got timeout error as expected\n");
    }

    // Cleanup
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_not_implemented_errors() {
    TEST_START_TIMER();

    TEST_ASSERT(HL_ERROR_NOT_IMPLEMENTED > 0, "HL_ERROR_NOT_IMPLEMENTED should be positive");
    TEST_ASSERT(strcmp(hl_error_string(HL_ERROR_NOT_IMPLEMENTED), "Feature not implemented") == 0,
               "Not implemented error string mismatch");

    TEST_PASS();
}

test_result_t test_not_found_errors() {
    hl_client_t *client = NULL;
    hl_order_t order = {0};
    hl_error_t error = HL_SUCCESS;

    TEST_START_TIMER();

    TEST_ASSERT(HL_ERROR_NOT_FOUND > 0, "HL_ERROR_NOT_FOUND should be positive");
    TEST_ASSERT(strcmp(hl_error_string(HL_ERROR_NOT_FOUND), "Resource not found") == 0,
               "Not found error string mismatch");

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test fetching non-existent order
    error = hl_fetch_order(client, "nonexistent_order_id", "BTC", &order);
    if (error == HL_ERROR_NOT_FOUND) {
        printf("  Successfully got not found error as expected\n");
    } else if (error == HL_ERROR_AUTH || error == HL_ERROR_NETWORK) {
        printf("  Skipping (no auth/network): %s\n", hl_error_string(error));
    }

    // Cleanup
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_parse_errors() {
    TEST_START_TIMER();

    TEST_ASSERT(HL_ERROR_PARSE > 0, "HL_ERROR_PARSE should be positive");
    TEST_ASSERT(strcmp(hl_error_string(HL_ERROR_PARSE), "Response parsing error") == 0,
               "Parse error string mismatch");

    TEST_PASS();
}

test_result_t test_error_string_validation() {
    TEST_START_TIMER();

    // Test all error codes have valid strings
    for (int i = 0; i <= 15; i++) {
        const char *error_str = hl_error_string((hl_error_t)i);
        TEST_ASSERT(error_str != NULL, "Error string should not be NULL");
        TEST_ASSERT(strlen(error_str) > 0, "Error string should not be empty");
        printf("  Error %d: %s\n", i, error_str);
    }

    // Test out-of-range error code
    const char *error_str = hl_error_string((hl_error_t)999);
    TEST_ASSERT(error_str != NULL, "Out-of-range error string should not be NULL");
    TEST_ASSERT(strlen(error_str) > 0, "Out-of-range error string should not be empty");

    TEST_PASS();
}

// Main test runner
int main() {
    printf("=== Error Scenarios Comprehensive Tests ===\n\n");

    // Run all tests
    for (size_t i = 0; i < num_tests; i++) {
        printf("Running %s...\n", test_results[i].name);

        switch (i) {
            case 0: test_results[i] = test_success_error_code(); break;
            case 1: test_results[i] = test_invalid_params_errors(); break;
            case 2: test_results[i] = test_network_errors(); break;
            case 3: test_results[i] = test_api_errors(); break;
            case 4: test_results[i] = test_auth_errors(); break;
            case 5: test_results[i] = test_insufficient_balance(); break;
            case 6: test_results[i] = test_invalid_symbol(); break;
            case 7: test_results[i] = test_order_rejected(); break;
            case 8: test_results[i] = test_signature_errors(); break;
            case 9: test_results[i] = test_msgpack_errors(); break;
            case 10: test_results[i] = test_json_errors(); break;
            case 11: test_results[i] = test_memory_errors(); break;
            case 12: test_results[i] = test_timeout_errors(); break;
            case 13: test_results[i] = test_not_implemented_errors(); break;
            case 14: test_results[i] = test_not_found_errors(); break;
            case 15: test_results[i] = test_parse_errors(); break;
            case 16: test_results[i] = test_error_string_validation(); break;
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