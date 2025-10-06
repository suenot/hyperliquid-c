# Order Modification

<cite>
**Referenced Files in This Document**   
- [hyperliquid.h](file://include/hyperliquid.h)
- [trading_api.c](file://src/trading_api.c)
- [orders.c](file://src/orders.c)
- [msgpack/serialize.c](file://src/msgpack/serialize.c)
- [crypto/eip712.c](file://src/crypto/eip712.c)
- [hl_error.h](file://include/hl_error.h)
- [hl_types.h](file://include/hl_types.h)
</cite>

## Table of Contents
1. [Function Signature and Parameters](#function-signature-and-parameters)
2. [Modification Mechanism](#modification-mechanism)
3. [Internal Flow](#internal-flow)
4. [Action Hash and Signature Generation](#action-hash-and-signature-generation)
5. [Non-Atomic Nature of Modification](#non-atomic-nature-of-modification)
6. [Limitations and Constraints](#limitations-and-constraints)
7. [Usage Patterns](#usage-patterns)
8. [Error Handling](#error-handling)

## Function Signature and Parameters

The `hl_modify_order` function enables modification of existing orders through a structured interface. It accepts four parameters: a client pointer, the original order ID as a `uint64_t`, a new order request structure, and an output result structure.

- `client`: Pointer to the `hl_client_t` structure containing authentication, network, and state information required for secure communication with the Hyperliquid exchange.
- `order_id`: The unique identifier of the existing order to be modified, represented as an unsigned 64-bit integer.
- `new_order`: A constant pointer to `hl_order_request_t`, which defines the updated parameters such as price, size, side, symbol, time-in-force, and reduce-only flag.
- `result`: A pointer to `hl_order_result_t` where the outcome of the modification operation is stored, including the new order ID, status, filled quantity, average price, or error details.

This signature ensures type safety and clear separation between input and output data structures.

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L246-L249)
- [hl_types.h](file://include/hl_types.h#L108-L126)

## Modification Mechanism

Order modification does not alter an existing order in-place but instead replaces it with a new one while preserving the trading context. When `hl_modify_order` is invoked, the system initiates a two-step process: first canceling the original order using its `order_id`, then placing a new order with the parameters specified in `new_order`. This replacement strategy maintains continuity in position management and avoids disrupting ongoing trading strategies.

The replacement preserves critical aspects such as trade intent and market exposure, ensuring that modifications align with the user's strategic goals. However, because this is not an atomic operation, there is a temporal gap between cancellation and placement during which market conditions may change, potentially affecting execution quality.

**Section sources**
- [orders.c](file://src/orders.c#L705-L726)
- [trading_api.c](file://src/trading_api.c#L225-L338)

## Internal Flow

The internal execution of `hl_modify_order` follows a sequential validation and processing pipeline:

1. **Input Validation**: The function validates all input pointers (`client`, `new_order`, `result`) and checks for valid `order_id`.
2. **Asset Resolution**: The symbol from `new_order` is used to resolve the corresponding asset ID via `get_asset_id`, which queries market data if necessary.
3. **Cancellation Request**: The existing order is canceled by constructing a `hl_cancel_t` structure containing the asset ID and order ID, then submitting it via `hl_cancel_order`.
4. **Placement Request**: Upon successful cancellation, a new order is placed using `hl_place_order` with the parameters in `new_order`.
5. **Result Aggregation**: The result from the placement operation is returned, including the new order ID and execution status.

Each step is protected by mutex locks to ensure thread safety, and error propagation occurs immediately upon failure at any stage.

**Section sources**
- [trading_api.c](file://src/trading_api.c#L79-L220)
- [orders.c](file://src/orders.c#L705-L726)

## Action Hash and Signature Generation

Security and integrity of modification operations are ensured through cryptographic signing using EIP-712 standards. For both the cancellation and placement phases, an action hash is computed and signed with the user's private key.

- **Cancellation Hash**: Generated via `hl_build_cancel_hash`, which serializes the cancel action into MessagePack format, appends the nonce and vault address (if present), and computes the Keccak256 hash.
- **Placement Hash**: Created using `hl_build_order_hash`, which performs similar serialization for the new order, incorporating grouping, nonce, and vault address.
- **Signature Process**: The hash is signed using `eip712_sign_agent`, which constructs the domain and struct hashes according to EIP-712 rules and applies ECDSA signing with the secp256k1 curve.

The resulting signature includes `r`, `s`, and `v` components, which are included in the JSON payload sent to the exchange API.

**Section sources**
- [msgpack/serialize.c](file://src/msgpack/serialize.c#L133-L232)
- [crypto/eip712.c](file://src/crypto/eip712.c#L260-L295)

## Non-Atomic Nature of Modification

It is crucial to understand that order modification is **not atomic**—it is implemented as a sequential cancel-then-place operation. This design introduces several implications:

- **Execution Gap**: There is a measurable time interval between the cancellation of the old order and the submission of the new one. During this window, the market may move, leading to slippage or missed opportunities.
- **Race Conditions**: In fast-moving markets, another trader may fill the original order after the cancellation request is issued but before it is processed, resulting in a failed modification.
- **Partial Failure**: If the cancellation succeeds but the new order fails (due to insufficient funds, invalid parameters, or network issues), the user will be left without an active order.

Applications must handle these scenarios gracefully, potentially by retrying the operation or alerting the user.

**Section sources**
- [orders.c](file://src/orders.c#L705-L726)
- [trading_api.c](file://src/trading_api.c#L79-L220)

## Limitations and Constraints

Several constraints govern the modification process:

- **Immutable Fields**: Certain order attributes cannot be modified post-placement, including the trading symbol, order type (limit/market), and side (buy/sell). These must remain consistent between the original and new orders.
- **Market Rules**: Modifications are subject to exchange-specific rules, such as minimum price increments, size requirements, and time-in-force policies. Violations result in rejection.
- **Signature Requirements**: The private key must be available and correctly formatted; signature generation failures will abort the operation.
- **Rate Limits**: Frequent modifications may trigger rate limiting on the API, requiring backoff strategies in automated systems.

Additionally, trailing orders or conditional triggers cannot be modified directly and require full cancellation and recreation.

**Section sources**
- [hl_error.h](file://include/hl_error.h#L10-L38)
- [trading_api.c](file://src/trading_api.c#L79-L220)

## Usage Patterns

### Trailing Stop Orders
Trailing stops can be dynamically adjusted by periodically modifying the stop price based on market movement. A common pattern involves:
1. Monitoring price trends using OHLCV or ticker data.
2. Calculating a new stop price based on a trailing delta.
3. Modifying the existing stop order with the updated price.

This allows traders to lock in profits while maintaining exposure to favorable trends.

### Dynamic Pricing Strategies
Algorithms that adjust limit prices in response to order book depth or volatility can use `hl_modify_order` to reposition resting orders. For example:
- In low-liquidity conditions, widen the spread to avoid premature fills.
- During high volatility, tighten the spread to capture better execution.

These strategies require tight integration with real-time market data and careful timing to minimize exposure during the cancel/place window.

**Section sources**
- [orders.c](file://src/orders.c#L705-L726)
- [examples/trading_bot.c](file://examples/trading_bot.c)

## Error Handling

Robust error handling is essential when modifying orders. The following error conditions are explicitly managed:

- **Invalid Order ID**: If the provided `order_id` does not exist or belongs to a filled/canceled order, `HL_ERROR_ORDER_REJECTED` is returned.
- **Signature Failures**: Issues during EIP-712 signing (e.g., malformed private key) result in `HL_ERROR_SIGNATURE`.
- **Network Errors**: Failures in HTTP communication return `HL_ERROR_NETWORK` or `HL_ERROR_TIMEOUT`.
- **Market Constraints**: Attempts to set invalid prices or sizes trigger `HL_ERROR_INVALID_PARAMS` or `HL_ERROR_API`.
- **Authentication Issues**: Missing or invalid wallet credentials lead to `HL_ERROR_AUTH`.

All errors are propagated through the `hl_order_result_t.error` field, providing human-readable messages for debugging.

**Section sources**
- [hl_error.h](file://include/hl_error.h#L10-L38)
- [trading_api.c](file://src/trading_api.c#L79-L220)
- [orders.c](file://src/orders.c#L705-L726)