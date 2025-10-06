# Funding Rates

<cite>
**Referenced Files in This Document**   
- [funding.c](file://src/funding.c)
- [hyperliquid.h](file://include/hyperliquid.h)
- [hl_http.h](file://include/hl_http.h)
- [hl_internal.h](file://include/hl_internal.h)
- [types.c](file://src/types.c)
- [simple_client.c](file://src/http/simple_client.c)
- [simple_ticker.c](file://examples/simple_ticker.c)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Core Data Structures](#core-data-structures)
3. [Funding Rate Retrieval Implementation](#funding-rate-retrieval-implementation)
4. [HTTP Request Construction and Response Handling](#http-request-construction-and-response-handling)
5. [Symbol Mapping and Data Parsing](#symbol-mapping-and-data-parsing)
6. [Usage Examples](#usage-examples)
7. [Relationship to Perpetual Contract Pricing](#relationship-to-perpetual-contract-pricing)
8. [Error Handling and Common Issues](#error-handling-and-common-issues)
9. [Trading Strategy Applications](#trading-strategy-applications)
10. [Performance Optimization](#performance-optimization)

## Introduction
This document provides comprehensive documentation for the funding rate calculation and retrieval system in the Hyperliquid C SDK. It details the implementation of `hl_fetch_funding_rates`, including input validation, HTTP request construction, response parsing, and data structure design. The documentation covers how funding rates are mapped to symbols, their relationship to perpetual contract pricing, and practical applications in trading strategies. Special attention is given to error handling, performance considerations, and usage patterns for both single and multiple symbol queries.

## Core Data Structures

The funding rate system is built around two primary data structures that organize and store funding rate information.

```mermaid
classDiagram
class hl_funding_rate_t {
+char symbol[32]
+char timestamp[32]
+char datetime[32]
+double funding_rate
+double mark_price
+double index_price
+double open_interest
+double premium
+char info[1024]
}
class hl_funding_rates_t {
+hl_funding_rate_t* rates
+size_t count
}
hl_funding_rates_t --> hl_funding_rate_t : "contains"
```

**Diagram sources**
- [funding.c](file://src/funding.c#L14-L24)
- [funding.c](file://src/funding.c#L26-L29)

**Section sources**
- [funding.c](file://src/funding.c#L14-L29)

## Funding Rate Retrieval Implementation

The `hl_fetch_funding_rates` function serves as the primary interface for retrieving funding rate data from the Hyperliquid exchange. This function implements a robust process for fetching, parsing, and organizing funding rate information for all available perpetual contracts.

```mermaid
sequenceDiagram
participant Client as "Application"
participant Funding as "hl_fetch_funding_rates"
participant HTTP as "HTTP Client"
participant Exchange as "Hyperliquid API"
Client->>Funding : hl_fetch_funding_rates(client, rates)
activate Funding
Funding->>Funding : Validate parameters
alt Invalid parameters
Funding-->>Client : HL_ERROR_INVALID_PARAMS
deactivate Funding
else Valid parameters
Funding->>Funding : Initialize output structure
Funding->>HTTP : Prepare request body
HTTP->>Exchange : POST /info with metaAndAssetCtxs
activate Exchange
Exchange-->>HTTP : JSON response [meta, assetCtxs]
deactivate Exchange
HTTP->>Funding : Return response
Funding->>Funding : Parse JSON response
alt Parse failure
Funding-->>Client : HL_ERROR_JSON
deactivate Funding
else Valid response
Funding->>Funding : Extract universe and asset contexts
Funding->>Funding : Allocate funding rates array
loop For each asset
Funding->>Funding : Map symbol and extract funding data
end
Funding->>Funding : Set timestamps and store raw data
Funding-->>Client : HL_SUCCESS
deactivate Funding
end
end
```

**Diagram sources**
- [funding.c](file://src/funding.c#L34-L199)
- [hyperliquid.h](file://include/hyperliquid.h#L546)

**Section sources**
- [funding.c](file://src/funding.c#L34-L199)

## HTTP Request Construction and Response Handling

The funding rate retrieval process involves constructing a specific HTTP POST request to the Hyperliquid API endpoint and handling the response with proper error checking and resource management.

```mermaid
flowchart TD
Start([Function Entry]) --> ValidateParams["Validate client and rates parameters"]
ValidateParams --> InputValid{"Parameters Valid?"}
InputValid --> |No| ReturnInvalid["Return HL_ERROR_INVALID_PARAMS"]
InputValid --> |Yes| ClearOutput["Clear output structure"]
ClearOutput --> PrepareRequest["Prepare request body: {\"type\":\"metaAndAssetCtxs\"}"]
PrepareRequest --> MakeRequest["Make HTTP POST request to /info"]
MakeRequest --> CheckResponse{"Response Valid?"}
CheckResponse --> |No| HandleNetwork["Return HL_ERROR_NETWORK"]
CheckResponse --> |Yes| ParseJSON["Parse JSON response"]
ParseJSON --> JSONValid{"JSON Valid?"}
JSONValid --> |No| ReturnJSONError["Return HL_ERROR_JSON"]
JSONValid --> |Yes| ExtractData["Extract meta and assetCtxs arrays"]
ExtractData --> SizeMatch{"Array sizes match?"}
SizeMatch --> |No| ReturnJSONError
SizeMatch --> |Yes| AllocateMemory["Allocate memory for funding rates"]
AllocateMemory --> MemoryValid{"Memory allocated?"}
MemoryValid --> |No| ReturnMemoryError["Return HL_ERROR_MEMORY"]
MemoryValid --> |Yes| ProcessAssets["Process each asset context"]
ProcessAssets --> SetTimestamps["Set current timestamps"]
SetTimestamps --> Cleanup["Clean up JSON and response"]
Cleanup --> ReturnSuccess["Return HL_SUCCESS"]
ReturnInvalid --> End([Function Exit])
HandleNetwork --> End
ReturnJSONError --> End
ReturnMemoryError --> End
ReturnSuccess --> End
```

**Diagram sources**
- [funding.c](file://src/funding.c#L34-L199)
- [hl_http.h](file://include/hl_http.h#L33-L39)
- [simple_client.c](file://src/http/simple_client.c#L175-L193)

**Section sources**
- [funding.c](file://src/funding.c#L34-L199)

## Symbol Mapping and Data Parsing

The system implements a systematic approach to map funding rate data to trading symbols by extracting coin names from the universe metadata and formatting them into standardized symbol formats.

```mermaid
sequenceDiagram
participant Funding as "hl_fetch_funding_rates"
participant Universe as "Universe Data"
participant Context as "Asset Context"
participant Rate as "hl_funding_rate_t"
loop For each asset index
Funding->>Universe : Get universe item at index
Universe-->>Funding : Return universe object
Funding->>Context : Get asset context at index
Context-->>Funding : Return context object
alt Objects valid
Funding->>Universe : Extract coin name
Universe-->>Funding : Return coin name
Funding->>Rate : Format symbol as {coin}/USDC : USDC
Funding->>Context : Extract funding, markPx, oraclePx
Context-->>Funding : Return funding data
Funding->>Rate : Set funding_rate, mark_price, index_price
Funding->>Rate : Set open_interest, premium
Funding->>Rate : Set timestamps
Funding->>Context : Store raw context data
Funding->>Funding : Increment valid rates counter
end
end
Funding->>Funding : Set total count of valid rates
```

**Diagram sources**
- [funding.c](file://src/funding.c#L34-L199)
- [types.c](file://src/types.c#L84-L89)

**Section sources**
- [funding.c](file://src/funding.c#L34-L199)

## Usage Examples

The SDK provides practical examples for retrieving and utilizing funding rate data in trading applications. The following demonstrates how to fetch and display funding rate information.

```mermaid
flowchart LR
A[Create Client] --> B[Initialize hl_funding_rates_t]
B --> C[Call hl_fetch_funding_rates]
C --> D{Success?}
D --> |Yes| E[Iterate through rates]
E --> F[Display symbol, funding rate, mark price]
F --> G[Display index price, open interest]
G --> H[Display premium, timestamps]
H --> I[Free allocated memory]
I --> J[Destroy client]
D --> |No| K[Handle error with hl_error_string]
K --> J
```

**Section sources**
- [simple_ticker.c](file://examples/simple_ticker.c#L1-L104)

## Relationship to Perpetual Contract Pricing

Funding rates play a critical role in the pricing mechanism of perpetual contracts, serving as a convergence tool between mark price and index price.

```mermaid
graph LR
A[Index Price] --> |Oracle Data| B(Perpetual Contract)
C[Mark Price] --> |Market-driven| B
D[Funding Rate] --> |Periodic Payments| E[Longs and Shorts]
B --> F[Price Convergence]
F --> G[Reduced Basis]
G --> H[Accurate Pricing]
I[Open Interest] --> |Market Depth| B
J[Premium] --> |Basis Indicator| D
K[Trading Activity] --> |Volume Impact| C
```

**Section sources**
- [funding.c](file://src/funding.c#L34-L199)
- [simple_ticker.c](file://examples/simple_ticker.c#L1-L104)

## Error Handling and Common Issues

The funding rate retrieval system implements comprehensive error handling for various failure scenarios that may occur during the data retrieval process.

```mermaid
stateDiagram-v2
[*] --> Idle
Idle --> ParameterValidation : hl_fetch_funding_rates called
ParameterValidation --> InvalidParams : client or rates NULL
ParameterValidation --> NetworkRequest : parameters valid
NetworkRequest --> NetworkError : HTTP failure
NetworkRequest --> JSONParsing : response received
JSONParsing --> JSONError : invalid JSON format
JSONParsing --> DataExtraction : JSON valid
DataExtraction --> SizeMismatch : universe and assetCtxs size mismatch
DataExtraction --> MemoryAllocation : sizes match
MemoryAllocation --> MemoryError : allocation failure
MemoryAllocation --> AssetProcessing : memory allocated
AssetProcessing --> Success : all assets processed
Success --> Cleanup : free resources
Cleanup --> ReturnSuccess : HL_SUCCESS
InvalidParams --> Cleanup
NetworkError --> Cleanup
JSONError --> Cleanup
SizeMismatch --> Cleanup
MemoryError --> Cleanup
Cleanup --> ReturnError : appropriate error code
ReturnSuccess --> Idle
ReturnError --> Idle
```

**Section sources**
- [funding.c](file://src/funding.c#L34-L199)
- [hyperliquid.h](file://include/hyperliquid.h#L546)

## Trading Strategy Applications

Funding rates provide valuable information for various trading strategies, particularly in the realm of arbitrage and yield generation.

```mermaid
flowchart TD
A[Funding Rate Data] --> B[Arbitrage Strategies]
A --> C[Yield Calculations]
A --> D[Risk Management]
B --> B1[Spot-Futures Arbitrage]
B --> B2[Cross-Exchange Arbitrage]
B --> B3[Calendar Spreads]
C --> C1[Daily Funding Yield]
C --> C2[Annualized Return]
C --> C3[Cost of Carry Analysis]
D --> D1[Position Sizing]
D --> D2[Leverage Optimization]
D --> D3[Market Regime Detection]
B1 --> E[Long Spot, Short Perp when funding > 0]
B2 --> F[Exploit rate differences across exchanges]
B3 --> G[Roll yield from backwardation/contango]
C1 --> H[Yield = Funding Rate * Position Size]
C2 --> I[Annual Return = Funding Rate * 3 * 365]
C3 --> J[Compare with risk-free rate]
```

**Section sources**
- [funding.c](file://src/funding.c#L34-L199)
- [simple_ticker.c](file://examples/simple_ticker.c#L1-L104)

## Performance Optimization

For applications requiring frequent polling of funding rates, several performance optimization strategies can be implemented to minimize resource usage and maximize efficiency.

```mermaid
flowchart LR
A[Frequent Polling Requirements] --> B[Caching Strategy]
A --> C[Batch Processing]
A --> D[Connection Reuse]
A --> E[Memory Management]
B --> B1[Implement local cache]
B --> B2[Set appropriate TTL]
B --> B3[Cache invalidation]
C --> C1[Batch multiple requests]
C --> C2[Reduce round trips]
C --> C3[Optimize network usage]
D --> D1[Reuse HTTP client]
D --> D2[Persistent connections]
D --> D3[Connection pooling]
E --> E1[Pre-allocate structures]
E --> E2[Reuse memory buffers]
E --> E3[Proper cleanup]
B1 --> F[Reduce API calls]
C1 --> G[Improve throughput]
D1 --> H[Reduce latency]
E1 --> I[Minimize allocations]
```

**Section sources**
- [funding.c](file://src/funding.c#L34-L199)
- [hl_http.h](file://include/hl_http.h#L33-L39)
- [simple_client.c](file://src/http/simple_client.c#L175-L193)