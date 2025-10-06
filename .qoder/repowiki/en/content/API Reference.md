# API Reference

<cite>
**Referenced Files in This Document**   
- [hl_client.h](file://include/hl_client.h)
- [hl_account.h](file://include/hl_account.h)
- [hl_markets.h](file://include/hl_markets.h)
- [hl_ws_client.h](file://include/hl_ws_client.h)
- [hyperliquid.h](file://include/hyperliquid.h)
- [hl_error.h](file://include/hl_error.h)
- [hl_types.h](file://include/hl_types.h)
</cite>

## Table of Contents
1. [Client Management](#client-management)
2. [Trading Operations](#trading-operations)
3. [Account Information](#account-information)
4. [Market Data](#market-data)
5. [WebSocket Client](#websocket-client)
6. [Error Handling](#error-handling)
7. [Data Types](#data-types)

## Client Management

### hl_client_create
**Purpose**: Creates a new Hyperliquid client instance with wallet credentials and network configuration.

**Parameters**:
- `wallet_address` (const char*): Ethereum wallet address (with 0x prefix)
- `private_key` (const char*): Private key (64 hex characters, no 0x prefix)
- `testnet` (bool): true for testnet, false for mainnet

**Return Value**: Pointer to hl_client_t on success, NULL on error

**Error Conditions**:
- HL_ERROR_INVALID_PARAMS: Invalid wallet address format or private key length
- Memory allocation failure

**Thread Safety**: The function is thread-safe due to internal mutex initialization

**Usage Example**:
```c
hl_client_t *client = hl_client_create("0x1234567890abcdef", "aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899", true);
if (!client) {
    printf("Failed to create client\n");
}
```

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L154-L156)
- [hl_client.h](file://include/hl_client.h#L78-L100)

## Trading Operations

### hl_place_order
**Purpose**: Places a new order on the Hyperliquid exchange.

**Parameters**:
- `client` (hl_client_t*): Client instance
- `request` (const hl_order_request_t*): Order request parameters
- `result` (hl_order_result_t*): Output structure for order result

**Return Value**: HL_SUCCESS on success, error code otherwise

**Error Conditions**:
- HL_ERROR_INVALID_PARAMS: NULL client, request, or result
- HL_ERROR_INVALID_SYMBOL: Unknown trading symbol
- HL_ERROR_SIGNATURE: Failed to build order hash or sign order
- HL_ERROR_API: HTTP request failed or invalid response
- HL_ERROR_INSUFFICIENT_BALANCE: Insufficient funds for order

**Thread Safety**: Thread-safe due to mutex protection during order placement

**Usage Example**:
```c
hl_order_request_t request = {
    .symbol = "BTC",
    .side = HL_SIDE_BUY,
    .price = 95000.0,
    .quantity = 0.001,
    .order_type = HL_ORDER_TYPE_LIMIT,
    .time_in_force = HL_TIF_GTC,
    .reduce_only = false
};
hl_order_result_t result;
hl_error_t error = hl_place_order(client, &request, &result);
if (error == HL_SUCCESS) {
    printf("Order placed: %s\n", result.order_id);
    free(result.order_id);
}
```

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L210-L212)
- [trading_api.c](file://src/trading_api.c#L79-L220)

## Account Information

### hl_fetch_balance
**Purpose**: Fetches account balance information for either perpetual or spot accounts.

**Parameters**:
- `client` (hl_client_t*): Client instance
- `type` (hl_account_type_t): Account type (HL_ACCOUNT_PERPETUAL or HL_ACCOUNT_SPOT)
- `balance` (hl_balance_t*): Output balance structure

**Return Value**: HL_SUCCESS on success, error code otherwise

**Error Conditions**:
- HL_ERROR_INVALID_PARAMS: NULL client or balance pointer
- Network or API errors during balance fetch

**Thread Safety**: Thread-safe as it uses the client's internal HTTP client with proper synchronization

**Usage Example**:
```c
hl_balance_t balance;
hl_error_t error = hl_fetch_balance(client, HL_ACCOUNT_PERPETUAL, &balance);
if (error == HL_SUCCESS) {
    printf("Account value: %f USDC\n", balance.account_value);
}
```

**Section sources**
- [hl_account.h](file://include/hl_account.h#L135-L139)
- [account.c](file://src/account.c#L249-L261)

### hl_fetch_positions
**Purpose**: Fetches all open positions for the account.

**Parameters**:
- `client` (hl_client_t*): Client instance
- `positions` (hl_position_t**): Output array of positions (caller must free)
- `count` (size_t*): Output number of positions

**Return Value**: HL_SUCCESS on success, error code otherwise

**Error Conditions**:
- HL_ERROR_INVALID_PARAMS: NULL client, positions, or count
- Network or API errors during position fetch

**Thread Safety**: Thread-safe due to client's internal synchronization mechanisms

**Usage Example**:
```c
hl_position_t* positions;
size_t count;
hl_error_t error = hl_fetch_positions(client, &positions, &count);
if (error == HL_SUCCESS) {
    for (size_t i = 0; i < count; i++) {
        printf("Position: %s %s %f @ %f\n", 
               positions[i].symbol, 
               positions[i].side == HL_POSITION_LONG ? "LONG" : "SHORT",
               positions[i].size, 
               positions[i].entry_price);
    }
    hl_free_positions(positions, count);
}
```

**Section sources**
- [hl_account.h](file://include/hl_account.h#L155-L159)

### hl_fetch_position
**Purpose**: Fetches a single position for a specific symbol.

**Parameters**:
- `client` (hl_client_t*): Client instance
- `symbol` (const char*): Market symbol
- `position` (hl_position_t*): Output position structure

**Return Value**: HL_SUCCESS on success, HL_ERROR_NOT_FOUND if no position exists

**Error Conditions**:
- HL_ERROR_INVALID_PARAMS: NULL client or position pointer
- HL_ERROR_NOT_FOUND: No position exists for the symbol
- Network or API errors

**Thread Safety**: Thread-safe due to client's internal synchronization

**Usage Example**:
```c
hl_position_t position;
hl_error_t error = hl_fetch_position(client, "BTC/USDC:USDC", &position);
if (error == HL_SUCCESS) {
    printf("BTC Position: %f @ %f\n", position.size, position.entry_price);
} else if (error == HL_ERROR_NOT_FOUND) {
    printf("No BTC position found\n");
}
```

**Section sources**
- [hl_account.h](file://include/hl_account.h#L170-L174)

### hl_fetch_trading_fee
**Purpose**: Fetches trading fee information for a specific symbol.

**Parameters**:
- `client` (hl_client_t*): Client instance
- `symbol` (const char*): Market symbol
- `fee` (hl_trading_fee_t*): Output fee structure

**Return Value**: HL_SUCCESS on success, error code otherwise

**Error Conditions**:
- HL_ERROR_INVALID_PARAMS: NULL client or fee pointer
- Network or API errors during fee fetch

**Thread Safety**: Thread-safe due to client's internal synchronization

**Usage Example**:
```c
hl_trading_fee_t fee;
hl_error_t error = hl_fetch_trading_fee(client, "BTC/USDC:USDC", &fee);
if (error == HL_SUCCESS) {
    printf("Maker fee: %f, Taker fee: %f\n", fee.maker_fee, fee.taker_fee);
}
```

**Section sources**
- [hl_account.h](file://include/hl_account.h#L191-L195)

## Market Data

### hl_fetch_markets
**Purpose**: Fetches all available markets from the exchange, including both perpetual and spot markets.

**Parameters**:
- `client` (hl_client_t*): Client instance
- `markets` (hl_markets_t*): Output markets collection

**Return Value**: HL_SUCCESS on success, error code otherwise

**Error Conditions**:
- HL_ERROR_INVALID_PARAMS: NULL client or markets pointer
- Network or API errors during market fetch
- Memory allocation failure

**Thread Safety**: Thread-safe due to client's internal synchronization

**Usage Example**:
```c
hl_markets_t markets;
hl_error_t error = hl_fetch_markets(client, &markets);
if (error == HL_SUCCESS) {
    printf("Fetched %zu markets\n", markets.count);
    // Process markets...
    hl_markets_free(&markets);
}
```

**Section sources**
- [hl_markets.h](file://include/hl_markets.h#L86-L86)
- [markets.c](file://src/markets.c#L389-L411)

### hl_fetch_ticker
**Purpose**: Fetches ticker information for a single symbol.

**Parameters**:
- `client` (hl_client_t*): Client instance
- `symbol` (const char*): Market symbol
- `ticker` (hl_ticker_t*): Output ticker structure

**Return Value**: HL_SUCCESS on success, error code otherwise

**Error Conditions**:
- HL_ERROR_INVALID_PARAMS: NULL client or ticker pointer
- Network or API errors during ticker fetch

**Thread Safety**: Thread-safe due to client's internal synchronization

**Usage Example**:
```c
hl_ticker_t ticker;
hl_error_t error = hl_fetch_ticker(client, "BTC/USDC:USDC", &ticker);
if (error == HL_SUCCESS) {
    printf("BTC Price: %f\n", ticker.last);
}
```

**Section sources**
- [hl_ticker.h](file://include/hl_ticker.h#L76-L80)

### hl_fetch_order_book
**Purpose**: Fetches the order book for a specific symbol.

**Parameters**:
- `client` (hl_client_t*): Client instance
- `symbol` (const char*): Market symbol
- `depth` (uint32_t): Maximum number of levels to return (0 for all available)
- `book` (hl_orderbook_t*): Output order book structure

**Return Value**: HL_SUCCESS on success, error code otherwise

**Error Conditions**:
- HL_ERROR_INVALID_PARAMS: NULL client or book pointer
- Network or API errors during order book fetch

**Thread Safety**: Thread-safe due to client's internal synchronization

**Usage Example**:
```c
hl_orderbook_t book;
hl_error_t error = hl_fetch_order_book(client, "BTC/USDC:USDC", 20, &book);
if (error == HL_SUCCESS) {
    printf("Best bid: %f, Best ask: %f\n", 
           hl_orderbook_get_best_bid(&book), 
           hl_orderbook_get_best_ask(&book));
    hl_free_orderbook(&book);
}
```

**Section sources**
- [hl_orderbook.h](file://include/hl_orderbook.h#L36-L40)

### hl_fetch_ohlcv
**Purpose**: Fetches OHLCV (Open, High, Low, Close, Volume) candlestick data for technical analysis.

**Parameters**:
- `client` (hl_client_t*): Client instance
- `symbol` (const char*): Market symbol
- `timeframe` (const char*): Timeframe (e.g., "1m", "1h", "1d")
- `since` (uint64_t*): Start timestamp in milliseconds (NULL for earliest available)
- `limit` (uint32_t*): Maximum number of candles to return (NULL for no limit)
- `until` (uint64_t*): End timestamp in milliseconds (NULL for latest available)
- `ohlcvs` (hl_ohlcvs_t*): Output OHLCV data

**Return Value**: HL_SUCCESS on success, error code otherwise

**Error Conditions**:
- HL_ERROR_INVALID_PARAMS: NULL client or ohlcvs pointer
- Invalid timeframe
- Network or API errors during OHLCV fetch

**Thread Safety**: Thread-safe due to client's internal synchronization

**Usage Example**:
```c
hl_ohlcvs_t ohlcvs;
hl_error_t error = hl_fetch_ohlcv(client, "BTC/USDC:USDC", HL_TIMEFRAME_1H, NULL, NULL, NULL, &ohlcvs);
if (error == HL_SUCCESS) {
    printf("Fetched %zu candles\n", ohlcvs.count);
    const hl_ohlcv_t* latest = hl_ohlcvs_get_latest(&ohlcvs);
    if (latest) {
        printf("Latest close: %f\n", latest->close);
    }
    hl_ohlcvs_free(&ohlcvs);
}
```

**Section sources**
- [hl_ohlcv.h](file://include/hl_ohlcv.h#L76-L80)

## WebSocket Client

### hl_ws_client_connect
**Purpose**: Connects to the WebSocket server for real-time data streaming.

**Parameters**:
- `client` (hl_ws_client_t*): WebSocket client instance

**Return Value**: true on success, false on failure

**Error Conditions**:
- NULL client pointer
- Failed to establish WebSocket connection
- Failed to create background thread for WebSocket operations

**Thread Safety**: Thread-safe due to internal mutex protection

**Usage Example**:
```c
hl_ws_config_t config;
hl_ws_config_default(&config, true); // Use testnet
hl_ws_client_t* ws_client = hl_ws_client_create(&config);
if (ws_client) {
    bool connected = hl_ws_client_connect(ws_client);
    if (connected) {
        printf("WebSocket connected\n");
    } else {
        printf("WebSocket connection failed\n");
    }
    hl_ws_client_destroy(ws_client);
}
```

**Section sources**
- [hl_ws_client.h](file://include/hl_ws_client.h#L66-L66)
- [ws_client.c](file://src/ws_client.c#L127-L161)

## Error Handling

### hl_error_t
**Purpose**: Enumerated error codes used throughout the API.

**Values**:
- HL_SUCCESS: 0 - Operation completed successfully
- HL_ERROR_INVALID_PARAMS: -1 - Invalid parameters provided
- HL_ERROR_NETWORK: -2 - Network communication error
- HL_ERROR_API: -3 - API request failed
- HL_ERROR_AUTH: -4 - Authentication error
- HL_ERROR_INSUFFICIENT_BALANCE: -5 - Insufficient balance for operation
- HL_ERROR_INVALID_SYMBOL: -6 - Invalid trading symbol
- HL_ERROR_ORDER_REJECTED: -7 - Order rejected by exchange
- HL_ERROR_SIGNATURE: -8 - Signature generation or validation error
- HL_ERROR_MSGPACK: -9 - MessagePack serialization error
- HL_ERROR_JSON: -10 - JSON parsing error
- HL_ERROR_MEMORY: -11 - Memory allocation failure
- HL_ERROR_TIMEOUT: -12 - Operation timeout
- HL_ERROR_NOT_IMPLEMENTED: -13 - Feature not implemented
- HL_ERROR_NOT_FOUND: -14 - Resource not found
- HL_ERROR_PARSE: -15 - Data parsing error

**Usage**: All API functions return hl_error_t to indicate success or failure.

**Section sources**
- [hl_error.h](file://include/hl_error.h#L12-L30)

### hl_error_string
**Purpose**: Converts an error code to a human-readable string.

**Parameters**:
- `error` (hl_error_t): Error code

**Return Value**: Human-readable error string

**Usage Example**:
```c
hl_error_t error = hl_fetch_balance(client, HL_ACCOUNT_PERPETUAL, &balance);
if (error != HL_SUCCESS) {
    printf("Error: %s\n", hl_error_string(error));
}
```

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L495-L499)

## Data Types

### hl_client_t
**Purpose**: Opaque client handle for all API operations.

**Description**: The main client structure that maintains connection state, authentication credentials, and configuration for interacting with the Hyperliquid exchange.

**Related Functions**: hl_client_create, hl_client_destroy, hl_client_is_testnet, hl_client_get_wallet_address, hl_client_get_private_key

**Section sources**
- [hl_client.h](file://include/hl_client.h#L30-L76)

### hl_order_request_t
**Purpose**: Structure defining parameters for placing an order.

**Fields**:
- `symbol` (const char*): Trading symbol (e.g., "BTC", "ETH")
- `side` (hl_side_t): Buy or sell
- `price` (double): Limit price (0 for market orders)
- `quantity` (double): Order quantity
- `order_type` (hl_order_type_t): Limit or market
- `time_in_force` (hl_time_in_force_t): Time in force
- `reduce_only` (bool): Reduce-only order
- `slippage_bps` (uint32_t): Slippage in basis points (market orders)

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L117-L133)

### hl_order_result_t
**Purpose**: Structure containing the result of an order placement operation.

**Fields**:
- `order_id` (char*): Order ID (allocated string, caller must free)
- `status` (hl_order_status_t): Order status
- `filled_quantity` (double): Filled quantity
- `average_price` (double): Average fill price
- `error` (char[256]): Error message (if any)

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L135-L144)

### hl_balance_t
**Purpose**: Structure containing account balance information.

**Fields**:
- `type` (hl_account_type_t): Account type
- `account_value` (double): Total account value in USDC
- `total_margin_used` (double): Total margin used
- `total_ntl_pos` (double): Total notional position value
- `total_raw_usd` (double): Total raw USD
- `withdrawable` (double): Withdrawable amount
- `cross_account_value` (double): Cross margin account value
- `cross_margin_used` (double): Cross margin used
- `cross_maintenance_margin_used` (double): Cross maintenance margin used
- `spot_balances` (hl_spot_balance_t*): Array of spot balances
- `spot_balance_count` (size_t): Number of spot balances
- `timestamp` (uint64_t): Response timestamp

**Section sources**
- [hl_account.h](file://include/hl_account.h#L60-L88)

### hl_position_t
**Purpose**: Structure containing information about an open position.

**Fields**:
- `coin` (char[32]): Coin symbol
- `symbol` (char[64]): Market symbol
- `side` (hl_position_side_t): Position side
- `size` (double): Position size
- `entry_price` (double): Average entry price
- `mark_price` (double): Current mark price
- `liquidation_price` (double): Liquidation price
- `unrealized_pnl` (double): Unrealized P&L
- `margin_used` (double): Margin used for this position
- `position_value` (double): Notional position value
- `return_on_equity` (double): ROE
- `leverage` (int): Current leverage
- `max_leverage` (int): Max leverage allowed
- `is_isolated` (bool): Isolated margin mode
- `cum_funding_all_time` (double): Cumulative funding all time
- `cum_funding_since_open` (double): Cumulative funding since open
- `cum_funding_since_change` (double): Cumulative funding since change

**Section sources**
- [hl_account.h](file://include/hl_account.h#L107-L133)

### hl_market_t
**Purpose**: Structure containing market information.

**Fields**:
- `symbol` (char[64]): Unified symbol
- `base` (char[32]): Base currency
- `quote` (char[32]): Quote currency
- `settle` (char[32]): Settlement currency
- `base_id` (char[16]): Base asset ID
- `asset_id` (uint32_t): Asset ID as integer
- `type` (hl_market_type_t): Market type
- `active` (bool): Is market active
- `amount_precision` (int): Amount precision
- `price_precision` (int): Price precision
- `max_leverage` (int): Maximum leverage
- `min_cost` (double): Minimum order cost
- `mark_price` (double): Mark price
- `oracle_price` (double): Oracle price
- `funding_rate` (double): Current funding rate
- `day_volume` (double): 24h volume
- `open_interest` (double): Open interest

**Section sources**
- [hl_markets.h](file://include/hl_markets.h#L45-L85)