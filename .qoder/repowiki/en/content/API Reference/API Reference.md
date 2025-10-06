# API Reference

<cite>
**Referenced Files in This Document**   
- [hyperliquid.h](file://include/hyperliquid.h)
- [hl_exchange.h](file://include/hl_exchange.h)
- [hl_account.h](file://include/hl_account.h)
- [hl_client.h](file://include/hl_client.h)
- [hl_markets.h](file://include/hl_markets.h)
- [hl_ticker.h](file://include/hl_ticker.h)
- [hl_orderbook.h](file://include/hl_orderbook.h)
- [hl_ohlcv.h](file://include/hl_ohlcv.h)
- [hl_types.h](file://include/hl_types.h)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Client Management](#client-management)
3. [Trading](#trading)
4. [Market Data](#market-data)
5. [Account Information](#account-information)
6. [Utilities](#utilities)
7. [Struct Definitions](#struct-definitions)
8. [Error Handling](#error-handling)
9. [Thread Safety](#thread-safety)
10. [Version Information](#version-information)

## Introduction
The Hyperliquid C library provides a production-ready API for trading on the Hyperliquid DEX. This API reference documents all public functions and structs, organized by functional area. The library supports both spot and perpetual trading with EIP-712 signatures, MessagePack serialization, and low-latency operations. All functions return error codes defined in `hl_error_t` and follow consistent parameter patterns.

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L1-L100)

## Client Management
Client management functions handle the creation, destruction, and configuration of the Hyperliquid client instance. The client maintains authentication credentials, HTTP connections, and thread safety through mutexes.

### hl_client_create
Creates a new Hyperliquid client instance with the specified wallet address and private key.

**Parameters**
- `wallet_address`: Ethereum wallet address (with 0x prefix)
- `private_key`: Private key (64 hex characters, no 0x prefix)
- `testnet`: true for testnet, false for mainnet

**Return Value**
- Returns a client handle on success, NULL on error

**Error Conditions**
- `HL_ERROR_INVALID_PARAMS`: Invalid wallet address or private key format
- `HL_ERROR_MEMORY`: Memory allocation failed

**Usage Example**
```c
hl_client_t *client = hl_client_create("0xAddress", "privkey", true);
```

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L220-L222)
- [client.c](file://src/client.c#L34-L87)

### hl_client_destroy
Destroys a client instance and frees all associated resources.

**Parameters**
- `client`: Client handle to destroy

**Usage Example**
```c
hl_client_destroy(client);
```

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L229-L229)
- [client.c](file://src/client.c#L89-L107)

### hl_test_connection
Tests the connection to the Hyperliquid API.

**Parameters**
- `client`: Client handle

**Return Value**
- Returns true if connection is successful, false otherwise

**Usage Example**
```c
bool connected = hl_test_connection(client);
```

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L237-L237)
- [client.c](file://src/client.c#L109-L140)

### hl_set_timeout
Sets the HTTP timeout for API requests.

**Parameters**
- `client`: Client handle
- `timeout_ms`: Timeout in milliseconds

**Usage Example**
```c
hl_set_timeout(client, 30000); // 30 second timeout
```

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L245-L245)
- [client.c](file://src/client.c#L142-L146)

## Trading
Trading functions enable order placement, modification, cancellation, and batch operations for efficient trading strategies.

### hl_place_order
Places a new order on the Hyperliquid exchange.

**Parameters**
- `client`: Client handle
- `request`: Order request structure
- `result`: Order result structure (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Error Conditions**
- `HL_ERROR_INVALID_PARAMS`: Invalid order parameters
- `HL_ERROR_INVALID_SYMBOL`: Unknown trading symbol
- `HL_ERROR_SIGNATURE`: Signature generation failed
- `HL_ERROR_API`: Order rejected by exchange

**Usage Example**
```c
hl_order_request_t request = {
    .symbol = "BTC",
    .side = HL_SIDE_BUY,
    .price = 95000.0,
    .quantity = 0.001,
    .order_type = HL_ORDER_TYPE_LIMIT
};
hl_order_result_t result;
hl_place_order(client, &request, &result);
```

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L276-L278)
- [trading_api.c](file://src/trading_api.c#L79-L220)

### hl_cancel_order
Cancels an existing order.

**Parameters**
- `client`: Client handle
- `symbol`: Trading symbol
- `order_id`: Order ID to cancel
- `result`: Cancel result structure (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L290-L293)

### hl_cancel_all_orders
Cancels all orders for a specific symbol or all symbols.

**Parameters**
- `client`: Client handle
- `symbol`: Trading symbol (NULL for all symbols)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L301-L301)

### hl_modify_order
Modifies an existing order by canceling and replacing it.

**Parameters**
- `client`: Client handle
- `order_id`: Order ID to modify
- `new_order`: New order parameters
- `result`: Order result structure (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L312-L315)

### hl_create_orders
Creates multiple orders in a single batch operation.

**Parameters**
- `client`: Client handle
- `orders`: Array of order requests
- `orders_count`: Number of orders in array
- `results`: Array for order results (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Usage Example**
```c
hl_order_request_t orders[] = { /* multiple orders */ };
hl_order_result_t results[2];
hl_create_orders(client, orders, 2, results);
```

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L615-L618)
- [orders.c](file://src/orders.c#L653-L674)

### hl_cancel_orders
Cancels multiple orders in a single batch operation.

**Parameters**
- `client`: Client handle
- `orders`: Array of order IDs to cancel
- `orders_count`: Number of order IDs
- `results`: Array for cancellation results (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L629-L632)
- [orders.c](file://src/orders.c#L679-L700)

### hl_edit_order
Edits an existing order by canceling and creating a new one.

**Parameters**
- `client`: Client handle
- `order_id`: Order ID to edit
- `request`: New order parameters
- `result`: Order result structure (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L643-L646)
- [orders.c](file://src/orders.c#L705-L726)

## Market Data
Market data functions provide access to real-time and historical market information including tickers, order books, OHLCV data, and trade history.

### hl_get_ticker
Retrieves the current market ticker for a symbol.

**Parameters**
- `client`: Client handle
- `symbol`: Trading symbol
- `ticker`: Ticker information structure (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Usage Example**
```c
hl_ticker_t ticker;
hl_get_ticker(client, "BTC", &ticker);
```

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L368-L370)
- [ticker.c](file://src/ticker.c#L188-L213)

### hl_get_orderbook
Retrieves the current order book for a symbol.

**Parameters**
- `client`: Client handle
- `symbol`: Trading symbol
- `depth`: Number of levels to retrieve
- `book`: Order book structure (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Usage Example**
```c
hl_orderbook_t book;
hl_get_orderbook(client, "BTC", 20, &book);
// ... use book data
hl_free_orderbook(&book);
```

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L381-L384)
- [orderbook.c](file://src/orderbook.c#L315-L317)

### hl_fetch_ohlcv
Fetches OHLCV (Open, High, Low, Close, Volume) candle data.

**Parameters**
- `client`: Client handle
- `symbol`: Trading symbol
- `timeframe`: Timeframe (e.g., "1h", "1d")
- `since`: Start timestamp (optional)
- `limit`: Maximum number of candles (optional)
- `until`: End timestamp (optional)
- `ohlcvs`: OHLCV data structure (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hl_ohlcv.h](file://include/hl_ohlcv.h#L80-L82)
- [ohlcv.c](file://src/ohlcv.c#L136-L283)

### hl_fetch_tickers
Fetches tickers for multiple symbols.

**Parameters**
- `client`: Client handle
- `symbols`: Array of symbols (NULL for all)
- `symbols_count`: Number of symbols
- `tickers`: Tickers array (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L505-L508)
- [market_extended.c](file://src/market_extended.c#L14-L135)

### hl_fetch_trades
Fetches public trade history for a symbol.

**Parameters**
- `client`: Client handle
- `symbol`: Trading symbol
- `since`: Start timestamp (optional)
- `limit`: Maximum number of trades (optional)
- `trades`: Trades array (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L487-L491)
- [trades.c](file://src/trades.c#L104-L191)

## Account Information
Account information functions provide access to user-specific data including balances, positions, and order history.

### hl_fetch_balance
Retrieves account balance information.

**Parameters**
- `client`: Client handle
- `type`: Account type (spot or perpetual)
- `balance`: Balance structure (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Usage Example**
```c
hl_balance_t balance;
hl_fetch_balance(client, HL_ACCOUNT_PERPETUAL, &balance);
```

**Section sources**
- [hl_account.h](file://include/hl_account.h#L134-L138)
- [account.c](file://src/account.c#L249-L261)

### hl_fetch_positions
Retrieves all open positions.

**Parameters**
- `client`: Client handle
- `positions`: Positions array (output)
- `count`: Number of positions (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hl_account.h](file://include/hl_account.h#L162-L166)
- [account.c](file://src/account.c#L450-L540)

### hl_fetch_position
Retrieves a specific position by symbol.

**Parameters**
- `client`: Client handle
- `symbol`: Trading symbol
- `position`: Position structure (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hl_account.h](file://include/hl_account.h#L176-L180)
- [account.c](file://src/account.c#L545-L587)

### hl_fetch_open_orders
Fetches currently open orders.

**Parameters**
- `client`: Client handle
- `symbol`: Trading symbol (NULL for all)
- `since`: Start timestamp (optional)
- `limit`: Maximum number of orders (optional)
- `orders`: Orders array (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L421-L425)
- [orders.c](file://src/orders.c#L18-L104)

### hl_fetch_closed_orders
Fetches closed orders.

**Parameters**
- `client`: Client handle
- `symbol`: Trading symbol (NULL for all)
- `since`: Start timestamp (optional)
- `limit`: Maximum number of orders (optional)
- `orders`: Orders array (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L437-L441)
- [orders.c](file://src/orders.c#L109-L219)

### hl_fetch_order
Fetches a specific order by ID.

**Parameters**
- `client`: Client handle
- `order_id`: Order ID
- `symbol`: Trading symbol
- `order`: Order structure (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L452-L455)
- [orders.c](file://src/orders.c#L516-L571)

### hl_fetch_my_trades
Fetches user's trade history.

**Parameters**
- `client`: Client handle
- `symbol`: Trading symbol (NULL for all)
- `since`: Start timestamp (optional)
- `limit`: Maximum number of trades (optional)
- `trades`: Trades array (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L471-L475)
- [trades.c](file://src/trades.c#L17-L99)

## Utilities
Utility functions provide additional functionality including leverage management, margin operations, and currency information.

### hl_set_leverage
Sets leverage for a symbol or globally.

**Parameters**
- `client`: Client handle
- `leverage`: Leverage value (1-50)
- `symbol`: Trading symbol (NULL for all)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Usage Example**
```c
hl_set_leverage(client, 10, "BTC");
```

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L522-L524)
- [leverage.c](file://src/leverage.c#L14-L125)

### hl_add_margin
Adds margin to a position.

**Parameters**
- `client`: Client handle
- `symbol`: Trading symbol
- `amount`: Amount of margin to add

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L656-L658)
- [margin.c](file://src/margin.c#L14-L109)

### hl_reduce_margin
Reduces margin from a position.

**Parameters**
- `client`: Client handle
- `symbol`: Trading symbol
- `amount`: Amount of margin to reduce

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L668-L670)
- [margin.c](file://src/margin.c#L114-L123)

### hl_fetch_currencies
Fetches all available currencies.

**Parameters**
- `client`: Client handle
- `currencies`: Currencies array (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L537-L537)
- [currencies.c](file://src/currencies.c#L34-L137)

### hl_fetch_funding_rates
Fetches current funding rates for all symbols.

**Parameters**
- `client`: Client handle
- `rates`: Funding rates array (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L546-L546)
- [funding.c](file://src/funding.c#L34-L199)

### hl_fetch_deposits
Fetches deposit history (stub implementation).

**Parameters**
- `client`: Client handle
- `currency`: Currency code (NULL for all)
- `since`: Start time (optional)
- `limit`: Maximum entries to retrieve
- `deposits`: Deposit history array (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L584-L588)
- [transfers.c](file://src/transfers.c#L36-L55)

### hl_fetch_withdrawals
Fetches withdrawal history (stub implementation).

**Parameters**
- `client`: Client handle
- `currency`: Currency code (NULL for all)
- `since`: Start time (optional)
- `limit`: Maximum entries to retrieve
- `withdrawals`: Withdrawal history array (output)

**Return Value**
- `HL_SUCCESS` on success, error code otherwise

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L600-L604)
- [transfers.c](file://src/transfers.c#L63-L82)

## Struct Definitions
This section documents all public structs defined in the Hyperliquid C library.

### hl_order_request_t
Represents an order request with all necessary parameters.

**Fields**
- `symbol`: Trading symbol (e.g., "BTC", "ETH")
- `side`: Buy or sell (HL_SIDE_BUY or HL_SIDE_SELL)
- `price`: Limit price (0 for market orders)
- `quantity`: Order quantity
- `order_type`: Limit or market (HL_ORDER_TYPE_LIMIT or HL_ORDER_TYPE_MARKET)
- `time_in_force`: Time in force (HL_TIF_GTC, HL_TIF_IOC, or HL_TIF_ALO)
- `reduce_only`: Reduce-only order
- `slippage_bps`: Slippage in basis points (market orders)

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L127-L136)

### hl_order_result_t
Represents the result of an order placement operation.

**Fields**
- `order_id`: Order ID (allocated string, caller must free)
- `status`: Order status
- `filled_quantity`: Filled quantity
- `average_price`: Average fill price
- `error`: Error message (if any)

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L139-L145)

### hl_ticker_t
Represents market ticker information.

**Fields**
- `symbol`: Symbol
- `last_price`: Last trade price
- `bid`: Best bid
- `ask`: Best ask
- `close`: Close price
- `previous_close`: Previous day close price
- `high_24h`: 24h high
- `low_24h`: 24h low
- `volume_24h`: 24h volume
- `quote_volume`: 24h quote volume
- `change_24h`: 24h price change (%)
- `timestamp`: Timestamp (ms)
- `datetime`: ISO 8601 datetime string
- `mark_price`: Mark price
- `oracle_price`: Oracle price
- `funding_rate`: Current funding rate
- `open_interest`: Open interest

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L168-L188)

### hl_orderbook_t
Represents an order book snapshot.

**Fields**
- `symbol`: Symbol
- `bids`: Bid levels (sorted high to low)
- `bids_count`: Number of bid levels
- `asks`: Ask levels (sorted low to high)
- `asks_count`: Number of ask levels
- `timestamp_ms`: Snapshot timestamp

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L197-L204)

### hl_trade_t
Represents a trade execution.

**Fields**
- `trade_id`: Trade ID
- `order_id`: Order ID
- `symbol`: Symbol
- `side`: Buy or sell
- `price`: Trade price
- `quantity`: Trade quantity
- `fee`: Trading fee
- `timestamp_ms`: Timestamp (milliseconds)

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L156-L165)

### hl_balance_t
Represents account balance information.

**Fields**
- `type`: Account type
- `account_value`: Total account value in USDC
- `total_margin_used`: Total margin used
- `total_ntl_pos`: Total notional position value
- `total_raw_usd`: Total raw USD
- `withdrawable`: Withdrawable amount
- `cross_account_value`: Cross margin account value
- `cross_margin_used`: Cross margin used
- `cross_maintenance_margin_used`: Cross maintenance margin used
- `spot_balances`: Spot balances array
- `spot_balance_count`: Number of spot balances
- `timestamp`: Response timestamp

**Section sources**
- [hl_account.h](file://include/hl_account.h#L52-L72)

### hl_position_t
Represents a position in a trading pair.

**Fields**
- `coin`: Coin symbol
- `symbol`: Market symbol (e.g., "BTC/USDC:USDC")
- `side`: Position side
- `size`: Position size (absolute value)
- `entry_price`: Average entry price
- `mark_price`: Current mark price
- `liquidation_price`: Liquidation price
- `unrealized_pnl`: Unrealized P&L
- `margin_used`: Margin used for this position
- `position_value`: Notional position value
- `return_on_equity`: ROE
- `leverage`: Current leverage
- `max_leverage`: Max leverage allowed
- `is_isolated`: Isolated margin mode
- `cum_funding_all_time`: Cumulative funding all time
- `cum_funding_since_open`: Cumulative funding since open
- `cum_funding_since_change`: Cumulative funding since change

**Section sources**
- [hl_account.h](file://include/hl_account.h#L85-L108)

## Error Handling
The Hyperliquid C library uses a consistent error handling approach with the `hl_error_t` enum.

### hl_error_t
Enumeration of all possible error codes.

**Values**
- `HL_SUCCESS`: Operation successful
- `HL_ERROR_INVALID_PARAMS`: Invalid parameters
- `HL_ERROR_NETWORK`: Network error
- `HL_ERROR_API`: API error
- `HL_ERROR_AUTH`: Authentication error
- `HL_ERROR_INSUFFICIENT_BALANCE`: Insufficient balance
- `HL_ERROR_INVALID_SYMBOL`: Invalid trading symbol
- `HL_ERROR_ORDER_REJECTED`: Order rejected by exchange
- `HL_ERROR_SIGNATURE`: Signature generation failed
- `HL_ERROR_MSGPACK`: MessagePack error
- `HL_ERROR_JSON`: JSON parsing error
- `HL_ERROR_MEMORY`: Memory allocation failed
- `HL_ERROR_TIMEOUT`: Operation timed out
- `HL_ERROR_NOT_IMPLEMENTED`: Feature not implemented
- `HL_ERROR_NOT_FOUND`: Resource not found
- `HL_ERROR_PARSE`: Response parsing error

### hl_error_string
Converts an error code to a human-readable string.

**Parameters**
- `error`: Error code

**Return Value**
- Human-readable error string

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L558-L558)
- [client.c](file://src/client.c#L153-L170)

## Thread Safety
All public functions in the Hyperliquid C library are thread-safe. The client maintains a mutex that is locked during critical operations such as order placement and signature generation. Multiple threads can safely use the same client instance without external synchronization.

**Section sources**
- [client.c](file://src/client.c#L34-L87)

## Version Information
The Hyperliquid C library provides version information through constants and functions.

### Version Constants
- `HL_VERSION_MAJOR`: 1
- `HL_VERSION_MINOR`: 0
- `HL_VERSION_PATCH`: 0
- `HL_VERSION_STRING`: "1.0.0"

### hl_version
Returns the library version as a string.

**Return Value**
- Version string (e.g., "1.0.0")

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L565-L565)
- [client.c](file://src/client.c#L172-L174)