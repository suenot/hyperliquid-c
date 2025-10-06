# Market Data

<cite>
**Referenced Files in This Document**   
- [simple_ticker.c](file://examples/simple_ticker.c)
- [simple_orderbook.c](file://examples/simple_orderbook.c)
- [simple_ohlcv.c](file://examples/simple_ohlcv.c)
- [hl_ticker.h](file://include/hl_ticker.h)
- [hl_orderbook.h](file://include/hl_orderbook.h)
- [hl_ohlcv.h](file://include/hl_ohlcv.h)
- [ticker.c](file://src/ticker.c)
- [orderbook.c](file://src/orderbook.c)
- [ohlcv.c](file://src/ohlcv.c)
- [hyperliquid.h](file://include/hyperliquid.h)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Ticker Data Retrieval](#ticker-data-retrieval)
3. [Order Book Data Retrieval](#order-book-data-retrieval)
4. [OHLCV Candle Data Retrieval](#ohlcv-candle-data-retrieval)
5. [HTTP Request Flow and JSON Parsing](#http-request-flow-and-json-parsing)
6. [Rate Limiting and Caching Strategies](#rate-limiting-and-caching-strategies)
7. [Performance Tips for High-Frequency Polling](#performance-tips-for-high-frequency-polling)
8. [Conclusion](#conclusion)

## Introduction
This document provides comprehensive guidance on retrieving market data from the Hyperliquid C SDK. It covers the three primary data types: tickers, order books, and OHLCV candles. Each section explains the parameters required for requests, the structure of returned data, and how to interpret key fields such as bid/ask prices, volume, and funding rates. Examples from `simple_ticker.c`, `simple_orderbook.c`, and `simple_ohlcv.c` are used to demonstrate usage patterns. The document also addresses HTTP GET request flow, JSON response parsing, rate limiting considerations, caching strategies, and performance optimization for high-frequency polling scenarios.

## Ticker Data Retrieval

The `hl_get_ticker` function retrieves real-time ticker information for a specified trading symbol. This includes price data, volume metrics, timestamps, and additional swap-specific information such as mark price, oracle price, funding rate, and open interest.

### Parameters
- `client`: Client instance created via `hl_client_create`
- `symbol`: Trading symbol (e.g., "BTC/USDC:USDC")
- `ticker`: Output structure to receive ticker data

### Return Value
Returns `HL_SUCCESS` on success or an error code otherwise.

### Ticker Structure
The `hl_ticker_t` structure contains the following fields:
- `symbol`: Market symbol
- `bid`: Best bid price
- `ask`: Best ask price
- `last_price`: Last trade price
- `close`: Close price
- `volume_24h`: 24-hour trading volume
- `quote_volume`: 24-hour quote volume
- `timestamp`: Timestamp in milliseconds
- `datetime`: ISO 8601 datetime string
- `mark_price`: Mark price (for perpetual swaps)
- `oracle_price`: Oracle price
- `funding_rate`: Current funding rate
- `open_interest`: Open interest

The example in `simple_ticker.c` demonstrates fetching ticker data for multiple symbols and displaying key metrics including bid/ask spread, volume, and swap-specific data.

**Section sources**
- [simple_ticker.c](file://examples/simple_ticker.c#L18-L103)
- [hl_ticker.h](file://include/hl_ticker.h#L26-L48)
- [ticker.c](file://src/ticker.c#L188-L213)

## Order Book Data Retrieval

The `hl_fetch_order_book` function retrieves Level 2 order book data for a specified symbol, providing bid and ask levels with corresponding prices and quantities.

### Parameters
- `client`: Client instance
- `symbol`: Trading symbol
- `depth`: Maximum number of levels to retrieve (0 for all available)
- `book`: Output order book structure

### Return Value
Returns `HL_SUCCESS` on success or an error code otherwise.

### Order Book Structure
The `hl_orderbook_t` structure contains:
- `symbol`: Trading symbol
- `bids`: Array of bid levels sorted in descending order by price
- `asks`: Array of ask levels sorted in ascending order by price
- `bids_count`: Number of bid levels
- `asks_count`: Number of ask levels
- `timestamp_ms`: Snapshot timestamp in milliseconds

Each `hl_book_level_t` contains:
- `price`: Price level
- `quantity`: Amount at that price level

The `simple_orderbook.c` example demonstrates fetching order books at different depths, analyzing bid/ask spreads, calculating volume at various depth levels, and comparing liquidity across assets.

Utility functions such as `hl_orderbook_get_best_bid`, `hl_orderbook_get_best_ask`, `hl_orderbook_get_spread`, `hl_orderbook_get_bid_volume`, and `hl_orderbook_get_ask_volume` provide convenient access to common order book metrics.

**Section sources**
- [simple_orderbook.c](file://examples/simple_orderbook.c#L1-L195)
- [hl_orderbook.h](file://include/hl_orderbook.h#L1-L95)
- [hyperliquid.h](file://include/hyperliquid.h#L197-L204)

## OHLCV Candle Data Retrieval

The `hl_fetch_ohlcv` function retrieves historical candlestick data for technical analysis and trading strategy development.

### Parameters
- `client`: Client instance
- `symbol`: Trading symbol
- `timeframe`: Time interval for candles (e.g., "1m", "1h", "1d")
- `since`: Start timestamp in milliseconds (NULL for earliest available)
- `limit`: Maximum number of candles to return (NULL for no limit)
- `until`: End timestamp in milliseconds (NULL for latest available)
- `ohlcvs`: Output OHLCV data collection

### Return Value
Returns `HL_SUCCESS` on success or an error code otherwise.

### OHLCV Structure
The `hl_ohlcv_t` structure contains:
- `timestamp`: Candle open timestamp (milliseconds)
- `open`: Open price
- `high`: High price
- `low`: Low price
- `close`: Close price
- `volume`: Trading volume

The `hl_ohlcvs_t` collection includes:
- `candles`: Array of OHLCV candles
- `count`: Number of candles
- `symbol`: Trading symbol
- `timeframe`: Timeframe string

Supported timeframes include: "1m", "3m", "5m", "15m", "30m", "1h", "2h", "4h", "8h", "12h", "1d", "3d", "1w", "1M".

The `simple_ohlcv.c` example demonstrates fetching OHLCV data with different timeframes and limits, performing technical analysis such as calculating simple moving averages (SMA), finding highest high and lowest low values, and filtering by time ranges.

**Section sources**
- [simple_ohlcv.c](file://examples/simple_ohlcv.c#L75-L191)
- [hl_ohlcv.h](file://include/hl_ohlcv.h#L25-L32)
- [ohlcv.c](file://src/ohlcv.c#L136-L283)

## HTTP Request Flow and JSON Parsing

Market data retrieval follows a consistent HTTP request flow:
1. Validate input parameters
2. Construct API request URL and body
3. Make HTTP POST request to the Hyperliquid API endpoint
4. Receive JSON response
5. Parse JSON response into appropriate data structures
6. Handle errors and edge cases

For ticker data, the request uses the "metaAndAssetCtxs" endpoint to fetch market metadata. For order books, it uses the "l2Book" endpoint. For OHLCV data, it uses the "candleSnapshot" endpoint.

JSON parsing is handled internally using the cJSON library. The response is validated, parsed into the appropriate structure, and memory is allocated for dynamic arrays (bids, asks, candles). Error handling ensures proper cleanup of allocated resources in case of parsing failures.

**Section sources**
- [ticker.c](file://src/ticker.c#L188-L213)
- [orderbook.c](file://src/orderbook.c)
- [ohlcv.c](file://src/ohlcv.c#L136-L283)

## Rate Limiting and Caching Strategies

The Hyperliquid API may impose rate limits on market data requests. To avoid exceeding these limits:

1. Implement request throttling with appropriate delays between consecutive requests
2. Use caching to store recently retrieved data and serve subsequent requests from cache
3. Batch multiple symbol requests when possible
4. Adjust polling frequency based on data freshness requirements

For ticker data, consider caching results for 1-2 seconds since prices typically don't change significantly in shorter intervals. For order books, cache duration should be shorter (100-500ms) due to higher volatility. OHLCV data can be cached for longer periods depending on the timeframe.

The SDK does not provide built-in caching; applications should implement their own caching layer based on specific requirements.

**Section sources**
- [simple_ticker.c](file://examples/simple_ticker.c#L18-L103)
- [simple_orderbook.c](file://examples/simple_orderbook.c#L1-L195)
- [simple_ohlcv.c](file://examples/simple_ohlcv.c#L75-L191)

## Performance Tips for High-Frequency Polling

When implementing high-frequency market data polling:

1. Reuse client instances rather than creating new ones for each request
2. Minimize memory allocations by reusing data structures when possible
3. Use appropriate depth values for order books (e.g., 20 levels instead of full book)
4. Limit OHLCV requests to necessary time ranges and candle counts
5. Process data asynchronously to avoid blocking the main thread
6. Monitor system resource usage (CPU, memory, network)
7. Implement exponential backoff for error recovery
8. Use connection keep-alive where supported

For ultra-low latency requirements, consider using WebSocket feeds instead of REST API polling, though this requires more complex connection management.

**Section sources**
- [simple_orderbook.c](file://examples/simple_orderbook.c#L1-L195)
- [simple_ohlcv.c](file://examples/simple_ohlcv.c#L75-L191)

## Conclusion
The Hyperliquid C SDK provides robust market data retrieval capabilities through three primary functions: `hl_get_ticker` for real-time price information, `hl_fetch_order_book` for Level 2 order book data, and `hl_fetch_ohlcv` for historical candlestick data. Each function follows a consistent pattern of parameter validation, HTTP request execution, JSON parsing, and error handling. By understanding the structure of returned data and implementing appropriate rate limiting and caching strategies, developers can build efficient trading applications. The provided examples demonstrate practical usage patterns and analysis techniques for each data type.