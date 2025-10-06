/**
 * @file simple_test.c
 * @brief Simple test to verify basic compilation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/hyperliquid.h"

int main() {
    printf("=== Simple Compilation Test ===\n");

    // Test basic type creation
    hl_order_t* order = hl_order_new();
    if (order) {
        printf("✅ Order creation successful\n");
        hl_order_free(order);
    } else {
        printf("❌ Order creation failed\n");
        return 1;
    }

    // Test trade creation
    hl_trade_t* trade = hl_trade_new();
    if (trade) {
        printf("✅ Trade creation successful\n");
        hl_trade_free(trade);
    } else {
        printf("❌ Trade creation failed\n");
        return 1;
    }

    printf("✅ All basic tests passed!\n");
    return 0;
}