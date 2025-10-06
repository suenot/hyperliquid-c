/**
 * @file orders.c
 * @brief Order management functions implementation
 */

#include "hyperliquid.h"
#include "hl_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Forward declarations
static hl_error_t parse_order_from_json(cJSON* order_json, hl_order_t* order);
static hl_error_t parse_trade_from_json(cJSON* trade_json, hl_trade_t* trade);

/**
 * @brief Fetch open orders
 */
hl_error_t hl_fetch_open_orders(hl_client_t* client,
                               const char* symbol,
                               const char* since,
                               uint32_t limit,
                               hl_orders_t* orders) {
    if (!client || !orders) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Clear output
    memset(orders, 0, sizeof(hl_orders_t));

    // Prepare request
    char request_body[1024];
    const char* user_address = hl_client_get_wallet_address_old(client);

    if (!user_address) {
        return HL_ERROR_AUTH;
    }

    // Use frontendOpenOrders method (more efficient)
    snprintf(request_body, sizeof(request_body),
             "{\"type\":\"frontendOpenOrders\",\"user\":\"%s\"}", user_address);

    // Make request
    http_response_t* response = NULL;
    lv3_error_t err = http_client_post_old(client, "/info", request_body, NULL, &response);

    if (err != LV3_SUCCESS || !response || response->status_code != 200) {
        if (response) {
            http_response_free(response);
        }
        return HL_ERROR_NETWORK;
    }

    // Parse JSON response
    cJSON* json = cJSON_Parse(response->body);
    if (!json) {
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    // Response is an array of orders
    if (!cJSON_IsArray(json)) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    int array_size = cJSON_GetArraySize(json);
    if (array_size == 0) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_SUCCESS; // No orders
    }

    // Allocate orders array
    orders->orders = calloc(array_size, sizeof(hl_order_t));
    if (!orders->orders) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_MEMORY;
    }

    // Parse each order
    size_t valid_orders = 0;
    cJSON* order_json = NULL;
    cJSON_ArrayForEach(order_json, json) {
        if (valid_orders >= array_size) break;

        hl_error_t parse_err = parse_order_from_json(order_json, &orders->orders[valid_orders]);
        if (parse_err == HL_SUCCESS) {
            // Set status to open if not specified
            if (strlen(orders->orders[valid_orders].status) == 0) {
                strcpy(orders->orders[valid_orders].status, "open");
            }
            valid_orders++;
        }
    }

    orders->count = valid_orders;

    cJSON_Delete(json);
    http_response_free(response);

    return HL_SUCCESS;
}

/**
 * @brief Fetch closed orders
 */
hl_error_t hl_fetch_closed_orders(hl_client_t* client,
                                 const char* symbol,
                                 const char* since,
                                 uint32_t limit,
                                 hl_orders_t* orders) {
    if (!client || !orders) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Clear output
    memset(orders, 0, sizeof(hl_orders_t));

    // Prepare request - use historicalOrders endpoint
    char request_body[1024];
    const char* user_address = hl_client_get_wallet_address_old(client);

    if (!user_address) {
        return HL_ERROR_AUTH;
    }

    // Use historicalOrders method to get all order history
    snprintf(request_body, sizeof(request_body),
             "{\"type\":\"historicalOrders\",\"user\":\"%s\"}", user_address);

    // Make request
    http_response_t* response = NULL;
    lv3_error_t err = http_client_post_old(client, "/info", request_body, NULL, &response);

    if (err != LV3_SUCCESS || !response || response->status_code != 200) {
        if (response) {
            http_response_free(response);
        }
        return HL_ERROR_NETWORK;
    }

    // Parse JSON response
    cJSON* json = cJSON_Parse(response->body);
    if (!json) {
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    // Response is an array of orders
    if (!cJSON_IsArray(json)) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    int array_size = cJSON_GetArraySize(json);
    if (array_size == 0) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_SUCCESS; // No orders
    }

    // Allocate temporary array for all orders
    hl_order_t* all_orders = calloc(array_size, sizeof(hl_order_t));
    if (!all_orders) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_MEMORY;
    }

    // Parse all orders
    size_t valid_orders = 0;
    cJSON* order_json = NULL;
    cJSON_ArrayForEach(order_json, json) {
        if (valid_orders >= array_size) break;

        hl_error_t parse_err = parse_order_from_json(order_json, &all_orders[valid_orders]);
        if (parse_err == HL_SUCCESS) {
            valid_orders++;
        }
    }

    // Filter closed orders (status != "open")
    size_t closed_count = 0;
    for (size_t i = 0; i < valid_orders; i++) {
        if (strcmp(all_orders[i].status, "open") != 0) {
            closed_count++;
        }
    }

    // Allocate result array for closed orders only
    if (closed_count > 0) {
        orders->orders = calloc(closed_count, sizeof(hl_order_t));
        if (!orders->orders) {
            free(all_orders);
            cJSON_Delete(json);
            http_response_free(response);
            return HL_ERROR_MEMORY;
        }

        // Copy closed orders
        size_t result_idx = 0;
        for (size_t i = 0; i < valid_orders; i++) {
            if (strcmp(all_orders[i].status, "open") != 0) {
                orders->orders[result_idx] = all_orders[i];
                result_idx++;
            }
        }
        orders->count = closed_count;
    }

    free(all_orders);
    cJSON_Delete(json);
    http_response_free(response);

    return HL_SUCCESS;
}

/**
 * @brief Fetch specific order by ID
 */
hl_error_t hl_fetch_order(hl_client_t* client,
                         const char* order_id,
                         const char* symbol,
                         hl_order_t* order) {
    if (!client || !order_id || !order) {
        return HL_ERROR_INVALID_PARAMS;
    }

    memset(order, 0, sizeof(hl_order_t));

    // Prepare request
    char request_body[1024];
    const char* user_address = hl_client_get_wallet_address_old(client);

    if (!user_address) {
        return HL_ERROR_AUTH;
    }

    // Use orderStatus method to get specific order
    snprintf(request_body, sizeof(request_body),
             "{\"type\":\"orderStatus\",\"user\":\"%s\",\"oid\":%s}",
             user_address, order_id);

    // Make request
    http_response_t* response = NULL;
    lv3_error_t err = http_client_post_old(client, "/info", request_body, NULL, &response);

    if (err != LV3_SUCCESS || !response || response->status_code != 200) {
        if (response) {
            http_response_free(response);
        }
        return HL_ERROR_NETWORK;
    }

    // Parse JSON response
    cJSON* json = cJSON_Parse(response->body);
    if (!json) {
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    // Response should be an object with order status
    if (!cJSON_IsObject(json)) {
        cJSON_Delete(json);
        http_response_free(response);
        return HL_ERROR_JSON;
    }

    // Parse order from response
    hl_error_t parse_err = parse_order_from_json(json, order);

    cJSON_Delete(json);
    http_response_free(response);

    return parse_err;
}

/**
 * @brief Parse order from JSON
 */
static hl_error_t parse_order_from_json(cJSON* order_json, hl_order_t* order) {
    if (!order_json || !order) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Extract basic fields
    cJSON* coin = cJSON_GetObjectItem(order_json, "coin");
    cJSON* oid = cJSON_GetObjectItem(order_json, "oid");
    cJSON* side = cJSON_GetObjectItem(order_json, "side");
    cJSON* sz = cJSON_GetObjectItem(order_json, "sz");
    cJSON* limit_px = cJSON_GetObjectItem(order_json, "limitPx");
    cJSON* timestamp = cJSON_GetObjectItem(order_json, "timestamp");

    if (!coin || !cJSON_IsString(coin) || !oid || !side || !cJSON_IsString(side)) {
        return HL_ERROR_JSON;
    }

    // Set symbol (coin)
    strncpy(order->symbol, coin->valuestring, sizeof(order->symbol) - 1);

    // Set order ID
    if (cJSON_IsNumber(oid)) {
        snprintf(order->id, sizeof(order->id), "%llu", (unsigned long long)oid->valuedouble);
    } else if (cJSON_IsString(oid)) {
        strncpy(order->id, oid->valuestring, sizeof(order->id) - 1);
    }

    // Set side
    if (strcmp(side->valuestring, "B") == 0) {
        strcpy(order->side, "buy");
    } else if (strcmp(side->valuestring, "A") == 0) {
        strcpy(order->side, "sell");
    }

    // Set type to limit (default for Hyperliquid)
    strcpy(order->type, "limit");

    // Set amount
    if (sz && cJSON_IsString(sz)) {
        order->amount = atof(sz->valuestring);
        order->filled = 0.0; // TODO: parse from response
        order->remaining = order->amount;
    }

    // Set price
    if (limit_px && cJSON_IsString(limit_px)) {
        order->price = atof(limit_px->valuestring);
    }

    // Set timestamps
    if (timestamp && cJSON_IsNumber(timestamp)) {
        unsigned long long ts = (unsigned long long)timestamp->valuedouble;
        snprintf(order->timestamp, sizeof(order->timestamp), "%llu", ts);
        snprintf(order->datetime, sizeof(order->datetime), "%llu", ts); // TODO: format properly
    }

    // Set status
    cJSON* status = cJSON_GetObjectItem(order_json, "status");
    if (status && cJSON_IsString(status)) {
        strncpy(order->status, status->valuestring, sizeof(order->status) - 1);
    } else {
        // Default to open for open orders
        strcpy(order->status, "open");
    }

    // Set time in force
    strcpy(order->time_in_force, "GTC");

    // Set leverage
    order->leverage = 1.0;

    return HL_SUCCESS;
}
