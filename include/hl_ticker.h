/**
 * @file hl_ticker.h
 * @brief Ticker data structures and functions for Hyperliquid C SDK
 *
 * Provides functions to fetch ticker information (prices, volumes, bid/ask)
 * for individual symbols or multiple symbols.
 */

#ifndef HL_TICKER_H
#define HL_TICKER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "hl_error.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct hl_client hl_client_t;
typedef struct hl_ticker hl_ticker_t;
typedef struct hl_tickers hl_tickers_t;

/**
 * @brief Ticker information
 */
struct hl_ticker {
    char symbol[64];           /**< Market symbol */

    // Prices
    double bid;                /**< Best bid price */
    double ask;                /**< Best ask price */
    double last;               /**< Last price (mid price) */
    double close;              /**< Close price */
    double previous_close;     /**< Previous day close price */

    // Volumes
    double quote_volume;       /**< 24h quote volume */

    // Timestamps
    uint64_t timestamp;        /**< Timestamp (ms) */
    char datetime[32];         /**< ISO 8601 datetime string */

    // Additional data (for swaps)
    double mark_price;         /**< Mark price */
    double oracle_price;       /**< Oracle price */
    double funding_rate;       /**< Current funding rate */
    double open_interest;      /**< Open interest */
};

/**
 * @brief Tickers collection
 */
struct hl_tickers {
    struct hl_ticker* tickers; /**< Array of tickers */
    size_t count;              /**< Number of tickers */
};

// ============================================================================
// Ticker API
// ============================================================================

/**
 * @brief Fetch ticker for a single symbol
 *
 * @param client Client instance
 * @param symbol Market symbol (e.g., "BTC/USDC:USDC")
 * @param ticker Output ticker structure
 * @return HL_SUCCESS on success, error code otherwise
 *
 * @note Uses cached market data if available, otherwise fetches markets first
 */
hl_error_t hl_fetch_ticker(hl_client_t* client, const char* symbol, hl_ticker_t* ticker);

/**
 * @brief Fetch tickers for multiple symbols
 *
 * @param client Client instance
 * @param symbols Array of market symbols (NULL for all markets)
 * @param symbols_count Number of symbols in array (0 for all markets)
 * @param tickers Output tickers collection (caller must free with hl_tickers_free)
 * @return HL_SUCCESS on success, error code otherwise
 *
 * @note Fetches fresh market data from API
 */
hl_error_t hl_fetch_tickers(hl_client_t* client, const char** symbols, size_t symbols_count, hl_tickers_t* tickers);

/**
 * @brief Free tickers collection
 *
 * @param tickers Tickers collection to free
 */
void hl_tickers_free(hl_tickers_t* tickers);

#ifdef __cplusplus
}
#endif

#endif // HL_TICKER_H
