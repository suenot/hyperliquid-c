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
