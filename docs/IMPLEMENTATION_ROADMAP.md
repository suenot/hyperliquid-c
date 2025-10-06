# Hyperliquid C SDK - Implementation Roadmap

**Цель**: Полная реализация всех 51 методов CCXT с тестами

**Текущий статус**: 2/51 (4%)  
**Целевой статус**: 51/51 (100%)

---

## 📋 **ROADMAP OVERVIEW**

| Phase | Методы | Приоритет | Срок | Статус |
|-------|--------|-----------|------|--------|
| Phase 1 | 4 | Foundation | ✅ Complete | ✅ DONE |
| Phase 2 | 1 | Connection | ✅ Complete | ✅ DONE |
| Phase 3 | 2 | Trading Core | ✅ Complete | ✅ DONE |
| Phase 4 | 20 | High Priority | 🔴 Critical | 3 weeks |
| Phase 5 | 18 | Medium Priority | 🟡 Important | 2 weeks |
| Phase 6 | 6 | Low Priority | 🟢 Nice to have | 1 week |

**Total Timeline**: 6 weeks for 100% coverage

---

## ✅ **COMPLETED PHASES** (7/51)

### **Phase 1: Foundation** ✅ (4/4 complete)
- [x] Crypto operations (keccak256, EIP-712)
- [x] MessagePack serialization
- [x] HTTP client
- [x] Client management

### **Phase 2: Connection** ✅ (1/1 complete)
- [x] test_connection

### **Phase 3: Trading Core** ✅ (2/2 complete)
- [x] create_order (place_limit_order)
- [x] cancel_order

---

## 🔴 **PHASE 4: HIGH PRIORITY** (0/20 complete)

**Срок**: 3 недели (21 день)  
**Критичность**: Блокирует production use

### **Week 1: Account & Market Foundation** (7 методов)

#### **Day 1-2: Account Balance**
**Файлы**:
- `src/account.c` - Новый модуль
- `include/hl_account.h` - Header
- `tests/integration/03_account/test_fetch_balance.c`

**API**:
```c
// Fetch balance for perpetual or spot
hl_error_t hl_fetch_balance(
    hl_client_t* client,
    hl_account_type_t type,  // HL_ACCOUNT_PERPETUAL | HL_ACCOUNT_SPOT
    hl_balance_t* balance
);

typedef struct {
    double total_usdc;           // Total USDC
    double used_margin;          // Used margin (perpetual)
    double withdrawable;         // Withdrawable amount
    double account_value;        // Total account value
    
    // Spot balances (if type == SPOT)
    hl_spot_balance_t* spot_balances;
    size_t spot_balance_count;
} hl_balance_t;
```

**Эндпоинт**: `POST /info` with `{"type": "clearinghouseState"}` or `{"type": "spotClearinghouseState"}`

---

#### **Day 3: Positions**
**Файлы**:
- `src/account.c` (extend)
- `tests/integration/03_account/test_fetch_positions.c`

**API**:
```c
// Fetch all open positions
hl_error_t hl_fetch_positions(
    hl_client_t* client,
    hl_position_t** positions,
    size_t* count
);

typedef struct {
    char symbol[32];             // Market symbol
    double size;                 // Position size (negative for short)
    double entry_price;          // Average entry price
    double mark_price;           // Current mark price
    double unrealized_pnl;       // Unrealized P&L
    double liquidation_price;    // Liquidation price
    double margin_used;          // Margin used
    int leverage;                // Leverage
    bool is_isolated;            // Margin mode
} hl_position_t;
```

**Эндпоинт**: `POST /info` with `{"type": "clearinghouseState"}`

---

#### **Day 4-5: Markets Metadata** ⚠️ **КРИТИЧНО**
**Файлы**:
- `src/market_data.c` - Новый модуль
- `include/hl_market_data.h`
- `tests/integration/02_market_data/test_fetch_markets.c`

**API**:
```c
// Fetch all markets (swap + spot)
hl_error_t hl_fetch_markets(
    hl_client_t* client,
    hl_market_t** markets,
    size_t* count
);

typedef struct {
    uint32_t asset_id;           // Internal asset ID
    char symbol[32];             // BTC/USDC:USDC
    char base[16];               // BTC
    char quote[16];              // USDC
    bool is_spot;                // Spot or perpetual
    bool active;                 // Trading enabled
    
    int max_leverage;            // Max leverage
    int sz_decimals;             // Size decimals
    double min_order_size;       // Min order size
    double tick_size;            // Price tick size
} hl_market_t;
```

**Эндпоинт**: `POST /info` with `{"type": "metaAndAssetCtxs"}` и `{"type": "spotMetaAndAssetCtxs"}`

**⚠️ ВАЖНО**: Этот метод разблокирует правильное получение asset IDs!

---

#### **Day 6: Ticker**
**Файлы**:
- `src/market_data.c` (extend)
- `tests/integration/02_market_data/test_fetch_ticker.c`

**API**:
```c
// Get current ticker for symbol
hl_error_t hl_fetch_ticker(
    hl_client_t* client,
    const char* symbol,
    hl_ticker_t* ticker
);

typedef struct {
    char symbol[32];
    double last_price;           // Last traded price
    double bid;                  // Best bid
    double ask;                  // Best ask
    double volume_24h;           // 24h volume
    double change_24h;           // 24h change %
    uint64_t timestamp;          // Timestamp
} hl_ticker_t;
```

**Эндпоинт**: `POST /info` with `{"type": "allMids"}` или `{"type": "metaAndAssetCtxs"}`

---

#### **Day 7: Order Book**
**Файлы**:
- `src/market_data.c` (extend)
- `tests/integration/02_market_data/test_fetch_order_book.c`

**API**:
```c
// Get L2 order book
hl_error_t hl_fetch_order_book(
    hl_client_t* client,
    const char* symbol,
    int depth,                   // Max levels
    hl_order_book_t* book
);

typedef struct {
    double price;
    double quantity;
} hl_level_t;

typedef struct {
    char symbol[32];
    hl_level_t* bids;
    size_t bid_count;
    hl_level_t* asks;
    size_t ask_count;
    uint64_t timestamp;
} hl_order_book_t;
```

**Эндпоинт**: `POST /info` with `{"type": "l2Book", "coin": "BTC"}`

---

### **Week 2: Order Management** (8 методов)

#### **Day 8: Fetch Order**
**Файлы**:
- `src/trading_api.c` (extend)
- `tests/integration/04_trading/test_fetch_order.c`

**API**:
```c
// Get order status
hl_error_t hl_fetch_order(
    hl_client_t* client,
    const char* symbol,
    const char* order_id,
    hl_order_info_t* order
);

typedef struct {
    char order_id[64];
    char client_order_id[64];
    char symbol[32];
    hl_side_t side;
    hl_order_type_t type;
    hl_order_status_t status;
    double price;
    double quantity;
    double filled;
    double remaining;
    uint64_t timestamp;
} hl_order_info_t;
```

**Эндпоинт**: `POST /info` with `{"type": "orderStatus", "oid": ...}`

---

#### **Day 9: Fetch Orders (History)**
**Файлы**:
- `src/trading_api.c` (extend)
- `tests/integration/04_trading/test_fetch_orders.c`

**API**:
```c
// Get order history
hl_error_t hl_fetch_orders(
    hl_client_t* client,
    const char* symbol,
    uint64_t since,              // Start timestamp (0 = all)
    int limit,                   // Max results
    hl_order_info_t** orders,
    size_t* count
);
```

**Эндпоинт**: `POST /info` with `{"type": "historicalOrders"}`

---

#### **Day 10: Fetch Open Orders**
**Файлы**:
- `src/trading_api.c` (extend)
- `tests/integration/04_trading/test_fetch_open_orders.c`

**API**:
```c
// Get currently open orders
hl_error_t hl_fetch_open_orders(
    hl_client_t* client,
    const char* symbol,          // NULL = all symbols
    hl_order_info_t** orders,
    size_t* count
);
```

**Эндпоинт**: `POST /info` with `{"type": "frontendOpenOrders"}`

---

#### **Day 11-12: Fetch My Trades**
**Файлы**:
- `src/trading_api.c` (extend)
- `tests/integration/04_trading/test_fetch_my_trades.c`

**API**:
```c
// Get trade history
hl_error_t hl_fetch_my_trades(
    hl_client_t* client,
    const char* symbol,
    uint64_t since,
    int limit,
    hl_trade_t** trades,
    size_t* count
);

typedef struct {
    char trade_id[64];
    char order_id[64];
    char symbol[32];
    hl_side_t side;
    double price;
    double quantity;
    double fee;
    char fee_currency[16];
    uint64_t timestamp;
} hl_trade_t;
```

**Эндпоинт**: `POST /info` with `{"type": "userFills"}` or `{"type": "userFillsByTime"}`

---

#### **Day 13-14: Create Orders (Batch)**
**Файлы**:
- `src/trading_api.c` (extend)
- `tests/integration/04_trading/test_create_orders.c`

**API**:
```c
// Place multiple orders atomically
hl_error_t hl_create_orders(
    hl_client_t* client,
    const hl_order_request_t* orders,
    size_t count,
    hl_order_result_t* results
);
```

**Эндпоинт**: `POST /exchange` with multiple orders in `{"action": {"type": "order", "orders": [...]}}`

---

#### **Day 15: Cancel Orders (Batch)**
**Файлы**:
- `src/trading_api.c` (extend)
- `tests/integration/04_trading/test_cancel_orders.c`

**API**:
```c
// Cancel multiple orders
hl_error_t hl_cancel_orders(
    hl_client_t* client,
    const char* symbol,
    const char** order_ids,
    size_t count,
    hl_cancel_result_t* results
);
```

**Эндпоинт**: `POST /exchange` with `{"action": {"type": "cancel", "cancels": [...]}}`

---

#### **Day 16-17: Edit Order**
**Файлы**:
- `src/trading_api.c` (extend)
- `tests/integration/04_trading/test_edit_order.c`

**API**:
```c
// Modify existing order
hl_error_t hl_edit_order(
    hl_client_t* client,
    const char* order_id,
    const hl_order_request_t* new_params,
    hl_order_result_t* result
);
```

**Эндпоинт**: `POST /exchange` with `{"action": {"type": "batchModify"}}`

---

#### **Day 18: Edit Orders (Batch)**
**Файлы**:
- `src/trading_api.c` (extend)
- `tests/integration/04_trading/test_edit_orders.c`

**API**:
```c
// Modify multiple orders
hl_error_t hl_edit_orders(
    hl_client_t* client,
    const hl_order_edit_t* edits,
    size_t count,
    hl_order_result_t* results
);
```

---

### **Week 3: Leverage, OHLCV, Transfers** (5 методов)

#### **Day 19-20: OHLCV (Candles)**
**Файлы**:
- `src/market_data.c` (extend)
- `tests/integration/02_market_data/test_fetch_ohlcv.c`

**API**:
```c
// Get historical candles
hl_error_t hl_fetch_ohlcv(
    hl_client_t* client,
    const char* symbol,
    hl_timeframe_t timeframe,  // 1m, 5m, 15m, 1h, 1d, etc
    uint64_t since,
    int limit,
    hl_candle_t** candles,
    size_t* count
);

typedef struct {
    uint64_t timestamp;
    double open;
    double high;
    double low;
    double close;
    double volume;
} hl_candle_t;
```

**Эндпоинт**: `POST /info` with `{"type": "candleSnapshot"}`

---

#### **Day 21: Set Leverage**
**Файлы**:
- `src/margin.c` - Новый модуль
- `include/hl_margin.h`
- `tests/integration/05_margin/test_set_leverage.c`

**API**:
```c
// Set leverage for symbol
hl_error_t hl_set_leverage(
    hl_client_t* client,
    const char* symbol,
    int leverage,                // 1-50
    bool is_cross                // Cross or isolated
);
```

**Эндпоинт**: `POST /exchange` with `{"action": {"type": "updateLeverage"}}`

---

#### **Day 22: Set Margin Mode**
**Файлы**:
- `src/margin.c` (extend)
- `tests/integration/05_margin/test_set_margin_mode.c`

**API**:
```c
// Set margin mode
hl_error_t hl_set_margin_mode(
    hl_client_t* client,
    const char* symbol,
    hl_margin_mode_t mode,       // HL_MARGIN_CROSS | HL_MARGIN_ISOLATED
    int leverage
);
```

---

#### **Day 23: Transfer (Spot↔Swap)**
**Файлы**:
- `src/transfers.c` - Новый модуль
- `include/hl_transfers.h`
- `tests/integration/06_transfers/test_transfer_spot_swap.c`

**API**:
```c
// Transfer between spot and perpetual
hl_error_t hl_transfer(
    hl_client_t* client,
    hl_account_type_t from,
    hl_account_type_t to,
    double amount
);
```

**Эндпоинт**: `POST /exchange` with `{"action": {"type": "usdClassTransfer"}}`

---

#### **Day 24: Withdraw**
**Файлы**:
- `src/transfers.c` (extend)
- `tests/integration/06_transfers/test_withdraw.c`

**API**:
```c
// Withdraw USDC to address
hl_error_t hl_withdraw(
    hl_client_t* client,
    double amount,
    const char* destination_address
);
```

**Эндпоинт**: `POST /exchange` with `{"action": {"type": "withdraw3"}}`

---

## 🟡 **PHASE 5: MEDIUM PRIORITY** (0/18 complete)

**Срок**: 2 недели (14 дней)

### **Week 4: Extended Market Data & Orders** (9 методов)

- [ ] fetch_tickers (all markets)
- [ ] fetch_currencies
- [ ] fetch_funding_rates
- [ ] fetch_trades (public)
- [ ] fetch_position (single)
- [ ] fetch_trading_fee
- [ ] fetch_closed_orders
- [ ] cancel_orders_for_symbols
- [ ] cancel_all_orders_after

### **Week 5: Margin & Transfers Extended** (9 методов)

- [ ] add_margin
- [ ] reduce_margin
- [ ] transfer (main↔subaccount)
- [ ] transfer (non-USDC spot)
- [ ] fetch_ledger
- [ ] fetch_deposits
- [ ] fetch_withdrawals
- [ ] fetch_funding_rate_history
- [ ] fetch_open_interest

---

## 🟢 **PHASE 6: LOW PRIORITY** (0/6 complete)

**Срок**: 1 неделя (7 дней)

### **Week 6: Advanced Features** (6 методов)

- [ ] fetch_funding_history
- [ ] create_vault
- [ ] approve_builder_fee
- [ ] reserve_request_weight
- [ ] fetch_open_interests
- [ ] parse utilities

---

## 📊 **PROGRESS TRACKING**

### **Completed**
```
Phase 1: ████████████████████ 100% (4/4)
Phase 2: ████████████████████ 100% (1/1)
Phase 3: ████████████████████ 100% (2/2)
```

### **In Progress**
```
Phase 4: ░░░░░░░░░░░░░░░░░░░░ 0% (0/20)
Phase 5: ░░░░░░░░░░░░░░░░░░░░ 0% (0/18)
Phase 6: ░░░░░░░░░░░░░░░░░░░░ 0% (0/6)
```

### **Overall**
```
Total: ██░░░░░░░░░░░░░░░░░░ 13.7% (7/51)
```

---

## 🎯 **MILESTONES**

| Milestone | Methods | Target Date | Status |
|-----------|---------|-------------|--------|
| **M1: Foundation** | 7 | 2025-10-06 | ✅ DONE |
| **M2: Week 1 Complete** | 14 | 2025-10-13 | 📋 Pending |
| **M3: Week 2 Complete** | 22 | 2025-10-20 | 📋 Pending |
| **M4: Week 3 Complete** | 27 | 2025-10-27 | 📋 Pending |
| **M5: Phase 4 Done** | 27 | 2025-10-27 | 📋 Pending |
| **M6: Phase 5 Done** | 45 | 2025-11-10 | 📋 Pending |
| **M7: 100% Coverage** | 51 | 2025-11-17 | 📋 Pending |

---

## 💡 **DEVELOPMENT NOTES**

### **Critical Dependencies**
1. **fetch_markets** MUST be implemented first (Day 4-5)
   - Provides asset IDs for all other methods
   - Blocks: all trading operations

2. **fetch_balance** enables account monitoring
   - Required for risk management
   - Blocks: position sizing logic

3. **fetch_positions** enables position tracking
   - Required for closing positions
   - Blocks: risk management

### **Testing Strategy**
- Each method gets dedicated test file
- Test both success and error cases
- Use small amounts on testnet
- Test rate limiting
- Test concurrent requests

### **Code Organization**
```
src/
├── account.c           # Balance, positions
├── market_data.c       # Markets, tickers, OHLCV
├── trading_api.c       # Orders (existing + new)
├── margin.c            # Leverage, margin mode
├── transfers.c         # Transfers, withdrawals
└── advanced.c          # Vaults, builder fees
```

---

**Last Updated**: 2025-10-06  
**Next Review**: Daily during Phase 4  
**Estimated Completion**: 2025-11-17 (6 weeks)

