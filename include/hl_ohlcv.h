/**
 * @file hl_ohlcv.h
 * @brief OHLCV (Open, High, Low, Close, Volume) data structures and functions for Hyperliquid C SDK
 *
 * Provides functions to fetch historical candlestick data for technical analysis
 * and trading strategies.
 */

#ifndef HL_OHLCV_H
#define HL_OHLCV_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "hl_error.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct hl_client hl_client_t;
typedef struct hl_ohlcv hl_ohlcv_t;
typedef struct hl_ohlcvs hl_ohlcvs_t;

/**
 * @brief OHLCV candlestick data
 */
struct hl_ohlcv {
    uint64_t timestamp;    /**< Candle open timestamp (milliseconds) */
    double open;           /**< Open price */
    double high;           /**< High price */
    double low;            /**< Low price */
    double close;          /**< Close price */
    double volume;         /**< Trading volume */
};

/**
 * @brief OHLCV data collection
 */
struct hl_ohlcvs {
    struct hl_ohlcv* candles; /**< Array of OHLCV candles */
    size_t count;          /**< Number of candles */
    char symbol[64];       /**< Trading symbol */
    char timeframe[16];    /**< Timeframe (e.g., "1m", "1h", "1d") */
};

/**
 * @brief Timeframe constants
 */
#define HL_TIMEFRAME_1M     "1m"
#define HL_TIMEFRAME_3M     "3m"
#define HL_TIMEFRAME_5M     "5m"
#define HL_TIMEFRAME_15M    "15m"
#define HL_TIMEFRAME_30M    "30m"
#define HL_TIMEFRAME_1H     "1h"
#define HL_TIMEFRAME_2H     "2h"
#define HL_TIMEFRAME_4H     "4h"
#define HL_TIMEFRAME_8H     "8h"
#define HL_TIMEFRAME_12H    "12h"
#define HL_TIMEFRAME_1D     "1d"
#define HL_TIMEFRAME_3D     "3d"
#define HL_TIMEFRAME_1W     "1w"
#define HL_TIMEFRAME_1M_MONTH     "1M"

// ============================================================================
// OHLCV API
// ============================================================================

/**
 * @brief Fetch OHLCV candlestick data
 *
 * @param client Client instance
 * @param symbol Market symbol (e.g., "BTC/USDC:USDC")
 * @param timeframe Timeframe (e.g., "1m", "1h", "1d")
 * @param since Start timestamp (milliseconds), NULL for earliest available
 * @param limit Maximum number of candles to return, NULL for no limit
 * @param until End timestamp (milliseconds), NULL for latest available
 * @param ohlcvs Output OHLCV data (caller must free with hl_ohlcvs_free)
 * @return HL_SUCCESS on success, error code otherwise
 *
 * @note Supported timeframes: 1m, 3m, 5m, 15m, 30m, 1h, 2h, 4h, 8h, 12h, 1d, 3d, 1w, 1M
 */
hl_error_t hl_fetch_ohlcv(hl_client_t* client, const char* symbol, const char* timeframe,
                         uint64_t* since, uint32_t* limit, uint64_t* until,
                         hl_ohlcvs_t* ohlcvs);

/**
 * @brief Free OHLCV data memory
 *
 * @param ohlcvs OHLCV data to free
 */
void hl_ohlcvs_free(hl_ohlcvs_t* ohlcvs);

/**
 * @brief Get latest candle from OHLCV data
 *
 * @param ohlcvs OHLCV data
 * @return Pointer to latest candle, or NULL if no data
 */
const hl_ohlcv_t* hl_ohlcvs_get_latest(const hl_ohlcvs_t* ohlcvs);

/**
 * @brief Calculate simple moving average
 *
 * @param ohlcvs OHLCV data
 * @param period SMA period
 * @param use_close Use close price if true, otherwise use (high+low)/2
 * @param sma Output array for SMA values (must be pre-allocated with size ohlcvs->count)
 * @return HL_SUCCESS on success, error code otherwise
 */
hl_error_t hl_ohlcvs_calculate_sma(const hl_ohlcvs_t* ohlcvs, size_t period,
                                  bool use_close, double* sma);

/**
 * @brief Find highest high in period
 *
 * @param ohlcvs OHLCV data
 * @param start_index Starting index
 * @param count Number of candles to check
 * @return Highest high value, or 0.0 if invalid parameters
 */
double hl_ohlcvs_highest_high(const hl_ohlcvs_t* ohlcvs, size_t start_index, size_t count);

/**
 * @brief Find lowest low in period
 *
 * @param ohlcvs OHLCV data
 * @param start_index Starting index
 * @param count Number of candles to check
 * @return Lowest low value, or 0.0 if invalid parameters
 */
double hl_ohlcvs_lowest_low(const hl_ohlcvs_t* ohlcvs, size_t start_index, size_t count);

/**
 * @brief Get candle by index
 *
 * @param ohlcvs OHLCV data
 * @param index Candle index (0 = oldest, count-1 = newest)
 * @return Pointer to candle, or NULL if invalid index
 */
const hl_ohlcv_t* hl_ohlcvs_get_candle(const hl_ohlcvs_t* ohlcvs, size_t index);

#ifdef __cplusplus
}
#endif

#endif // HL_OHLCV_H
