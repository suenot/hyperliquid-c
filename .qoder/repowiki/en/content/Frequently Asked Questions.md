# Frequently Asked Questions

<cite>
**Referenced Files in This Document**   
- [README.md](file://README.md)
- [STATUS.md](file://STATUS.md)
- [src/client.c](file://src/client.c)
- [src/trading_api.c](file://src/trading_api.c)
- [src/account.c](file://src/account.c)
- [src/orderbook.c](file://src/orderbook.c)
- [src/ohlcv.c](file://src/ohlcv.c)
- [src/trades.c](file://src/trades.c)
- [src/crypto/eip712.c](file://src/crypto/eip712.c)
- [src/websocket.c](file://src/websocket.c)
- [include/hyperliquid.h](file://include/hyperliquid.h)
- [include/hl_account.h](file://include/hl_account.h)
- [include/hl_ticker.h](file://include/hl_ticker.h)
- [include/hl_orderbook.h](file://include/hl_orderbook.h)
- [include/hl_ohlcv.h](file://include/hl_ohlcv.h)
</cite>

## Table of Contents
1. [Installation](#installation)
2. [Trading](#trading)
3. [Market Data](#market-data)
4. [Security](#security)
5. [Troubleshooting](#troubleshooting)
6. [Library Compatibility](#library-compatibility)

## Installation

### How do I install the Hyperliquid C SDK on Linux/macOS?
To install the Hyperliquid C SDK on Linux or macOS, first clone the repository and install the required dependencies. On Ubuntu/Debian systems, use `sudo apt-get install libcurl4-openssl-dev libjansson-dev libsecp256k1-dev uuid-dev`. For macOS with Homebrew, run `brew install curl jansson libsecp256k1 ossp-uuid`. After installing dependencies, build the SDK using `make` and run tests with `make test`.

**Section sources**
- [README.md](file://README.md#L150-L170)

### What are the prerequisites for Windows installation?
For Windows installation, use MSYS2 or a similar environment. Install required packages with `pacman -S mingw-w64-x86_64-curl mingw-w64-x86_64-jansson mingw-w64-x86_64-libsecp256k1`. Then build the SDK using MinGW with the command `make CC=gcc`. Ensure that all dependencies are properly linked during compilation.

**Section sources**
- [README.md](file://README.md#L172-L178)

### How do I verify the SDK installation?
After installation, run `make test` to execute the full test suite. This verifies that all components including HTTP client, crypto functions, and market data retrieval are working correctly. Integration tests can be run with `make test-integration`, which requires valid testnet credentials in a `.env` file.

**Section sources**
- [README.md](file://README.md#L280-L288)

## Trading

### How do I place a trading order using the SDK?
Use the `hl_place_order()` function with a properly configured `hl_order_request_t` structure. The request must include the symbol, side (buy/sell), price, and quantity. The function returns an `hl_order_result_t` containing the order ID upon success. Example: `hl_place_order(client, &request, &result)`.

**Section sources**
- [src/trading_api.c](file://src/trading_api.c#L79-L220)
- [include/hyperliquid.h](file://include/hyperliquid.h#L276-L278)

### How do I cancel an existing order?
To cancel an order, use `hl_cancel_order()` with the client instance, symbol, and order ID. The function returns a result structure indicating whether the cancellation was successful. For batch cancellations, use `hl_cancel_orders()` with an array of order IDs.

**Section sources**
- [src/trading_api.c](file://src/trading_api.c#L225-L338)
- [include/hyperliquid.h](file://include/hyperliquid.h#L289-L292)

### Can I place multiple orders simultaneously?
Yes, use `hl_create_orders()` to place multiple orders in a single call. This function accepts an array of `hl_order_request_t` structures and returns an array of results. Each order is processed individually, and the function continues processing even if some orders fail.

**Section sources**
- [src/orders.c](file://src/orders.c#L653-L674)
- [include/hyperliquid.h](file://include/hyperliquid.h#L615-L618)

### How do I edit an existing order?
The SDK provides `hl_edit_order()` which implements the edit functionality by canceling the existing order and creating a new one with updated parameters. This atomic operation ensures consistency in the order lifecycle management.

**Section sources**
- [README.md](file://README.md#L83-L88)

## Market Data

### How do I fetch account balance information?
Use `hl_fetch_balance()` with the client instance and account type (spot/perpetual). The balance data is returned in an `hl_balance_t` structure containing account value, margin usage, and individual asset balances. For spot accounts, spot balance details are included in the structure.

**Section sources**
- [src/account.c](file://src/account.c#L249-L261)
- [include/hl_account.h](file://include/hl_account.h#L134-L138)

### How do I retrieve position information?
Call `hl_fetch_positions()` to get all open positions. The function returns an array of `hl_position_t` structures with position details including size, entry price, and unrealized PnL. Individual positions can be fetched using `hl_fetch_position()` with a specific symbol.

**Section sources**
- [src/account.c](file://src/account.c#L450-L540)
- [include/hl_account.h](file://include/hl_account.h#L162-L166)

### How do I get market ticker data?
Use `hl_fetch_ticker()` to retrieve current market data for a specific symbol. The ticker includes bid/ask prices, last price, 24-hour volume, and additional data like mark price and funding rate for perpetual contracts.

**Section sources**
- [include/hl_ticker.h](file://include/hl_ticker.h#L72-L72)

### How do I access order book data?
The `hl_fetch_order_book()` function retrieves the order book for a given symbol and depth. It returns an `hl_orderbook_t` structure containing bid and ask levels sorted appropriately. The function automatically handles both swap and spot market types.

**Section sources**
- [src/orderbook.c](file://src/orderbook.c#L102-L215)
- [include/hl_orderbook.h](file://include/hl_orderbook.h#L39-L39)

### How do I retrieve OHLCV candle data?
Use `hl_fetch_ohlcv()` to get historical candle data. Specify the symbol, timeframe ("1m", "1h", "1d"), and optional parameters like `since` and `limit`. The function returns an array of `hl_ohlcv_t` structures with open, high, low, close prices and volume.

**Section sources**
- [src/ohlcv.c](file://src/ohlcv.c#L136-L283)
- [include/hl_ohlcv.h](file://include/hl_ohlcv.h#L80-L82)

### How do I get recent trade history?
Call `hl_fetch_trades()` with the symbol and optional limit parameter. This returns an array of `hl_trade_t` structures containing trade details such as price, amount, and timestamp. For user-specific trades, use `hl_fetch_my_trades()`.

**Section sources**
- [src/trades.c](file://src/trades.c#L104-L191)
- [include/hyperliquid.h](file://include/hyperliquid.h#L487-L491)

## Security

### How does EIP-712 signing work in this SDK?
The SDK implements EIP-712 signing using `libsecp256k1` for cryptographic operations. When placing authenticated requests, the SDK computes the domain hash and message hash, then signs the message with the private key. The recovery ID is calculated by testing all possible values and verifying against the expected Ethereum address.

**Section sources**
- [src/crypto/eip712.c](file://src/crypto/eip712.c)
- [STATUS.md](file://STATUS.md#L280-L315)

### How are private keys handled in the SDK?
Private keys are stored in memory within the `hl_client_t` structure and are zeroed out when the client is destroyed using `hl_client_destroy()`. The keys are never logged or transmitted in plain text. They are only used locally for signing operations before being converted to hexadecimal format for API requests.

**Section sources**
- [src/client.c](file://src/client.c#L34-L87)
- [src/client.c](file://src/client.c#L89-L107)

### Is the SDK thread-safe for private key operations?
Yes, the SDK is thread-safe for all operations including those involving private keys. A mutex is initialized in the client structure and locked during all sensitive operations such as order placement and cancellation. This prevents race conditions when multiple threads access the same client instance.

**Section sources**
- [src/client.c](file://src/client.c#L34-L87)

## Troubleshooting

### What does error code HL_ERROR_SIGNATURE mean?
This error indicates a problem with the EIP-712 signature generation process. Common causes include invalid private keys, incorrect message hashing, or problems with the recovery ID calculation. Verify that your private key is correct and properly formatted (64 hex characters without 0x prefix).

**Section sources**
- [README.md](file://README.md#L350-L365)

### Why am I getting HL_ERROR_INVALID_SYMBOL?
This error occurs when the requested trading symbol is not recognized by the exchange. Ensure that the symbol is correctly formatted (e.g., "BTC/USDC:USDC") and that you have loaded market data using `hl_client_load_markets()` or equivalent functions before making symbol-based requests.

**Section sources**
- [src/trading_api.c](file://src/trading_api.c#L79-L220)

### How do I handle WebSocket connectivity issues?
For WebSocket connectivity problems, ensure that you have initialized the WebSocket client with `hl_ws_init_client()` and that your network allows connections to the WebSocket endpoints. The SDK includes auto-reconnection logic, but you should implement proper error handling callbacks to monitor connection status.

**Section sources**
- [src/websocket.c](file://src/websocket.c#L156-L181)

### What should I do if I encounter memory allocation errors?
Memory allocation errors (HL_ERROR_MEMORY) typically occur when the system is low on memory or there are memory leaks in the application. Ensure that you properly clean up all allocated resources using the appropriate free functions (e.g., `hl_free_balances()`, `hl_markets_free()`). Use tools like Valgrind to detect memory leaks in your application.

**Section sources**
- [README.md](file://README.md#L350-L365)

## Library Compatibility

### Is this library compatible with CCXT?
Yes, the Hyperliquid C SDK is fully CCXT-compatible with 100% method coverage for the core API. It implements the standard CCXT interface including `exchange.describe()`, capability mapping, and standardized data structures. This allows existing CCXT applications to integrate with Hyperliquid using this C implementation.

**Section sources**
- [README.md](file://README.md#L15-L20)
- [STATUS.md](file://STATUS.md#L420-L435)

### Can I use this SDK with other C/C++ trading systems?
Yes, the SDK is designed for easy integration with other C/C++ systems. It uses standard C99 syntax and depends on widely available libraries (`libcurl`, `cJSON`, `libsecp256k1`). The clean API surface and comprehensive documentation make it suitable for integration into larger trading frameworks and algorithmic trading systems.

**Section sources**
- [README.md](file://README.md#L15-L20)