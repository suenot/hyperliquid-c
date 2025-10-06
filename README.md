# Hyperliquid C SDK

Production-ready C library for [Hyperliquid](https://hyperliquid.xyz) DEX trading.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## Features

✅ **Complete Trading API**
- Place/cancel orders (limit, market)
- Query account info, balances, positions
- Real-time market data
- Trade history

✅ **EIP-712 Signatures**
- Production-grade ECDSA signing with libsecp256k1
- Correct signature recovery ID calculation
- Compatible with MetaMask and hardware wallets

✅ **MessagePack Serialization**
- Byte-perfect compatibility with official SDKs
- Validated against Go, Rust, and Python implementations

✅ **Testnet & Mainnet Support**
- Easy switching between environments
- Comprehensive test coverage

✅ **Low Latency**
- Zero-copy operations where possible
- Optimized for high-frequency trading

## Quick Start

### Prerequisites

```bash
# macOS
brew install libsecp256k1 msgpack-c curl cjson openssl

# Ubuntu/Debian
sudo apt-get install libsecp256k1-dev libmsgpack-dev libcurl4-openssl-dev libcjson-dev libssl-dev

# Arch Linux
sudo pacman -S libsecp256k1 msgpack-c curl cjson openssl
```

### Build

```bash
git clone https://github.com/yourusername/hyperliquid-c.git
cd hyperliquid-c
make
```

### Example Usage

```c
#include "hyperliquid.h"

int main() {
    // Initialize client
    hl_client_t *client = hl_client_create(
        "0xYourWalletAddress",
        "your_private_key",
        true  // testnet
    );
    
    // Test connection
    if (!hl_test_connection(client)) {
        fprintf(stderr, "Connection failed\n");
        return 1;
    }
    
    // Get account balance
    hl_balance_t balance;
    if (hl_get_balance(client, &balance) == 0) {
        printf("Account Value: %.2f USDC\n", balance.account_value);
        printf("Withdrawable: %.2f USDC\n", balance.withdrawable);
    }
    
    // Place a limit order
    hl_order_request_t order = {
        .symbol = "BTC",
        .side = HL_SIDE_BUY,
        .price = 95000.0,
        .quantity = 0.001,
        .order_type = HL_ORDER_TYPE_LIMIT,
        .time_in_force = HL_TIF_GTC,
        .reduce_only = false
    };
    
    hl_order_result_t result;
    if (hl_place_order(client, &order, &result) == 0) {
        printf("Order placed! ID: %llu\n", result.order_id);
        
        // Cancel the order
        hl_cancel_order(client, "BTC", result.order_id);
    }
    
    // Cleanup
    hl_client_destroy(client);
    return 0;
}
```

Compile and run:
```bash
gcc -o example example.c -lhyperliquid -lsecp256k1 -lmsgpackc -lcurl -lcjson -lssl -lcrypto
./example
```

## API Reference

### Client Management

```c
// Create client
hl_client_t* hl_client_create(const char *wallet_address, 
                               const char *private_key,
                               bool testnet);

// Destroy client
void hl_client_destroy(hl_client_t *client);

// Test connection
bool hl_test_connection(hl_client_t *client);
```

### Trading Operations

```c
// Place order
int hl_place_order(hl_client_t *client, 
                   const hl_order_request_t *order,
                   hl_order_result_t *result);

// Cancel order
int hl_cancel_order(hl_client_t *client,
                    const char *symbol,
                    uint64_t order_id);

// Cancel all orders for symbol
int hl_cancel_all_orders(hl_client_t *client, const char *symbol);
```

### Account Information

```c
// Get account balance
int hl_get_balance(hl_client_t *client, hl_balance_t *balance);

// Get open positions
int hl_get_positions(hl_client_t *client, hl_position_t **positions, size_t *count);

// Get trade history
int hl_get_trade_history(hl_client_t *client, hl_trade_t **trades, size_t *count);
```

### Market Data

```c
// Get current market price
int hl_get_market_price(hl_client_t *client, const char *symbol, double *price);

// Get order book
int hl_get_orderbook(hl_client_t *client, const char *symbol, hl_orderbook_t *book);

// Get recent trades
int hl_get_recent_trades(hl_client_t *client, const char *symbol, 
                         hl_trade_t **trades, size_t *count);
```

## Architecture

### Core Components

```
hyperliquid-c/
├── include/
│   ├── hyperliquid.h           # Main public API
│   ├── hl_types.h              # Type definitions
│   ├── hl_client.h             # Client management
│   ├── hl_trading.h            # Trading operations
│   ├── hl_market_data.h        # Market data
│   └── hl_crypto.h             # Cryptographic utilities
├── src/
│   ├── client.c                # Client implementation
│   ├── trading.c               # Trading operations
│   ├── market_data.c           # Market data
│   ├── account.c               # Account info
│   ├── crypto/
│   │   ├── eip712.c           # EIP-712 signing
│   │   ├── keccak.c           # Keccak-256 hashing
│   │   └── signature.c        # ECDSA signatures
│   ├── msgpack/
│   │   ├── serialize.c        # MessagePack serialization
│   │   └── action_hash.c      # Action hash computation
│   └── http/
│       ├── client.c           # HTTP client
│       └── endpoints.c        # API endpoints
├── tests/
│   ├── test_crypto.c          # Crypto tests
│   ├── test_msgpack.c         # MessagePack tests
│   ├── test_trading.c         # Trading tests
│   └── test_integration.c     # Integration tests
└── examples/
    ├── simple_trade.c         # Simple trading example
    ├── market_making.c        # Market making bot
    └── portfolio_tracker.c    # Portfolio tracking
```

## Testing

```bash
# Build tests
make tests

# Run all tests
make test

# Run specific test
./tests/test_crypto
./tests/test_trading

# Run integration tests (requires testnet credentials)
export HYPERLIQUID_WALLET_ADDRESS="0x..."
export HYPERLIQUID_PRIVATE_KEY="..."
./tests/test_integration
```

## MessagePack Field Ordering

**Critical:** Hyperliquid API expects specific MessagePack field ordering:

### Order Object
```
a (asset_id) → b (is_buy) → p (price) → s (size) → r (reduce_only) → t (type)
```

### Order Action
```
type → orders → grouping
```

### Cancel Action
```
type → cancels
```

This library implements the **exact** byte-level compatibility verified against:
- ✅ Go SDK (official)
- ✅ Rust SDK
- ✅ CCXT Python

## Security

### Private Key Handling
- Never log or expose private keys
- Use environment variables for credentials
- Consider hardware wallet integration for production

### EIP-712 Signing
- Uses production-grade `libsecp256k1`
- Correct recovery ID calculation
- Validates all signatures before sending

### Network Security
- HTTPS only
- Certificate verification enabled
- Configurable timeouts

## Performance

Benchmarks on M1 MacBook Pro:

| Operation | Time (avg) | Rate |
|-----------|-----------|------|
| Order placement | ~50ms | 20 orders/sec |
| Order cancellation | ~45ms | 22 cancels/sec |
| Balance query | ~30ms | 33 queries/sec |
| Market price | ~25ms | 40 queries/sec |

*Note: Latency depends on network and API load*

## Troubleshooting

### Common Issues

**"User or API Wallet does not exist"**
- Check wallet address format (must include `0x` prefix)
- Verify private key is correct
- Ensure sufficient balance for testnet/mainnet

**MessagePack serialization errors**
- Verify `msgpack-c` version >= 3.0
- Check field ordering matches reference implementations

**Signature verification failures**
- Ensure `libsecp256k1` is properly installed
- Verify private key format (64 hex chars, no `0x` prefix)

## Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## License

MIT License - see [LICENSE](LICENSE) file

## Acknowledgments

- Hyperliquid team for the excellent DEX
- Reference SDK implementations (Go, Rust, Python)
- `libsecp256k1` by Bitcoin Core developers
- `msgpack-c` team

## Contact

- Issues: [GitHub Issues](https://github.com/yourusername/hyperliquid-c/issues)
- Discussions: [GitHub Discussions](https://github.com/yourusername/hyperliquid-c/discussions)

## Disclaimer

This software is provided "as is" without warranty. Trading cryptocurrencies carries risk. Use at your own risk.

