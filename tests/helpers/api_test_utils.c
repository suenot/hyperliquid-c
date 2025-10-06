/**
 * @file api_test_utils.c
 * @brief Utilities for API testing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../include/hyperliquid.h"

static int test_credentials_loaded = 0;
static char test_wallet_address[256] = {0};
static char test_private_key[256] = {0};

/**
 * @brief Load test credentials from environment variables
 */
int load_test_credentials(void) {
    if (test_credentials_loaded) {
        return 0; // Already loaded
    }

    const char* wallet_addr = getenv("HYPERLIQUID_TESTNET_WALLET_ADDRESS");
    const char* private_key = getenv("HYPERLIQUID_TESTNET_PRIVATE_KEY");

    if (!wallet_addr || !private_key) {
        printf("❌ Missing test credentials. Set HYPERLIQUID_TESTNET_WALLET_ADDRESS and HYPERLIQUID_TESTNET_PRIVATE_KEY\n");
        return -1;
    }

    strncpy(test_wallet_address, wallet_addr, sizeof(test_wallet_address) - 1);
    strncpy(test_private_key, private_key, sizeof(test_private_key) - 1);

    test_credentials_loaded = 1;
    printf("✅ Test credentials loaded for wallet: %s\n", test_wallet_address);
    return 0;
}

/**
 * @brief Create test client
 */
hl_client_t* create_test_client(void) {
    if (load_test_credentials() != 0) {
        return NULL;
    }

    // Note: This is a simplified client creation for testing
    // In real implementation, this would use the actual client creation API
    return NULL; // Placeholder - would create real client
}

/**
 * @brief Get test wallet address
 */
const char* get_test_wallet_address(void) {
    load_test_credentials();
    return test_wallet_address;
}

/**
 * @brief Get test private key
 */
const char* get_test_private_key(void) {
    load_test_credentials();
    return test_private_key;
}

/**
 * @brief Print test header
 */
void print_test_header(const char* test_name) {
    printf("\n=== %s ===\n", test_name);
}

/**
 * @brief Print test result
 */
void print_test_result(const char* test_name, int success) {
    if (success) {
        printf("✅ %s PASSED\n", test_name);
    } else {
        printf("❌ %s FAILED\n", test_name);
    }
}

/**
 * @brief Assert function for testing
 */
void test_assert(int condition, const char* message) {
    if (!condition) {
        printf("❌ Assertion failed: %s\n", message);
        exit(1);
    }
}