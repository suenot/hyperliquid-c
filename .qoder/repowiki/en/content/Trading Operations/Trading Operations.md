# Trading Operations

<cite>
**Referenced Files in This Document**   
- [hl_order_request_t](file://include/hyperliquid.h#L108-L117)
- [hl_order_result_t](file://include/hyperliquid.h#L120-L126)
- [hl_cancel_result_t](file://include/hyperliquid.h#L129-L132)
- [hl_place_order](file://src/trading_api.c#L79-L220)
- [hl_cancel_order](file://src/trading_api.c#L225-L338)
- [hl_edit_order](file://src/orders.c#L678-L727)
- [hl_create_orders](file://src/orders.c#L601-L627)
- [hl_cancel_orders](file://src/orders.c#L629-L655)
- [hl_fetch_open_orders](file://src/orders.c#L18-L104)
- [hl_fetch_closed_orders](file://src/orders.c#L109-L219)
- [hl_fetch_canceled_orders](file://src/orders.c#L224-L334)
- [hl_fetch_canceled_and_closed_orders](file://src/orders.c#L339-L455)
- [hl_fetch_orders](file://src/orders.c#L457-L599)
- [hl_fetch_order](file://src/orders.c#L507-L555)
- [hl_error_t](file://include/hl_error.h#L10-L37)
</cite>

## Table of Contents
1. [Order Placement](#order-placement)
2. [Order Cancellation](#order-cancellation)
3. [Order Modification](#order-modification)
4. [Order Lifecycle Management](#order-lifecycle-management)
5. [Order Response Handling](#order-response-handling)
6. [Error Handling and Codes](#error-handling-and-codes)

## Order Placement

The `hl_place_order()` function enables users to submit new trading orders to the Hyperliquid exchange. This function accepts a client instance, an order request structure, and returns a result structure containing the outcome of the operation.

The `hl_order_request_t` structure defines the parameters for order creation:
- **symbol**: Trading symbol (e.g., "BTC", "ETH")
- **side**: Buy or sell direction
- **price**: Limit price (set to 0 for market orders)
- **quantity**: Order quantity
- **order_type**: Limit or market order type
- **time_in_force**: Currently defaults to "Gtc" (Good Till Cancel)
- **reduce_only**: Flag indicating if the order should only reduce position
- **slippage_bps**: Slippage tolerance in basis points (for market orders)

Market orders are executed immediately at the best available price, while limit orders are placed at the specified price and executed when market conditions match. The function handles EIP-712 signing for transaction security and communicates with the exchange API endpoint to submit the order.

**Section sources**
- [hl_order_request_t](file://include/hyperliquid.h#L108-L117)
- [hl_place_order](file://src/trading_api.c#L79-L220)

## Order Cancellation

Order cancellation is supported through two primary functions: `hl_cancel_order()` for single order cancellation and `hl_cancel_orders()` for batch operations.

The `hl_cancel_order()` function requires:
- **client**: Client instance for authentication
- **symbol**: Trading symbol associated with the order
- **order_id**: Unique identifier of the order to cancel
- **result**: Output structure containing cancellation result

For batch cancellation, `hl_cancel_orders()` accepts an array of order IDs and returns an array of results. This enables efficient cancellation of multiple orders in a single operation, with each cancellation processed individually and results collected in the output array.

The cancellation process involves:
1. Validating input parameters
2. Retrieving asset ID for the specified symbol
3. Building the cancellation request structure
4. Creating and signing the EIP-712 message
5. Submitting the cancellation request to the exchange API

**Section sources**
- [hl_cancel_order](file://src/trading_api.c#L225-L338)
- [hl_cancel_orders](file://src/orders.c#L629-L655)
- [hl_cancel_result_t](file://include/hyperliquid.h#L129-L132)

## Order Modification

Order modification is implemented through the `hl_edit_order()` function, which follows a cancel-and-create pattern. This approach ensures atomicity and consistency in order updates.

The modification process:
1. Takes the existing order ID and new order parameters
2. Cancels the original order using `hl_cancel_order()`
3. Creates a new order with updated parameters using `hl_place_order()`
4. Returns the result of the new order creation

This implementation provides a reliable way to update order parameters such as price, quantity, or time-in-force settings. The function handles error propagation from both the cancellation and creation steps, ensuring that failures in either phase are properly reported.

**Section sources**
- [hl_edit_order](file://src/orders.c#L678-L727)

## Order Lifecycle Management

The system provides comprehensive functions for tracking orders throughout their lifecycle from submission to execution or rejection.

Key order retrieval functions include:
- **hl_fetch_open_orders()**: Retrieves currently active orders
- **hl_fetch_closed_orders()**: Retrieves filled or partially filled orders
- **hl_fetch_canceled_orders()**: Retrieves orders that were canceled
- **hl_fetch_canceled_and_closed_orders()**: Retrieves both canceled and closed orders
- **hl_fetch_orders()**: Retrieves all orders regardless of status
- **hl_fetch_order()**: Retrieves a specific order by ID

These functions query the exchange API to obtain order information, parse the JSON response, and populate the `hl_orders_t` structure with order details. The order status field indicates the current state of the order, with possible values including "open", "closed", and "canceled".

The order lifecycle typically follows this sequence:
1. Order submission via `hl_place_order()`
2. Order status becomes "open" upon successful submission
3. Order may be partially or fully filled, updating the filled quantity
4. Order status changes to "closed" when completely filled
5. Order status changes to "canceled" if explicitly canceled or rejected

**Section sources**
- [hl_fetch_open_orders](file://src/orders.c#L18-L104)
- [hl_fetch_closed_orders](file://src/orders.c#L109-L219)
- [hl_fetch_canceled_orders](file://src/orders.c#L224-L334)
- [hl_fetch_canceled_and_closed_orders](file://src/orders.c#L339-L455)
- [hl_fetch_orders](file://src/orders.c#L457-L599)
- [hl_fetch_order](file://src/orders.c#L507-L555)

## Order Response Handling

Successful order operations return results through dedicated result structures that provide detailed information about the transaction outcome.

The `hl_order_result_t` structure contains:
- **order_id**: Unique identifier assigned by the exchange
- **status**: Current order status
- **filled_quantity**: Amount of the order that has been filled
- **average_price**: Average execution price for filled portions
- **error**: Error message string in case of failure

The `hl_cancel_result_t` structure provides:
- **cancelled**: Boolean indicating successful cancellation
- **error**: Error message string if cancellation failed

These result structures enable applications to track order execution, update user interfaces, and implement appropriate business logic based on order status changes. The order ID returned in successful placements can be used for subsequent operations such as cancellation or status queries.

**Section sources**
- [hl_order_result_t](file://include/hyperliquid.h#L120-L126)
- [hl_cancel_result_t](file://include/hyperliquid.h#L129-L132)

## Error Handling and Codes

The trading API implements a comprehensive error handling system with specific error codes for different failure scenarios.

Key error codes defined in `hl_error_t` include:
- **HL_SUCCESS**: Operation completed successfully
- **HL_ERROR_INVALID_PARAMS**: Invalid input parameters
- **HL_ERROR_NETWORK**: Network communication failure
- **HL_ERROR_API**: API request failed
- **HL_ERROR_AUTH**: Authentication failure
- **HL_ERROR_INVALID_SYMBOL**: Unknown trading symbol
- **HL_ERROR_SIGNATURE**: Signature generation or validation failure

These error codes enable applications to implement appropriate error recovery strategies and provide meaningful feedback to users. The system also includes replay protection through nonce generation based on timestamps, preventing duplicate order submissions.

The API ensures thread safety through mutex locking during critical operations, protecting against race conditions in multi-threaded environments. Idempotency is maintained through the use of unique nonces and proper error handling, ensuring consistent behavior across multiple attempts.

**Section sources**
- [hl_error_t](file://include/hl_error.h#L10-L37)