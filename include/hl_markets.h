/**
 * @file hl_markets.h
 * @brief Market data and symbol mapping for Hyperliquid C SDK
 *
 * Provides functions to fetch all available markets, get asset IDs by symbol,
 * and convert between different symbol representations.
 */

#ifndef HL_MARKETS_H
#define HL_MARKETS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "hl_error.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct hl_client hl_client_t;
typedef struct hl_market hl_market_t;
typedef struct hl_markets hl_markets_t;

/**
 * @brief Market type
 */
typedef enum {
    HL_MARKET_SWAP,    /**< Perpetual swap market */
    HL_MARKET_SPOT     /**< Spot market */
} hl_market_type_t;

/**
 * @brief Market information
 */
struct hl_market {
    char symbol[64];           /**< Unified symbol (e.g., "BTC/USDC:USDC") */
    char base[32];             /**< Base currency (e.g., "BTC") */
    char quote[32];            /**< Quote currency (e.g., "USDC") */
    char settle[32];           /**< Settlement currency (e.g., "USDC") */
    char base_id[16];          /**< Base asset ID (numeric string) */
    uint32_t asset_id;         /**< Asset ID as integer */

    hl_market_type_t type;     /**< Market type */
    bool active;               /**< Is market active */

    // Precision
    int amount_precision;      /**< Amount precision (szDecimals) */
    int price_precision;       /**< Price precision */

    // Limits
    int max_leverage;          /**< Maximum leverage */
    double min_cost;           /**< Minimum order cost */

    // Current prices (if available)
    double mark_price;         /**< Mark price */
    double oracle_price;       /**< Oracle price */
    double funding_rate;       /**< Current funding rate */

    // Volumes
    double day_volume;         /**< 24h volume */
    double open_interest;      /**< Open interest (for swaps) */
};

/**
 * @brief Markets collection
 */
struct hl_markets {
    struct hl_market* markets; /**< Array of markets */
    size_t count;              /**< Number of markets */
};

// ============================================================================
// Markets API
// ============================================================================

/**
 * @brief Fetch all available markets
 *
 * @param client Client instance
 * @param markets Output markets collection (caller must free with hl_markets_free)
 * @return HL_SUCCESS on success, error code otherwise
 *
 * @note Fetches both perpetual and spot markets
 */
hl_error_t hl_fetch_markets(hl_client_t* client, hl_markets_t* markets);

/**
 * @brief Free markets collection
 *
 * @param markets Markets collection to free
 */
void hl_markets_free(hl_markets_t* markets);

/**
 * @brief Get asset ID by symbol
 *
 * @param markets Markets collection
 * @param symbol Unified symbol (e.g., "BTC/USDC:USDC")
 * @param asset_id Output asset ID
 * @return HL_SUCCESS if found, HL_ERROR_NOT_FOUND otherwise
 */
hl_error_t hl_get_asset_id(const hl_markets_t* markets, const char* symbol, uint32_t* asset_id);

/**
 * @brief Get market info by symbol
 *
 * @param markets Markets collection
 * @param symbol Unified symbol (e.g., "BTC/USDC:USDC")
 * @param market Output market info (pointer to internal data, do not free)
 * @return HL_SUCCESS if found, HL_ERROR_NOT_FOUND otherwise
 */
hl_error_t hl_get_market(const hl_markets_t* markets, const char* symbol, const hl_market_t** market);

/**
 * @brief Get market info by asset ID
 *
 * @param markets Markets collection
 * @param asset_id Asset ID
 * @param market Output market info (pointer to internal data, do not free)
 * @return HL_SUCCESS if found, HL_ERROR_NOT_FOUND otherwise
 */
hl_error_t hl_get_market_by_id(const hl_markets_t* markets, uint32_t asset_id, const hl_market_t** market);

#ifdef __cplusplus
}
#endif

#endif // HL_MARKETS_H
