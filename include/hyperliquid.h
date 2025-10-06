/**
 * @file hyperliquid.h
 * @brief Hyperliquid C SDK - Main API Header
 * @version 1.0.0
 * 
 * Production-ready C library for Hyperliquid DEX trading.
 * 
 * Features:
 * - Complete trading API (orders, positions, balances)
 * - EIP-712 signatures with libsecp256k1
 * - MessagePack serialization (byte-perfect compatibility)
 * - Testnet and mainnet support
 * - Low latency, zero-copy operations
 * 
 * Example:
 * ```c
 * hl_client_t *client = hl_client_create("0xAddress", "privkey", true);
 * hl_order_request_t order = {
 *     .symbol = "BTC",
 *     .side = HL_SIDE_BUY,
 *     .price = 95000.0,
 *     .quantity = 0.001
 * };
 * hl_order_result_t result;
 * hl_place_order(client, &order, &result);
 * hl_client_destroy(client);
 * ```
 */

#ifndef HYPERLIQUID_H
#define HYPERLIQUID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Core CCXT-compatible includes
#include "hl_exchange.h"
#include "hl_types.h"
#include "hl_client.h"

// Legacy includes (for backward compatibility)
#include "hl_account.h"
#include "hl_markets.h"
#include "hl_ticker.h"
#include "hl_orderbook.h"
#include "hl_ohlcv.h"

// hl_account.h included at end of file after base types defined

/***************************************************************************
 * VERSION
 ***************************************************************************/

#define HL_VERSION_MAJOR 1
#define HL_VERSION_MINOR 0
#define HL_VERSION_PATCH 0
#define HL_VERSION_STRING "1.0.0"

/***************************************************************************
 * ERROR CODES
 ***************************************************************************/

typedef enum {
    HL_SUCCESS = 0,              /**< Operation successful */
    HL_ERROR_INVALID_PARAMS = -1, /**< Invalid parameters */
    HL_ERROR_NETWORK = -2,       /**< Network error */
    HL_ERROR_API = -3,           /**< API error */
    HL_ERROR_AUTH = -4,          /**< Authentication error */
    HL_ERROR_INSUFFICIENT_BALANCE = -5, /**< Insufficient balance */
    HL_ERROR_INVALID_SYMBOL = -6, /**< Invalid trading symbol */
    HL_ERROR_ORDER_REJECTED = -7, /**< Order rejected by exchange */
    HL_ERROR_SIGNATURE = -8,     /**< Signature generation failed */
    HL_ERROR_MSGPACK = -9,       /**< MessagePack error */
    HL_ERROR_JSON = -10,         /**< JSON parsing error */
    HL_ERROR_MEMORY = -11,       /**< Memory allocation failed */
    HL_ERROR_TIMEOUT = -12,      /**< Operation timed out */
    HL_ERROR_NOT_IMPLEMENTED = -13, /**< Feature not implemented */
    HL_ERROR_NOT_FOUND = -14,    /**< Resource not found */
    HL_ERROR_PARSE = -15         /**< Response parsing error */
} hl_error_t;
#define HL_ERROR_T_DEFINED

/***************************************************************************
 * ENUMS
 ***************************************************************************/

/** Order side */
typedef enum {
    HL_SIDE_BUY,
    HL_SIDE_SELL
} hl_side_t;

/** Order type */
typedef enum {
    HL_ORDER_TYPE_LIMIT,
    HL_ORDER_TYPE_MARKET
} hl_order_type_t;

/** Time in force */
typedef enum {
    HL_TIF_GTC,  /**< Good Till Cancel */
    HL_TIF_IOC,  /**< Immediate Or Cancel */
    HL_TIF_ALO   /**< Add Liquidity Only */
} hl_time_in_force_t;

/** Order status */
typedef enum {
    HL_ORDER_STATUS_OPEN,
    HL_ORDER_STATUS_FILLED,
    HL_ORDER_STATUS_PARTIALLY_FILLED,
    HL_ORDER_STATUS_CANCELLED,
    HL_ORDER_STATUS_REJECTED
} hl_order_status_t;

/***************************************************************************
 * CORE TYPES
 ***************************************************************************/

/** Opaque client handle */
typedef struct hl_client hl_client_t;

/** Order request */
typedef struct {
    const char *symbol;          /**< Trading symbol (e.g., "BTC", "ETH") */
    hl_side_t side;              /**< Buy or sell */
    double price;                /**< Limit price (0 for market orders) */
    double quantity;             /**< Order quantity */
    hl_order_type_t order_type;  /**< Limit or market */
    hl_time_in_force_t time_in_force; /**< Time in force */
    bool reduce_only;            /**< Reduce-only order */
    uint32_t slippage_bps;       /**< Slippage in basis points (market orders) */
} hl_order_request_t;

/** Order result */
typedef struct {
    char *order_id;              /**< Order ID (allocated string, caller must free) */
    hl_order_status_t status;    /**< Order status */
    double filled_quantity;      /**< Filled quantity */
    double average_price;        /**< Average fill price */
    char error[256];             /**< Error message (if any) */
} hl_order_result_t;

/** Cancel result */
typedef struct {
    bool cancelled;              /**< True if order was cancelled successfully */
    char error[256];             /**< Error message (if any) */
} hl_cancel_result_t;

// Balance and Position types moved to hl_account.h

/** Trade info */
typedef struct {
    uint64_t trade_id;           /**< Trade ID */
    uint64_t order_id;           /**< Order ID */
    char symbol[32];             /**< Symbol */
    hl_side_t side;              /**< Buy or sell */
    double price;                /**< Trade price */
    double quantity;             /**< Trade quantity */
    double fee;                  /**< Trading fee */
    uint64_t timestamp_ms;       /**< Timestamp (milliseconds) */
} hl_trade_t;

/** Market ticker */
typedef struct {
    char symbol[64];             /**< Symbol */
    double last_price;           /**< Last trade price */
    double bid;                  /**< Best bid */
    double ask;                  /**< Best ask */
    double close;                /**< Close price */
    double previous_close;       /**< Previous day close price */
    double high_24h;             /**< 24h high */
    double low_24h;              /**< 24h low */
    double volume_24h;           /**< 24h volume */
    double quote_volume;         /**< 24h quote volume */
    double change_24h;           /**< 24h price change (%) */
    uint64_t timestamp;          /**< Timestamp (ms) */
    char datetime[32];           /**< ISO 8601 datetime string */

    // Additional data for perpetuals
    double mark_price;           /**< Mark price */
    double oracle_price;         /**< Oracle price */
    double funding_rate;         /**< Current funding rate */
    double open_interest;        /**< Open interest */
} hl_ticker_t;

/** Order book level */
typedef struct {
    double price;                /**< Price level */
    double quantity;             /**< Quantity at this level */
} hl_book_level_t;

/** Order book */
typedef struct {
    char symbol[32];             /**< Symbol */
    hl_book_level_t *bids;       /**< Bid levels (sorted high to low) */
    size_t bids_count;           /**< Number of bid levels */
    hl_book_level_t *asks;       /**< Ask levels (sorted low to high) */
    size_t asks_count;           /**< Number of ask levels */
    uint64_t timestamp_ms;       /**< Snapshot timestamp */
} hl_orderbook_t;

/***************************************************************************
 * CLIENT MANAGEMENT
 ***************************************************************************/

/**
 * @brief Create Hyperliquid client
 * 
 * @param wallet_address Ethereum wallet address (with 0x prefix)
 * @param private_key Private key (64 hex characters, no 0x prefix)
 * @param testnet true for testnet, false for mainnet
 * @return Client handle, or NULL on error
 * 
 * @note Private key is stored securely and never logged
 */
hl_client_t* hl_client_create(const char *wallet_address, 
                               const char *private_key,
                               bool testnet);

/**
 * @brief Destroy client and free resources
 * 
 * @param client Client handle
 */
void hl_client_destroy(hl_client_t *client);

/**
 * @brief Test connection to Hyperliquid API
 * 
 * @param client Client handle
 * @return true if connection successful, false otherwise
 */
bool hl_test_connection(hl_client_t *client);

/**
 * @brief Set HTTP timeout
 * 
 * @param client Client handle
 * @param timeout_ms Timeout in milliseconds
 */
void hl_set_timeout(hl_client_t *client, uint32_t timeout_ms);

/***************************************************************************
 * TRADING OPERATIONS
 ***************************************************************************/

/**
 * @brief Place order
 * 
 * @param client Client handle
 * @param request Order request
 * @param result Order result (output)
 * @return HL_SUCCESS on success, error code otherwise
 * 
 * Example:
 * ```c
 * hl_order_request_t request = {
 *     .symbol = "BTC",
 *     .side = HL_SIDE_BUY,
 *     .price = 95000.0,
 *     .quantity = 0.001,
 *     .order_type = HL_ORDER_TYPE_LIMIT,
 *     .time_in_force = HL_TIF_GTC,
 *     .reduce_only = false
 * };
 * hl_order_result_t result;
 * if (hl_place_order(client, &request, &result) == HL_SUCCESS) {
 *     printf("Order ID: %s\n", result.order_id);
 * }
 * ```
 */
hl_error_t hl_place_order(hl_client_t *client, 
                          const hl_order_request_t *request,
                          hl_order_result_t *result);

/**
 * @brief Cancel order
 * 
 * @param client Client handle
 * @param symbol Trading symbol
 * @param order_id Order ID to cancel (string)
 * @param result Cancel result (output)
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_cancel_order(hl_client_t *client,
                           const char *symbol,
                           const char *order_id,
                           hl_cancel_result_t *result);

/**
 * @brief Cancel all orders for symbol
 * 
 * @param client Client handle
 * @param symbol Trading symbol (or NULL for all symbols)
 * @return HL_SUCCESS on success, error code otherwise
 */
int hl_cancel_all_orders(hl_client_t *client, const char *symbol);

/**
 * @brief Modify order (cancel and replace)
 * 
 * @param client Client handle
 * @param order_id Order ID to modify
 * @param new_order New order parameters
 * @param result Order result (output)
 * @return HL_SUCCESS on success, error code otherwise
 */
int hl_modify_order(hl_client_t *client,
                    uint64_t order_id,
                    const hl_order_request_t *new_order,
                    hl_order_result_t *result);

/***************************************************************************
 * ACCOUNT INFORMATION
 ***************************************************************************/

// Account and position functions moved to hl_account.h

/**
 * @brief Get trade history
 * 
 * @param client Client handle
 * @param symbol Trading symbol (or NULL for all)
 * @param trades Array of trades (output, must be freed with hl_free_trades)
 * @param count Number of trades (output)
 * @return HL_SUCCESS on success, error code otherwise
 */
int hl_get_trade_history(hl_client_t *client,
                         const char *symbol,
                         hl_trade_t **trades,
                         size_t *count);

/**
 * @brief Free trades array
 * 
 * @param trades Array allocated by hl_get_trade_history
 */
void hl_free_trades(hl_trade_t *trades);

/***************************************************************************
 * MARKET DATA
 ***************************************************************************/

/**
 * @brief Get current market price
 * 
 * @param client Client handle
 * @param symbol Trading symbol
 * @param price Current price (output)
 * @return HL_SUCCESS on success, error code otherwise
 */
int hl_get_market_price(hl_client_t *client, 
                        const char *symbol, 
                        double *price);

/**
 * @brief Get market ticker
 * 
 * @param client Client handle
 * @param symbol Trading symbol
 * @param ticker Ticker info (output)
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_get_ticker(hl_client_t *client,
                        const char *symbol,
                        hl_ticker_t *ticker);

/**
 * @brief Get order book
 * 
 * @param client Client handle
 * @param symbol Trading symbol
 * @param depth Number of levels (e.g., 20)
 * @param book Order book (output, must be freed with hl_free_orderbook)
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_get_orderbook(hl_client_t *client,
                          const char *symbol,
                          uint32_t depth,
                          hl_orderbook_t *book);

/**
 * @brief Free order book
 * 
 * @param book Order book allocated by hl_get_orderbook
 */
void hl_free_orderbook(hl_orderbook_t *book);

/***************************************************************************
 * UTILITY FUNCTIONS
 ***************************************************************************/

/**
 * @brief Get error string
 * 
 * @param error Error code
 * @return Human-readable error string
 */
const char* hl_error_string(hl_error_t error);

/**
 * @brief Get library version
 * 
 * @return Version string (e.g., "1.0.0")
 */
const char* hl_version(void);

/**
 * @brief Enable debug logging
 * 
 * @param enabled true to enable, false to disable
 */
void hl_set_debug(bool enabled);

/***************************************************************************
 * EXTENDED MODULES
 ***************************************************************************/

// Include extended API modules after base types are defined
#include "hl_account.h"
#include "hl_markets.h"
#include "hl_orderbook.h"
#include "hl_ohlcv.h"

#ifdef __cplusplus
}
#endif

#endif // HYPERLIQUID_H

