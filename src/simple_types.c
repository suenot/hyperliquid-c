/**
 * @file simple_types.c
 * @brief Simple implementation of basic types for testing
 */

#include <stdlib.h>
#include <string.h>
#include "../include/hl_types.h"

hl_order_t* hl_order_new(void) {
    hl_order_t* order = calloc(1, sizeof(hl_order_t));
    if (order) {
        // Initialize with default values
        strcpy(order->status, "open");
        order->price = 0.0;
        order->amount = 0.0;
        order->filled = 0.0;
        order->remaining = 0.0;
    }
    return order;
}

void hl_order_free(hl_order_t* order) {
    if (order) {
        free(order);
    }
}

hl_trade_t* hl_trade_new(void) {
    hl_trade_t* trade = calloc(1, sizeof(hl_trade_t));
    if (trade) {
        // Initialize with default values
        trade->price = 0.0;
        trade->amount = 0.0;
        trade->cost = 0.0;
        trade->fee.cost = 0.0;
    }
    return trade;
}

void hl_trade_free(hl_trade_t* trade) {
    if (trade) {
        free(trade);
    }
}