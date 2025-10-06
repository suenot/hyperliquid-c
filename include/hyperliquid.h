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
 * hl_order_request_api_t order = {
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
#include "hl_error.h"
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
 * ERROR CODES - now in hl_error.h
 ***************************************************************************/

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

/** Order request (public API) */
typedef struct {
    const char *symbol;          /**< Trading symbol (e.g., "BTC", "ETH") */
    hl_side_t side;              /**< Buy or sell */
    double price;                /**< Limit price (0 for market orders) */
    double quantity;             /**< Order quantity */
    hl_order_type_t order_type;  /**< Limit or market */
    hl_time_in_force_t time_in_force; /**< Time in force */
    bool reduce_only;            /**< Reduce-only order */
    uint32_t slippage_bps;       /**< Slippage in basis points (market orders) */
} hl_order_request_api_t;

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

// Balance, Position, and Trade types moved to separate headers

// Legacy ticker and orderbook types - moved to separate headers
// typedef struct hl_ticker hl_ticker_t;
// typedef struct hl_orderbook hl_orderbook_t;

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
 * hl_order_request_api_t request = {
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
                          const hl_order_request_api_t *request,
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
                    const hl_order_request_api_t *new_order,
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

/**
 * @brief Free orders array
 *
 * @param orders Orders array allocated by fetch functions
 */
void hl_free_orders(hl_orders_t *orders);

// hl_free_trades declared above

/***************************************************************************
 * ORDER MANAGEMENT
 ***************************************************************************/

/**
 * @brief Fetch open orders
 *
 * @param client Client instance
 * @param symbol Trading symbol (NULL for all symbols)
 * @param since Timestamp to start from (NULL for all)
 * @param limit Maximum number of orders (NULL for unlimited)
 * @param orders Output orders array
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_fetch_open_orders(hl_client_t* client,
                               const char* symbol,
                               const char* since,
                               uint32_t limit,
                               hl_orders_t* orders);

/**
 * @brief Fetch closed orders
 *
 * @param client Client instance
 * @param symbol Trading symbol (NULL for all symbols)
 * @param since Timestamp to start from (NULL for all)
 * @param limit Maximum number of orders (NULL for unlimited)
 * @param orders Output orders array
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_fetch_closed_orders(hl_client_t* client,
                                 const char* symbol,
                                 const char* since,
                                 uint32_t limit,
                                 hl_orders_t* orders);

/**
 * @brief Fetch specific order by ID
 *
 * @param client Client instance
 * @param order_id Order ID
 * @param symbol Trading symbol
 * @param order Output order structure
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_fetch_order(hl_client_t* client,
                         const char* order_id,
                         const char* symbol,
                         hl_order_t* order);

/***************************************************************************
 * TRADE HISTORY
 ***************************************************************************/

/**
 * @brief Fetch user trades
 *
 * @param client Client instance
 * @param symbol Trading symbol (NULL for all symbols)
 * @param since Timestamp to start from (NULL for all)
 * @param limit Maximum number of trades (NULL for unlimited)
 * @param trades Output trades array
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_fetch_my_trades(hl_client_t* client,
                             const char* symbol,
                             const char* since,
                             uint32_t limit,
                             hl_trades_t* trades);

/**
 * @brief Fetch public trades
 *
 * @param client Client instance
 * @param symbol Trading symbol
 * @param since Timestamp to start from (NULL for all)
 * @param limit Maximum number of trades (NULL for unlimited)
 * @param trades Output trades array
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_fetch_trades(hl_client_t* client,
                          const char* symbol,
                          const char* since,
                          uint32_t limit,
                          hl_trades_t* trades);

/***************************************************************************
 * MARKET DATA EXTENDED
 ***************************************************************************/

/**
 * @brief Fetch multiple tickers
 *
 * @param client Client instance
 * @param symbols Array of symbols (NULL for all)
 * @param tickers Output tickers array
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_fetch_tickers(hl_client_t* client,
                           const char** symbols,
                           size_t symbols_count,
                           hl_tickers_t* tickers);

/***************************************************************************
 * LEVERAGE & MARGIN
 ***************************************************************************/

/**
 * @brief Set leverage for symbol
 *
 * @param client Client instance
 * @param leverage Leverage value (1-50)
 * @param symbol Trading symbol (NULL for all)
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_set_leverage(hl_client_t* client,
                          int leverage,
                          const char* symbol);

/***************************************************************************
 * CURRENCY & FUNDING DATA
 ***************************************************************************/

/**
 * @brief Fetch all available currencies
 *
 * @param client Client instance
 * @param currencies Output currencies array
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_fetch_currencies(hl_client_t* client, void* currencies);

/**
 * @brief Fetch funding rates for all symbols
 *
 * @param client Client instance
 * @param rates Output funding rates array
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_fetch_funding_rates(hl_client_t* client, void* rates);

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

/**
 * @brief Fetch deposit history
 *
 * @param client Client instance
 * @param currency Currency code (NULL for all)
 * @param since Start time (NULL for recent)
 * @param limit Maximum entries to retrieve
 * @param deposits Output deposit history array
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_fetch_deposits(hl_client_t* client,
                            const char* currency,
                            const char* since,
                            uint32_t limit,
                            void* deposits);

/**
 * @brief Fetch withdrawal history
 *
 * @param client Client instance
 * @param currency Currency code (NULL for all)
 * @param since Start time (NULL for recent)
 * @param limit Maximum entries to retrieve
 * @param withdrawals Output withdrawal history array
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_fetch_withdrawals(hl_client_t* client,
                               const char* currency,
                               const char* since,
                               uint32_t limit,
                               void* withdrawals);

/**
 * @brief Create multiple orders
 *
 * @param client Client instance
 * @param orders Array of order requests
 * @param orders_count Number of orders in array
 * @param results Output array for order results
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_create_orders(hl_client_t* client,
                           const hl_order_request_api_t* orders,
                           size_t orders_count,
                           hl_order_result_t* results);

/**
 * @brief Cancel multiple orders
 *
 * @param client Client instance
 * @param orders Array of order IDs to cancel
 * @param orders_count Number of order IDs
 * @param results Output array for cancellation results
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_cancel_orders(hl_client_t* client,
                           const char** orders,
                           size_t orders_count,
                           hl_cancel_result_t* results);

/**
 * @brief Edit an existing order
 *
 * @param client Client instance
 * @param order_id Order ID to edit
 * @param request New order parameters
 * @param result Output order result
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_edit_order(hl_client_t* client,
                        const char* order_id,
                        const hl_order_request_api_t* request,
                        hl_order_result_t* result);

/**
 * @brief Add margin to a position
 *
 * @param client Client instance
 * @param symbol Trading symbol
 * @param amount Amount of margin to add
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_add_margin(hl_client_t* client,
                        const char* symbol,
                        double amount);

/**
 * @brief Reduce margin from a position
 *
 * @param client Client instance
 * @param symbol Trading symbol
 * @param amount Amount of margin to reduce
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_reduce_margin(hl_client_t* client,
                           const char* symbol,
                           double amount);

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

