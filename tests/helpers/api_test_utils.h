/**
 * @file api_test_utils.h
 * @brief Utilities for API testing
 */

#ifndef API_TEST_UTILS_H
#define API_TEST_UTILS_H

#include "../../include/hyperliquid.h"

/**
 * @brief Load test credentials from environment variables
 * @return 0 on success, -1 on error
 */
int load_test_credentials(void);

/**
 * @brief Create test client
 * @return Client handle or NULL on error
 */
hl_client_t* create_test_client(void);

/**
 * @brief Get test wallet address
 * @return Wallet address string
 */
const char* get_test_wallet_address(void);

/**
 * @brief Get test private key
 * @return Private key string
 */
const char* get_test_private_key(void);

/**
 * @brief Print test header
 */
void print_test_header(const char* test_name);

/**
 * @brief Print test result
 */
void print_test_result(const char* test_name, int success);

/**
 * @brief Assert function for testing
 */
void test_assert(int condition, const char* message);

#endif // API_TEST_UTILS_H