# Error Handling

<cite>
**Referenced Files in This Document**   
- [hyperliquid.h](file://include/hyperliquid.h)
- [hl_logger.h](file://include/hl_logger.h)
- [client.c](file://src/client.c)
- [trading_api.c](file://src/trading_api.c)
- [simple_trade.c](file://examples/simple_trade.c)
- [trading_bot.c](file://examples/trading_bot.c)
</cite>

## Table of Contents
1. [Error Code Convention](#error-code-convention)
2. [Defined Error Codes](#defined-error-codes)
3. [Return Value Interpretation](#return-value-interpretation)
4. [Error Checking Patterns](#error-checking-patterns)
5. [Recoverable vs Unrecoverable Errors](#recoverable-vs-unrecoverable-errors)
6. [Error Logging](#error-logging)
7. [Common Error Scenarios](#common-error-scenarios)
8. [Retry Strategies](#retry-strategies)

## Error Code Convention

The Hyperliquid C SDK follows a standardized error code convention where negative integer values indicate specific error conditions and zero indicates success. This pattern is consistent across all API functions and simplifies error checking in client applications. All functions that can fail return a value of type `hl_error_t`, which is an enumeration defined in the SDK.

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L58-L85)

## Defined Error Codes

The following error codes are defined in the Hyperliquid C SDK, primarily in `hyperliquid.h`. These codes cover network issues, authentication failures, invalid parameters, and rate limiting scenarios.

```c
typedef enum {
    HL_SUCCESS = 0,                          // Operation successful
    HL_ERROR_INVALID_PARAMS = -1,           // Invalid parameters
    HL_ERROR_NETWORK = -2,                  // Network error
    HL_ERROR_API = -3,                      // API error
    HL_ERROR_AUTH = -4,                     // Authentication error
    HL_ERROR_INSUFFICIENT_BALANCE = -5,     // Insufficient balance
    HL_ERROR_INVALID_SYMBOL = -6,           // Invalid trading symbol
    HL_ERROR_ORDER_REJECTED = -7,           // Order rejected by exchange
    HL_ERROR_SIGNATURE = -8,                // Signature generation failed
    HL_ERROR_MSGPACK = -9,                  // MessagePack error
    HL_ERROR_JSON = -10,                    // JSON parsing error
    HL_ERROR_MEMORY = -11,                  // Memory allocation failed
    HL_ERROR_TIMEOUT = -12,                 // Operation timed out
    HL_ERROR_NOT_IMPLEMENTED = -13,         // Feature not implemented
    HL_ERROR_NOT_FOUND = -14,               // Resource not found
    HL_ERROR_PARSE = -15                    // Response parsing error
} hl_error_t;
```

These error codes are also referenced in other header files like `hl_types.h` and `hl_exchange.h`, but the canonical definition resides in `hyperliquid.h`. The error codes provide clear semantic meaning for different failure modes, enabling developers to implement appropriate error handling logic.

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L58-L85)
- [hl_types.h](file://include/hl_types.h)
- [hl_exchange.h](file://include/hl_exchange.h)

## Return Value Interpretation

All API functions in the Hyperliquid C SDK return an `hl_error_t` value that should be checked after each call. A return value of `HL_SUCCESS` (0) indicates the operation completed successfully, while any negative value indicates a specific error condition. Functions that return data typically use output parameters, and the return value solely indicates success or failure.

When an error occurs, additional error information may be available in output structures. For example, the `hl_order_result_t` structure contains an `error` field that may contain API-specific error messages when `hl_place_order` fails. This dual-layer error reporting provides both programmatic error codes and human-readable error descriptions.

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L276-L278)
- [trading_api.c](file://src/trading_api.c#L79-L220)

## Error Checking Patterns

Proper error checking is demonstrated throughout the example applications. The typical pattern involves checking the return value immediately after calling an API function and handling errors appropriately. The `simple_trade.c` example shows this pattern clearly:

```c
hl_error_t err = hl_place_order(client, &order, &result);
if (err == HL_SUCCESS) {
    printf("✅ Order placed successfully!\n");
} else {
    fprintf(stderr, "❌ Failed to place order: %s\n", hl_error_string(err));
    if (strlen(result.error) > 0) {
        fprintf(stderr, "   API Error: %s\n", result.error);
    }
}
```

This pattern ensures that errors are caught immediately and provides both the error code and any additional API error message. The `trading_bot.c` example shows more comprehensive error handling with user feedback and continued operation after non-critical errors.

**Section sources**
- [simple_trade.c](file://examples/simple_trade.c#L118-L135)
- [trading_bot.c](file://examples/trading_bot.c#L249-L255)

## Recoverable vs Unrecoverable Errors

The SDK distinguishes between recoverable and unrecoverable errors. Recoverable errors include transient conditions like network issues (`HL_ERROR_NETWORK`), timeouts (`HL_ERROR_TIMEOUT`), and temporary API errors (`HL_ERROR_API`). These errors typically allow the application to retry the operation after a delay.

Unrecoverable errors include permanent conditions like invalid parameters (`HL_ERROR_INVALID_PARAMS`), authentication failures (`HL_ERROR_AUTH`), insufficient balance (`HL_ERROR_INSUFFICIENT_BALANCE`), and invalid symbols (`HL_ERROR_INVALID_SYMBOL`). These errors indicate that the operation cannot succeed without changes to the input parameters or user intervention.

The distinction is important for implementing appropriate retry logic. Recoverable errors may be retried with exponential backoff, while unrecoverable errors should be reported to the user and not retried automatically.

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L58-L85)
- [trading_api.c](file://src/trading_api.c#L79-L220)

## Error Logging

The SDK provides basic logging capabilities through the `hl_logger.h` header, which defines simple logging macros. Applications can enable debug logging using `hl_set_debug(true)`, though this function is currently a placeholder and not fully implemented.

The logging system uses standard output and error streams with different severity levels:
- `HL_LOG_INFO` for informational messages
- `HL_LOG_WARN` for warnings
- `HL_LOG_ERROR` for errors
- `HL_LOG_DEBUG` for debug messages (only when DEBUG is defined)

Applications should use these macros consistently for logging, and can supplement them with their own logging infrastructure. The example applications demonstrate proper error reporting using `fprintf` to stderr for error conditions.

**Section sources**
- [hl_logger.h](file://include/hl_logger.h#L0-L36)
- [client.c](file://src/client.c#L148-L151)

## Common Error Scenarios

Several common error scenarios are addressed in the SDK and example applications:

**Connection timeouts**: Handled by the underlying HTTP client with `HL_ERROR_TIMEOUT`. The `hl_set_timeout` function allows clients to configure the timeout duration.

**Invalid signatures**: Occur when the private key cannot be used to sign messages properly, resulting in `HL_ERROR_SIGNATURE`. This typically indicates a problem with the private key format or cryptographic operations.

**Insufficient balance**: Returns `HL_ERROR_INSUFFICIENT_BALANCE` when an order cannot be placed due to inadequate funds. The `simple_trade.c` example checks the balance before placing an order to avoid this error.

**Authentication failures**: Return `HL_ERROR_AUTH` when the wallet address or private key are invalid or when the signature verification fails on the server side.

**Rate limiting**: While not explicitly defined as a separate error code, rate limiting by the exchange API would typically result in `HL_ERROR_API` or `HL_ERROR_NETWORK` with appropriate messages in the response.

The example applications demonstrate how to handle these scenarios gracefully, providing user feedback and continuing operation where possible.

**Section sources**
- [simple_trade.c](file://examples/simple_trade.c#L85-L105)
- [trading_api.c](file://src/trading_api.c#L79-L220)
- [hyperliquid.h](file://include/hyperliquid.h#L58-L85)

## Retry Strategies

For transient errors, the recommended strategy is exponential backoff with jitter. While the SDK does not implement automatic retry logic, applications should implement this pattern for recoverable errors like network issues and timeouts.

A typical retry strategy involves:
1. Detecting recoverable errors (network, timeout, API errors)
2. Waiting a base delay (e.g., 1 second) before retrying
3. Doubling the delay after each failed attempt
4. Adding random jitter to avoid thundering herd problems
5. Limiting the maximum number of retries or total retry time

The `trading_bot.c` example demonstrates a simple retry-capable design with its main loop, though it does not implement exponential backoff explicitly. Applications should implement retry logic around critical operations, especially order placement and balance queries, while respecting the exchange's rate limits.

**Section sources**
- [trading_bot.c](file://examples/trading_bot.c#L300-L320)
- [simple_trade.c](file://examples/simple_trade.c#L118-L135)