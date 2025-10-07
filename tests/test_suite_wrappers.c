/**
 * @file test_suite_wrappers.c
 * @brief Wrapper functions for test suites
 */

#include <stdio.h>

// Wrapper functions for calling test suites
// These allow the main test runner to call each test suite

// Unit test wrappers
extern int main(void);  // This will be redefined for each test

int run_client_tests(void) {
    extern int main_client_unit(void);
    return main_client_unit();
}

int run_types_tests(void) {
    extern int main_types_unit(void);
    return main_types_unit();
}

int run_connection_tests(void) {
    extern int main_connection_integration(void);
    return main_connection_integration();
}

int run_place_order_tests(void) {
    extern int main_place_order_integration(void);
    return main_place_order_integration();
}

int run_market_data_tests(void) {
    extern int main_market_data_integration(void);
    return main_market_data_integration();
}

int run_order_management_tests(void) {
    extern int main_order_management_integration(void);
    return main_order_management_integration();
}

int run_account_management_tests(void) {
    extern int main_account_management_integration(void);
    return main_account_management_integration();
}

int run_missing_api_tests(void) {
    extern int main_missing_api_unit(void);
    return main_missing_api_unit();
}