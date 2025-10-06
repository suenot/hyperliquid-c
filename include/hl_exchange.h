/**
 * @file hl_exchange.h
 * @brief CCXT-compatible exchange configuration and capabilities
 */

#ifndef HL_EXCHANGE_H
#define HL_EXCHANGE_H

#include <stdbool.h>
#include <stdint.h>

// Forward declarations
typedef struct hl_capabilities hl_capabilities_t;
typedef struct hl_timeframes hl_timeframes_t;
typedef struct hl_api_endpoints hl_api_endpoints_t;
typedef struct hl_urls hl_urls_t;
typedef struct hl_exchange_config hl_exchange_config_t;

/**
 * @brief Exchange capabilities (CCXT 'has' equivalent)
 */
struct hl_capabilities {
    // Markets
    bool spot;
    bool swap;
    bool future;
    bool option;

    // Trading
    bool create_order;
    bool cancel_order;
    bool edit_order;
    bool create_orders;
    bool cancel_orders;
    bool edit_orders;
    bool cancel_all_orders;
    bool create_market_buy_order_with_cost;
    bool create_market_order_with_cost;
    bool create_market_sell_order_with_cost;

    // Advanced Orders
    bool create_stop_order;
    bool create_trigger_order;
    bool create_reduce_only_order;
    bool create_order_with_take_profit_and_stop_loss;

    // Market Data
    bool fetch_markets;
    bool fetch_ticker;
    bool fetch_tickers;
    bool fetch_order_book;
    bool fetch_ohlcv;
    bool fetch_trades;
    bool fetch_open_interest;
    bool fetch_open_interests;

    // Account
    bool fetch_balance;
    bool fetch_positions;
    bool fetch_position;
    bool fetch_my_trades;
    bool fetch_open_orders;
    bool fetch_closed_orders;
    bool fetch_canceled_orders;
    bool fetch_order;
    bool fetch_orders;
    bool fetch_trading_fee;
    bool fetch_trading_fees;

    // Funding
    bool fetch_funding_rate;
    bool fetch_funding_rates;
    bool fetch_funding_history;
    bool fetch_funding_rate_history;

    // Deposits/Withdrawals
    bool fetch_deposits;
    bool fetch_withdrawals;
    bool fetch_ledger;

    // Currencies
    bool fetch_currencies;

    // Advanced
    bool set_leverage;
    bool add_margin;
    bool reduce_margin;
    bool transfer;
    bool withdraw;

    // WebSocket (Pro)
    bool ws;
    bool watch_ticker;
    bool watch_tickers;
    bool watch_order_book;
    bool watch_ohlcv;
    bool watch_trades;
    bool watch_trades_for_symbols;
    bool watch_orders;
    bool watch_my_trades;
    bool create_order_ws;
    bool cancel_order_ws;
    bool create_orders_ws;
    bool edit_order_ws;
};

/**
 * @brief Timeframes supported by exchange
 */
struct hl_timeframes {
    const char* timeframe_1m;
    const char* timeframe_3m;
    const char* timeframe_5m;
    const char* timeframe_15m;
    const char* timeframe_30m;
    const char* timeframe_1h;
    const char* timeframe_2h;
    const char* timeframe_4h;
    const char* timeframe_8h;
    const char* timeframe_12h;
    const char* timeframe_1d;
    const char* timeframe_3d;
    const char* timeframe_1w;
    const char* timeframe_1M;
};

/**
 * @brief API endpoints configuration
 */
struct hl_api_endpoints {
    struct {
        const char* info;
        const char* exchange;
    } public;

    struct {
        const char* exchange;
    } private;
};

/**
 * @brief Exchange URLs
 */
struct hl_urls {
    const char* logo;
    const char* www;
    const char* doc;
    const char* fees;
    const char* referral;

    struct {
        const char* public;
        const char* private;
    } api;

    struct {
        const char* public;
        const char* private;
    } test;

    struct {
        const char* public;
    } ws;

    struct {
        const char* public;
    } test_ws;
};

/**
 * @brief Complete exchange configuration (CCXT describe() equivalent)
 */
struct hl_exchange_config {
    const char* id;
    const char* name;
    const char* version;
    int rate_limit;
    bool certified;
    bool pro;
    bool dex;

    hl_urls_t* urls;
    hl_capabilities_t* has;
    hl_timeframes_t* timeframes;
    hl_api_endpoints_t* api;
};

/**
 * @brief Get default Hyperliquid exchange configuration
 * @return Pointer to exchange config (singleton)
 */
const hl_exchange_config_t* hl_exchange_describe(void);

/**
 * @brief Get exchange capabilities
 * @param config Exchange configuration
 * @return Pointer to capabilities
 */
const hl_capabilities_t* hl_exchange_capabilities(const hl_exchange_config_t* config);

/**
 * @brief Check if exchange supports a specific feature
 * @param config Exchange configuration
 * @param feature Feature name (e.g., "fetch_balance")
 * @return true if supported
 */
bool hl_exchange_has(const hl_exchange_config_t* config, const char* feature);

/**
 * @brief Get supported timeframes
 * @param config Exchange configuration
 * @return Pointer to timeframes
 */
const hl_timeframes_t* hl_exchange_timeframes(const hl_exchange_config_t* config);

#endif // HL_EXCHANGE_H
