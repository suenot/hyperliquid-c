/**
 * @file test_client.c
 * @brief Unit tests for client management
 */

#include "../helpers/test_common.h"

/**
 * @brief Test client creation and destruction
 */
test_result_t test_client_create_destroy(void) {
    const char* wallet_address = test_get_env("HYPERLIQUID_TESTNET_WALLET_ADDRESS", "0x1234567890123456789012345678901234567890");
    const char* private_key = test_get_env("HYPERLIQUID_TESTNET_PRIVATE_KEY", "abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890");

    // Test client creation
    hl_client_t* client = hl_client_create(wallet_address, private_key, true);
    test_assert_not_null(client, "Client creation");

    // Test client destruction
    hl_client_destroy(client);

    return TEST_PASS;
}

/**
 * @brief Test client creation with invalid parameters
 */
test_result_t test_client_invalid_params(void) {
    // Test NULL wallet address
    hl_client_t* client1 = hl_client_create(NULL, "abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890", true);
    test_assert(client1 == NULL, "Client should fail with NULL wallet address");

    // Test NULL private key
    hl_client_t* client2 = hl_client_create("0x1234567890123456789012345678901234567890", NULL, true);
    test_assert(client2 == NULL, "Client should fail with NULL private key");

    // Test invalid wallet address (too short)
    hl_client_t* client3 = hl_client_create("0x123", "abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890", true);
    test_assert(client3 == NULL, "Client should fail with invalid wallet address");

    // Test invalid private key (wrong length)
    hl_client_t* client4 = hl_client_create("0x1234567890123456789012345678901234567890", "123", true);
    test_assert(client4 == NULL, "Client should fail with invalid private key");

    return TEST_PASS;
}

/**
 * @brief Test client configuration
 */
test_result_t test_client_configuration(void) {
    const char* wallet_address = test_get_env("HYPERLIQUID_TESTNET_WALLET_ADDRESS", "0x1234567890123456789012345678901234567890");
    const char* private_key = test_get_env("HYPERLIQUID_TESTNET_PRIVATE_KEY", "abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890");

    hl_client_t* client = hl_client_create(wallet_address, private_key, true);
    test_assert_not_null(client, "Client creation");

    // Test timeout setting
    hl_set_timeout(client, 5000);

    // Test debug mode
    hl_set_debug(true);
    hl_set_debug(false);

    hl_client_destroy(client);
    return TEST_PASS;
}

/**
 * @brief Test version info
 */
test_result_t test_version_info(void) {
    const char* version = hl_version();
    test_assert_not_null((void*)version, "Version string");
    test_assert(strlen(version) > 0, "Version string not empty");

    printf("SDK Version: %s\n", version);
    return TEST_PASS;
}

/**
 * @brief Test error codes
 */
test_result_t test_error_codes(void) {
    // Test error string conversion
    const char* success_str = hl_error_string(HL_SUCCESS);
    test_assert_not_null((void*)success_str, "Success error string");

    const char* invalid_str = hl_error_string(HL_ERROR_INVALID_PARAMS);
    test_assert_not_null((void*)invalid_str, "Invalid params error string");

    const char* network_str = hl_error_string(HL_ERROR_NETWORK);
    test_assert_not_null((void*)network_str, "Network error string");

    printf("Error codes test passed\n");
    return TEST_PASS;
}

int main(void) {
    test_load_env();

    printf("╔══════════════════════════════════════════╗\n");
    printf("║  UNIT TESTS: Client Management          ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    test_func_t tests[] = {
        test_client_create_destroy,
        test_client_invalid_params,
        test_client_configuration,
        test_version_info,
        test_error_codes
    };

    return test_run_suite("Client Unit Tests", tests, sizeof(tests)/sizeof(test_func_t));
}