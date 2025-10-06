# Order Placement

<cite>
**Referenced Files in This Document**   
- [simple_trade.c](file://examples/simple_trade.c)
- [hyperliquid.h](file://include/hyperliquid.h)
- [trading_api.c](file://src/trading_api.c)
- [hl_msgpack.h](file://include/hl_msgpack.h)
- [eip712.c](file://src/crypto/eip712.c)
- [hl_internal.h](file://include/hl_internal.h)
</cite>

## Table of Contents
1. [Order Request Structure](#order-request-structure)  
2. [Constructing Market and Limit Orders](#constructing-market-and-limit-orders)  
3. [Internal Workflow of hl_place_order](#internal-workflow-of-hl_place_order)  
4. [Thread Safety and Nonce Generation](#thread-safety-and-nonce-generation)  
5. [Error Handling](#error-handling)  
6. [Order Result Interpretation](#order-result-interpretation)  
7. [Practical Examples from simple_trade.c](#practical-examples-from-simple_tradec)  

## Order Request Structure

The `hl_order_request_t` structure defines the parameters for placing an order on the Hyperliquid exchange. It includes the following fields:

- **symbol**: Trading symbol (e.g., "BTC", "ETH")  
- **side**: Order side, either `HL_SIDE_BUY` or `HL_SIDE_SELL`  
- **price**: Limit price (set to 0 for market orders)  
- **quantity**: Order quantity in base asset units  
- **order_type**: Either `HL_ORDER_TYPE_LIMIT` or `HL_ORDER_TYPE_MARKET`  
- **time_in_force**: Time in force policy: `HL_TIF_GTC` (Good Till Cancel), `HL_TIF_IOC` (Immediate Or Cancel), or `HL_TIF_ALO` (Add Liquidity Only)  
- **reduce_only**: Boolean flag indicating whether the order is reduce-only (used to close or reduce a position)  
- **slippage_bps**: Slippage tolerance in basis points (used for market orders only)  

This structure is defined in `hyperliquid.h` and is used as input to the `hl_place_order()` function.

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L108-L117)

## Constructing Market and Limit Orders

To place a **limit order**, set the `order_type` to `HL_ORDER_TYPE_LIMIT`, specify a non-zero `price`, and set an appropriate `time_in_force`. For example:

```c
hl_order_request_t limit_order = {
    .symbol = "BTC",
    .side = HL_SIDE_BUY,
    .price = 95000.0,
    .quantity = 0.001,
    .order_type = HL_ORDER_TYPE_LIMIT,
    .time_in_force = HL_TIF_GTC,
    .reduce_only = false,
    .slippage_bps = 0
};
```

For a **market order**, set `order_type` to `HL_ORDER_TYPE_MARKET`, set `price` to 0, and optionally specify `slippage_bps` to control acceptable price deviation. Example:

```c
hl_order_request_t market_order = {
    .symbol = "BTC",
    .side = HL_SIDE_SELL,
    .price = 0.0,
    .quantity = 0.001,
    .order_type = HL_ORDER_TYPE_MARKET,
    .time_in_force = HL_TIF_IOC,
    .reduce_only = true,
    .slippage_bps = 50  // 0.5% slippage tolerance
};
```

Both order types are validated and processed through the same `hl_place_order()` interface.

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L108-L117)

## Internal Workflow of hl_place_order

The `hl_place_order()` function orchestrates the full order placement process. The internal workflow is as follows:

1. **Parameter Validation**: Ensures that the `client`, `request`, and `result` pointers are valid. Returns `HL_ERROR_INVALID_PARAMS` if any are null.

2. **Asset ID Resolution**: Uses `get_asset_id()` to resolve the symbol (e.g., "BTC") into a numeric asset ID by querying the `/info` endpoint via `hl_fetch_markets()`. Special handling is applied for "SOL".

3. **Price and Size Formatting**: Converts `price` and `quantity` from `double` to string format using `snprintf()` to preserve precision and avoid floating-point representation issues.

4. **Order Hashing**: Constructs an `hl_order_t` structure and computes its hash using `hl_build_order_hash()`. This function serializes the order into MessagePack format, appends the nonce and vault address (if any), and computes the Keccak256 hash (referred to as `connection_id`).

5. **EIP-712 Signing**: Signs the `connection_id` hash using `eip712_sign_agent()`. This function implements EIP-712 typed data signing with domain separator and struct hash computation. The signature includes `r`, `s`, and recovery `v` components.

6. **JSON Request Assembly**: Builds a JSON payload containing:
   - The order action with asset ID, side, price, size, reduce-only flag, and time-in-force
   - The nonce (current timestamp in milliseconds)
   - The signature (`r`, `s`, `v`)
   - Vault address (null in this case)

7. **HTTP Request Execution**: Sends a POST request to `/exchange` on either the testnet or mainnet API endpoint. The request includes the JSON body and `Content-Type: application/json` header.

8. **Response Parsing**: On successful HTTP response (status 200), parses the JSON body to extract the `oid` (order ID) using string search (`strstr`). The order ID is copied into `result->order_id` using `strdup()`.

**Section sources**
- [trading_api.c](file://src/trading_api.c#L79-L220)
- [hl_msgpack.h](file://include/hl_msgpack.h#L93-L98)
- [eip712.c](file://src/crypto/eip712.c#L260-L295)

## Thread Safety and Nonce Generation

The `hl_place_order()` function is thread-safe due to the use of a client-level mutex. The mutex is acquired at the beginning of the function using `pthread_mutex_lock()` and released at the end with `pthread_mutex_unlock()`, ensuring that only one thread can execute the order placement logic at a time.

Nonce generation is handled by `get_timestamp_ms()`, which returns the current Unix timestamp in milliseconds using `gettimeofday()`. This high-resolution timestamp ensures uniqueness across requests and prevents replay attacks. The nonce is included in the hash input to the EIP-712 signature, binding the request to a specific point in time.

**Section sources**
- [trading_api.c](file://src/trading_api.c#L31-L50)
- [trading_api.c](file://src/trading_api.c#L22-L26)
- [hl_internal.h](file://include/hl_internal.h#L55-L55)

## Error Handling

The function implements comprehensive error handling at multiple levels:

- **Invalid Parameters**: Returns `HL_ERROR_INVALID_PARAMS` if any input pointer is null.
- **Unknown Symbol**: Returns `HL_ERROR_INVALID_SYMBOL` if the symbol cannot be resolved to an asset ID.
- **Signature Failure**: Returns `HL_ERROR_SIGNATURE` if either `hl_build_order_hash()` or `eip712_sign_agent()` fails.
- **HTTP Errors**: Maps `lv3_error_t` codes to `hl_error_t` using `lv3_to_hl_error()`. HTTP non-200 responses return `HL_ERROR_NETWORK` or `HL_ERROR_API`.
- **Response Parsing Errors**: Returns `HL_ERROR_API` if the response body is empty or if the order ID cannot be parsed.

All error messages are written to `result->error` (a 256-byte buffer) for caller inspection.

**Section sources**
- [trading_api.c](file://src/trading_api.c#L79-L220)
- [hl_error.h](file://include/hl_error.h#L10-L37)

## Order Result Interpretation

The `hl_order_result_t` structure provides feedback on the outcome of the order placement:

- **order_id**: A dynamically allocated string containing the exchange-assigned order ID. The caller must free this memory.
- **status**: Initial status is set to `HL_ORDER_STATUS_OPEN`. Other possible values include `FILLED`, `PARTIALLY_FILLED`, `CANCELLED`, or `REJECTED`.
- **filled_quantity**: Set to 0.0 initially; will be updated upon fills.
- **average_price**: Set to 0.0 initially; reflects average fill price when partially or fully filled.
- **error**: Contains a human-readable error message if the operation failed.

If the order is placed successfully, `hl_place_order()` returns `HL_SUCCESS` and populates `order_id`. If not, it returns an error code and sets the `error` field accordingly.

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L120-L126)

## Practical Examples from simple_trade.c

The `simple_trade.c` example demonstrates placing a **long position** using a limit buy order:

```c
hl_order_request_t order = {
    .symbol = "BTC",
    .side = HL_SIDE_BUY,
    .price = market_price * 0.80,
    .quantity = 0.001,
    .order_type = HL_ORDER_TYPE_LIMIT,
    .time_in_force = HL_TIF_GTC,
    .reduce_only = false
};
```

It also shows placing a **short position** by setting `side = HL_SIDE_SELL`. After placing the order, it checks the result and cancels the order if it remains open.

The example includes full error checking, balance queries, and position monitoring, making it a complete reference for integrating order placement functionality.

**Section sources**
- [simple_trade.c](file://examples/simple_trade.c#L0-L165)