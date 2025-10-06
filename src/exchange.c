/**
 * @file exchange.c
 * @brief CCXT-compatible exchange configuration implementation
 */

#include "hl_exchange.h"
#include <stdlib.h>
#include <string.h>

// Static exchange configuration (singleton)
static const hl_timeframes_t HL_TIMEFRAMES = {
    .timeframe_1m = "1m",
    .timeframe_3m = "3m",
    .timeframe_5m = "5m",
    .timeframe_15m = "15m",
    .timeframe_30m = "30m",
    .timeframe_1h = "1h",
    .timeframe_2h = "2h",
    .timeframe_4h = "4h",
    .timeframe_8h = "8h",
    .timeframe_12h = "12h",
    .timeframe_1d = "1d",
    .timeframe_3d = "3d",
    .timeframe_1w = "1w",
    .timeframe_1M = "1M",
};

static const hl_api_endpoints_t HL_API_ENDPOINTS = {
    .public = {
        .info = "info",
        .exchange = "exchange"
    },
    .private = {
        .exchange = "exchange"
    }
};

static const hl_urls_t HL_URLS = {
    .logo = "https://github.com/ccxt/ccxt/assets/43336371/b371bc6c-4a8c-489f-87f4-20a913dd8d4b",
    .api = {
        .public = "https://api.hyperliquid.xyz",
        .private = "https://api.hyperliquid.xyz"
    },
    .test = {
        .public = "https://api.hyperliquid-testnet.xyz",
        .private = "https://api.hyperliquid-testnet.xyz"
    },
    .ws = {
        .public = "wss://api.hyperliquid.xyz/ws"
    },
    .test_ws = {
        .public = "wss://api.hyperliquid-testnet.xyz/ws"
    },
    .www = "https://hyperliquid.xyz",
    .doc = "https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api",
    .fees = "https://hyperliquid.gitbook.io/hyperliquid-docs/trading/fees",
    .referral = "https://app.hyperliquid.xyz/"
};

static const hl_capabilities_t HL_CAPABILITIES = {
    // Markets
    .spot = true,
    .swap = true,
    .future = true,
    .option = false,

    // Trading
    .create_order = true,
    .cancel_order = true,
    .edit_order = true,
    .create_orders = true,
    .cancel_orders = true,
    .edit_orders = true,
    .cancel_all_orders = false,
    .create_market_buy_order_with_cost = false,
    .create_market_order_with_cost = false,
    .create_market_sell_order_with_cost = false,

    // Advanced Orders
    .create_stop_order = true,
    .create_trigger_order = true,
    .create_reduce_only_order = true,
    .create_order_with_take_profit_and_stop_loss = true,

    // Market Data
    .fetch_markets = true,
    .fetch_ticker = true,
    .fetch_tickers = true,
    .fetch_order_book = true,
    .fetch_ohlcv = true,
    .fetch_trades = true,
    .fetch_open_interest = true,
    .fetch_open_interests = true,

    // Account
    .fetch_balance = true,
    .fetch_positions = true,
    .fetch_position = true,
    .fetch_my_trades = true,
    .fetch_open_orders = true,
    .fetch_closed_orders = true,
    .fetch_canceled_orders = true,
    .fetch_order = true,
    .fetch_orders = true,
    .fetch_trading_fee = true,
    .fetch_trading_fees = false,

    // Funding
    .fetch_funding_rate = false,
    .fetch_funding_rates = true,
    .fetch_funding_history = true,
    .fetch_funding_rate_history = true,

    // Deposits/Withdrawals
    .fetch_deposits = true,
    .fetch_withdrawals = true,
    .fetch_ledger = true,

    // Advanced
    .set_leverage = true,
    .add_margin = true,
    .reduce_margin = true,
    .transfer = true,
    .withdraw = true,

    // WebSocket (Pro)
    .ws = true,
    .watch_ticker = true,
    .watch_tickers = true,
    .watch_order_book = true,
    .watch_ohlcv = true,
    .watch_trades = true,
    .watch_trades_for_symbols = false,
    .watch_orders = true,
    .watch_my_trades = true,
    .create_order_ws = true,
    .cancel_order_ws = true,
    .create_orders_ws = true,
    .edit_order_ws = true,
};

static const hl_exchange_config_t HL_EXCHANGE_CONFIG = {
    .id = "hyperliquid",
    .name = "Hyperliquid",
    .version = "v1",
    .rate_limit = 50,
    .certified = true,
    .pro = true,
    .dex = true,
    .urls = &HL_URLS,
    .has = &HL_CAPABILITIES,
    .timeframes = &HL_TIMEFRAMES,
    .api = &HL_API_ENDPOINTS,
};

const hl_exchange_config_t* hl_exchange_describe(void) {
    return &HL_EXCHANGE_CONFIG;
}

const hl_capabilities_t* hl_exchange_capabilities(const hl_exchange_config_t* config) {
    return config ? config->has : NULL;
}

bool hl_exchange_has(const hl_exchange_config_t* config, const char* feature) {
    if (!config || !feature || !config->has) {
        return false;
    }

    // String-based capability checking
    const hl_capabilities_t* has = config->has;

    if (strcmp(feature, "spot") == 0) return has->spot;
    if (strcmp(feature, "swap") == 0) return has->swap;
    if (strcmp(feature, "future") == 0) return has->future;
    if (strcmp(feature, "option") == 0) return has->option;

    if (strcmp(feature, "create_order") == 0) return has->create_order;
    if (strcmp(feature, "cancel_order") == 0) return has->cancel_order;
    if (strcmp(feature, "edit_order") == 0) return has->edit_order;
    if (strcmp(feature, "create_orders") == 0) return has->create_orders;
    if (strcmp(feature, "cancel_orders") == 0) return has->cancel_orders;
    if (strcmp(feature, "edit_orders") == 0) return has->edit_orders;
    if (strcmp(feature, "cancel_all_orders") == 0) return has->cancel_all_orders;

    if (strcmp(feature, "create_stop_order") == 0) return has->create_stop_order;
    if (strcmp(feature, "create_trigger_order") == 0) return has->create_trigger_order;
    if (strcmp(feature, "create_reduce_only_order") == 0) return has->create_reduce_only_order;
    if (strcmp(feature, "create_order_with_take_profit_and_stop_loss") == 0) return has->create_order_with_take_profit_and_stop_loss;

    if (strcmp(feature, "fetch_markets") == 0) return has->fetch_markets;
    if (strcmp(feature, "fetch_ticker") == 0) return has->fetch_ticker;
    if (strcmp(feature, "fetch_tickers") == 0) return has->fetch_tickers;
    if (strcmp(feature, "fetch_order_book") == 0) return has->fetch_order_book;
    if (strcmp(feature, "fetch_ohlcv") == 0) return has->fetch_ohlcv;
    if (strcmp(feature, "fetch_trades") == 0) return has->fetch_trades;
    if (strcmp(feature, "fetch_open_interest") == 0) return has->fetch_open_interest;
    if (strcmp(feature, "fetch_open_interests") == 0) return has->fetch_open_interests;

    if (strcmp(feature, "fetch_balance") == 0) return has->fetch_balance;
    if (strcmp(feature, "fetch_positions") == 0) return has->fetch_positions;
    if (strcmp(feature, "fetch_position") == 0) return has->fetch_position;
    if (strcmp(feature, "fetch_my_trades") == 0) return has->fetch_my_trades;
    if (strcmp(feature, "fetch_open_orders") == 0) return has->fetch_open_orders;
    if (strcmp(feature, "fetch_closed_orders") == 0) return has->fetch_closed_orders;
    if (strcmp(feature, "fetch_canceled_orders") == 0) return has->fetch_canceled_orders;
    if (strcmp(feature, "fetch_order") == 0) return has->fetch_order;
    if (strcmp(feature, "fetch_orders") == 0) return has->fetch_orders;
    if (strcmp(feature, "fetch_trading_fee") == 0) return has->fetch_trading_fee;
    if (strcmp(feature, "fetch_trading_fees") == 0) return has->fetch_trading_fees;

    if (strcmp(feature, "fetch_funding_rate") == 0) return has->fetch_funding_rate;
    if (strcmp(feature, "fetch_funding_rates") == 0) return has->fetch_funding_rates;
    if (strcmp(feature, "fetch_funding_history") == 0) return has->fetch_funding_history;
    if (strcmp(feature, "fetch_funding_rate_history") == 0) return has->fetch_funding_rate_history;

    if (strcmp(feature, "fetch_deposits") == 0) return has->fetch_deposits;
    if (strcmp(feature, "fetch_withdrawals") == 0) return has->fetch_withdrawals;
    if (strcmp(feature, "fetch_ledger") == 0) return has->fetch_ledger;

    if (strcmp(feature, "set_leverage") == 0) return has->set_leverage;
    if (strcmp(feature, "add_margin") == 0) return has->add_margin;
    if (strcmp(feature, "reduce_margin") == 0) return has->reduce_margin;
    if (strcmp(feature, "transfer") == 0) return has->transfer;
    if (strcmp(feature, "withdraw") == 0) return has->withdraw;

    if (strcmp(feature, "ws") == 0) return has->ws;
    if (strcmp(feature, "watch_ticker") == 0) return has->watch_ticker;
    if (strcmp(feature, "watch_tickers") == 0) return has->watch_tickers;
    if (strcmp(feature, "watch_order_book") == 0) return has->watch_order_book;
    if (strcmp(feature, "watch_ohlcv") == 0) return has->watch_ohlcv;
    if (strcmp(feature, "watch_trades") == 0) return has->watch_trades;
    if (strcmp(feature, "watch_trades_for_symbols") == 0) return has->watch_trades_for_symbols;
    if (strcmp(feature, "watch_orders") == 0) return has->watch_orders;
    if (strcmp(feature, "watch_my_trades") == 0) return has->watch_my_trades;
    if (strcmp(feature, "create_order_ws") == 0) return has->create_order_ws;
    if (strcmp(feature, "cancel_order_ws") == 0) return has->cancel_order_ws;
    if (strcmp(feature, "create_orders_ws") == 0) return has->create_orders_ws;
    if (strcmp(feature, "edit_order_ws") == 0) return has->edit_order_ws;

    return false;
}

const hl_timeframes_t* hl_exchange_timeframes(const hl_exchange_config_t* config) {
    return config ? config->timeframes : NULL;
}
