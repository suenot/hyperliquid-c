# Hyperliquid C SDK - API Coverage Analysis

**Дата анализа**: 2025-10-06  
**Источник**: CCXT Hyperliquid Python Implementation (3744 строки)

## 📊 **ОБЩАЯ СТАТИСТИКА**

- **Всего методов в CCXT**: 51
- **Реализовано в C SDK**: 2 (4%)
- **Не реализовано**: 49 (96%)

---

## ✅ **РЕАЛИЗОВАННЫЕ МЕТОДЫ** (2/51)

### Trading Operations
1. ✅ **`create_order`** (place_order) - Размещение лимитного/рыночного ордера
2. ✅ **`cancel_order`** - Отмена ордера

---

## ❌ **НЕ РЕАЛИЗОВАННЫЕ МЕТОДЫ** (49/51)

### 1. **Market Data (Public API)** - 13 методов

| Метод | Описание | Приоритет | Сложность |
|-------|----------|-----------|-----------|
| `fetch_markets` | Получить список всех рынков (spot/swap) | 🔴 HIGH | MEDIUM |
| `fetch_currencies` | Получить список всех валют | 🟡 MEDIUM | LOW |
| `fetch_ticker` | Получить тикер (цену) для символа | 🔴 HIGH | LOW |
| `fetch_tickers` | Получить все тикеры | 🟡 MEDIUM | LOW |
| `fetch_order_book` | Получить стакан ордеров (L2) | 🔴 HIGH | MEDIUM |
| `fetch_trades` | Получить историю публичных сделок | 🟡 MEDIUM | MEDIUM |
| `fetch_ohlcv` | Получить свечи (OHLCV) | 🔴 HIGH | MEDIUM |
| `fetch_funding_rates` | Получить funding rates для perpetuals | 🟡 MEDIUM | LOW |
| `fetch_funding_rate_history` | История funding rates | 🟢 LOW | MEDIUM |
| `fetch_open_interest` | Получить open interest | 🟢 LOW | LOW |
| `fetch_open_interests` | Получить open interest для всех рынков | 🟢 LOW | LOW |
| `fetch_spot_markets` | Только спотовые рынки | 🟡 MEDIUM | LOW |
| `fetch_swap_markets` | Только swap/perpetual рынки | 🟡 MEDIUM | LOW |

---

### 2. **Account & Balance** - 5 методов

| Метод | Описание | Приоритет | Сложность |
|-------|----------|-----------|-----------|
| `fetch_balance` | Получить баланс (spot/perpetual) | 🔴 HIGH | LOW |
| `fetch_positions` | Получить открытые позиции | 🔴 HIGH | MEDIUM |
| `fetch_position` | Получить одну позицию | 🟡 MEDIUM | LOW |
| `fetch_trading_fee` | Получить комиссии торговли | 🟢 LOW | LOW |
| `fetch_ledger` | История операций (ledger) | 🟢 LOW | MEDIUM |

---

### 3. **Order Management** - 13 методов

| Метод | Описание | Приоритет | Сложность |
|-------|----------|-----------|-----------|
| `create_orders` | Создать несколько ордеров | 🔴 HIGH | MEDIUM |
| `cancel_orders` | Отменить несколько ордеров | 🔴 HIGH | LOW |
| `cancel_orders_for_symbols` | Отменить ордера для нескольких символов | 🟡 MEDIUM | MEDIUM |
| `cancel_all_orders_after` | Dead man's switch (отмена через таймаут) | 🟢 LOW | MEDIUM |
| `edit_order` | Изменить ордер | 🟡 MEDIUM | MEDIUM |
| `edit_orders` | Изменить несколько ордеров | 🟡 MEDIUM | MEDIUM |
| `fetch_order` | Получить информацию об ордере | 🔴 HIGH | LOW |
| `fetch_orders` | Получить все ордера (история) | 🔴 HIGH | MEDIUM |
| `fetch_open_orders` | Получить открытые ордера | 🔴 HIGH | MEDIUM |
| `fetch_closed_orders` | Получить закрытые ордера | 🟡 MEDIUM | MEDIUM |
| `fetch_canceled_orders` | Получить отменённые ордера | 🟢 LOW | LOW |
| `fetch_canceled_and_closed_orders` | Получить отменённые + закрытые | 🟢 LOW | LOW |
| `fetch_my_trades` | Получить мои сделки | 🔴 HIGH | MEDIUM |

---

### 4. **Margin & Leverage** - 5 методов

| Метод | Описание | Приоритет | Сложность |
|-------|----------|-----------|-----------|
| `set_leverage` | Установить кредитное плечо | 🔴 HIGH | LOW |
| `set_margin_mode` | Установить режим маржи (cross/isolated) | 🔴 HIGH | LOW |
| `add_margin` | Добавить маржу к позиции | 🟡 MEDIUM | LOW |
| `reduce_margin` | Уменьшить маржу позиции | 🟡 MEDIUM | LOW |
| `modify_margin_helper` | Внутренний хелпер для маржи | 🟢 LOW | LOW |

---

### 5. **Transfers & Withdrawals** - 5 методов

| Метод | Описание | Приоритет | Сложность |
|-------|----------|-----------|-----------|
| `transfer` | Внутренний трансфер (spot↔swap, main↔subaccount) | 🟡 MEDIUM | MEDIUM |
| `withdraw` | Вывод USDC | 🟡 MEDIUM | MEDIUM |
| `fetch_deposits` | Получить историю депозитов | 🟢 LOW | LOW |
| `fetch_withdrawals` | Получить историю выводов | 🟢 LOW | LOW |
| `parse_transaction` | Парсинг транзакции | 🟢 LOW | LOW |

---

### 6. **Vault Management** - 2 метода

| Метод | Описание | Приоритет | Сложность |
|-------|----------|-----------|-----------|
| `create_vault` | Создать vault | 🟢 LOW | LOW |
| `approve_builder_fee` | Одобрить builder fee | 🟢 LOW | LOW |

---

### 7. **Utility & Helpers** - 6 методов

| Метод | Описание | Приоритет | Сложность |
|-------|----------|-----------|-----------|
| `reserve_request_weight` | Зарезервировать лимиты запросов | 🟢 LOW | LOW |
| `fetch_funding_history` | История funding payments | 🟢 LOW | MEDIUM |
| `handle_builder_fee_approval` | Обработка builder fee | 🟢 LOW | LOW |
| `test_connection` | Тест подключения | ✅ (ЕСТЬ) | LOW |
| `calculate_price_precision` | Расчёт точности цены | 🟢 LOW | LOW |
| `amount_to_precision` / `price_to_precision` | Форматирование чисел | 🟡 MEDIUM | LOW |

---

## 🎯 **РЕКОМЕНДАЦИИ ПО ПРИОРИТЕТАМ**

### **Phase 4: High Priority (Core Trading Functionality)** 🔴

**Срок**: 2-3 недели

1. **Account & Balance**
   - ✅ `fetch_balance` - КРИТИЧНО для мониторинга
   - ✅ `fetch_positions` - КРИТИЧНО для риск-менеджмента

2. **Market Data**
   - ✅ `fetch_markets` - Нужен для получения asset IDs
   - ✅ `fetch_ticker` - Актуальная цена рынка
   - ✅ `fetch_order_book` - Для анализа ликвидности
   - ✅ `fetch_ohlcv` - Свечи для анализа

3. **Order Management**
   - ✅ `fetch_order` - Проверка статуса ордера
   - ✅ `fetch_orders` - История ордеров
   - ✅ `fetch_open_orders` - Активные ордера
   - ✅ `fetch_my_trades` - История исполненных сделок
   - ✅ `create_orders` - Батчинг ордеров

4. **Margin Management**
   - ✅ `set_leverage` - Управление плечом
   - ✅ `set_margin_mode` - Cross/Isolated

---

### **Phase 5: Medium Priority (Enhanced Features)** 🟡

**Срок**: 1-2 недели

1. **Advanced Order Management**
   - `cancel_orders` - Батчинг отмены
   - `edit_order` / `edit_orders` - Изменение ордеров
   - `cancel_orders_for_symbols` - Отмена по символам

2. **Market Data Extended**
   - `fetch_tickers` - Все тикеры
   - `fetch_currencies` - Метаданные валют
   - `fetch_funding_rates` - Funding для стратегий

3. **Transfers**
   - `transfer` - Spot↔Swap, Main↔Sub
   - `withdraw` - Вывод средств

4. **Margin**
   - `add_margin` / `reduce_margin` - Управление маржой

---

### **Phase 6: Low Priority (Nice to Have)** 🟢

**Срок**: 1 неделя

1. **History & Analytics**
   - `fetch_funding_history`
   - `fetch_funding_rate_history`
   - `fetch_open_interest`
   - `fetch_ledger`
   - `fetch_deposits` / `fetch_withdrawals`

2. **Advanced Features**
   - `cancel_all_orders_after` (Dead man's switch)
   - `create_vault`
   - `approve_builder_fee`
   - `reserve_request_weight`

3. **Utility**
   - `price_to_precision` / `amount_to_precision`
   - `calculate_price_precision`

---

## 📋 **СТРУКТУРА ЭНДПОИНТОВ**

### **Public API** (`/info`)

```c
POST /info
{
  "type": "meta",               // fetch_currencies
  "type": "metaAndAssetCtxs",   // fetch_swap_markets
  "type": "spotMetaAndAssetCtxs", // fetch_spot_markets
  "type": "clearinghouseState", // fetch_balance (perpetual)
  "type": "spotClearinghouseState", // fetch_balance (spot)
  "type": "l2Book",             // fetch_order_book
  "type": "allMids",            // fetch_tickers
  "type": "candleSnapshot",     // fetch_ohlcv
  "type": "fundingHistory",     // fetch_funding_rate_history
  "type": "openOrders",         // fetch_open_orders
  "type": "frontendOpenOrders", // fetch_open_orders (alternative)
  "type": "historicalOrders",   // fetch_orders
  "type": "orderStatus",        // fetch_order
  "type": "userFills",          // fetch_my_trades
  "type": "userFillsByTime",    // fetch_my_trades (с фильтром)
  "type": "userFees",           // fetch_trading_fee
  "type": "userFunding",        // fetch_funding_history
  "type": "userNonFundingLedgerUpdates" // fetch_ledger/deposits/withdrawals
}
```

### **Private API** (`/exchange`)

```c
POST /exchange
{
  "action": {
    "type": "order",            // create_order/create_orders
    "type": "cancel",           // cancel_order/cancel_orders
    "type": "cancelByCloid",    // cancel by client order ID
    "type": "batchModify",      // edit_order/edit_orders
    "type": "scheduleCancel",   // cancel_all_orders_after
    "type": "updateLeverage",   // set_leverage/set_margin_mode
    "type": "updateIsolatedMargin", // add_margin/reduce_margin
    "type": "usdClassTransfer", // transfer (spot↔swap)
    "type": "subAccountTransfer", // transfer (main↔sub)
    "type": "subAccountSpotTransfer", // transfer non-USDC
    "type": "withdraw3",        // withdraw
    "type": "vaultTransfer",    // vault operations
    "type": "createVault",      // create_vault
    "type": "approveBuilderFee", // approve_builder_fee
    "type": "reserveRequestWeight" // reserve_request_weight
  },
  "nonce": timestamp_ms,
  "signature": {
    "r": "0x...",
    "s": "0x...",
    "v": 27/28
  }
}
```

---

## 🔥 **КРИТИЧНЫЕ ЗАМЕЧАНИЯ**

### 1. **Asset ID Management**

В текущей реализации `get_asset_id()` хардкодит несколько ID:

```c
static uint32_t get_asset_id(const char *symbol) {
    if (strcmp(symbol, "BTC") == 0) return 3;
    if (strcmp(symbol, "ETH") == 0) return 2;
    // ...
    return 0; // ОПАСНО: возвращает 0 для неизвестных
}
```

**Проблемы**:
- ❌ Неполный список (в Hyperliquid 100+ активов)
- ❌ ID могут меняться между mainnet/testnet
- ❌ Hardcode не масштабируется

**Решение**:
```c
// Нужно реализовать:
hl_error_t hl_fetch_markets(hl_client_t* client, hl_market_t** markets, size_t* count);
hl_error_t hl_get_asset_id(hl_client_t* client, const char* symbol, uint32_t* asset_id);
```

---

### 2. **Timestamp Management**

```c
static uint64_t get_timestamp_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
}
```

**Проблемы**:
- ⚠️ Зависит от системного времени (NTP drift)
- ⚠️ Нет синхронизации с биржей

**Решение**:
```c
// Добавить:
hl_error_t hl_sync_time(hl_client_t* client); // Синхронизация с биржей
uint64_t hl_get_server_time(hl_client_t* client); // Cached server time
```

---

### 3. **Error Handling**

Текущий `lv3_error_t` ограничен:

```c
typedef enum {
    LV3_SUCCESS = 0,
    LV3_ERROR_INVALID_PARAMS,
    LV3_ERROR_NETWORK,
    // ... всего ~10 кодов
} lv3_error_t;
```

**Нужно добавить**:
- `HL_ERROR_INSUFFICIENT_MARGIN`
- `HL_ERROR_ORDER_SIZE_TOO_SMALL`
- `HL_ERROR_PRICE_OUT_OF_RANGE`
- `HL_ERROR_LEVERAGE_TOO_HIGH`
- `HL_ERROR_POSITION_NOT_FOUND`
- и т.д.

---

## 📦 **ПРИОРИТЕТНЫЙ ПЛАН РАЗРАБОТКИ**

### **Неделя 1: Account & Market Data**
```
1. fetch_balance       (2 дня)
2. fetch_positions     (1 день)
3. fetch_markets       (2 дня)
4. fetch_ticker        (1 день)
5. fetch_order_book    (1 день)
```

### **Неделя 2: Order Management**
```
1. fetch_order         (1 день)
2. fetch_orders        (1 день)
3. fetch_open_orders   (1 день)
4. fetch_my_trades     (2 дня)
5. create_orders       (2 дня)
```

### **Неделя 3: OHLCV & Advanced**
```
1. fetch_ohlcv         (2 дня)
2. set_leverage        (1 день)
3. set_margin_mode     (1 день)
4. cancel_orders       (1 день)
5. edit_order          (2 дня)
```

---

## 💡 **ЗАКЛЮЧЕНИЕ**

Текущая реализация C SDK покрывает только **4%** функциональности CCXT. Для production-ready системы необходимо реализовать минимум **20 критичных методов** из Phase 4-5.

**Рекомендация**: Начать с Phase 4 (High Priority), т.к. без `fetch_balance`, `fetch_positions`, и `fetch_markets` невозможно создать надёжного торгового бота.

---

**Автор анализа**: AI Expert  
**Проект**: Hyperliquid C SDK v1.0  
**Репозиторий**: `/Users/suenot/projects/listing-v3/listing-v3-c/hyperliqui/hyperliquid-c`

