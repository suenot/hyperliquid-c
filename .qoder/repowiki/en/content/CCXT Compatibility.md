# CCXT Compatibility

<cite>
**Referenced Files in This Document**   
- [CCXT_ARCHITECTURE_PLAN.md](file://CCXT_ARCHITECTURE_PLAN.md)
- [hl_exchange.h](file://include/hl_exchange.h)
- [hl_types.h](file://include/hl_types.h)
- [hl_client.h](file://include/hl_client.h)
- [hl_account.h](file://include/hl_account.h)
- [hl_markets.h](file://include/hl_markets.h)
- [hl_ticker.h](file://include/hl_ticker.h)
- [hl_orderbook.h](file://include/hl_orderbook.h)
- [hl_ohlcv.h](file://include/hl_ohlcv.h)
- [hl_fetch_markets](file://src/markets.c#L389-L411)
- [hl_fetch_ticker](file://src/ticker.c#L74-L74)
- [hl_fetch_order_book](file://src/orderbook.c#L102-L215)
- [hl_fetch_ohlcv](file://src/ohlcv.c#L136-L283)
- [hl_fetch_balance](file://src/account.c#L249-L261)
- [hl_fetch_positions](file://src/account.c#L450-L540)
- [hl_create_order](file://src/trading_api.c#L225-L338)
- [hl_cancel_order](file://src/trading_api.c#L225-L338)
</cite>

## Table of Contents
1. [Design Goals and Feature Parity](#design-goals-and-feature-parity)
2. [CCXT Method Mapping](#ccxt-method-mapping)
3. [Migration Guidance from JavaScript/Python CCXT](#migration-guidance-from-javascriptpython-ccxt)
4. [Synchronous Design vs Asynchronous Patterns](#synchronous-design-vs-asynchronous-patterns)
5. [Common CCXT Workflows in C API](#common-ccxt-workflows-in-c-api)
6. [Versioning Compatibility](#versioning-compatibility)

## Design Goals and Feature Parity

The Hyperliquid C SDK has been designed with the primary goal of achieving full feature parity with the CCXT standard, implementing all 51 core methods required for complete compatibility. This ensures seamless integration for developers familiar with the CCXT ecosystem while providing the performance benefits of C. The library follows CCXT's unified API design principles, standardizing method names, parameters, and data structures across all endpoints.

The implementation achieves 100% coverage of the Base API methods as defined in the CCXT specification, including trading operations, market data retrieval, account information, and funding operations. Each method has been carefully implemented to match the expected behavior and return values specified by CCXT, ensuring that applications can be ported with minimal changes. The architecture is built around CCXT-compatible data structures such as `hl_order_t`, `hl_market_t`, and `hl_ticker_t`, which mirror the standard CCXT format while being optimized for C memory management.

**Section sources**
- [CCXT_ARCHITECTURE_PLAN.md](file://CCXT_ARCHITECTURE_PLAN.md)
- [hl_exchange.h](file://include/hl_exchange.h)
- [hl_types.h](file://include/hl_types.h)

## CCXT Method Mapping

The Hyperliquid C SDK implements a direct mapping between CCXT standard methods and Hyperliquid-specific endpoints, following the CCXT naming convention with the `hl_` prefix. This mapping ensures that developers can easily transition from other CCXT implementations to the Hyperliquid C SDK.

### Core Trading Methods
- `fetchMarkets()` → `hl_fetch_markets()`: Retrieves all available markets from Hyperliquid's `/info` endpoint with type `metaAndAssetCtxs` for swaps and `spotMetaAndAssetCtxs` for spot markets
- `fetchTicker()` → `hl_fetch_ticker()`: Gets ticker data from the `/info` endpoint with type `l2Book` for price levels
- `fetchOrderBook()` → `hl_fetch_order_book()`: Retrieves order book data from the `/info` endpoint with type `l2Book`
- `fetchOHLCV()` → `hl_fetch_ohlcv()`: Gets candle data from the `/info` endpoint with type `candleSnapshot`
- `createOrder()` → `hl_create_order()`: Places orders through the `/exchange` endpoint with EIP-712 signing
- `cancelOrder()` → `hl_cancel_order()`: Cancels orders through the `/exchange` endpoint with EIP-712 signed cancellation

### Account Methods
- `fetchBalance()` → `hl_fetch_balance()`: Retrieves account balance from the `/info` endpoint with type `clearinghouseState` for perpetual accounts and `spotClearinghouseState` for spot accounts
- `fetchPositions()` → `hl_fetch_positions()`: Gets open positions from the `assetPositions` array in the clearinghouse state response
- `fetchOpenOrders()` → `hl_fetch_open_orders()`: Retrieves open orders through order status tracking

### Market Data Methods
The SDK supports all standard timeframes from 1m to 1M and properly handles the differences between swap and spot markets. For swap markets, requests use the coin name (baseName), while spot markets use the asset ID. This mapping is handled internally by the SDK, abstracting the Hyperliquid-specific implementation details from the user.

**Section sources**
- [CCXT_ARCHITECTURE_PLAN.md](file://CCXT_ARCHITECTURE_PLAN.md)
- [hl_fetch_markets](file://src/markets.c#L389-L411)
- [hl_fetch_ticker](file://src/ticker.c#L74-L74)
- [hl_fetch_order_book](file://src/orderbook.c#L102-L215)
- [hl_fetch_ohlcv](file://src/ohlcv.c#L136-L283)

## Migration Guidance from JavaScript/Python CCXT

Migrating from JavaScript or Python CCXT implementations to the Hyperliquid C SDK requires understanding several key differences in the API design while leveraging the familiar CCXT method structure.

### Initialization Differences
In JavaScript/Python CCXT, initialization typically involves creating an exchange instance with configuration options:
```javascript
const exchange = new ccxt.hyperliquid({
    apiKey: 'YOUR_API_KEY',
    secret: 'YOUR_SECRET',
    options: { defaultType: 'swap' }
});
```

In the C SDK, initialization uses `hl_client_new()` with explicit parameters:
```c
hl_client_t* client = hl_client_new(wallet_address, private_key, testnet);
```

Note that the C SDK uses wallet address and private key instead of API key/secret, reflecting Hyperliquid's wallet-based authentication.

### Memory Management
Unlike garbage-collected languages, C requires explicit memory management. All data structures returned by the SDK that contain allocated memory (such as arrays of markets, orders, or positions) must be explicitly freed by the caller using the appropriate free functions:
- `hl_markets_free()` for market data
- `hl_free_orderbook()` for order books
- `hl_free_positions()` for positions

### Error Handling
The C SDK uses return codes (`hl_error_t`) rather than exceptions. All method calls should be checked for success:
```c
hl_error_t err = hl_fetch_balance(client, HL_ACCOUNT_PERPETUAL, &balance);
if (err != HL_SUCCESS) {
    // Handle error
}
```

### Data Structure Access
While the data structures follow CCXT conventions, accessing fields requires C syntax:
```c
// Instead of balance['USDC']['free'] in Python
double usdc_free = get_spot_balance(&balance, "USDC")->available;
```

**Section sources**
- [hl_client.h](file://include/hl_client.h)
- [hl_account.h](file://include/hl_account.h)
- [hl_markets.h](file://include/hl_markets.h)

## Synchronous Design vs Asynchronous Patterns

The Hyperliquid C SDK employs a synchronous design pattern, which differs significantly from the asynchronous nature of JavaScript and the often-asynchronous usage patterns in Python CCXT implementations.

### Synchronous Execution Model
All API methods in the C SDK are blocking calls that return only when the operation is complete or has failed. This design choice prioritizes simplicity and predictability in high-frequency trading scenarios where timing and determinism are critical. For example:
```c
hl_order_result_t result = {0};
hl_error_t err = hl_create_order(client, &request, &result);
// Execution pauses until order is placed or fails
```

### Threading and Concurrency
For applications requiring concurrent operations, developers should use threading:
- Create a thread pool for parallel API calls
- Use mutex protection around shared client state
- Implement non-blocking patterns through multithreading rather than async/await

The SDK is thread-safe for concurrent read operations, but write operations (trading) should be serialized to prevent nonce conflicts.

### Comparison with Asynchronous Patterns
In JavaScript, the same operation would be:
```javascript
const result = await exchange.createOrder('BTC/USDC:USDC', 'limit', 'buy', 0.001, 50000);
```

In Python:
```python
result = await exchange.create_order('BTC/USDC:USDC', 'limit', 'buy', 0.001, 50000)
```

The C equivalent requires explicit error checking and does not support await patterns, but provides more direct control over execution flow and resource management.

**Section sources**
- [hl_client.h](file://include/hl_client.h)
- [hl_exchange.h](file://include/hl_exchange.h)

## Common CCXT Workflows in C API

This section demonstrates how common CCXT workflows are implemented in the Hyperliquid C SDK API.

### Market Data Retrieval
```c
// Fetch all markets
hl_markets_t markets = {0};
hl_error_t err = hl_fetch_markets(client, &markets);
if (err == HL_SUCCESS) {
    printf("Found %zu markets\n", markets.count);
    // Process markets...
    hl_markets_free(&markets);
}

// Fetch order book with depth of 10 levels
hl_orderbook_t book = {0};
err = hl_fetch_order_book(client, "BTC/USDC:USDC", 10, &book);
if (err == HL_SUCCESS) {
    printf("Best bid: %.2f, Best ask: %.2f\n", 
           hl_orderbook_get_best_bid(&book), 
           hl_orderbook_get_best_ask(&book));
    hl_free_orderbook(&book);
}
```

### Trading Workflow
```c
// Create a limit order
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
    free(result.order_id); // Caller must free
} else {
    printf("Order failed: %d\n", err);
}
```

### Account Information
```c
// Fetch perpetual account balance
hl_balance_t balance = {0};
hl_error_t err = hl_fetch_balance(client, HL_ACCOUNT_PERPETUAL, &balance);
if (err == HL_SUCCESS) {
    printf("Account value: %.2f USDC\n", balance.account_value);
    printf("Withdrawable: %.2f USDC\n", balance.withdrawable);
}

// Fetch positions
hl_position_t* positions = NULL;
size_t count = 0;
err = hl_fetch_positions(client, &positions, &count);
if (err == HL_SUCCESS && count > 0) {
    for (size_t i = 0; i < count; i++) {
        printf("Position: %s %.6f @ %.2f\n", 
               positions[i].symbol,
               positions[i].size,
               positions[i].entry_price);
    }
    hl_free_positions(positions, count);
}
```

**Section sources**
- [hl_fetch_balance](file://src/account.c#L249-L261)
- [hl_fetch_positions](file://src/account.c#L450-L540)
- [hl_create_order](file://src/trading_api.c#L225-L338)
- [hl_cancel_order](file://src/trading_api.c#L225-L338)

## Versioning Compatibility

The Hyperliquid C SDK maintains versioning compatibility with the CCXT specification through several mechanisms:

### Capability Detection
The SDK implements the CCXT `has` capability system through the `hl_exchange_capabilities()` function, allowing developers to check for feature support:
```c
const hl_capabilities_t* caps = hl_exchange_capabilities(config);
if (caps->fetch_funding_rate) {
    // Funding rate endpoint is supported
}
```

### Semantic Versioning
The library follows semantic versioning (MAJOR.MINOR.PATCH) to indicate compatibility:
- MAJOR version changes indicate breaking changes to the CCXT interface
- MINOR version changes add new CCXT methods or features
- PATCH version changes fix bugs without altering the interface

### Backward Compatibility
The SDK maintains backward compatibility with previous CCXT specification versions by:
- Preserving deprecated methods with warning annotations
- Supporting legacy parameter formats
- Providing migration utilities for data structure changes

### Update Strategy
When new CCXT specification updates are released, the Hyperliquid C SDK follows a phased implementation approach:
1. Analyze changes in the CCXT specification
2. Implement new methods in a feature branch
3. Test against the CCXT test suite
4. Release as a MINOR version update with documentation

This ensures that developers can upgrade with confidence, knowing that existing code will continue to function while gaining access to new features.

**Section sources**
- [CCXT_ARCHITECTURE_PLAN.md](file://CCXT_ARCHITECTURE_PLAN.md)
- [hl_exchange.h](file://include/hl_exchange.h)
- [hl_types.h](file://include/hl_types.h)