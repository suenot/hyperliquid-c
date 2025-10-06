/**
 * @file hl_orderbook.h
 * @brief Order book data structures and functions for Hyperliquid C SDK
 *
 * Provides functions to fetch real-time order book data with bid/ask prices
 * and volumes for trading symbols.
 */

#ifndef HL_ORDERBOOK_H
#define HL_ORDERBOOK_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations (types defined in hyperliquid.h)
// Note: hl_client_t, hl_error_t, hl_book_level_t, hl_orderbook_t are defined in hyperliquid.h

// ============================================================================
// Order Book API
// ============================================================================

/**
 * @brief Fetch order book for a symbol
 *
 * @param client Client instance
 * @param symbol Market symbol (e.g., "BTC/USDC:USDC")
 * @param depth Maximum number of levels to return (0 for all available)
 * @param book Output order book structure (caller must free with hl_free_orderbook)
 * @return HL_SUCCESS on success, error code otherwise
 *
 * @note Fetches fresh L2 order book data from the API
 * @note Bids are sorted highest to lowest price
 * @note Asks are sorted lowest to highest price
 */
hl_error_t hl_fetch_order_book(hl_client_t* client, const char* symbol, uint32_t depth, hl_orderbook_t* book);

/**
 * @brief Free order book memory
 *
 * @param book Order book to free
 */
void hl_free_orderbook(hl_orderbook_t* book);

/**
 * @brief Get best bid price from order book
 *
 * @param book Order book
 * @return Best bid price, or 0.0 if no bids
 */
double hl_orderbook_get_best_bid(const hl_orderbook_t* book);

/**
 * @brief Get best ask price from order book
 *
 * @param book Order book
 * @return Best ask price, or 0.0 if no asks
 */
double hl_orderbook_get_best_ask(const hl_orderbook_t* book);

/**
 * @brief Get spread (ask - bid) from order book
 *
 * @param book Order book
 * @return Spread, or 0.0 if insufficient data
 */
double hl_orderbook_get_spread(const hl_orderbook_t* book);

/**
 * @brief Get bid volume at specified depth
 *
 * @param book Order book
 * @param depth Number of levels to sum (0 for all)
 * @return Total bid volume, or 0.0 if no bids
 */
double hl_orderbook_get_bid_volume(const hl_orderbook_t* book, uint32_t depth);

/**
 * @brief Get ask volume at specified depth
 *
 * @param book Order book
 * @param depth Number of levels to sum (0 for all)
 * @return Total ask volume, or 0.0 if no asks
 */
double hl_orderbook_get_ask_volume(const hl_orderbook_t* book, uint32_t depth);

#ifdef __cplusplus
}
#endif

#endif // HL_ORDERBOOK_H
