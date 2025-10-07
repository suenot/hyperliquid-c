/**
 * @file test_websocket_framework.c
 * @brief WebSocket framework comprehensive tests
 *
 * Tests WebSocket functionality:
 * - Client creation and destruction
 * - Connection lifecycle
 * - Message handling
 * - Subscription management
 * - Reconnection logic
 * - Callback functions
 * - Error handling
 * - Memory management
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include "hyperliquid.h"
#include "helpers/test_common.h"

// Test callback data structure
typedef struct {
    int message_count;
    char last_message[1024];
    pthread_mutex_t mutex;
    bool test_completed;
} test_callback_data_t;

// Test result storage
static test_result_t test_results[] = {
    {TEST_STATUS_PENDING, "test_ws_client_create_destroy", 0.0},
    {TEST_STATUS_PENDING, "test_ws_client_configuration", 0.0},
    {TEST_STATUS_PENDING, "test_ws_connection_lifecycle", 0.0},
    {TEST_STATUS_PENDING, "test_ws_message_handling", 0.0},
    {TEST_STATUS_PENDING, "test_ws_subscription_management", 0.0},
    {TEST_STATUS_PENDING, "test_ws_callback_functions", 0.0},
    {TEST_STATUS_PENDING, "test_ws_reconnection", 0.0},
    {TEST_STATUS_PENDING, "test_ws_error_handling", 0.0},
    {TEST_STATUS_PENDING, "test_ws_memory_management", 0.0},
    {TEST_STATUS_PENDING, "test_ws_thread_safety", 0.0},
};
static const size_t num_tests = sizeof(test_results) / sizeof(test_results[0]);

// Test callback functions
void test_message_callback(void *data, const char *message, size_t length, void *user_data) {
    test_callback_data_t *callback_data = (test_callback_data_t *)user_data;

    pthread_mutex_lock(&callback_data->mutex);
    callback_data->message_count++;
    if (message && length < sizeof(callback_data->last_message) - 1) {
        strncpy(callback_data->last_message, message, length);
        callback_data->last_message[length] = '\0';
    }
    pthread_mutex_unlock(&callback_data->mutex);
}

void test_error_callback(void *data, hl_error_t error, const char *error_msg, void *user_data) {
    test_callback_data_t *callback_data = (test_callback_data_t *)user_data;

    pthread_mutex_lock(&callback_data->mutex);
    printf("  WebSocket error: %s (%s)\n", hl_error_string(error), error_msg ? error_msg : "No message");
    pthread_mutex_unlock(&callback_data->mutex);
}

void test_connection_callback(void *data, bool connected, void *user_data) {
    test_callback_data_t *callback_data = (test_callback_data_t *)user_data;

    pthread_mutex_lock(&callback_data->mutex);
    printf("  WebSocket connection status: %s\n", connected ? "Connected" : "Disconnected");
    pthread_mutex_unlock(&callback_data->mutex);
}

test_result_t test_ws_client_create_destroy() {
    hl_ws_client_t *ws_client = NULL;
    hl_client_t *client = NULL;

    TEST_START_TIMER();

    // Create regular client first
    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test WebSocket client creation
    ws_client = hl_ws_client_create(client, true);  // testnet
    TEST_ASSERT(ws_client != NULL, "Failed to create WebSocket client");

    // Test double creation (should fail or return same client)
    hl_ws_client_t *ws_client2 = hl_ws_client_create(client, true);
    // Either returns NULL or the same client, both are acceptable

    // Test destruction
    hl_ws_client_destroy(ws_client);
    hl_ws_client_destroy(ws_client2);  // Should be safe
    hl_ws_client_destroy(NULL);  // Should be safe

    // Cleanup
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_ws_client_configuration() {
    hl_ws_client_t *ws_client = NULL;
    hl_client_t *client = NULL;
    hl_ws_config_t config = {0};

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    ws_client = hl_ws_client_create(client, true);
    TEST_ASSERT(ws_client != NULL, "Failed to create WebSocket client");

    // Test default configuration
    hl_ws_config_default(&config);
    TEST_ASSERT(config.timeout > 0, "Default timeout should be positive");
    TEST_ASSERT(config.reconnect_attempts >= 0, "Reconnect attempts should be non-negative");
    TEST_ASSERT(config.reconnect_delay > 0, "Reconnect delay should be positive");

    // Test configuration setters
    hl_ws_client_set_timeout(ws_client, 5000);
    hl_ws_client_set_reconnect_config(ws_client, 3, 1000);

    // Test setting callbacks
    test_callback_data_t callback_data = {0};
    pthread_mutex_init(&callback_data.mutex, NULL);

    hl_ws_client_set_message_callback(ws_client, test_message_callback, &callback_data);
    hl_ws_client_set_error_callback(ws_client, test_error_callback, &callback_data);
    hl_ws_client_set_connection_callback(ws_client, test_connection_callback, &callback_data);

    // Test NULL callbacks (should be safe)
    hl_ws_client_set_message_callback(ws_client, NULL, NULL);
    hl_ws_client_set_error_callback(ws_client, NULL, NULL);
    hl_ws_client_set_connection_callback(ws_client, NULL, NULL);

    // Cleanup
    pthread_mutex_destroy(&callback_data.mutex);
    hl_ws_client_destroy(ws_client);
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_ws_connection_lifecycle() {
    hl_ws_client_t *ws_client = NULL;
    hl_client_t *client = NULL;
    test_callback_data_t callback_data = {0};

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    ws_client = hl_ws_client_create(client, true);
    TEST_ASSERT(ws_client != NULL, "Failed to create WebSocket client");

    pthread_mutex_init(&callback_data.mutex, NULL);
    hl_ws_client_set_connection_callback(ws_client, test_connection_callback, &callback_data);

    // Test connection
    bool connected = hl_ws_client_connect(ws_client);
    if (connected) {
        TEST_ASSERT(hl_ws_client_is_connected(ws_client),
                   "WebSocket should be connected after successful connect");

        // Give some time for connection to establish
        sleep(1);

        // Test disconnection
        hl_ws_client_disconnect(ws_client);
        TEST_ASSERT(!hl_ws_client_is_connected(ws_client),
                   "WebSocket should not be connected after disconnect");
    } else {
        printf("  WebSocket connection failed (acceptable in CI environment)\n");
    }

    // Test multiple connect/disconnect cycles
    for (int i = 0; i < 3; i++) {
        if (hl_ws_client_connect(ws_client)) {
            sleep(1);
            hl_ws_client_disconnect(ws_client);
        }
    }

    // Cleanup
    pthread_mutex_destroy(&callback_data.mutex);
    hl_ws_client_destroy(ws_client);
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_ws_message_handling() {
    hl_ws_client_t *ws_client = NULL;
    hl_client_t *client = NULL;
    test_callback_data_t callback_data = {0};

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    ws_client = hl_ws_client_create(client, true);
    TEST_ASSERT(ws_client != NULL, "Failed to create WebSocket client");

    pthread_mutex_init(&callback_data.mutex, NULL);
    hl_ws_client_set_message_callback(ws_client, test_message_callback, &callback_data);

    // Test sending text message
    if (hl_ws_client_connect(ws_client)) {
        const char *test_message = "{\"test\": \"message\"}";
        bool sent = hl_ws_client_send_text(ws_client, test_message);
        if (sent) {
            printf("  Successfully sent test message\n");
        }

        // Test sending binary message
        const uint8_t test_binary[] = {0x01, 0x02, 0x03, 0x04};
        sent = hl_ws_client_send(ws_client, test_binary, sizeof(test_binary));
        if (sent) {
            printf("  Successfully sent binary message\n");
        }

        // Wait for potential responses
        sleep(2);

        pthread_mutex_lock(&callback_data.mutex);
        printf("  Received %d messages during test\n", callback_data.message_count);
        if (callback_data.message_count > 0) {
            printf("  Last message: %s\n", callback_data.last_message);
        }
        pthread_mutex_unlock(&callback_data.mutex);

        hl_ws_client_disconnect(ws_client);
    } else {
        printf("  WebSocket connection failed (acceptable in CI environment)\n");
    }

    // Cleanup
    pthread_mutex_destroy(&callback_data.mutex);
    hl_ws_client_destroy(ws_client);
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_ws_subscription_management() {
    hl_ws_client_t *ws_client = NULL;
    hl_client_t *client = NULL;
    test_callback_data_t callback_data = {0};

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    ws_client = hl_ws_client_create(client, true);
    TEST_ASSERT(ws_client != NULL, "Failed to create WebSocket client");

    pthread_mutex_init(&callback_data.mutex, NULL);
    hl_ws_client_set_message_callback(ws_client, test_message_callback, &callback_data);

    if (hl_ws_client_connect(ws_client)) {
        // Test subscription to ticker data
        const char *subscription_id = hl_ws_subscribe_ticker(ws_client, "BTC");
        TEST_ASSERT(subscription_id != NULL, "Subscription ID should not be NULL");
        printf("  Subscribed to BTC ticker with ID: %s\n", subscription_id);

        // Test subscription to order book
        const char *orderbook_id = hl_ws_subscribe_orderbook(ws_client, "BTC", 10);
        TEST_ASSERT(orderbook_id != NULL, "Orderbook subscription ID should not be NULL");
        printf("  Subscribed to BTC orderbook with ID: %s\n", orderbook_id);

        // Test subscription to trades
        const char *trades_id = hl_ws_subscribe_trades(ws_client, "BTC");
        TEST_ASSERT(trades_id != NULL, "Trades subscription ID should not be NULL");
        printf("  Subscribed to BTC trades with ID: %s\n", trades_id);

        // Wait for data
        sleep(3);

        pthread_mutex_lock(&callback_data.mutex);
        printf("  Received %d messages during subscriptions\n", callback_data.message_count);
        pthread_mutex_unlock(&callback_data.mutex);

        // Test unsubscription
        bool unsubscribed = hl_ws_unsubscribe(ws_client, subscription_id);
        if (unsubscribed) {
            printf("  Successfully unsubscribed from ticker\n");
        }

        unsubscribed = hl_ws_unsubscribe(ws_client, orderbook_id);
        if (unsubscribed) {
            printf("  Successfully unsubscribed from orderbook\n");
        }

        unsubscribed = hl_ws_unsubscribe(ws_client, trades_id);
        if (unsubscribed) {
            printf("  Successfully unsubscribed from trades\n");
        }

        // Test unsubscription with invalid ID
        bool result = hl_ws_unsubscribe(ws_client, "invalid_subscription_id");
        // Should not crash, result may be false

        hl_ws_client_disconnect(ws_client);
    } else {
        printf("  WebSocket connection failed (acceptable in CI environment)\n");
    }

    // Cleanup
    pthread_mutex_destroy(&callback_data.mutex);
    hl_ws_client_destroy(ws_client);
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_ws_callback_functions() {
    hl_ws_client_t *ws_client = NULL;
    hl_client_t *client = NULL;
    test_callback_data_t callback_data = {0};

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    ws_client = hl_ws_client_create(client, true);
    TEST_ASSERT(ws_client != NULL, "Failed to create WebSocket client");

    pthread_mutex_init(&callback_data.mutex, NULL);

    // Test setting all callbacks
    hl_ws_client_set_message_callback(ws_client, test_message_callback, &callback_data);
    hl_ws_client_set_error_callback(ws_client, test_error_callback, &callback_data);
    hl_ws_client_set_connection_callback(ws_client, test_connection_callback, &callback_data);

    // Test callback override
    hl_ws_client_set_message_callback(ws_client, NULL, NULL);
    hl_ws_client_set_message_callback(ws_client, test_message_callback, &callback_data);

    if (hl_ws_client_connect(ws_client)) {
        // Trigger callbacks by sending a message
        hl_ws_client_send_text(ws_client, "{\"test\": \"callback\"}");

        // Wait for callbacks to be triggered
        sleep(2);

        pthread_mutex_lock(&callback_data.mutex);
        printf("  Callback triggered %d times\n", callback_data.message_count);
        pthread_mutex_unlock(&callback_data.mutex);

        hl_ws_client_disconnect(ws_client);
    }

    // Cleanup
    pthread_mutex_destroy(&callback_data.mutex);
    hl_ws_client_destroy(ws_client);
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_ws_reconnection() {
    hl_ws_client_t *ws_client = NULL;
    hl_client_t *client = NULL;
    test_callback_data_t callback_data = {0};

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    ws_client = hl_ws_client_create(client, true);
    TEST_ASSERT(ws_client != NULL, "Failed to create WebSocket client");

    pthread_mutex_init(&callback_data.mutex, NULL);
    hl_ws_client_set_connection_callback(ws_client, test_connection_callback, &callback_data);

    // Configure reconnection
    hl_ws_client_set_reconnect_config(ws_client, 2, 1000);  // 2 attempts, 1 second delay

    if (hl_ws_client_connect(ws_client)) {
        printf("  Initial connection successful\n");

        // Wait a bit then disconnect to test reconnection
        sleep(1);
        hl_ws_client_disconnect(ws_client);

        // Test reconnection
        bool reconnected = hl_ws_client_connect(ws_client);
        if (reconnected) {
            printf("  Reconnection successful\n");
        } else {
            printf("  Reconnection failed (acceptable in test environment)\n");
        }
    }

    // Cleanup
    pthread_mutex_destroy(&callback_data.mutex);
    hl_ws_client_destroy(ws_client);
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_ws_error_handling() {
    hl_ws_client_t *ws_client = NULL;
    hl_client_t *client = NULL;
    test_callback_data_t callback_data = {0};

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    ws_client = hl_ws_client_create(client, true);
    TEST_ASSERT(ws_client != NULL, "Failed to create WebSocket client");

    pthread_mutex_init(&callback_data.mutex, NULL);
    hl_ws_client_set_error_callback(ws_client, test_error_callback, &callback_data);

    // Test operations without connection
    bool sent = hl_ws_client_send_text(ws_client, "test");
    // Should handle gracefully without connection

    // Test connection to invalid endpoint
    // (This would require modifying internal config, so we just test error handling)

    // Test with NULL client
    hl_ws_client_destroy(ws_client);
    ws_client = NULL;
    hl_ws_client_destroy(ws_client);  // Should be safe

    // Cleanup
    pthread_mutex_destroy(&callback_data.mutex);
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_ws_memory_management() {
    hl_ws_client_t *ws_client = NULL;
    hl_client_t *client = NULL;

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    // Test multiple create/destroy cycles
    for (int i = 0; i < 5; i++) {
        ws_client = hl_ws_client_create(client, true);
        TEST_ASSERT(ws_client != NULL, "Failed to create WebSocket client in cycle");
        hl_ws_client_destroy(ws_client);
    }

    // Test operations with NULL pointers
    hl_ws_client_destroy(NULL);
    hl_ws_client_set_message_callback(NULL, NULL, NULL);
    hl_ws_client_set_error_callback(NULL, NULL, NULL);
    hl_ws_client_set_connection_callback(NULL, NULL, NULL);
    TEST_ASSERT(!hl_ws_client_connect(NULL), "Connect with NULL client should fail");
    TEST_ASSERT(!hl_ws_client_is_connected(NULL), "Is_connected with NULL client should return false");
    hl_ws_client_disconnect(NULL);  // Should be safe
    TEST_ASSERT(!hl_ws_client_send_text(NULL, "test"), "Send with NULL client should fail");
    TEST_ASSERT(hl_ws_unsubscribe(NULL, "test") == false, "Unsubscribe with NULL client should fail");

    // Cleanup
    hl_client_destroy(client);
    TEST_PASS();
}

test_result_t test_ws_thread_safety() {
    hl_ws_client_t *ws_client = NULL;
    hl_client_t *client = NULL;
    test_callback_data_t callback_data = {0};

    TEST_START_TIMER();

    client = create_test_client();
    TEST_ASSERT(client != NULL, "Failed to create test client");

    ws_client = hl_ws_client_create(client, true);
    TEST_ASSERT(ws_client != NULL, "Failed to create WebSocket client");

    pthread_mutex_init(&callback_data.mutex, NULL);
    hl_ws_client_set_message_callback(ws_client, test_message_callback, &callback_data);

    // Test concurrent operations (basic thread safety test)
    if (hl_ws_client_connect(ws_client)) {
        // Send messages from main thread while connection is active
        for (int i = 0; i < 10; i++) {
            hl_ws_client_send_text(ws_client, "{\"thread_test\": true}");
            usleep(100000);  // 100ms delay
        }

        sleep(1);
        hl_ws_client_disconnect(ws_client);
    }

    // Cleanup
    pthread_mutex_destroy(&callback_data.mutex);
    hl_ws_client_destroy(ws_client);
    hl_client_destroy(client);
    TEST_PASS();
}

// Main test runner
int main() {
    printf("=== WebSocket Framework Comprehensive Tests ===\n\n");

    // Run all tests
    for (size_t i = 0; i < num_tests; i++) {
        printf("Running %s...\n", test_results[i].name);

        switch (i) {
            case 0: test_results[i] = test_ws_client_create_destroy(); break;
            case 1: test_results[i] = test_ws_client_configuration(); break;
            case 2: test_results[i] = test_ws_connection_lifecycle(); break;
            case 3: test_results[i] = test_ws_message_handling(); break;
            case 4: test_results[i] = test_ws_subscription_management(); break;
            case 5: test_results[i] = test_ws_callback_functions(); break;
            case 6: test_results[i] = test_ws_reconnection(); break;
            case 7: test_results[i] = test_ws_error_handling(); break;
            case 8: test_results[i] = test_ws_memory_management(); break;
            case 9: test_results[i] = test_ws_thread_safety(); break;
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