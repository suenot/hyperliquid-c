/**
 * @file types.c
 * @brief CCXT-compatible data structures implementation
 */

#include "hl_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// Order functions
hl_order_t* hl_order_new(void) {
    hl_order_t* order = calloc(1, sizeof(hl_order_t));
    if (order) {
        strcpy(order->time_in_force, "GTC");
        strcpy(order->status, "open");
        order->leverage = 1.0;
    }
    return order;
}

void hl_order_free(hl_order_t* order) {
    if (order) {
        free(order);
    }
}

// Trade functions
hl_trade_t* hl_trade_new(void) {
    hl_trade_t* trade = calloc(1, sizeof(hl_trade_t));
    return trade;
}

void hl_trade_free(hl_trade_t* trade) {
    if (trade) {
        free(trade);
    }
}

/**
 * @brief Free orders array
 */
void hl_free_orders(hl_orders_t *orders) {
    if (orders && orders->orders) {
        for (size_t i = 0; i < orders->count; i++) {
            hl_order_free(&orders->orders[i]);
        }
        free(orders->orders);
        orders->orders = NULL;
        orders->count = 0;
    }
}

/**
 * @brief Free trades array
 */
void hl_free_trades(hl_trades_t *trades) {
    if (trades && trades->trades) {
        for (size_t i = 0; i < trades->count; i++) {
            hl_trade_free(&trades->trades[i]);
        }
        free(trades->trades);
        trades->trades = NULL;
        trades->count = 0;
    }
}

// Utility functions for parsing timestamps
void hl_timestamp_to_datetime(char* datetime, size_t size, const char* timestamp) {
    if (!datetime || size == 0) return;

    // Convert timestamp to ISO 8601 datetime
    time_t ts = (time_t)atol(timestamp);
    struct tm* tm_info = gmtime(&ts);

    if (tm_info) {
        strftime(datetime, size, "%Y-%m-%dT%H:%M:%S.%fZ", tm_info);
    } else {
        strncpy(datetime, timestamp, size - 1);
        datetime[size - 1] = '\0';
    }
}

void hl_current_timestamp(char* timestamp, size_t size) {
    if (!timestamp || size == 0) return;

    time_t now = time(NULL);
    snprintf(timestamp, size, "%ld", now * 1000); // milliseconds
}
