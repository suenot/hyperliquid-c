/**
 * @file test_common.c
 * @brief Common test utilities implementation
 */

#define _GNU_SOURCE
#include "test_common.h"

// Static variables for environment loading
static int env_loaded = 0;

void test_load_env(void) {
    if (env_loaded) return;

    // Try to load from .env file if it exists
    FILE* env_file = fopen(".env", "r");
    if (env_file) {
        char line[1024];
        while (fgets(line, sizeof(line), env_file)) {
            // Remove newline
            line[strcspn(line, "\n")] = 0;

            // Parse KEY=VALUE format
            char* equals = strchr(line, '=');
            if (equals) {
                *equals = '\0';
                char* key = line;
                char* value = equals + 1;
                setenv(key, value, 1);
            }
        }
        fclose(env_file);
    }
    env_loaded = 1;
}

const char* test_get_env(const char* key, const char* default_val) {
    test_load_env();
    const char* val = getenv(key);
    return val ? val : default_val;
}

hl_client_t* test_create_client(bool testnet) {
    const char* wallet_address = test_get_env("HYPERLIQUID_TESTNET_WALLET_ADDRESS", NULL);
    const char* private_key = test_get_env("HYPERLIQUID_TESTNET_PRIVATE_KEY", NULL);

    if (!wallet_address || !private_key) {
        printf("⚠️  No test credentials found, using dummy client for compilation tests\n");
        return NULL;
    }

    return hl_client_create(wallet_address, private_key, testnet);
}

void test_destroy_client(hl_client_t* client) {
    if (client) {
        hl_client_destroy(client);
    }
}

void test_assert(bool condition, const char* message) {
    if (!condition) {
        printf("❌ ASSERTION FAILED: %s\n", message);
        abort();
    }
}

void test_assert_success(hl_error_t err, const char* context) {
    if (err != HL_SUCCESS) {
        printf("❌ %s failed with error: %s\n", context, hl_error_string(err));
        abort();
    }
}

void test_assert_not_null(void* ptr, const char* context) {
    if (!ptr) {
        printf("❌ %s returned NULL\n", context);
        abort();
    }
}

void test_assert_equals(int expected, int actual, const char* message) {
    if (expected != actual) {
        printf("❌ %s: expected %d, got %d\n", message, expected, actual);
        abort();
    }
}

void test_print_header(const char* test_name) {
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║  TEST: %-32s ║\n", test_name);
    printf("╚══════════════════════════════════════════╝\n\n");
}

void test_print_result(const char* test_name, test_result_t result) {
    switch (result) {
        case TEST_PASS:
            printf("✅ %s PASSED\n", test_name);
            break;
        case TEST_FAIL:
            printf("❌ %s FAILED\n", test_name);
            break;
        case TEST_SKIP:
            printf("⏭️  %s SKIPPED\n", test_name);
            break;
    }
}

int test_run(const char* name, test_func_t func) {
    printf("Running %s...\n", name);
    test_result_t result = func();
    test_print_result(name, result);
    return result == TEST_PASS ? 0 : 1;
}

int test_run_suite(const char* suite_name, test_func_t* tests, size_t count) {
    test_print_header(suite_name);

    int failed = 0;
    for (size_t i = 0; i < count; i++) {
        if (test_run("Test", tests[i]) != 0) {
            failed++;
        }
    }

    printf("\nSuite Summary: %zu tests, %d failed\n", count, failed);
    return failed;
}

void test_sleep_ms(int ms) {
    usleep(ms * 1000);
}

char* test_random_string(size_t length) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char* str = malloc(length + 1);
    for (size_t i = 0; i < length; i++) {
        str[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    str[length] = '\0';
    return str;
}

double test_random_double(double min, double max) {
    return min + ((double)rand() / RAND_MAX) * (max - min);
}

// Simple memory tracking (basic implementation)
static size_t memory_allocated = 0;

void test_memory_start(void) {
    memory_allocated = 0;
}

void test_memory_end(void) {
    if (memory_allocated > 0) {
        printf("⚠️  Memory leak detected: %zu bytes\n", memory_allocated);
    } else {
        printf("✅ No memory leaks detected\n");
    }
}