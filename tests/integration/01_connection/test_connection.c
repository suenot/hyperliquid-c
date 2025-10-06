/**
 * @file test_connection.c
 * @brief Integration tests for API connection
 */

#include "../../helpers/test_common.h"

/**
 * @brief Test basic connection to testnet
 */
test_result_t test_connection_testnet(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        printf("⚠️  No test credentials available, skipping connection test\n");
        return TEST_SKIP;
    }

    // Test connection
    bool connected = hl_test_connection(client);
    test_assert(connected, "Connection to testnet");

    test_destroy_client(client);
    printf("✅ Testnet connection test passed\n");
    return TEST_PASS;
}

/**
 * @brief Test connection timeout setting
 */
test_result_t test_connection_timeout(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Set timeout to 5 seconds
    hl_set_timeout(client, 5000);

    // Test connection with timeout
    bool connected = hl_test_connection(client);
    test_assert(connected, "Connection with timeout");

    test_destroy_client(client);
    printf("✅ Connection timeout test passed\n");
    return TEST_PASS;
}

/**
 * @brief Test multiple connections
 */
test_result_t test_multiple_connections(void) {
    hl_client_t* client1 = test_create_client(true);
    hl_client_t* client2 = test_create_client(true);

    if (!client1 || !client2) {
        if (client1) test_destroy_client(client1);
        if (client2) test_destroy_client(client2);
        return TEST_SKIP;
    }

    // Test both connections work
    bool connected1 = hl_test_connection(client1);
    bool connected2 = hl_test_connection(client2);

    test_assert(connected1, "First client connection");
    test_assert(connected2, "Second client connection");

    test_destroy_client(client1);
    test_destroy_client(client2);
    printf("✅ Multiple connections test passed\n");
    return TEST_PASS;
}

/**
 * @brief Test connection with debug mode
 */
test_result_t test_connection_debug(void) {
    hl_client_t* client = test_create_client(true);
    if (!client) {
        return TEST_SKIP;
    }

    // Enable debug mode
    hl_set_debug(true);

    // Test connection in debug mode
    bool connected = hl_test_connection(client);
    test_assert(connected, "Connection in debug mode");

    // Disable debug mode
    hl_set_debug(false);

    test_destroy_client(client);
    printf("✅ Debug mode connection test passed\n");
    return TEST_PASS;
}

int main(void) {
    test_load_env();

    printf("╔══════════════════════════════════════════╗\n");
    printf("║  INTEGRATION TESTS: Connection           ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    test_func_t tests[] = {
        test_connection_testnet,
        test_connection_timeout,
        test_multiple_connections,
        test_connection_debug
    };

    return test_run_suite("Connection Integration Tests", tests, sizeof(tests)/sizeof(test_func_t));
}