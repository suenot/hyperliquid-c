# Hyperliquid C SDK - Complete API Reference

## Table of Contents

1. [Core Types](#core-types)
2. [Client Management](#client-management)
3. [Trading API](#trading-api)
4. [Market Data API](#market-data-api)
5. [Account API](#account-api)
6. [Order Management](#order-management)
7. [WebSocket API](#websocket-api)
8. [Utility Functions](#utility-functions)
9. [Error Codes](#error-codes)

## Core Types

### Error Codes
```c
typedef enum {
    HL_SUCCESS = 0,                    // Operation successful
    HL_ERROR_NETWORK = -1,             // Network communication error
    HL_ERROR_JSON = -2,                // JSON parsing/serialization error
    HL_ERROR_AUTH = -3,                // Authentication error
    HL_ERROR_INVALID_PARAMS = -4,      // Invalid parameters provided
    HL_ERROR_API = -5,                 // API-specific error
    HL_ERROR_MEMORY = -6,              // Memory allocation error
    HL_ERROR_NOT_IMPLEMENTED = -7      // Feature not implemented
} hl_error_t;
```

### Client Options
```c
typedef struct hl_options {
    bool testnet;                      // Use testnet (default: true)
    int timeout;                       // Request timeout in ms (default: 10000)
    int rate_limit;                    // Rate limit between requests in ms (default: 50)
} hl_options_t;
```

## Client Management

### hl_client_create
```c
hl_client_t* hl_client_create(const hl_options_t* options);
```
Creates a new Hyperliquid client instance.

**Parameters:**
- `options`: Client configuration options (NULL for defaults)

**Returns:** Client instance or NULL on error

**Example:**
```c
hl_options_t options = {.testnet = true, .timeout = 5000};
hl_client_t* client = hl_client_create(&options);
```

### hl_client_destroy
```c
void hl_client_destroy(hl_client_t* client);
```
Destroys a client instance and frees all associated resources.

**Parameters:**
- `client`: Client instance to destroy

### hl_test_connection
```c
hl_error_t hl_test_connection(hl_client_t* client);
```
Tests connectivity to Hyperliquid API.

**Parameters:**
- `client`: Client instance

**Returns:** HL_SUCCESS if connection successful

## Trading API

### hl_create_order
```c
hl_error_t hl_create_order(hl_client_t* client,
                          const hl_order_request_t* request,
                          hl_order_result_t* result);
```
Creates a new trading order.

**Parameters:**
- `client`: Client instance
- `request`: Order request details
- `result`: Order result (output)

**Returns:** HL_SUCCESS on success

**Order Request Structure:**
```c
typedef struct {
    const char* symbol;      // Trading pair (e.g., "BTC/USDC:USDC")
    const char* type;        // "market" or "limit"
    const char* side;        // "buy" or "sell"
    const char* amount;      // Order amount as string
    const char* price;       // Limit price as string (NULL for market orders)
} hl_order_request_t;
```

**Order Result Structure:**
```c
typedef struct {
    char* order_id;          // Order ID (allocated, caller must free)
    char timestamp[32];      // Order timestamp
    char status[16];         // Order status
} hl_order_result_t;
```

**Example:**
```c
hl_order_request_t request = {
    .symbol = "BTC/USDC:USDC",
    .type = "limit",
    .side = "buy",
    .amount = "0.001",
    .price = "50000.0"
};

hl_order_result_t result = {0};
hl_error_t err = hl_create_order(client, &request, &result);
if (err == HL_SUCCESS) {
    printf("Order created: %s\n", result.order_id);
    free(result.order_id);
}
```

### hl_cancel_order
```c
hl_error_t hl_cancel_order(hl_client_t* client,
                          const char* symbol,
                          const char* order_id,
                          hl_cancel_result_t* result);
```
Cancels an existing order.

**Parameters:**
- `client`: Client instance
- `symbol`: Trading pair symbol
- `order_id`: Order ID to cancel
- `result`: Cancellation result (output)

**Returns:** HL_SUCCESS on success

## Market Data API

### hl_fetch_markets
```c
hl_error_t hl_fetch_markets(hl_client_t* client, hl_markets_t* markets);
```
Retrieves all available trading markets.

**Parameters:**
- `client`: Client instance
- `markets`: Markets data (output)

**Returns:** HL_SUCCESS on success

**Markets Structure:**
```c
typedef struct {
    hl_market_t* markets;    // Array of markets
    size_t count;            // Number of markets
} hl_markets_t;

typedef struct {
    char id[32];             // Market ID
    char symbol[32];         // Trading symbol
    char base[16];           // Base currency
    char quote[16];          // Quote currency
    char type[16];           // Market type ("spot" or "swap")
    double min_amount;       // Minimum order amount
    double max_amount;       // Maximum order amount
    double amount_precision; // Amount precision
    double price_precision;  // Price precision
    bool active;             // Market is active
} hl_market_t;
```

### hl_fetch_ticker
```c
hl_error_t hl_fetch_ticker(hl_client_t* client,
                          const char* symbol,
                          hl_ticker_t* ticker);
```
Retrieves current ticker information for a symbol.

**Parameters:**
- `client`: Client instance
- `symbol`: Trading symbol
- `ticker`: Ticker data (output)

**Returns:** HL_SUCCESS on success

**Ticker Structure:**
```c
typedef struct {
    char symbol[32];         // Trading symbol
    double last_price;       // Last trade price
    double bid;              // Best bid price
    double ask;              // Best ask price
    double volume;           // 24h volume
    double quote_volume;     // 24h quote volume
    char timestamp[32];      // Timestamp
    char datetime[32];       // ISO datetime
} hl_ticker_t;
```

### hl_fetch_order_book
```c
hl_error_t hl_fetch_order_book(hl_client_t* client,
                              const char* symbol,
                              uint32_t depth,
                              hl_orderbook_t* book);
```
Retrieves order book for a symbol.

**Parameters:**
- `client`: Client instance
- `symbol`: Trading symbol
- `depth`: Maximum depth to retrieve
- `book`: Order book data (output)

**Returns:** HL_SUCCESS on success

**Order Book Structure:**
```c
typedef struct {
    char symbol[32];         // Trading symbol
    hl_book_level_t* bids;   // Bid levels (sorted descending)
    hl_book_level_t* asks;   // Ask levels (sorted ascending)
    size_t bids_count;       // Number of bid levels
    size_t asks_count;       // Number of ask levels
    char timestamp[32];      // Timestamp
} hl_orderbook_t;

typedef struct {
    double price;            // Price level
    double amount;           // Amount at level
} hl_book_level_t;
```

### hl_fetch_ohlcv
```c
hl_error_t hl_fetch_ohlcv(hl_client_t* client,
                         const char* symbol,
                         const char* timeframe,
                         uint32_t limit,
                         const char* since,
                         hl_ohlcvs_t* ohlcv);
```
Retrieves OHLCV candlestick data.

**Parameters:**
- `client`: Client instance
- `symbol`: Trading symbol
- `timeframe`: Timeframe ("1m", "1h", "1d", etc.)
- `limit`: Maximum number of candles
- `since`: Start time (timestamp string)
- `ohlcv`: OHLCV data (output)

**Returns:** HL_SUCCESS on success

**Supported Timeframes:**
- `"1m"`: 1 minute
- `"5m"`: 5 minutes
- `"15m"`: 15 minutes
- `"1h"`: 1 hour
- `"4h"`: 4 hours
- `"1d"`: 1 day

## Account API

### hl_fetch_balance
```c
hl_error_t hl_fetch_balance(hl_client_t* client, hl_balances_t* balances);
```
Retrieves account balance information.

**Parameters:**
- `client`: Client instance
- `balances`: Balance data (output)

**Returns:** HL_SUCCESS on success

**Balance Structure:**
```c
typedef struct {
    hl_balance_t* balances;  // Array of balances
    size_t count;            // Number of balances
} hl_balances_t;

typedef struct {
    char asset[32];          // Asset symbol
    double free;             // Available balance
    double used;             // Balance locked in orders
    double total;            // Total balance
} hl_balance_t;
```

### hl_fetch_positions
```c
hl_error_t hl_fetch_positions(hl_client_t* client, hl_positions_t* positions);
```
Retrieves open positions.

**Parameters:**
- `client`: Client instance
- `positions`: Position data (output)

**Returns:** HL_SUCCESS on success

**Position Structure:**
```c
typedef struct {
    hl_position_t* positions; // Array of positions
    size_t count;             // Number of positions
} hl_positions_t;

typedef struct {
    char symbol[32];         // Position symbol
    char side[8];            // "long" or "short"
    double amount;           // Position size
    double entry_price;      // Entry price
    double mark_price;       // Mark price
    double pnl;              // Unrealized P&L
    double pnl_percent;      // P&L percentage
    double leverage;         // Leverage
    double liquidation_price;// Liquidation price
} hl_position_t;
```

## Order Management

### hl_fetch_open_orders
```c
hl_error_t hl_fetch_open_orders(hl_client_t* client,
                               const char* symbol,
                               const char* since,
                               uint32_t limit,
                               hl_orders_t* orders);
```
Retrieves open orders.

**Parameters:**
- `client`: Client instance
- `symbol`: Trading symbol (NULL for all)
- `since`: Start time filter
- `limit`: Maximum orders to retrieve
- `orders`: Order data (output)

**Returns:** HL_SUCCESS on success

### hl_fetch_closed_orders
```c
hl_error_t hl_fetch_closed_orders(hl_client_t* client,
                                 const char* symbol,
                                 const char* since,
                                 uint32_t limit,
                                 hl_orders_t* orders);
```
Retrieves closed orders (filled and canceled).

**Parameters:**
- `client`: Client instance
- `symbol`: Trading symbol (NULL for all)
- `since`: Start time filter
- `limit`: Maximum orders to retrieve
- `orders`: Order data (output)

**Returns:** HL_SUCCESS on success

### hl_fetch_my_trades
```c
hl_error_t hl_fetch_my_trades(hl_client_t* client,
                             const char* symbol,
                             const char* since,
                             uint32_t limit,
                             hl_trades_t* trades);
```
Retrieves user's trade history.

**Parameters:**
- `client`: Client instance
- `symbol`: Trading symbol (NULL for all)
- `since`: Start time filter
- `limit`: Maximum trades to retrieve
- `trades`: Trade data (output)

**Returns:** HL_SUCCESS on success

## WebSocket API

### hl_ws_init_client
```c
bool hl_ws_init_client(hl_client_t* client, bool testnet);
```
Initializes WebSocket functionality for a client.

**Parameters:**
- `client`: Client instance
- `testnet`: Use testnet URLs

**Returns:** true on success

### hl_watch_ticker
```c
const char* hl_watch_ticker(hl_client_t* client,
                           const char* symbol,
                           hl_ws_data_callback_t callback,
                           void* user_data);
```
Subscribes to real-time ticker updates.

**Parameters:**
- `client`: Client instance
- `symbol`: Trading symbol
- `callback`: Data callback function
- `user_data`: User data for callback

**Returns:** Subscription ID or NULL on error

**Callback Signature:**
```c
typedef void (*hl_ws_data_callback_t)(void* data, void* user_data);
```

### hl_watch_order_book
```c
const char* hl_watch_order_book(hl_client_t* client,
                               const char* symbol,
                               uint32_t depth,
                               hl_ws_data_callback_t callback,
                               void* user_data);
```
Subscribes to real-time order book updates.

**Parameters:**
- `client`: Client instance
- `symbol`: Trading symbol
- `depth`: Order book depth
- `callback`: Data callback function
- `user_data`: User data for callback

**Returns:** Subscription ID or NULL on error

### hl_watch_orders
```c
const char* hl_watch_orders(hl_client_t* client,
                           const char* symbol,
                           hl_ws_data_callback_t callback,
                           void* user_data);
```
Subscribes to user order updates (requires authentication).

**Parameters:**
- `client`: Client instance
- `symbol`: Trading symbol (NULL for all)
- `callback`: Data callback function
- `user_data`: User data for callback

**Returns:** Subscription ID or NULL on error

### hl_unwatch
```c
bool hl_unwatch(hl_client_t* client, const char* subscription_id);
```
Unsubscribes from a WebSocket feed.

**Parameters:**
- `client`: Client instance
- `subscription_id`: Subscription ID to cancel

**Returns:** true on success

## Utility Functions

### Memory Management
```c
void hl_free_balances(hl_balances_t* balances);
void hl_free_positions(hl_positions_t* positions);
void hl_free_orders(hl_orders_t* orders);
void hl_free_trades(hl_trades_t* trades);
void hl_free_markets(hl_markets_t* markets);
void hl_free_orderbook(hl_orderbook_t* book);
void hl_free_ohlcvs(hl_ohlcvs_t* ohlcv);
```
Free allocated data structures.

### Exchange Information
```c
const hl_exchange_config_t* hl_exchange_describe(void);
bool hl_exchange_has_capability(const char* feature);
```
CCXT-compatible exchange information.

### Error Handling
```c
const char* hl_error_string(hl_error_t error);
```
Convert error code to human-readable string.

## Error Codes

| Error Code | Description |
|------------|-------------|
| `HL_SUCCESS` | Operation completed successfully |
| `HL_ERROR_NETWORK` | Network communication failed |
| `HL_ERROR_JSON` | JSON parsing/serialization error |
| `HL_ERROR_AUTH` | Authentication failed |
| `HL_ERROR_INVALID_PARAMS` | Invalid parameters provided |
| `HL_ERROR_API` | API returned an error |
| `HL_ERROR_MEMORY` | Memory allocation failed |
| `HL_ERROR_NOT_IMPLEMENTED` | Feature not yet implemented |

## Examples

### Complete Trading Bot
```c
#include "hyperliquid.h"

int main() {
    // Create client
    hl_client_t* client = hl_client_create(NULL);

    // Test connection
    if (hl_test_connection(client) != HL_SUCCESS) {
        fprintf(stderr, "Connection failed\n");
        return 1;
    }

    // Get account balance
    hl_balances_t balances = {0};
    if (hl_fetch_balance(client, &balances) == HL_SUCCESS) {
        for (size_t i = 0; i < balances.count; i++) {
            printf("%s: %.8f\n", balances.balances[i].asset,
                   balances.balances[i].total);
        }
        hl_free_balances(&balances);
    }

    // Place a limit order
    hl_order_request_t order = {
        .symbol = "BTC/USDC:USDC",
        .type = "limit",
        .side = "buy",
        .amount = "0.001",
        .price = "45000.0"
    };

    hl_order_result_t result = {0};
    hl_error_t err = hl_create_order(client, &order, &result);
    if (err == HL_SUCCESS) {
        printf("Order placed: %s\n", result.order_id);
        free(result.order_id);
    }

    // Get open orders
    hl_orders_t orders = {0};
    if (hl_fetch_open_orders(client, NULL, NULL, 10, &orders) == HL_SUCCESS) {
        printf("Open orders: %zu\n", orders.count);
        hl_free_orders(&orders);
    }

    hl_client_destroy(client);
    return 0;
}
```

### WebSocket Streaming
```c
#include "hyperliquid.h"

void on_ticker(void* data, void* user_data) {
    // Handle ticker update
    printf("Ticker update!\n");
}

int main() {
    hl_client_t* client = hl_client_create(NULL);

    // Initialize WebSocket
    hl_ws_init_client(client, true);

    // Subscribe to ticker
    const char* sub_id = hl_watch_ticker(client, "BTC/USDC:USDC",
                                       on_ticker, NULL);

    // Event loop (simplified)
    while (1) {
        // Process events, handle signals, etc.
        sleep(1);
    }

    hl_unwatch(client, sub_id);
    hl_ws_cleanup_client(client);
    hl_client_destroy(client);

    return 0;
}
```

## Thread Safety

All SDK functions are thread-safe and can be called concurrently from multiple threads. The SDK uses internal mutexes to protect shared state.

## Performance Considerations

- **Connection pooling**: Reuse client instances
- **Rate limiting**: Respect API limits (50ms default)
- **Memory management**: Always free allocated structures
- **WebSocket**: Use for high-frequency data, REST for occasional requests

## Security Notes

- Private keys are never transmitted in plain text
- All API calls use HTTPS/WSS
- EIP-712 signing for authenticated requests
- No sensitive data in logs

---

For more examples, see the `examples/` directory in the SDK repository.
