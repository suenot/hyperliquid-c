# Funding Rate History

<cite>
**Referenced Files in This Document**   
- [funding.c](file://src/funding.c#L231-L359)
- [hl_types.h](file://include/hl_types.h)
- [README.md](file://README.md)
</cite>

## Table of Contents
1. [Function Signature and Parameters](#function-signature-and-parameters)
2. [Purpose of Funding Rates in Perpetual Contracts](#purpose-of-funding-rates-in-perpetual-contracts)
3. [Implementation Details](#implementation-details)
4. [Input/Output Expectations](#inputoutput-expectations)
5. [Calculating Net Funding Payments](#calculating-net-funding-payments)
6. [Pagination and Rate Limiting](#pagination-and-rate-limiting)
7. [Use Cases for Trading Strategy Development](#use-cases-for-trading-strategy-development)

## Function Signature and Parameters

The `hl_fetch_funding_rate_history()` function retrieves historical funding rate data for perpetual contracts on Hyperliquid. The function signature is defined as:

```c
hl_error_t hl_fetch_funding_rate_history(hl_client_t* client,
                                        const char* symbol,
                                        const char* since,
                                        uint32_t limit,
                                        hl_funding_history_t* history)
```

The parameters include:
- `client`: Pointer to the initialized Hyperliquid client instance
- `symbol`: Trading symbol in format "COIN/USDC:USDC" (e.g., "BTC/USDC:USDC")
- `since`: Optional timestamp in milliseconds to filter results from a specific start time
- `limit`: Maximum number of records to retrieve; defaults to 500 if not specified
- `history`: Output parameter that will contain the retrieved funding history data

The function returns an `hl_error_t` enum value indicating success or specific error conditions.

**Section sources**
- [funding.c](file://src/funding.c#L231-L359)

## Purpose of Funding Rates in Perpetual Contracts

Funding rates are a critical mechanism in perpetual futures contracts that help maintain price alignment between the perpetual contract and the underlying spot market. These periodic payments are exchanged between long and short position holders based on the premium/discount of the contract price relative to the index price.

When the perpetual contract trades at a premium to the index price, funding rates are positive, meaning long position holders pay short position holders. Conversely, when the contract trades at a discount, funding rates are negative, and short position holders pay long position holders.

This mechanism incentivizes traders to take positions that bring the contract price back in line with the spot price, preventing significant and sustained deviations. For traders, funding rates represent an additional cost or benefit that must be factored into their trading strategies, particularly for positions held over extended periods.

**Section sources**
- [README.md](file://README.md#L239-L248)

## Implementation Details

The implementation of `hl_fetch_funding_rate_history()` involves several key steps for building time-filtered requests and parsing historical funding records:

1. **Parameter Validation**: The function first validates that the client, symbol, and history parameters are not null.

2. **Symbol Processing**: The function extracts the coin name from the symbol (e.g., "BTC/USDC:USDC" becomes "BTC") by parsing the string before the first '/' character.

3. **Request Construction**: The function builds a JSON request body with the type "fundingHistory" and the extracted coin name. If a `since` timestamp is provided, it's included in the request; otherwise, the function calculates a default start time based on the limit parameter (or 500 hours if limit is zero).

4. **HTTP Request**: The function makes a POST request to the "/info" endpoint with the constructed JSON body using the internal HTTP client.

5. **Response Parsing**: Upon receiving a successful response, the function parses the JSON array of funding history entries. For each entry, it extracts:
   - Timestamp from the "time" field
   - Funding rate from the "fundingRate" field
   - Symbol information reconstructed from the coin name

6. **Memory Management**: The function allocates memory for the funding history entries and properly handles cleanup in case of errors.

7. **Data Population**: The parsed data is populated into the `hl_funding_history_t` structure with proper type conversion from JSON strings or numbers to double values.

**Section sources**
- [funding.c](file://src/funding.c#L231-L359)

## Input/Output Expectations

Based on the README.md specification, the input and output expectations for `hl_fetch_funding_rate_history()` are as follows:

**Input Parameters:**
- `client`: Must be a valid, initialized client instance with proper authentication credentials
- `symbol`: Must be a valid trading symbol in the format "COIN/USDC:USDC" or just the coin name
- `since`: Optional timestamp in milliseconds since Unix epoch; if null or empty, defaults to a calculated time window
- `limit`: Maximum number of records to return; values above the exchange limit will be capped

**Output Structure (`hl_funding_history_t`):**
```c
typedef struct {
    hl_funding_history_entry_t* entries;
    size_t count;
} hl_funding_history_t;
```

Each `hl_funding_history_entry_t` contains:
- `symbol`: Trading symbol (e.g., "BTC/USDC:USDC")
- `timestamp`: Millisecond timestamp as string
- `datetime`: ISO 8601 datetime representation
- `funding_rate`: Funding rate as a double value
- `info`: Raw JSON response data as string

The function allocates memory for the entries array, which must be freed by the caller using `hl_free_funding_history()` when no longer needed.

**Section sources**
- [funding.c](file://src/funding.c#L213-L226)
- [README.md](file://README.md#L239-L248)

## Calculating Net Funding Payments

To calculate net funding payments over time, traders can process the funding rate history data as follows:

1. **Identify Position Type**: Determine whether the trader held long or short positions during each funding interval.

2. **Calculate Individual Payments**: For each funding rate entry, multiply the funding rate by the position size and the funding interval (typically 8 hours for Hyperliquid).

3. **Sum Payments**: Aggregate all individual payments over the desired time period, with appropriate sign based on position type:
   - Long positions: Payment = funding_rate × position_size
   - Short positions: Payment = -funding_rate × position_size

4. **Net Result**: The sum of all payments represents the net funding cost or benefit over the period.

For example, a trader holding 1 BTC in a long position when the funding rate is 0.001 (0.1%) would pay 0.001 BTC in funding. If the same trader held the position through three funding periods with rates of 0.001, 0.0015, and 0.0005, the total funding cost would be (0.001 + 0.0015 + 0.0005) × 1 = 0.003 BTC.

**Section sources**
- [funding.c](file://src/funding.c#L231-L359)

## Pagination and Rate Limiting

The function implements pagination strategies through the `since` parameter, allowing clients to retrieve historical funding data in chunks:

1. **Initial Request**: Make a request with a specific `since` timestamp to retrieve data from that point forward.

2. **Subsequent Requests**: Use the earliest timestamp from the previous response as the `since` parameter for the next request to retrieve older data.

3. **Forward Pagination**: To retrieve newer data, use the latest timestamp from the previous response plus one millisecond as the `since` parameter.

The function also addresses rate limiting considerations:
- The `limit` parameter helps control the response size and reduce API load
- When no `since` parameter is provided, the function defaults to a reasonable time window (500 hours) rather than attempting to retrieve all historical data
- The implementation uses the exchange's native pagination rather than client-side filtering to minimize network traffic

Best practices for handling rate limits include:
- Implementing exponential backoff for failed requests
- Caching results to avoid redundant API calls
- Using appropriate `limit` values based on actual data needs
- Distributing requests over time rather than making bulk requests

**Section sources**
- [funding.c](file://src/funding.c#L231-L359)

## Use Cases for Trading Strategy Development

Funding rate history analysis provides valuable insights for developing and refining trading strategies:

1. **Trend Analysis**: By examining funding rate trends over time, traders can identify periods of extreme sentiment. Sustained high positive funding rates may indicate excessive long leverage and potential for long squeezes, while prolonged negative rates may suggest oversold conditions.

2. **Mean Reversion Strategies**: Traders can develop strategies that take positions opposite to the funding rate direction, betting on reversion to neutral funding levels.

3. **Carry Trade Identification**: Coins with consistently negative funding rates present opportunities for profitable long positions, as the trader receives regular payments from short holders.

4. **Risk Management**: Historical funding rate volatility can inform position sizing and stop-loss placement, particularly for longer-term holdings.

5. **Market Regime Detection**: Changes in funding rate behavior can signal shifts in market regimes, such as transitions from range-bound to trending markets.

6. **Correlation Analysis**: Comparing funding rates across different assets can reveal relationships and relative value opportunities in the derivatives market.

7. **Backtesting**: Historical funding data allows for accurate backtesting of strategies that incorporate funding costs, providing more realistic performance estimates.

These analyses help traders make more informed decisions about position entry, exit, and management in perpetual contract markets.

**Section sources**
- [README.md](file://README.md#L239-L248)
- [funding.c](file://src/funding.c#L231-L359)