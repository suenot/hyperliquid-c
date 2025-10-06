/**
 * @file hl_types.h
 * @brief CCXT-compatible unified data structures (compatibility layer)
 */

#ifndef HL_TYPES_H
#define HL_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

// Forward declarations (types defined in other headers)
typedef struct hl_balance hl_balance_t;
typedef struct hl_position hl_position_t;
typedef struct hl_market hl_market_t;
typedef struct hl_markets hl_markets_t;
typedef struct hl_ticker hl_ticker_t;
typedef struct hl_tickers hl_tickers_t;
typedef struct hl_orderbook hl_orderbook_t;
typedef struct hl_ohlcv hl_ohlcv_t;
typedef struct hl_ohlcvs hl_ohlcvs_t;

// New structures for CCXT compatibility
typedef struct hl_order hl_order_t;
typedef struct hl_trade hl_trade_t;

// Forward declarations for collections
struct hl_orders;
struct hl_trades;
struct hl_balances;

typedef struct hl_orders hl_orders_t;
typedef struct hl_trades hl_trades_t;
typedef struct hl_balances hl_balances_t;

/**
 * @brief Order structure (CCXT compatible)
 * Note: This extends the existing structures with CCXT fields
 */
struct hl_order {
    char id[64];                    /**< order ID */
    char client_order_id[64];       /**< client order ID */
    char timestamp[32];             /**< timestamp */
    char datetime[32];              /**< ISO 8601 datetime */
    char last_trade_timestamp[32];  /**< last trade timestamp */
    char symbol[32];                /**< trading symbol */
    char type[16];                  /**< order type: 'limit', 'market', 'stop', 'stop-limit' */
    char side[8];                   /**< 'buy' or 'sell' */
    char time_in_force[8];          /**< 'GTC', 'IOC', 'FOK', 'PO' */
    char status[16];                /**< order status: 'open', 'closed', 'canceled' */

    // Prices and amounts
    double price;                   /**< order price */
    double amount;                  /**< order amount */
    double filled;                  /**< filled amount */
    double remaining;               /**< remaining amount */
    double cost;                    /**< total cost */
    double average;                 /**< average fill price */
    double stop_price;              /**< stop price */
    double trigger_price;           /**< trigger price */

    // Fees
    struct {
        double cost;
        char currency[16];
    } fee;

    // Additional
    bool reduce_only;               /**< reduce-only flag */
    bool post_only;                 /**< post-only flag */
    double leverage;                /**< leverage */

    char info[1024];                /**< raw exchange response */
};

/**
 * @brief Trade structure (CCXT compatible)
 */
struct hl_trade {
    char id[64];                    /**< trade ID */
    char order_id[64];              /**< order ID */
    char timestamp[32];             /**< timestamp */
    char datetime[32];              /**< ISO 8601 datetime */
    char symbol[32];                /**< trading symbol */
    char side[8];                   /**< 'buy' or 'sell' */
    char type[16];                  /**< order type */

    double price;                   /**< trade price */
    double amount;                  /**< trade amount */
    double cost;                    /**< trade cost */

    // Fee
    struct {
        double cost;
        char currency[16];
    } fee;

    char info[1024];                /**< raw exchange data */
};

// Collection type definitions
struct hl_orders {
    hl_order_t* orders;
    size_t count;
};

struct hl_trades {
    hl_trade_t* trades;
    size_t count;
};

struct hl_balances {
    hl_balance_t* balances;
    size_t count;
};

// Utility functions for data structures

/**
 * @brief Create new order structure
 * @return Pointer to allocated order (must be freed)
 */
hl_order_t* hl_order_new(void);

/**
 * @brief Free order structure
 * @param order Order to free
 */
void hl_order_free(hl_order_t* order);

/**
 * @brief Create new trade structure
 * @return Pointer to allocated trade (must be freed)
 */
hl_trade_t* hl_trade_new(void);

/**
 * @brief Free trade structure
 * @param trade Trade to free
 */
void hl_trade_free(hl_trade_t* trade);

#endif // HL_TYPES_H
