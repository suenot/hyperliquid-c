# Hyperliquid C SDK - Quick Start Guide

Get trading on Hyperliquid in 5 minutes! 🚀

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

## Your First Trade

### Step 1: Set Up Credentials

Get your wallet credentials from Hyperliquid:

1. Go to https://app.hyperliquid-testnet.xyz (testnet) or https://app.hyperliquid.xyz (mainnet)
2. Connect wallet (MetaMask, etc.)
3. Get API key from Settings → API
4. Note your wallet address and private key

**Set environment variables:**

```bash
export HYPERLIQUID_WALLET_ADDRESS="0xYourWalletAddress"
export HYPERLIQUID_PRIVATE_KEY="your_private_key_64_hex_chars"
export HYPERLIQUID_TESTNET="true"  # Use testnet for testing!
```

⚠️ **IMPORTANT:** Always test on testnet first! Set `HYPERLIQUID_TESTNET="true"`

### Step 2: Create Your First Program

Create `my_first_trade.c`:

```c
#include "hyperliquid.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
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
    
    // Get balance
    hl_balance_t balance;
    hl_get_balance(client, &balance);
    printf("Balance: $%.2f USDC\n", balance.account_value);
    
    // Get BTC price
    double price;
    hl_get_market_price(client, "BTC", &price);
    printf("BTC Price: $%.2f\n", price);
    
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
    
    // Cleanup
    hl_client_destroy(client);
    return 0;
}
```

### Step 3: Compile and Run

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

Output:
```
✅ Connected!
Balance: $999.00 USDC
BTC Price: $96000.00
✅ Order placed! ID: 123456789
✅ Order canceled!
```

## Common Use Cases

### 1. Market Making Bot

```c
// Place orders on both sides
hl_order_request_t bid = {
    .symbol = "BTC",
    .side = HL_SIDE_BUY,
    .price = mid_price * 0.999,  // 0.1% below mid
    .quantity = 0.001,
    .order_type = HL_ORDER_TYPE_LIMIT,
    .time_in_force = HL_TIF_GTC
};

hl_order_request_t ask = {
    .symbol = "BTC",
    .side = HL_SIDE_SELL,
    .price = mid_price * 1.001,  // 0.1% above mid
    .quantity = 0.001,
    .order_type = HL_ORDER_TYPE_LIMIT,
    .time_in_force = HL_TIF_GTC
};

hl_order_result_t bid_result, ask_result;
hl_place_order(client, &bid, &bid_result);
hl_place_order(client, &ask, &ask_result);
```

### 2. Monitor Positions

```c
hl_position_t *positions;
size_t count;

if (hl_get_positions(client, &positions, &count) == HL_SUCCESS) {
    for (size_t i = 0; i < count; i++) {
        printf("%s: %.4f @ $%.2f (PnL: $%.2f)\n",
               positions[i].symbol,
               positions[i].size,
               positions[i].entry_price,
               positions[i].unrealized_pnl);
    }
    hl_free_positions(positions);
}
```

### 3. Trade History

```c
hl_trade_t *trades;
size_t count;

if (hl_get_trade_history(client, "BTC", &trades, &count) == HL_SUCCESS) {
    printf("Last %zu trades:\n", count);
    for (size_t i = 0; i < count; i++) {
        printf("  %s %.4f @ $%.2f (fee: $%.4f)\n",
               trades[i].side == HL_SIDE_BUY ? "BUY " : "SELL",
               trades[i].quantity,
               trades[i].price,
               trades[i].fee);
    }
    hl_free_trades(trades);
}
```

## Troubleshooting

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

## Next Steps

1. **Read Examples**: Check `examples/` directory
   - `simple_trade.c` - Basic trading
   - `market_making.c` - Market maker bot
   - `portfolio_tracker.c` - Position monitoring

2. **Run Tests**: `make test`
   - Verify everything works on your system
   - Learn from integration tests

3. **Read Architecture**: See `ARCHITECTURE.md`
   - Understand design decisions
   - Learn performance tips

4. **Read API Reference**: See `README.md`
   - Complete API documentation
   - All function signatures

## Safety Checklist

Before trading real money:

- [ ] Tested extensively on testnet
- [ ] Understand order types and parameters
- [ ] Set up proper error handling
- [ ] Implement position size limits
- [ ] Add monitoring and alerts
- [ ] Secure private key storage
- [ ] Test failure scenarios
- [ ] Review all API calls
- [ ] Understand fees and costs
- [ ] Have emergency stop mechanism

## Getting Help

- **GitHub Issues**: [Report bugs](https://github.com/yourusername/hyperliquid-c/issues)
- **Discussions**: [Ask questions](https://github.com/yourusername/hyperliquid-c/discussions)
- **Documentation**: Read `README.md`, `ARCHITECTURE.md`
- **Examples**: See `examples/` directory

## Contributing

Contributions welcome! See `CONTRIBUTING.md` (coming soon).

---

**⚠️ Risk Warning**: Cryptocurrency trading carries significant risk. Never trade with funds you cannot afford to lose. This software is provided "as is" without warranty.

**🎉 Happy Trading!** Start small, test thoroughly, and scale up gradually.

