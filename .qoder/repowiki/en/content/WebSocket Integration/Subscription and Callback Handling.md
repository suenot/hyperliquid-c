# Subscription and Callback Handling

<cite>
**Referenced Files in This Document**   
- [hl_ws_client.h](file://include/hl_ws_client.h)
- [ws_client.c](file://src/ws_client.c)
- [websocket.c](file://src/websocket.c)
- [websocket_demo.c](file://examples/websocket_demo.c)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Callback Registration Mechanism](#callback-registration-mechanism)
3. [Message Callback Function Signature](#message-callback-function-signature)
4. [Subscription Management](#subscription-management)
5. [Real-Time Data Handling Examples](#real-time-data-handling-examples)
6. [Thread Safety and Data Synchronization](#thread-safety-and-data-synchronization)
7. [Latency Minimization Strategies](#latency-minimization-strategies)

## Introduction
This document details the subscription mechanisms and callback registration system in the Hyperliquid C SDK's WebSocket client implementation. It explains how real-time market data and user updates are delivered through a callback-based architecture, covering the configuration of message handlers, subscription protocols for different data types, and thread safety considerations.

## Callback Registration Mechanism

The WebSocket client uses `hl_ws_client_set_message_callback()` to configure message handlers that receive raw WebSocket messages. This function stores both the callback function pointer and user data in thread-safe storage, ensuring safe access across the WebSocket client's lifecycle.

When a callback is registered, both the public client structure and internal implementation structure are updated under mutex protection. This dual storage approach allows the WebSocket thread to access the callback directly from internal state while maintaining the public API contract.

The registration process involves:
1. Validating the client pointer
2. Acquiring a mutex to protect shared state
3. Updating callback references in both public and internal structures
4. Storing associated user data
5. Releasing the mutex

This design ensures that callback updates are atomic and thread-safe, preventing race conditions between the application thread setting callbacks and the WebSocket thread invoking them.

**Section sources**
- [ws_client.c](file://src/ws_client.c#L221-L234)
- [hl_ws_client.h](file://include/hl_ws_client.h#L104-L106)

## Message Callback Function Signature

The `hl_ws_message_callback_t` function signature defines the contract for message handlers:
```c
typedef void (*hl_ws_message_callback_t)(const char* message, size_t size, void* user_data);
```

This callback is invoked whenever a complete message is received from the WebSocket server. The parameters provide:
- `message`: Pointer to the raw message payload (typically JSON)
- `size`: Length of the message in bytes
- `user_data`: Application-provided context passed during registration

The callback executes in the context of the WebSocket client's background thread, which means implementations must be thread-safe and avoid blocking operations. The user data parameter enables applications to pass context (such as object references or state) to the callback without relying on global variables.

Message callbacks are typically used as dispatchers that parse incoming messages and route them to appropriate handlers based on message type, rather than performing extensive processing directly in the callback.

**Section sources**
- [hl_ws_client.h](file://include/hl_ws_client.h#L14-L14)

## Subscription Management

Subscriptions for different data types are structured as JSON messages sent to the WebSocket server. Each subscription type follows a consistent pattern with a method field and subscription object containing the type and relevant parameters.

### Ticker Subscriptions
Ticker updates are requested with:
```json
{"method":"subscribe","subscription":{"type":"ticker","coin":"BTC/USDC:USDC"}}
```

### Order Book Subscriptions
Order book updates use:
```json
{"method":"subscribe","subscription":{"type":"l2Book","coin":"ETH/USDC:USDC"}}
```

### Trade Subscriptions
User trade updates require authentication and use the user's wallet address:
```json
{"method":"subscribe","subscription":{"type":"userFills","user":"0x..."}}
```

Each successful subscription returns a unique subscription ID that can be used to unsubscribe later. The client maintains an internal array of active subscriptions, tracking their channel type, symbol, callback, and user data. When messages arrive, they are dispatched to the appropriate callback based on the message content and subscription routing.

The subscription system automatically handles connection management, attempting to connect the WebSocket if not already connected before sending subscription requests.

**Section sources**
- [websocket.c](file://src/websocket.c#L156-L181)
- [websocket.c](file://src/websocket.c#L197-L222)
- [websocket.c](file://src/websocket.c#L320-L348)

## Real-Time Data Handling Examples

The `websocket_demo.c` example demonstrates practical usage of the subscription system. It shows how to:

1. Initialize a WebSocket client through the high-level `hl_client_t` interface
2. Subscribe to multiple data streams (tickers, order books, user orders, trades)
3. Implement callback functions for different update types
4. Manage the subscription lifecycle including unsubscription

The demo creates separate callbacks for ticker updates, order book changes, order status updates, and trade executions. Each callback receives a generic data pointer and user data, allowing for type-specific processing while maintaining a consistent interface.

Subscription functions return human-readable subscription IDs that can be used for debugging and management. The example shows how to use `hl_unwatch()` to cleanly unsubscribe from specific data streams before shutting down the client.

**Section sources**
- [websocket_demo.c](file://examples/websocket_demo.c#L0-L201)

## Thread Safety and Data Synchronization

The WebSocket implementation uses pthread mutexes to ensure thread safety when accessing shared state. The `ws_client_internal_t` structure contains a mutex that protects callback references, connection state, and other shared data.

All operations that modify callback pointers or client state acquire this mutex, preventing race conditions between:
- The application thread registering callbacks
- The WebSocket thread invoking callbacks
- Background connection management threads

Data synchronization between the WebSocket thread and application logic is the responsibility of the application. The SDK delivers messages via callbacks in the WebSocket thread context, and applications must implement appropriate mechanisms (such as thread-safe queues or atomic operations) to transfer data to their main application logic.

The design assumes that callbacks will quickly dispatch work to other threads or copy data for later processing, rather than performing blocking operations that could delay the WebSocket message loop.

**Section sources**
- [ws_client.c](file://src/ws_client.c#L13-L28)
- [ws_client.c](file://src/ws_client.c#L221-L234)

## Latency Minimization Strategies

To minimize message processing latency, applications should follow these strategies:

1. **Keep callbacks lightweight**: Message callbacks should avoid blocking operations like I/O, network requests, or complex computations that could delay subsequent message processing.

2. **Use efficient parsing**: When processing JSON messages, use streaming parsers or optimized libraries to reduce parsing overhead.

3. **Batch processing**: Consider batching multiple messages before processing to amortize processing costs, especially for high-frequency data like order book updates.

4. **Dedicated processing threads**: Offload message processing to worker threads to prevent the WebSocket thread from being blocked.

5. **Pre-allocate memory**: Reuse buffers and pre-allocate data structures to avoid allocation overhead during message processing.

6. **Selective subscriptions**: Subscribe only to necessary data streams and symbols to reduce message volume.

The underlying WebSocket client already implements connection optimization features like automatic reconnection, ping/pong heartbeats, and message fragmentation handling to maintain low-latency connectivity.

**Section sources**
- [ws_client.c](file://src/ws_client.c#L0-L287)
- [websocket.c](file://src/websocket.c#L0-L375)