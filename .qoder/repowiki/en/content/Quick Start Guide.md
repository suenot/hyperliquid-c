# Quick Start Guide

<cite>
**Referenced Files in This Document**   
- [QUICKSTART.md](file://QUICKSTART.md)
- [hyperliquid.h](file://include/hyperliquid.h)
- [client.c](file://src/client.c)
- [account.c](file://src/account.c)
- [markets.c](file://src/markets.c)
- [trading_api.c](file://src/trading_api.c)
- [simple_trade.c](file://examples/simple_trade.c)
- [simple_balance.c](file://examples/simple_balance.c)
</cite>

## Table of Contents
1. [Installation](#installation)
2. [Client Initialization](#client-initialization)
3. [Fetching Market Data](#fetching-market-data)
4. [Executing Your First Trade](#executing-your-first-trade)
5. [Configuration and Security](#configuration-and-security)
6. [Compilation and Execution](#compilation-and-execution)
7. [Common Pitfalls and Troubleshooting](#common-pitfalls-and-troubleshooting)

## Installation

### 1. Install Dependencies

#### macOS
```bash
brew install libsecp256k1 msgpack-c curl cjson openssl
```

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    libsecp256k1-dev \
    libmsgpack-dev \
    libcurl4-openssl-dev \
    libcjson-dev \
    libssl-dev \
    build-essential
```

#### Arch Linux
```bash
sudo pacman -S libsecp256k1 msgpack-c curl cjson openssl gcc make
```

### 2. Clone and Build
```bash
git clone https://github.com/yourusername/hyperliquid-c.git
cd hyperliquid-c
make
```

You should see:
```
✅ Static library built successfully
✅ Shared library built successfully
```

### 3. Install Library (Optional)
```bash
sudo make install
```
This installs to `/usr/local/lib` and `/usr/local/include`.

**Section sources**
- [QUICKSTART.md](file://QUICKSTART.md#L1-L50)

## Client Initialization

Initialize the Hyperliquid client using your wallet credentials:

```c
#include "hyperliquid.h"
#include <stdio.h>
#include <stdlib.h>

// Get credentials from environment
const char *wallet = getenv("HYPERLIQUID_WALLET_ADDRESS");
const char *privkey = getenv("HYPERLIQUID_PRIVATE_KEY");

// Create client (testnet = true)
hl_client_t *client = hl_client_create(wallet, privkey, true);

// Test connection
if (!hl_test_connection(client)) {
    printf("Connection failed!\n");
    return 1;
}
printf("✅ Connected!\n");
```

The `hl_client_create()` function validates wallet address format (must include `0x` prefix) and private key length (64 hex characters without `0x` prefix). The client handles thread safety with mutex protection and initializes HTTP connections automatically.

**Section sources**
- [hyperliquid.h](file://include/hyperliquid.h#L154-L156)
- [client.c](file://src/client.c#L34-L87)

## Fetching Market Data

### Fetch Account Balance
```c
// Get balance
hl_balance_t balance;
hl_fetch_balance(client, HL_ACCOUNT_PERPETUAL, &balance);
printf("Balance: $%.2f USDC\n", balance.account_value);
```

### Fetch Market Information
```c
// Get BTC price
double price;
hl_get_market_price(client, "BTC", &price);
printf("BTC Price: $%.2f\n", price);

// Fetch all markets
hl_markets_t markets;
hl_fetch_markets(client, &markets);
// Use markets data...
hl_markets_free(&markets);
```

The `hl_fetch_balance()` function retrieves account information for either perpetual or spot accounts, while `hl_fetch_markets()` fetches both swap and spot markets from the exchange API.

**Section sources**
- [account.c](file://src/account.c#L249-L261)
- [markets.c](file://src/markets.c#L389-L411)
- [hyperliquid.h](file://include/hyperliquid.h#L86-L86)

## Executing Your First Trade

Place a limit order with proper error handling:

```c
// Place order (20% below market)
hl_order_request_t order = {
    .symbol = "BTC",
    .side = HL_SIDE_BUY,
    .price = price * 0.80,
    .quantity = 0.001,
    .order_type = HL_ORDER_TYPE_LIMIT,
    .time_in_force = HL_TIF_GTC,
    .reduce_only = false
};

hl_order_result_t result;
if (hl_place_order(client, &order, &result) == HL_SUCCESS) {
    printf("✅ Order placed! ID: %llu\n", result.order_id);
    
    // Cancel it
    hl_cancel_order(client, "BTC", result.order_id);
    printf("✅ Order canceled!\n");
}
```

The `hl_place_order()` function handles EIP-712 signing, nonce generation, and HTTP request construction automatically. It returns detailed error information through the result structure.

**Section sources**
- [trading_api.c](file://src/trading_api.c#L79-L220)
- [hyperliquid.h](file://include/hyperliquid.h#L210-L212)

## Configuration and Security

### Environment Variables
Set these environment variables before running your application:

```bash
export HYPERLIQUID_WALLET_ADDRESS="0xYourWalletAddress"
export HYPERLIQUID_PRIVATE_KEY="your_private_key_64_hex_chars"
export HYPERLIQUID_TESTNET="true"  # Use testnet for testing!
```

### Security Best Practices
- Always test on testnet first
- Store private keys in environment variables, not code
- Use restricted API keys with only necessary permissions
- Implement proper error handling for all API calls
- Never commit credentials to version control
- Use isolated wallets for trading bots

**Section sources**
- [QUICKSTART.md](file://QUICKSTART.md#L52-L85)

## Compilation and Execution

### Compile and Run
```bash
# If installed system-wide
gcc -o my_first_trade my_first_trade.c -lhyperliquid \
    -lsecp256k1 -lmsgpackc -lcurl -lcjson -lssl -lcrypto

# If using local build
gcc -o my_first_trade my_first_trade.c \
    -I hyperliquid-c/include \
    -L hyperliquid-c/build/lib -lhyperliquid \
    -lsecp256k1 -lmsgpackc -lcurl -lcjson -lssl -lcrypto

# Run
./my_first_trade
```

**Section sources**
- [QUICKSTART.md](file://QUICKSTART.md#L150-L180)

## Common Pitfalls and Troubleshooting

### "Failed to create client"
- Check wallet address format (must include `0x` prefix)
- Verify private key is 64 hex characters (no `0x` prefix)
- Ensure environment variables are set

### "Connection failed"
- Test internet connection
- Verify API is accessible: `curl https://api.hyperliquid-testnet.xyz/info`
- Check firewall settings

### "Order rejected"
- Insufficient balance (check with `hl_get_balance`)
- Price too far from market (within 80% of market price)
- Minimum order size: $10 USDC equivalent

### "Signature verification failed"
- Ensure `libsecp256k1` is properly installed
- Verify private key matches wallet address
- Check if using correct testnet/mainnet

**Section sources**
- [QUICKSTART.md](file://QUICKSTART.md#L220-L260)