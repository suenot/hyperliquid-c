/**
 * @file test_common.h
 * @brief Common test utilities and helpers
 */

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "../../include/hyperliquid.h"

/**
 * @brief Test result codes
 */
typedef enum {
    TEST_PASS = 0,
    TEST_FAIL = 1,
    TEST_SKIP = 2
} test_result_t;

/**
 * @brief Test function type
 */
typedef test_result_t (*test_func_t)(void);

/**
 * @brief Load environment variables from .env file
 */
void test_load_env(void);

/**
 * @brief Get environment variable with default
 */
const char* test_get_env(const char* key, const char* default_val);

/**
 * @brief Create test client
 */
hl_client_t* test_create_client(bool testnet);

/**
 * @brief Destroy test client
 */
void test_destroy_client(hl_client_t* client);

/**
 * @brief Test assertion helpers
 */
void test_assert(bool condition, const char* message);
void test_assert_success(hl_error_t err, const char* context);
void test_assert_not_null(void* ptr, const char* context);
void test_assert_equals(int expected, int actual, const char* message);

/**
 * @brief Print test header
 */
void test_print_header(const char* test_name);

/**
 * @brief Print test result
 */
void test_print_result(const char* test_name, test_result_t result);

/**
 * @brief Run a single test
 */
int test_run(const char* name, test_func_t func);

/**
 * @brief Run test suite
 */
int test_run_suite(const char* suite_name, test_func_t* tests, size_t count);

/**
 * @brief Sleep for testing (in milliseconds)
 */
void test_sleep_ms(int ms);

/**
 * @brief Generate random test data
 */
char* test_random_string(size_t length);
double test_random_double(double min, double max);

/**
 * @brief Memory leak check helper
 */
void test_memory_start(void);
void test_memory_end(void);

#endif // TEST_COMMON_H