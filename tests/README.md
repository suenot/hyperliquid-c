# Hyperliquid C SDK - Test Suite

**Цель**: Полное покрытие всех 51 методов API с интеграционными тестами на testnet.

## 📁 Структура Тестов

```
tests/
├── README.md                          # Этот файл
├── unit/                              # Unit тесты (модули)
│   ├── test_crypto.c                 # Crypto (keccak, EIP-712)
│   ├── test_msgpack.c                # MessagePack serialization
│   ├── test_http.c                   # HTTP client
│   └── test_client.c                 # Client creation/config
│
├── integration/                       # Integration тесты (testnet)
│   ├── 01_connection/
│   │   └── test_connection.c         # Test connection
│   │
│   ├── 02_market_data/
│   │   ├── test_fetch_markets.c      # fetch_markets
│   │   ├── test_fetch_currencies.c   # fetch_currencies
│   │   ├── test_fetch_ticker.c       # fetch_ticker
│   │   ├── test_fetch_tickers.c      # fetch_tickers
│   │   ├── test_fetch_order_book.c   # fetch_order_book
│   │   ├── test_fetch_trades.c       # fetch_trades (public)
│   │   ├── test_fetch_ohlcv.c        # fetch_ohlcv
│   │   ├── test_fetch_funding_rates.c # fetch_funding_rates
│   │   └── test_fetch_open_interest.c # fetch_open_interest
│   │
│   ├── 03_account/
│   │   ├── test_fetch_balance.c      # fetch_balance
│   │   ├── test_fetch_positions.c    # fetch_positions
│   │   ├── test_fetch_position.c     # fetch_position (single)
│   │   ├── test_fetch_trading_fee.c  # fetch_trading_fee
│   │   └── test_fetch_ledger.c       # fetch_ledger
│   │
│   ├── 04_trading/
│   │   ├── test_create_order.c       # create_order (place_order)
│   │   ├── test_create_orders.c      # create_orders (batch)
│   │   ├── test_cancel_order.c       # cancel_order
│   │   ├── test_cancel_orders.c      # cancel_orders (batch)
│   │   ├── test_edit_order.c         # edit_order
│   │   ├── test_edit_orders.c        # edit_orders (batch)
│   │   ├── test_fetch_order.c        # fetch_order
│   │   ├── test_fetch_orders.c       # fetch_orders
│   │   ├── test_fetch_open_orders.c  # fetch_open_orders
│   │   ├── test_fetch_closed_orders.c # fetch_closed_orders
│   │   ├── test_fetch_my_trades.c    # fetch_my_trades
│   │   └── test_cancel_all_orders_after.c # Dead man's switch
│   │
│   ├── 05_margin/
│   │   ├── test_set_leverage.c       # set_leverage
│   │   ├── test_set_margin_mode.c    # set_margin_mode
│   │   ├── test_add_margin.c         # add_margin
│   │   └── test_reduce_margin.c      # reduce_margin
│   │
│   ├── 06_transfers/
│   │   ├── test_transfer_spot_swap.c # transfer (spot↔swap)
│   │   ├── test_transfer_subaccount.c # transfer (main↔sub)
│   │   ├── test_withdraw.c           # withdraw
│   │   ├── test_fetch_deposits.c     # fetch_deposits
│   │   └── test_fetch_withdrawals.c  # fetch_withdrawals
│   │
│   ├── 07_vault/
│   │   ├── test_create_vault.c       # create_vault
│   │   └── test_approve_builder_fee.c # approve_builder_fee
│   │
│   └── 08_advanced/
│       ├── test_fetch_funding_history.c # fetch_funding_history
│       ├── test_fetch_funding_rate_history.c # fetch_funding_rate_history
│       ├── test_reserve_request_weight.c # reserve_request_weight
│       └── test_cancel_orders_for_symbols.c # cancel_orders_for_symbols
│
├── helpers/                           # Test helpers
│   ├── test_common.h                 # Общие функции
│   ├── test_common.c                 # Load .env, assert helpers
│   └── test_credentials.c            # Credential management
│
└── runner/                            # Test runners
    ├── run_all.c                     # Run all tests
    ├── run_unit.c                    # Run unit tests only
    ├── run_integration.c             # Run integration tests only
    └── run_priority.c                # Run high priority tests
```

## 🎯 Категории Тестов

### **1. Unit Tests** (Без сети)
- Crypto operations (keccak, EIP-712)
- MessagePack serialization
- HTTP request building
- Client initialization
- Error code conversion

### **2. Integration Tests** (Testnet)
- **Priority 1 (HIGH)**: 20 критичных методов
- **Priority 2 (MEDIUM)**: 18 полезных методов
- **Priority 3 (LOW)**: 13 дополнительных методов

## 📝 Naming Convention

```c
// Unit test
test_<module>_<function>

// Integration test
test_<action>_<resource>

Примеры:
- test_crypto_keccak256          (unit)
- test_msgpack_order             (unit)
- test_fetch_balance             (integration)
- test_create_order              (integration)
```

## 🚀 Запуск Тестов

```bash
# Все тесты
make test

# Только unit тесты
make test-unit

# Только integration тесты
make test-integration

# High priority тесты
make test-priority

# Конкретная категория
make test-market-data
make test-trading
make test-account

# Один тест
make test-fetch-balance
```

## ✅ Test Coverage Tracking

### **Phase 1: Foundation (Unit)** - 4 теста ✅
- [x] test_crypto
- [x] test_msgpack
- [x] test_http
- [x] test_client

### **Phase 2: Connection** - 1 тест ✅
- [x] test_connection

### **Phase 3: Trading Core** - 2 теста ✅
- [x] test_create_order
- [x] test_cancel_order

### **Phase 4: High Priority** - 20 тестов ⏳
#### Account & Balance (2)
- [ ] test_fetch_balance
- [ ] test_fetch_positions

#### Market Data (4)
- [ ] test_fetch_markets
- [ ] test_fetch_ticker
- [ ] test_fetch_order_book
- [ ] test_fetch_ohlcv

#### Order Management (8)
- [ ] test_create_orders
- [ ] test_fetch_order
- [ ] test_fetch_orders
- [ ] test_fetch_open_orders
- [ ] test_fetch_my_trades
- [ ] test_cancel_orders
- [ ] test_edit_order
- [ ] test_edit_orders

#### Margin & Leverage (2)
- [ ] test_set_leverage
- [ ] test_set_margin_mode

#### Transfers (2)
- [ ] test_transfer_spot_swap
- [ ] test_withdraw

### **Phase 5: Medium Priority** - 18 тестов 📋
#### Market Data Extended (4)
- [ ] test_fetch_tickers
- [ ] test_fetch_currencies
- [ ] test_fetch_funding_rates
- [ ] test_fetch_trades

#### Account Extended (3)
- [ ] test_fetch_position
- [ ] test_fetch_trading_fee
- [ ] test_fetch_ledger

#### Order Management Extended (5)
- [ ] test_fetch_closed_orders
- [ ] test_cancel_orders_for_symbols
- [ ] test_cancel_all_orders_after

#### Margin Extended (2)
- [ ] test_add_margin
- [ ] test_reduce_margin

#### Transfers Extended (4)
- [ ] test_transfer_subaccount
- [ ] test_fetch_deposits
- [ ] test_fetch_withdrawals

### **Phase 6: Low Priority** - 6 тестов 📌
#### Advanced (6)
- [ ] test_fetch_funding_history
- [ ] test_fetch_funding_rate_history
- [ ] test_fetch_open_interest
- [ ] test_create_vault
- [ ] test_approve_builder_fee
- [ ] test_reserve_request_weight

## 📊 Coverage Statistics

```
Total Methods: 51
Unit Tests: 4 (foundation)
Integration Tests: 47 (API methods)

Current Coverage: 7/51 (13.7%)
Target Coverage: 51/51 (100%)
```

## 🔧 Test Helper Functions

**`tests/helpers/test_common.h`**:
```c
// Setup & Teardown
hl_client_t* test_create_client(bool testnet);
void test_destroy_client(hl_client_t* client);

// Assertions
void test_assert(bool condition, const char* message);
void test_assert_success(hl_error_t err, const char* context);
void test_assert_not_null(void* ptr, const char* context);

// Utilities
void test_load_env(void);
const char* test_get_env(const char* key, const char* default_val);
void test_print_result(const char* test_name, bool passed);

// Test execution
typedef bool (*test_func_t)(void);
int test_run(const char* name, test_func_t func);
int test_run_suite(const char* suite_name, test_func_t* tests, size_t count);
```

## 🎨 Test Template

**Пример integration теста**:
```c
#include "test_common.h"
#include "hyperliquid.h"

/**
 * @brief Test fetch_balance on testnet
 */
bool test_fetch_balance_perpetual(void) {
    // Setup
    hl_client_t* client = test_create_client(true);
    test_assert_not_null(client, "Client creation");
    
    // Execute
    hl_balance_t balance = {0};
    hl_error_t err = hl_fetch_balance(client, HL_ACCOUNT_PERPETUAL, &balance);
    
    // Verify
    test_assert_success(err, "fetch_balance");
    test_assert(balance.total_usdc > 0, "Balance has USDC");
    
    // Cleanup
    test_destroy_client(client);
    
    return true;
}

bool test_fetch_balance_spot(void) {
    // Similar structure
}

int main(void) {
    test_load_env();
    
    printf("╔══════════════════════════════════════╗\n");
    printf("║  TEST: fetch_balance                 ║\n");
    printf("╚══════════════════════════════════════╝\n\n");
    
    test_func_t tests[] = {
        test_fetch_balance_perpetual,
        test_fetch_balance_spot
    };
    
    return test_run_suite("fetch_balance", tests, 2);
}
```

## 🔄 CI/CD Integration

**GitHub Actions** (`.github/workflows/test.yml`):
```yaml
name: Test Suite

on: [push, pull_request]

jobs:
  unit-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: make deps
      - name: Run unit tests
        run: make test-unit
  
  integration-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Setup testnet credentials
        env:
          HYPERLIQUID_TESTNET_WALLET_ADDRESS: ${{ secrets.TESTNET_WALLET }}
          HYPERLIQUID_TESTNET_PRIVATE_KEY: ${{ secrets.TESTNET_KEY }}
        run: |
          echo "HYPERLIQUID_TESTNET_WALLET_ADDRESS=$HYPERLIQUID_TESTNET_WALLET_ADDRESS" > .env
          echo "HYPERLIQUID_TESTNET_PRIVATE_KEY=$HYPERLIQUID_TESTNET_PRIVATE_KEY" >> .env
      - name: Run integration tests
        run: make test-integration
```

## 📈 Progress Tracking

**Ежедневный отчёт**:
```bash
make test-report
# Output:
# ╔════════════════════════════════════════╗
# ║  Hyperliquid C SDK - Test Coverage    ║
# ╚════════════════════════════════════════╝
# 
# Phase 1 (Foundation):  4/4   (100%) ✅
# Phase 2 (Connection):  1/1   (100%) ✅
# Phase 3 (Trading):     2/2   (100%) ✅
# Phase 4 (High Pri):    5/20  (25%)  ⏳
# Phase 5 (Medium Pri):  0/18  (0%)   📋
# Phase 6 (Low Pri):     0/6   (0%)   📌
# 
# Total: 12/51 (23.5%)
# 
# Next to implement:
# - test_fetch_balance
# - test_fetch_markets
# - test_fetch_positions
```

## 🎯 Development Workflow

1. **Реализовать метод** в `src/`
2. **Добавить тест** в `tests/integration/XX_category/`
3. **Запустить тест**: `make test-<method>`
4. **Проверить coverage**: `make test-report`
5. **Commit**: `git commit -m "feat: add fetch_balance with tests"`

## 💡 Best Practices

1. **One test file per API method**
2. **Test both success and error cases**
3. **Use small amounts on testnet**
4. **Clean up after tests** (cancel orders, close positions)
5. **Test with realistic data**
6. **Document expected behavior**
7. **Include rate limit handling**
8. **Test thread safety** where applicable

---

**Last Updated**: 2025-10-06  
**Status**: 7/51 tests complete (13.7%)  
**Target**: 100% coverage by end of Phase 6

