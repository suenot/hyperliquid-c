# CCXT-Compatible Hyperliquid C SDK Architecture Plan

## 🎯 **Цель**
Создать C библиотеку полностью совместимую со стандартом CCXT (Cryptocurrency eXchange Trading Library) для Hyperliquid биржи.

## 📊 **CCXT Структура Анализа**

### **Base API (REST) - 51+ методов:**
```c
// Core Trading
hl_create_order()           // ✅ IMPLEMENTED
hl_cancel_order()           // ✅ IMPLEMENTED
hl_edit_order()
hl_create_orders()
hl_cancel_orders()
hl_edit_orders()

// Market Data
hl_fetch_markets()          // ✅ IMPLEMENTED
hl_fetch_ticker()           // ✅ IMPLEMENTED
hl_fetch_tickers()
hl_fetch_order_book()       // ✅ IMPLEMENTED
hl_fetch_ohlcv()            // ✅ IMPLEMENTED
hl_fetch_trades()
hl_fetch_open_interest()

// Account
hl_fetch_balance()          // ✅ IMPLEMENTED
hl_fetch_positions()        // ✅ IMPLEMENTED
hl_fetch_my_trades()
hl_fetch_open_orders()
hl_fetch_closed_orders()
hl_fetch_canceled_orders()
hl_fetch_order()
hl_fetch_orders()
hl_fetch_trading_fee()

// Funding
hl_fetch_funding_rate()
hl_fetch_funding_rates()
hl_fetch_funding_history()
hl_fetch_funding_rate_history()

// Deposits/Withdrawals
hl_fetch_deposits()
hl_fetch_withdrawals()
hl_fetch_ledger()

// Advanced
hl_set_leverage()
hl_add_margin()
hl_reduce_margin()
hl_transfer()
hl_withdraw()
```

### **Pro API (WebSocket) - 15+ методов:**
```c
// Real-time Data
hl_watch_ticker()
hl_watch_tickers()
hl_watch_order_book()
hl_watch_ohlcv()
hl_watch_trades()
hl_watch_my_trades()

// Real-time Trading
hl_watch_orders()
hl_create_order_ws()
hl_cancel_order_ws()
hl_create_orders_ws()
hl_edit_order_ws()

// Real-time Account
hl_watch_balance()          // NOT SUPPORTED
hl_watch_positions()        // NOT SUPPORTED
```

## 🏗️ **Новая Архитектура**

### **1. Exchange Configuration**
```c
typedef struct {
    const char* id;                    // "hyperliquid"
    const char* name;                  // "Hyperliquid"
    const char* version;               // "v1"
    int rate_limit;                    // 50 ms
    bool certified;                    // true
    bool pro;                          // true (has WebSocket)
    bool dex;                          // true (decentralized)

    // URLs
    struct {
        const char* api_rest;
        const char* api_ws;
        const char* test_rest;
        const char* test_ws;
        const char* www;
        const char* doc;
    } urls;

    // Capabilities
    hl_capabilities_t* has;

    // Timeframes
    hl_timeframes_t* timeframes;

    // API endpoints
    hl_api_endpoints_t* api;

} hl_exchange_config_t;
```

### **2. Capabilities Structure**
```c
typedef struct {
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

    // Account
    bool fetch_balance;
    bool fetch_positions;
    bool fetch_my_trades;
    bool fetch_open_orders;
    bool fetch_closed_orders;
    bool fetch_order;
    bool fetch_trading_fee;

    // WebSocket (Pro)
    bool ws;
    bool watch_ticker;
    bool watch_tickers;
    bool watch_order_book;
    bool watch_ohlcv;
    bool watch_trades;
    bool watch_orders;
    bool watch_my_trades;
    bool create_order_ws;
    bool cancel_order_ws;

} hl_capabilities_t;
```

### **3. Unified Data Structures**

#### **Order Structure (CCXT Compatible)**
```c
typedef struct {
    char id[64];                    // order ID
    char client_order_id[64];       // client order ID
    char timestamp[32];             // timestamp
    char datetime[32];              // ISO 8601 datetime
    char last_trade_timestamp[32];  // last trade timestamp
    char symbol[32];                // trading symbol
    char type[16];                  // order type: 'limit', 'market', etc
    char side[8];                   // 'buy' or 'sell'
    char time_in_force[8];          // 'GTC', 'IOC', 'FOK'
    char status[16];                // order status

    // Prices and amounts
    double price;                   // order price
    double amount;                  // order amount
    double filled;                  // filled amount
    double remaining;               // remaining amount
    double cost;                    // total cost
    double average;                 // average fill price
    double stop_price;              // stop price
    double trigger_price;           // trigger price

    // Fees
    double fee_cost;
    char fee_currency[16];

    // Additional
    bool reduce_only;               // reduce-only flag
    bool post_only;                 // post-only flag
    double leverage;                // leverage
    char info[1024];                // raw exchange response
} hl_order_t;
```

#### **Market Structure**
```c
typedef struct {
    char id[32];                    // market id
    char symbol[32];                // unified symbol
    char base[16];                  // base currency
    char quote[16];                 // quote currency
    char settle[16];                // settlement currency
    char type[16];                  // 'spot' or 'swap'

    // Trading rules
    bool active;                    // market is active
    bool spot;                      // spot market
    bool swap;                      // swap market
    bool future;                    // future market
    bool option;                    // option market

    // Precision
    int amount_precision;           // amount precision
    int price_precision;            // price precision
    int cost_precision;             // cost precision

    // Limits
    struct {
        double amount_min;
        double amount_max;
        double price_min;
        double price_max;
        double cost_min;
        double cost_max;
    } limits;

    // Leverage
    struct {
        double min;
        double max;
    } leverage;

    char info[1024];                // raw exchange data
} hl_market_t;
```

#### **Ticker Structure**
```c
typedef struct {
    char symbol[32];                // trading symbol
    char timestamp[32];             // timestamp
    char datetime[32];              // ISO 8601 datetime

    // Prices
    double last;                    // last price
    double open;                    // opening price
    double close;                   // closing price
    double bid;                     // best bid price
    double ask;                     // best ask price
    double high;                    // highest price
    double low;                     // lowest price

    // Volumes
    double base_volume;             // base volume
    double quote_volume;            // quote volume

    // Additional (swap-specific)
    double mark_price;              // mark price
    double index_price;             // index price
    double funding_rate;            // funding rate
    double open_interest;           // open interest

    char info[1024];                // raw exchange data
} hl_ticker_t;
```

### **4. Client Structure**
```c
typedef struct hl_client {
    // Configuration
    hl_exchange_config_t* config;
    bool testnet;

    // Authentication
    char api_key[128];
    char secret[128];
    char wallet_address[64];

    // HTTP/WebSocket clients
    hl_http_client_t* http_client;
    hl_ws_client_t* ws_client;

    // State
    hl_markets_t* markets;           // cached markets
    hl_currencies_t* currencies;     // cached currencies

    // Options
    hl_options_t* options;

    // Callbacks
    hl_callback_t on_error;
    hl_callback_t on_message;
} hl_client_t;
```

### **5. API Methods Classification**

#### **Public REST API:**
- `hl_fetch_markets()` - get all markets
- `hl_fetch_ticker(symbol)` - get ticker
- `hl_fetch_tickers(symbols)` - get multiple tickers
- `hl_fetch_order_book(symbol, limit)` - get order book
- `hl_fetch_ohlcv(symbol, timeframe, since, limit)` - get OHLCV
- `hl_fetch_trades(symbol, since, limit)` - get trades
- `hl_fetch_open_interest(symbol)` - get open interest

#### **Private REST API:**
- `hl_fetch_balance()` - get account balance
- `hl_fetch_positions(symbols)` - get positions
- `hl_fetch_my_trades(symbol, since, limit)` - get user trades
- `hl_fetch_open_orders(symbol, since, limit)` - get open orders
- `hl_create_order(symbol, type, side, amount, price, params)` - place order
- `hl_cancel_order(id, symbol, params)` - cancel order
- `hl_fetch_order(id, symbol)` - get order info

#### **WebSocket API (Pro):**
- `hl_watch_ticker(symbol, callback)` - subscribe to ticker updates
- `hl_watch_order_book(symbol, limit, callback)` - subscribe to order book
- `hl_watch_ohlcv(symbol, timeframe, callback)` - subscribe to OHLCV
- `hl_watch_orders(callback)` - subscribe to order updates
- `hl_create_order_ws(...)` - place order via WebSocket

## 📋 **Новый План Разработки**

### **Phase 7: Architecture Redesign (Current)**
1. ✅ Analyze CCXT structure and methods
2. 🔄 Create unified data structures
3. 🔄 Refactor client architecture
4. 🔄 Update API headers
5. 🔄 Migrate existing methods to new structures

### **Phase 8: Base API Completion**
1. Implement missing Base API methods
2. Standardize error handling
3. Add comprehensive tests

### **Phase 9: Pro API Implementation**
1. Implement WebSocket client
2. Add real-time data subscriptions
3. WebSocket trading methods

### **Phase 10: Testing & Documentation**
1. Full test coverage
2. CCXT compatibility verification
3. Documentation and examples

## 🎯 **Преимущества Новой Архитектуры**

1. **CCXT Compatibility** - 100% совместимость с CCXT стандартом
2. **Unified API** - консистентные методы и структуры данных
3. **Extensibility** - легко добавлять новые биржи
4. **Real-time Support** - WebSocket для live данных
5. **Production Ready** - enterprise-grade качество

## 📊 **Совместимость с CCXT**

| CCXT Method | C Implementation | Status |
|-------------|------------------|--------|
| describe() | hl_exchange_describe() | ✅ Planned |
| fetchMarkets() | hl_fetch_markets() | ✅ Done |
| fetchBalance() | hl_fetch_balance() | ✅ Done |
| createOrder() | hl_create_order() | ✅ Done |
| fetchTicker() | hl_fetch_ticker() | ✅ Done |
| watchOrderBook() | hl_watch_order_book() | 🔄 Planned |
| ... | ... | ... |

**Цель:** Полная совместимость со всеми 51+ методами CCXT для Hyperliquid.
