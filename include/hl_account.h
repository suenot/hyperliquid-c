/**
 * @file hl_account.h
 * @brief Account & balance management for Hyperliquid C SDK
 * 
 * Provides functions to query account balance, positions, and trading fees.
 */

#ifndef HL_ACCOUNT_H
#define HL_ACCOUNT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "hl_error.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations (types defined in hl_types.h)
typedef struct hl_client hl_client_t;
typedef struct hl_balance hl_balance_t;
typedef struct hl_position hl_position_t;

/**
 * @brief Account types
 */
typedef enum {
    HL_ACCOUNT_PERPETUAL,    /**< Perpetual/swap account */
    HL_ACCOUNT_SPOT          /**< Spot account */
} hl_account_type_t;

/**
 * @brief Margin mode
 */
typedef enum {
    HL_MARGIN_CROSS,         /**< Cross margin */
    HL_MARGIN_ISOLATED       /**< Isolated margin */
} hl_margin_mode_t;

/**
 * @brief Spot balance for a single coin
 */
typedef struct {
    char coin[32];           /**< Coin symbol (e.g., "USDC", "BTC") */
    double total;            /**< Total balance */
    double hold;             /**< Amount on hold (in open orders) */
    double available;        /**< Available balance (total - hold) */
} hl_spot_balance_t;

/**
 * @brief Account balance
 */
struct hl_balance {
    hl_account_type_t type;  /**< Account type */

    // Perpetual account fields
    double account_value;    /**< Total account value in USDC */
    double total_margin_used;/**< Total margin used */
    double total_ntl_pos;    /**< Total notional position value */
    double total_raw_usd;    /**< Total raw USD */
    double withdrawable;     /**< Withdrawable amount */

    // Cross margin summary
    double cross_account_value;
    double cross_margin_used;
    double cross_maintenance_margin_used;

    // Spot balances (only if type == SPOT)
    hl_spot_balance_t* spot_balances;
    size_t spot_balance_count;

    uint64_t timestamp;      /**< Response timestamp */
};

/**
 * @brief Position side
 */
typedef enum {
    HL_POSITION_LONG,        /**< Long position */
    HL_POSITION_SHORT        /**< Short position */
} hl_position_side_t;

/**
 * @brief Open position
 */
struct hl_position {
    char coin[32];           /**< Coin symbol */
    char symbol[64];         /**< Market symbol (e.g., "BTC/USDC:USDC") */

    hl_position_side_t side; /**< Position side */
    double size;             /**< Position size (absolute value) */
    double entry_price;      /**< Average entry price */
    double mark_price;       /**< Current mark price */
    double liquidation_price;/**< Liquidation price */

    double unrealized_pnl;   /**< Unrealized P&L */
    double margin_used;      /**< Margin used for this position */
    double position_value;   /**< Notional position value */
    double return_on_equity; /**< ROE */

    int leverage;            /**< Current leverage */
    int max_leverage;        /**< Max leverage allowed */
    bool is_isolated;        /**< Isolated margin mode */

    // Cumulative funding
    double cum_funding_all_time;
    double cum_funding_since_open;
    double cum_funding_since_change;
};

/**
 * @brief Trading fee information
 */
typedef struct {
    char symbol[64];         /**< Market symbol */
    double maker_fee;        /**< Maker fee rate */
    double taker_fee;        /**< Taker fee rate */
} hl_trading_fee_t;

// ============================================================================
// Account Balance API
// ============================================================================

/**
 * @brief Fetch account balance
 * 
 * @param client Client instance
 * @param type Account type (perpetual or spot)
 * @param balance Output balance structure
 * @return HL_SUCCESS on success, error code otherwise
 * 
 * @note For perpetual account, balance->spot_balances will be NULL
 * @note For spot account, balance->spot_balances must be freed by caller
 */
hl_error_t hl_fetch_balance(
    hl_client_t* client,
    hl_account_type_t type,
    hl_balance_t* balance
);

/**
 * @brief Free spot balances array
 * 
 * @param balances Array to free
 * @param count Number of elements
 */
void hl_free_spot_balances(hl_spot_balance_t* balances, size_t count);

// ============================================================================
// Positions API
// ============================================================================

/**
 * @brief Fetch all open positions
 * 
 * @param client Client instance
 * @param positions Output array of positions (caller must free)
 * @param count Output number of positions
 * @return HL_SUCCESS on success, error code otherwise
 * 
 * @note Caller must free positions array with hl_free_positions()
 */
hl_error_t hl_fetch_positions(
    hl_client_t* client,
    hl_position_t** positions,
    size_t* count
);

/**
 * @brief Fetch single position for symbol
 * 
 * @param client Client instance
 * @param symbol Market symbol
 * @param position Output position structure
 * @return HL_SUCCESS on success, HL_ERROR_NOT_FOUND if no position
 */
hl_error_t hl_fetch_position(
    hl_client_t* client,
    const char* symbol,
    hl_position_t* position
);

/**
 * @brief Free positions array
 * 
 * @param positions Array to free
 * @param count Number of elements
 */
void hl_free_positions(hl_position_t* positions, size_t count);

// ============================================================================
// Trading Fees API
// ============================================================================

/**
 * @brief Fetch trading fee for symbol
 * 
 * @param client Client instance
 * @param symbol Market symbol
 * @param fee Output fee structure
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_fetch_trading_fee(
    hl_client_t* client,
    const char* symbol,
    hl_trading_fee_t* fee
);

#ifdef __cplusplus
}
#endif

#endif // HL_ACCOUNT_H

