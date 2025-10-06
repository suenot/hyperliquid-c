# Hyperliquid C SDK - Architecture

## Design Principles

1. **Zero-Copy Operations**: Minimize memory allocations and copies
2. **Thread-Safety**: All public APIs are thread-safe
3. **Error Handling**: Explicit error codes, no exceptions
4. **Memory Management**: Clear ownership semantics
5. **Testability**: 100% unit test coverage target
6. **Performance**: Low latency, high throughput
7. **Security**: Secure private key handling, no logging of secrets

## Module Structure

```
src/
├── client.c              # Client lifecycle and configuration
├── trading.c             # Order placement, cancellation, modification
├── account.c             # Balance, positions, trade history
├── market_data.c         # Prices, tickers, order books
├── crypto/
│   ├── eip712.c         # EIP-712 signing (libsecp256k1)
│   ├── keccak.c         # Keccak-256 hashing (SHA3IUF)
│   └── signature.c      # ECDSA signature generation
├── msgpack/
│   ├── serialize.c      # MessagePack serialization
│   └── action_hash.c    # Action hash computation
└── http/
    ├── client.c         # HTTP client (libcurl)
    └── endpoints.c      # API endpoint definitions
```

## Data Flow

### Order Placement Flow

```
hl_place_order()
    ↓
[1] Validate parameters
    ↓
[2] Build order struct (hl_order_t)
    ↓
[3] Serialize to MessagePack (msgpack/serialize.c)
    ↓
[4] Compute action hash (msgpack/action_hash.c)
    ↓
[5] Sign with EIP-712 (crypto/eip712.c)
    ↓
[6] Build JSON request
    ↓
[7] Send HTTP POST (http/client.c)
    ↓
[8] Parse JSON response
    ↓
[9] Return result
```

## Key Components

### 1. Client (`hl_client_t`)

Opaque handle containing:
- Wallet address (Ethereum format)
- Private key (securely stored, never logged)
- Testnet/mainnet flag
- HTTP client handle
- Configuration (timeouts, retries, etc.)

### 2. Crypto Module

**EIP-712 Signing:**
- Uses `libsecp256k1` for ECDSA
- Correct recovery ID calculation (v value)
- Domain separator for Hyperliquid
- Agent struct hashing

**Keccak-256:**
- Uses SHA3IUF library
- Ethereum-style Keccak (not NIST SHA3)
- Used for address derivation and hashing

### 3. MessagePack Module

**Critical Field Ordering:**

Order object: `a → b → p → s → r → t`
- a: asset_id (uint32)
- b: is_buy (bool)
- p: price (string)
- s: size (string)
- r: reduce_only (bool)
- t: order type (map with "limit")

Order action: `type → orders → grouping`
- type: "order" (string)
- orders: array of order objects
- grouping: "na" (string)

Cancel action: `type → cancels`
- type: "cancel" (string)
- cancels: array of {a, o}

**Why order matters:**
Hyperliquid API expects byte-perfect MessagePack matching Go/Rust/Python SDKs.
Dictionary insertion order is preserved in serialization.

### 4. HTTP Module

**Endpoints:**
- POST `/exchange` - Trading operations (orders, cancels)
- POST `/info` - Account info, market data

**Features:**
- HTTPS only
- Configurable timeouts
- Automatic retries (exponential backoff)
- Connection pooling
- Error handling

## Memory Management

### Ownership Rules

1. **Client owns everything**: All internal data freed on `hl_client_destroy()`
2. **Arrays returned to user**: Must be freed with `hl_free_*()` functions
3. **No hidden allocations**: All allocations explicit in API
4. **RAII pattern**: Resource acquisition is initialization

### Example

```c
// Client owns all internal resources
hl_client_t *client = hl_client_create(...);

// User owns returned array, must free
hl_position_t *positions;
size_t count;
hl_get_positions(client, &positions, &count);
// ... use positions ...
hl_free_positions(positions);  // User responsibility

// Cleanup frees all internal resources
hl_client_destroy(client);
```

## Thread Safety

### Thread-Safe APIs (with mutex)
- `hl_place_order()`
- `hl_cancel_order()`
- `hl_get_balance()`
- All public APIs

### Not Thread-Safe
- Creating/destroying same client from multiple threads

### Best Practices
- One client per thread for high-frequency trading
- Or serialize access to single client with mutex

## Error Handling

### Error Code Convention

```c
typedef enum {
    HL_SUCCESS = 0,           // Success
    HL_ERROR_* = negative     // Errors are negative
} hl_error_t;
```

### Error Propagation

```c
int function() {
    if (error) {
        return HL_ERROR_*;  // Propagate error
    }
    return HL_SUCCESS;
}
```

### Error Checking

```c
if (hl_place_order(client, &order, &result) != HL_SUCCESS) {
    fprintf(stderr, "Error: %s\n", hl_error_string(result));
    // Handle error
}
```

## Performance Considerations

### Optimization Strategies

1. **Pre-allocate buffers**: Reuse buffers for JSON/MessagePack
2. **Connection pooling**: Keep HTTP connections alive
3. **Minimal allocations**: Stack allocation where possible
4. **Fast path**: Optimize common case (successful order)
5. **SIMD**: Future: SIMD for Keccak-256

### Benchmarks (Target)

| Operation | Latency (P50) | Latency (P99) | Throughput |
|-----------|---------------|---------------|------------|
| Order place | <50ms | <100ms | 20 orders/s |
| Order cancel | <45ms | <90ms | 22 cancels/s |
| Balance query | <30ms | <60ms | 33 queries/s |
| Market price | <25ms | <50ms | 40 queries/s |

*Network latency dominates; local processing <1ms*

## Testing Strategy

### Unit Tests
- Each function tested in isolation
- Mock HTTP responses
- 100% code coverage target

### Integration Tests
- Live testnet API calls
- Full order lifecycle
- Error scenarios

### Stress Tests
- High-frequency order placement
- Memory leak detection (valgrind)
- Thread safety (helgrind)

## Security Considerations

### Private Key Handling
1. **Never logged**: No debug prints of private keys
2. **Secure storage**: Wiped on cleanup
3. **Memory locking**: Consider mlock() for production
4. **Environment variables**: Read once, clear immediately

### API Security
1. **HTTPS only**: No plaintext HTTP
2. **Certificate verification**: libcurl SSL verification enabled
3. **Signature validation**: Double-check all signatures
4. **Rate limiting**: Respect API rate limits

## Future Enhancements

### Phase 2 (Q2 2025)
- WebSocket support for real-time data
- Advanced order types (stop-loss, take-profit)
- Multi-asset orders
- Portfolio analytics

### Phase 3 (Q3 2025)
- Hardware wallet integration (Ledger, Trezor)
- Zero-knowledge proof support
- Cross-chain bridging
- MEV protection

## References

- [Hyperliquid API Docs](https://hyperliquid.gitbook.io/)
- [EIP-712 Specification](https://eips.ethereum.org/EIPS/eip-712)
- [libsecp256k1](https://github.com/bitcoin-core/secp256k1)
- [MessagePack C](https://github.com/msgpack/msgpack-c)
