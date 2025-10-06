# Order Book

<cite>
**Referenced Files in This Document**   
- [hl_orderbook.h](file://include/hl_orderbook.h)
- [hl_types.h](file://include/hl_types.h)
- [orderbook.c](file://src/orderbook.c)
- [simple_orderbook.c](file://examples/simple_orderbook.c)
- [API_REFERENCE.md](file://docs/API_REFERENCE.md)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Order Book Data Retrieval with hl_fetch_order_book](#order-book-data-retrieval-with-hl_fetch_order_book)
3. [Order Book Structure and Components](#order-book-structure-and-components)
4. [Depth Parameter and Performance Implications](#depth-parameter-and-performance-implications)
5. [Data Analysis and Trading Applications](#data-analysis-and-trading-applications)
6. [Error Handling and Robustness](#error-handling-and-robustness)
7. [Use Cases in Trading Strategies](#use-cases-in-trading-strategies)
8. [Data Freshness and Latency Trade-offs](#data-freshness-and-latency-trade-offs)
9. [Conclusion](#conclusion)

## Introduction

The order book is a fundamental component in electronic trading systems, representing the full list of outstanding buy (bid) and sell (ask) orders for a given financial instrument. In the Hyperliquid C SDK, the `hl_fetch_orderbook()` function enables real-time access to Level 2 (L2) market depth data, which is essential for algorithmic trading, market analysis, and execution strategy development. This document details the implementation, usage, and strategic applications of order book data retrieval, focusing on the `hl_orderbook_t` and `hl_book_level_t` structures, depth control, liquidity analysis, and practical trading use cases.

**Section sources**
- [API_REFERENCE.md](file://docs/API_REFERENCE.md#L214-L247)

## Order Book Data Retrieval with hl_fetch_order_book

The `hl_fetch_order_book()` function is the primary interface for retrieving real-time order book data from the Hyperliquid exchange. It requires a client instance, a trading symbol (e.g., "BTC/USDC:USDC"), a depth parameter specifying the number of price levels to retrieve, and an output structure to store the result.

```c
hl_error_t hl_fetch_order_book(hl_client_t* client, const char* symbol, uint32_t depth, hl_orderbook_t* book);
```

This function performs an HTTP POST request to the exchange's `/info` endpoint with a JSON payload specifying the `"l2Book"` type and the target coin or asset ID. The response is parsed into the `hl_orderbook_t` structure, which contains bid and ask levels, timestamps, and symbol information. The function ensures memory safety by zero-initializing the output structure and provides automatic cleanup via `hl_free_orderbook()`.

**Section sources**
- [orderbook.c](file://src/orderbook.c#L102-L215)
- [hl_orderbook.h](file://include/hl_orderbook.h#L39-L39)

## Order Book Structure and Components

The order book data is represented by two key structures: `hl_orderbook_t` and `hl_book_level_t`.

### hl_orderbook_t Structure

This structure encapsulates the complete order book for a given symbol:

```c
typedef struct {
    char symbol[32];                 // Trading symbol
    hl_book_level_t* bids;           // Bid levels (sorted descending)
    hl_book_level_t* asks;           // Ask levels (sorted ascending)
    size_t bids_count;               // Number of bid levels
    size_t asks_count;               // Number of ask levels
    uint64_t timestamp_ms;           // Timestamp in milliseconds
} hl_orderbook_t;
```

The `bids` array is sorted in descending order by price (highest bid first), while the `asks` array is sorted in ascending order (lowest ask first). This ordering facilitates rapid access to the best bid and ask prices.

### hl_book_level_t Structure

Each price level in the order book is represented by:

```c
typedef struct {
    double price;                    // Price level
    double quantity;                 // Amount at level
} hl_book_level_t;
```

The `quantity` field represents the tradable volume available at that specific price point.

**Section sources**
- [API_REFERENCE.md](file://docs/API_REFERENCE.md#L214-L247)
- [hl_orderbook.h](file://include/hl_orderbook.h#L39-L39)
- [hl_types.h](file://include/hl_types.h#L19-L19)

## Depth Parameter and Performance Implications

The `depth` parameter in `hl_fetch_order_book()` controls the maximum number of bid and ask levels returned. A value of `0` retrieves all available levels, while a positive integer limits the response to that many levels per side.

### Impact on Response Size and Latency

- **Shallow Depth (e.g., 3–10 levels)**: Reduces bandwidth usage and parsing time, ideal for high-frequency applications requiring only top-of-book data.
- **Deep Depth (e.g., 50+ levels)**: Provides comprehensive market depth for liquidity analysis but increases memory allocation, network transfer time, and processing overhead.
- **Unlimited Depth (0)**: Retrieves full order book, useful for market structure analysis but may impact performance on low-latency systems.

The SDK enforces depth limiting during JSON parsing via the `parse_orderbook_levels()` function, ensuring that only the requested number of levels are processed and stored.

**Section sources**
- [orderbook.c](file://src/orderbook.c#L55-L97)
- [hl_orderbook.h](file://include/hl_orderbook.h#L39-L39)

## Data Analysis and Trading Applications

The SDK provides utility functions to extract key market metrics from the order book:

### Top-of-Book Extraction

```c
double best_bid = hl_orderbook_get_best_bid(&book);
double best_ask = hl_orderbook_get_best_ask(&book);
```

These functions return the highest bid and lowest ask prices, respectively, enabling immediate access to the current market price.

### Bid-Ask Spread Calculation

```c
double spread = hl_orderbook_get_spread(&book);
```

The spread is calculated as `best_ask - best_bid`, a critical measure of market liquidity and transaction cost.

### Volume Aggregation

```c
double bid_volume = hl_orderbook_get_bid_volume(&book, 5);
double ask_volume = hl_orderbook_get_ask_volume(&book, 5);
```

These functions sum the quantities across the top N levels (or all levels if depth is 0), useful for estimating slippage and market impact.

**Section sources**
- [orderbook.c](file://src/orderbook.c#L240-L289)
- [hl_orderbook.h](file://include/hl_orderbook.h#L54-L79)
- [simple_orderbook.c](file://examples/simple_orderbook.c#L0-L195)

## Error Handling and Robustness

The `hl_fetch_order_book()` function implements comprehensive error handling:

- **Invalid Parameters**: Returns `HL_ERROR_INVALID_PARAMS` if client, symbol, or book pointer is NULL.
- **Network Errors**: Returns `HL_ERROR_NETWORK` for HTTP communication failures.
- **API Errors**: Returns `HL_ERROR_API` for non-200 HTTP status codes.
- **Parsing Errors**: Returns `HL_ERROR_PARSE` for malformed JSON or missing fields.
- **Memory Allocation**: Returns `HL_ERROR_MEMORY` if memory allocation fails.

Clients must check the return value and handle errors appropriately. The `hl_free_orderbook()` function safely deallocates memory, preventing leaks even if the fetch operation partially failed.

**Section sources**
- [orderbook.c](file://src/orderbook.c#L102-L215)
- [hl_orderbook.h](file://include/hl_orderbook.h#L39-L39)

## Use Cases in Trading Strategies

### Market-Making Logic

Order book data enables market makers to quote competitive bid/ask prices within the spread, adjusting quotes based on inventory and depth imbalances.

### Slippage Estimation

By aggregating volume across multiple levels, traders can estimate the price impact of large orders and adjust execution strategies accordingly.

### Large Order Detection

Sudden changes in depth at specific price levels may indicate iceberg orders or institutional activity, providing alpha signals for momentum or reversal strategies.

### Liquidity Analysis

Comparing bid and ask volumes across assets helps identify more liquid markets with tighter spreads and lower transaction costs.

**Section sources**
- [simple_orderbook.c](file://examples/simple_orderbook.c#L0-L195)

## Data Freshness and Latency Trade-offs

Order book data is timestamped with millisecond precision (`timestamp_ms`), allowing clients to assess data freshness. However, there is a fundamental trade-off between depth and latency:

- **High Depth, Low Frequency**: Suitable for swing trading or statistical arbitrage where full market structure is needed.
- **Low Depth, High Frequency**: Ideal for high-frequency trading where minimal latency is critical.

Clients should balance depth requirements with polling frequency to avoid rate limiting and ensure timely decision-making.

**Section sources**
- [orderbook.c](file://src/orderbook.c#L200-L215)

## Conclusion

The `hl_fetch_order_book()` function and associated data structures provide a robust foundation for accessing and analyzing real-time market depth data on Hyperliquid. By understanding the order book structure, depth implications, and analytical capabilities, developers can build sophisticated trading strategies that leverage liquidity, spread, and volume insights. Proper error handling and awareness of latency trade-offs ensure reliable and performant integration into algorithmic trading systems.