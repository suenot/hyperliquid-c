/**
 * @file transfers.c
 * @brief Deposit and withdrawal history functions
 */

#include "hyperliquid.h"
#include <stdlib.h>
#include <string.h>

// Forward declarations for transfer structures
typedef struct hl_transfer hl_transfer_t;

struct hl_transfer {
    char id[64];               /**< transfer ID */
    char timestamp[32];        /**< timestamp */
    char datetime[32];         /**< ISO datetime */
    char currency[16];         /**< currency code */
    double amount;             /**< transfer amount */
    char status[16];           /**< transfer status */
    char tx_hash[128];         /**< blockchain transaction hash */
    char address[128];         /**< destination/source address */
    double fee;                /**< transfer fee */
    char info[512];            /**< raw exchange data */
};

typedef struct {
    hl_transfer_t* transfers;
    size_t count;
} hl_transfers_t;

/**
 * @brief Fetch deposit history
 *
 * Note: This is a stub implementation. Deposits/withdrawals typically require
 * wallet/blockchain integration which is beyond basic exchange API scope.
 */
hl_error_t hl_fetch_deposits(hl_client_t* client,
                            const char* currency,
                            const char* since,
                            uint32_t limit,
                            hl_transfers_t* deposits) {
    if (!client || !deposits) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Clear output
    memset(deposits, 0, sizeof(hl_transfers_t));

    // Stub implementation - no deposits available through basic exchange API
    // In production, this would integrate with wallet APIs or blockchain explorers
    // For now, return empty result (which is valid)

    printf("Note: hl_fetch_deposits() requires wallet integration (currently stub)\n");

    return HL_SUCCESS; // No deposits found
}

/**
 * @brief Fetch withdrawal history
 *
 * Note: This is a stub implementation. Deposits/withdrawals typically require
 * wallet/blockchain integration which is beyond basic exchange API scope.
 */
hl_error_t hl_fetch_withdrawals(hl_client_t* client,
                               const char* currency,
                               const char* since,
                               uint32_t limit,
                               hl_transfers_t* withdrawals) {
    if (!client || !withdrawals) {
        return HL_ERROR_INVALID_PARAMS;
    }

    // Clear output
    memset(withdrawals, 0, sizeof(hl_transfers_t));

    // Stub implementation - no withdrawals available through basic exchange API
    // In production, this would integrate with wallet APIs or blockchain explorers
    // For now, return empty result (which is valid)

    printf("Note: hl_fetch_withdrawals() requires wallet integration (currently stub)\n");

    return HL_SUCCESS; // No withdrawals found
}

/**
 * @brief Free transfers array
 */
void hl_free_transfers(hl_transfers_t* transfers) {
    if (transfers && transfers->transfers) {
        free(transfers->transfers);
        transfers->transfers = NULL;
        transfers->count = 0;
    }
}
