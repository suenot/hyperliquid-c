/**
 * @file simple_balance.c
 * @brief Example: Fetch account balance
 *
 * This example demonstrates how to:
 * - Create a Hyperliquid client
 * - Fetch perpetual account balance
 * - Fetch spot account balance
 * - Handle errors properly
 *
 * Build: make -f Makefile.tests example_balance
 * Run: ./build/bin/example_balance
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/hyperliquid.h"

int main(void) {
    printf("🔍 Hyperliquid Balance Example\n");
    printf("============================\n\n");

    // Create client (testnet)
    hl_client_t* client = hl_client_create(
        "0xAAF96800a2609604c64620df4B5280694E1D812d",
        "0x2ebd552aed6ceb534b27f229e2cb62799b34d6a074c2f6b8b450d44b31445af8",
        true  // testnet
    );

    if (!client) {
        printf("❌ Failed to create client\n");
        return 1;
    }

    printf("✅ Client created successfully\n\n");

    // Fetch perpetual balance
    printf("📊 Fetching perpetual balance...\n");
    hl_balance_t perpetual_balance = {0};
    hl_error_t err = hl_fetch_balance(client, HL_ACCOUNT_PERPETUAL, &perpetual_balance);

    if (err != HL_SUCCESS) {
        printf("❌ Failed to fetch perpetual balance: %s\n", hl_error_string(err));
        hl_client_destroy(client);
        return 1;
    }

    printf("✅ Perpetual Balance:\n");
    printf("   Account Value: %.2f USDC\n", perpetual_balance.account_value);
    printf("   Margin Used: %.2f USDC\n", perpetual_balance.total_margin_used);
    printf("   Withdrawable: %.2f USDC\n", perpetual_balance.withdrawable);
    printf("   Total Notional: %.2f USDC\n", perpetual_balance.total_ntl_pos);
    printf("\n");

    // Fetch spot balance
    printf("📊 Fetching spot balance...\n");
    hl_balance_t spot_balance = {0};
    err = hl_fetch_balance(client, HL_ACCOUNT_SPOT, &spot_balance);

    if (err != HL_SUCCESS) {
        printf("❌ Failed to fetch spot balance: %s\n", hl_error_string(err));
        hl_client_destroy(client);
        return 1;
    }

    printf("✅ Spot Balance:\n");
    printf("   Coins: %zu\n", spot_balance.spot_balance_count);

    for (size_t i = 0; i < spot_balance.spot_balance_count; i++) {
        printf("   %s: %.6f (available: %.6f)\n",
               spot_balance.spot_balances[i].coin,
               spot_balance.spot_balances[i].total,
               spot_balance.spot_balances[i].available);
    }

    // Free spot balances if any
    if (spot_balance.spot_balances) {
        hl_free_spot_balances(spot_balance.spot_balances, spot_balance.spot_balance_count);
    }

    // Cleanup
    hl_client_destroy(client);

    printf("\n🎉 Example completed successfully!\n");

    return 0;
}
