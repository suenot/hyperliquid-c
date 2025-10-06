# Examples and Use Cases

<cite>
**Referenced Files in This Document**   
- [trading_bot.c](file://examples/trading_bot.c)
- [comprehensive_demo.c](file://examples/comprehensive_demo.c)
- [hl_client.h](file://include/hl_client.h)
- [hl_exchange.h](file://include/hl_exchange.h)
- [hl_error.h](file://include/hl_error.h)
- [hl_ws_client.h](file://include/hl_ws_client.h)
- [README.md](file://README.md)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Trading Bot Example Analysis](#trading-bot-example-analysis)
3. [Comprehensive Demo Example Analysis](#comprehensive-demo-example-analysis)
4. [Common Implementation Patterns](#common-implementation-patterns)
5. [Error Handling and Resource Management](#error-handling-and-resource-management)
6. [Configuration and Initialization](#configuration-and-initialization)
7. [Suggested Extensions and Learning Exercises](#suggested-extensions-and-learning-exercises)
8. [Conclusion](#conclusion)

## Introduction
This document provides a comprehensive analysis of the example programs included in the Hyperliquid C SDK repository. The focus is on practical implementation examples that demonstrate the complete capabilities of the SDK for trading applications. The analysis covers two primary examples: the trading_bot.c which illustrates a complete trading strategy implementation, and comprehensive_demo.c which shows integration of multiple features across the SDK. These examples serve as practical guides for developers looking to implement trading strategies, manage accounts, and process market data using the Hyperliquid C SDK.

The examples demonstrate the SDK's CCXT-compatible interface, showing how to perform essential trading operations such as order placement, balance monitoring, market data fetching, and WebSocket streaming. By examining these examples in detail, developers can understand the implementation patterns, error handling strategies, and resource management techniques used throughout the SDK.

**Section sources**
- [trading_bot.c](file://examples/trading_bot.c#L1-L382)
- [comprehensive_demo.c](file://examples/comprehensive_demo.c#L1-L397)

## Trading Bot Example Analysis
The trading_bot.c example provides a complete implementation of a trading strategy that demonstrates the core functionality of the Hyperliquid C SDK. This example serves as a practical guide for implementing a trading bot with real-time market monitoring, order management, and WebSocket integration.

The bot begins by creating a client instance using hl_client_create() with wallet address and private key parameters, establishing a connection to the Hyperliquid API. It implements signal handling through the signal_handler function to ensure graceful shutdown when interrupted by SIGINT or SIGTERM signals, demonstrating proper resource cleanup practices.

The example showcases several key trading operations through dedicated functions. The display_balance() function demonstrates how to fetch and display account balances using hl_fetch_balance(), formatting the output in a tabular format for readability. The display_markets() function illustrates market data retrieval with hl_fetch_markets(), showing how to process and display available trading pairs. The display_ticker() function demonstrates real-time price monitoring for specific symbols.

A key feature of this example is the implementation of a complete trading workflow. The place_test_order() function shows how to create a limit order by first fetching the current ticker price, calculating a price 5% below market value, and then using hl_create_order() to submit the order. The function stores the order ID in a global variable (g_active_order_id) for potential cancellation, demonstrating state management in a trading application.

The cancel_active_order() function illustrates order cancellation using hl_cancel_order(), showing proper error handling and cleanup of the stored order ID. The display_open_orders() function demonstrates how to retrieve and display current open orders using hl_fetch_open_orders(), providing visibility into active trading positions.

WebSocket integration is implemented through the setup_websocket() function, which initializes the WebSocket client with hl_ws_init_client() and subscribes to real-time data streams using hl_watch_ticker() for market updates and hl_watch_orders() for order status changes. The bot_loop() function implements a periodic monitoring loop that checks balances, open orders, and market prices at regular intervals.

The example concludes with an interactive menu system that allows users to choose between different operations, including placing test orders, canceling orders, starting WebSocket streaming, or running the automated bot loop. This interactive approach makes the example both educational and practical for testing trading strategies.

**Section sources**
- [trading_bot.c](file://examples/trading_bot.c#L1-L382)
- [hl_client.h](file://include/hl_client.h#L14-L189)
- [hl_error.h](file://include/hl_error.h#L1-L38)

## Comprehensive Demo Example Analysis
The comprehensive_demo.c example provides a detailed demonstration of the Hyperliquid C SDK's capabilities, showcasing 84.3% of CCXT methods (43/51) implemented in the SDK. This example is structured as a step-by-step demonstration of various features, making it an excellent resource for understanding the breadth of functionality available in the SDK.

The demo is organized into ten distinct sections, each focusing on a specific aspect of the SDK. The demo_basic_connectivity() function demonstrates fundamental operations including client creation with hl_client_create(), connection testing with hl_test_connection(), and basic market data retrieval with hl_fetch_markets(), hl_fetch_ticker(), and hl_fetch_order_book(). This section establishes the foundation for all subsequent operations.

The demo_account_management() function illustrates account-related operations, showing how to retrieve account balances with hl_fetch_balance(), open positions with hl_fetch_positions(), and open orders with hl_fetch_open_orders(). This demonstrates the SDK's ability to provide a comprehensive view of a trading account's state.

The demo_order_management() function focuses on order lifecycle management, demonstrating how to retrieve different types of orders using specialized functions: hl_fetch_open_orders() for active orders, hl_fetch_closed_orders() for completed orders, hl_fetch_canceled_orders() for canceled orders, and hl_fetch_orders() for all orders. This comprehensive approach to order management is essential for building robust trading applications.

Advanced analytics capabilities are demonstrated in demo_advanced_analytics(), which shows how to retrieve funding rates, open interests, OHLCV data with hl_fetch_ohlcv(), and trade history with hl_fetch_trades(). The demo_historical_data() function extends this by showing user trade history retrieval with hl_fetch_my_trades() and other historical data endpoints.

WebSocket capabilities are thoroughly covered in demo_websocket_capabilities(), which documents the complete WebSocket framework including real-time ticker updates, order book streaming, trade feeds, order status updates, and WebSocket-based order placement. The demo_trading_capabilities() function outlines the complete trading workflow, including market and limit orders, order cancellation, status tracking, and bulk order queries.

The example concludes with demonstrations of CCXT compatibility, performance characteristics, and a final summary. The CCXT compatibility section shows how the SDK implements standard CCXT interfaces with hl_exchange_describe() for exchange description and hl_exchange_has_capability() for capability checking. The performance section highlights the SDK's optimization for high-frequency trading with low latency, memory efficiency, thread safety, and enterprise security features.

The comprehensive_demo.c example serves as both a feature showcase and a reference implementation, providing developers with a clear understanding of how to integrate various SDK components into a cohesive trading application.

**Section sources**
- [comprehensive_demo.c](file://examples/comprehensive_demo.c#L1-L397)
- [hl_exchange.h](file://include/hl_exchange.h#L1-L217)
- [README.md](file://README.md#L1-L728)

## Common Implementation Patterns
The example programs in the Hyperliquid C SDK demonstrate several consistent implementation patterns that reflect best practices for C-based trading applications. These patterns cover client initialization, error handling, memory management, and asynchronous operations.

Client initialization follows a consistent pattern across examples, using hl_client_create() with wallet address, private key, and testnet parameters. The examples consistently check for NULL return values and implement proper error handling when client creation fails. This pattern ensures that applications can gracefully handle initialization failures.

Error handling is implemented through return codes rather than exceptions, following C conventions. The examples consistently check the return value of SDK functions against HL_SUCCESS and provide appropriate error messages using the error codes defined in hl_error.h. This approach allows for fine-grained error handling and debugging.

Memory management patterns are evident in the examples, with proper allocation and deallocation of resources. The examples consistently call cleanup functions such as hl_free_balances(), hl_markets_free(), and hl_free_orders() after using data structures returned by SDK functions. This prevents memory leaks and ensures proper resource management.

The examples demonstrate a consistent pattern for handling different account types, particularly distinguishing between perpetual and spot accounts. Functions like hl_fetch_balance() accept an account type parameter, allowing developers to retrieve balance information for specific account types. This pattern supports the multi-account nature of the Hyperliquid exchange.

WebSocket integration follows a standardized pattern across examples. The process begins with hl_ws_init_client() to initialize the WebSocket client, followed by subscription functions like hl_watch_ticker() and hl_watch_orders() to establish data streams. The examples consistently implement callback functions to handle incoming data, demonstrating event-driven programming patterns.

Configuration and options management is demonstrated through the use of structured parameters. While the basic examples use default options, the patterns suggest how more complex configuration could be implemented, such as setting timeouts, rate limits, and other client options.

The examples also demonstrate patterns for state management in trading applications, using global variables to store important state information like active order IDs and client references. While this approach works for simple examples, it suggests patterns that could be extended to more sophisticated state management in production applications.

**Section sources**
- [trading_bot.c](file://examples/trading_bot.c#L1-L382)
- [comprehensive_demo.c](file://examples/comprehensive_demo.c#L1-L397)
- [hl_client.h](file://include/hl_client.h#L14-L189)

## Error Handling and Resource Management
The example programs in the Hyperliquid C SDK demonstrate robust error handling and resource management practices that are essential for reliable trading applications. These practices ensure that applications can handle failures gracefully and maintain system stability.

Error handling is implemented through a comprehensive error code system defined in hl_error.h. The examples consistently check the return values of SDK functions against HL_SUCCESS and provide appropriate error messages. The error codes cover various failure modes including invalid parameters, network issues, API errors, authentication problems, insufficient balance, invalid symbols, order rejection, signature failures, and parsing errors.

The examples demonstrate proper error propagation and handling at multiple levels. When an SDK function returns an error, the examples typically log the error code and either terminate the program or continue with alternative operations. For instance, in trading_bot.c, if hl_fetch_balance() fails, the program logs the error but continues execution rather than terminating, allowing the rest of the functionality to remain available.

Resource management is handled systematically throughout the examples. The SDK provides dedicated cleanup functions for each data structure, and the examples consistently call these functions to prevent memory leaks. For example, after calling hl_fetch_balance(), the examples call hl_free_balances() to free the allocated memory. Similarly, hl_markets_free() is called after hl_fetch_markets(), and hl_free_orders() is called after hl_fetch_open_orders().

The examples demonstrate proper client lifecycle management, with hl_client_destroy() called at the end of execution to clean up all resources associated with the client instance. This includes cleaning up HTTP connections, WebSocket connections, and any allocated memory within the client structure.

Memory allocation patterns are carefully managed in the examples. When the SDK allocates memory that must be freed by the caller (such as the order_id in hl_order_result_t), the examples consistently include the necessary free() calls. This prevents memory leaks and ensures that applications can run for extended periods without degrading performance.

The examples also demonstrate proper handling of string resources, using functions like strdup() when storing string data and ensuring that corresponding free() calls are made during cleanup. The global variable g_active_order_id is allocated with strdup() and freed both when canceled and during final cleanup.

For WebSocket operations, the examples show proper connection management, including initialization with hl_ws_init_client() and cleanup with hl_ws_cleanup_client(). The comprehensive_demo.c example demonstrates how to handle WebSocket disconnections and implement reconnection logic, which is crucial for maintaining reliable real-time data streams.

Signal handling is implemented in trading_bot.c through the signal_handler function, which sets a global flag (running) to false when interrupted. This allows the main loop to terminate gracefully and ensures that cleanup code is executed before the program exits.

**Section sources**
- [trading_bot.c](file://examples/trading_bot.c#L1-L382)
- [comprehensive_demo.c](file://examples/comprehensive_demo.c#L1-L397)
- [hl_error.h](file://include/hl_error.h#L1-L38)

## Configuration and Initialization
The example programs demonstrate clear patterns for configuration and initialization of the Hyperliquid C SDK, providing a foundation for building reliable trading applications. These patterns cover client creation, connection testing, and basic setup procedures.

Client initialization follows a consistent pattern using the hl_client_create() function, which requires a wallet address, private key, and testnet flag. The examples consistently validate the return value of this function, checking for NULL to handle initialization failures. This pattern ensures that applications can detect and respond to configuration issues early in the startup process.

Connection testing is implemented through the hl_test_connection() function, which verifies that the client can communicate with the Hyperliquid API. The examples consistently check the return value of this function against HL_SUCCESS and provide appropriate error messages when the connection test fails. This pattern allows applications to confirm connectivity before attempting more complex operations.

The examples demonstrate proper handling of authentication credentials, storing the wallet address and private key as string parameters to hl_client_create(). The comprehensive_demo.c example shows how to create a client without authentication parameters, indicating support for read-only operations that don't require authentication.

Configuration options are implied in the examples, with the testnet parameter allowing selection between testnet and mainnet environments. This pattern supports development and testing workflows, enabling developers to test their applications on the testnet before deploying to production.

The examples show a progressive initialization pattern, starting with basic connectivity and gradually adding more complex functionality. The comprehensive_demo.c example follows this pattern explicitly, with demo_basic_connectivity() establishing the foundation before moving to more advanced features. This approach allows developers to verify each component works correctly before integrating it into a complete system.

Resource cleanup is handled systematically, with hl_client_destroy() called at the end of execution to release all resources associated with the client. The examples consistently place this cleanup code in a central location, ensuring it is called regardless of how the program terminates.

The examples also demonstrate proper error handling during initialization, with appropriate error messages and graceful degradation when certain features are not available. For instance, if balance fetching fails due to missing authentication, the comprehensive_demo.c example continues execution rather than terminating, allowing other functionality to remain available.

**Section sources**
- [trading_bot.c](file://examples/trading_bot.c#L1-L382)
- [comprehensive_demo.c](file://examples/comprehensive_demo.c#L1-L397)
- [hl_client.h](file://include/hl_client.h#L14-L189)

## Suggested Extensions and Learning Exercises
Based on the example programs in the Hyperliquid C SDK, several extensions and learning exercises can be suggested to deepen understanding and enhance the functionality of trading applications. These extensions build upon the existing examples and introduce advanced concepts that are valuable for real-world trading systems.

One valuable extension is the addition of comprehensive risk management features. Building on the trading_bot.c example, developers could implement position sizing algorithms that calculate order amounts based on account equity and risk tolerance. This could include features like maximum position size limits, stop-loss placement based on volatility measures, and trailing stop functionality. The extension would require modifying the place_test_order() function to incorporate risk calculations and add new functions for stop-loss management.

Another important extension is enhanced logging and monitoring. The current examples use printf() for output, but a production system would benefit from a more sophisticated logging framework. Developers could implement a logging system with different severity levels (debug, info, warning, error) and output destinations (console, file, network). This would involve creating a logging module and replacing printf() calls with appropriate logging functions throughout the examples.

Performance optimization is another valuable learning exercise. Developers could implement connection pooling by creating a client manager that maintains multiple client instances and distributes requests among them. This would improve throughput for applications that need to make many concurrent requests. The exercise would involve creating a client pool data structure and modifying the examples to use the pool instead of individual clients.

Algorithmic trading strategies could be implemented as an extension to the trading_bot.c example. Developers could add technical indicators like moving averages, RSI, or MACD by implementing calculation functions and integrating them with the market data retrieval functions. The bot_loop() function could then be modified to make trading decisions based on these indicators rather than simply monitoring the market.

Error recovery and resilience features would enhance the reliability of the examples. Developers could implement automatic reconnection logic for WebSocket connections, with exponential backoff and maximum retry limits. They could also add circuit breaker patterns to temporarily disable trading operations after a certain number of consecutive failures, preventing cascading failures during market volatility.

Security enhancements would be valuable for production systems. Developers could implement secure credential storage by creating functions to encrypt and decrypt private keys, rather than storing them in plaintext. They could also add two-factor authentication integration and implement audit logging for all trading operations.

Finally, a modular architecture extension would improve code organization and reusability. Developers could refactor the examples into separate modules for account management, market data, trading operations, and risk management. This would involve creating header files for each module and implementing clean interfaces between them, promoting better code organization and easier maintenance.

**Section sources**
- [trading_bot.c](file://examples/trading_bot.c#L1-L382)
- [comprehensive_demo.c](file://examples/comprehensive_demo.c#L1-L397)
- [README.md](file://README.md#L1-L728)

## Conclusion
The example programs in the Hyperliquid C SDK provide comprehensive demonstrations of trading application development using the C programming language. The trading_bot.c and comprehensive_demo.c examples serve as excellent educational resources, illustrating complete trading workflows, account management, market data processing, and WebSocket integration.

These examples demonstrate the SDK's CCXT-compatible interface and its implementation of essential trading functionality. The consistent patterns for client initialization, error handling, memory management, and asynchronous operations provide a solid foundation for building reliable trading applications. The examples effectively showcase how to integrate various SDK components into cohesive systems that can monitor markets, manage orders, and execute trading strategies.

The comprehensive_demo.c example, in particular, serves as a feature catalog, demonstrating 84.3% of CCXT methods and providing insight into the SDK's capabilities for high-frequency trading, enterprise security, and real-time data processing. The trading_bot.c example provides a practical implementation of a trading strategy with interactive controls and graceful shutdown handling.

The examples also highlight areas for extension and improvement, suggesting valuable learning exercises in risk management, logging, performance optimization, algorithmic trading, error recovery, security, and modular architecture. These extensions can help developers deepen their understanding of trading system development and build more sophisticated applications.

Overall, the examples in the Hyperliquid C SDK repository provide a strong foundation for developers looking to build trading applications. They demonstrate best practices in C programming while showcasing the capabilities of the SDK for real-world trading scenarios.

**Section sources**
- [trading_bot.c](file://examples/trading_bot.c#L1-L382)
- [comprehensive_demo.c](file://examples/comprehensive_demo.c#L1-L397)
- [README.md](file://README.md#L1-L728)