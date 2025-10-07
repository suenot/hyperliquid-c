/**
 * @file run_all_tests.c
 * @brief Main test runner for all test suites
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helpers/test_common.h"

// Forward declarations for test suites
extern int run_client_tests(void);
extern int run_types_tests(void);
extern int run_connection_tests(void);
extern int run_place_order_tests(void);
extern int run_market_data_tests(void);
extern int run_order_management_tests(void);
extern int run_account_management_tests(void);
extern int run_missing_api_tests(void);

// Simple test suite structure
typedef struct {
    const char* name;
    int (*run_func)(void);
    bool critical;  // Critical tests that must pass
} test_suite_t;

/**
 * @brief Print main test header
 */
void print_main_header(void) {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  Hyperliquid C SDK - Comprehensive Test Suite           ║\n");
    printf("║  Version: %s                                   ║\n", hl_version());
    printf("╚════════════════════════════════════════════════════════╝\n\n");
}

/**
 * @brief Print test suite summary
 */
void print_suite_summary(const char* suite_name, int result, double elapsed_seconds) {
    printf("\n┌─ %s ", suite_name);
    for (size_t i = strlen(suite_name); i < 50; i++) printf("─");
    printf("┐\n");
    printf("│ Result: %s (took %.2fs)                                 │\n",
           result == 0 ? "✅ PASS" : "❌ FAIL", elapsed_seconds);
    printf("└");
    for (int i = 0; i < 54; i++) printf("─");
    printf("┘\n\n");
}

/**
 * @brief Print final summary
 */
void print_final_summary(int total_suites, int passed_suites, int failed_suites, double total_elapsed) {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  FINAL TEST SUMMARY                                       ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Total Test Suites: %38d                     ║\n", total_suites);
    printf("║ Passed: %46d                     ║\n", passed_suites);
    printf("║ Failed: %46d                     ║\n", failed_suites);
    printf("║ Success Rate: %41.1f%%                    ║\n", total_suites > 0 ? (double)passed_suites / total_suites * 100.0 : 0.0);
    printf("║ Total Time: %43.2fs                   ║\n", total_elapsed);
    printf("╚════════════════════════════════════════════════════════╝\n\n");

    if (failed_suites == 0) {
        printf("🎉 All test suites passed! The SDK is working correctly.\n");
    } else {
        printf("⚠️  %d test suite(s) failed. Please check the output above.\n", failed_suites);
    }
}

/**
 * @brief Get current time in seconds
 */
double get_time_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char* argv[]) {
    // Load environment variables
    test_load_env();

    // Print header
    print_main_header();

    // Define all test suites
    test_suite_t test_suites[] = {
        // Unit Tests
        {"Client Unit Tests", run_client_tests, true},
        {"Types Unit Tests", run_types_tests, true},
        {"Missing API Functions Tests", run_missing_api_tests, true},

        // Integration Tests
        {"Connection Tests", run_connection_tests, true},
        {"Place Order Tests", run_place_order_tests, false},  // May skip without credentials
        {"Market Data Tests", run_market_data_tests, true},
        {"Order Management Tests", run_order_management_tests, false},
        {"Account Management Tests", run_account_management_tests, false}
    };

    int total_suites = sizeof(test_suites) / sizeof(test_suite_t);
    int passed_suites = 0;
    int failed_suites = 0;
    double total_start = get_time_seconds();

    // Run all test suites
    for (int i = 0; i < total_suites; i++) {
        printf("🧪 Running %s...\n", test_suites[i].name);

        double suite_start = get_time_seconds();
        int result = test_suites[i].run_func();
        double suite_elapsed = get_time_seconds() - suite_start;

        print_suite_summary(test_suites[i].name, result, suite_elapsed);

        if (result == 0) {
            passed_suites++;
        } else {
            failed_suites++;
            if (test_suites[i].critical) {
                printf("❌ Critical test suite failed: %s\n", test_suites[i].name);
            }
        }
    }

    double total_elapsed = get_time_seconds() - total_start;

    // Print final summary
    print_final_summary(total_suites, passed_suites, failed_suites, total_elapsed);

    // Return appropriate exit code
    return failed_suites > 0 ? 1 : 0;
}