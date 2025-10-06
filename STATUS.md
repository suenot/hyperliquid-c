# Hyperliquid C SDK - Implementation Status

🚀 **New standalone library based on production-tested code from listing-v3-c**

## Overview

This SDK is being extracted from the working Hyperliquid integration in `listing-v3-c` project, which successfully:
- ✅ Places orders on Hyperliquid testnet
- ✅ Cancels orders
- ✅ Retrieves balances, positions, trade history
- ✅ Implements correct EIP-712 signing with libsecp256k1
- ✅ Uses byte-perfect MessagePack serialization (validated against Go/Rust/Python SDKs)

## Phase 1: Foundation (CURRENT)

### ✅ Completed

1. **Project Structure**
   - [x] Directory layout (`src/`, `include/`, `tests/`, `examples/`, `docs/`)
   - [x] Build system (Makefile with targets: all, test, clean, install)
   - [x] Documentation (README, QUICKSTART, ARCHITECTURE, LICENSE)
   - [x] `.gitignore` and version control setup

2. **Public API Design**
   - [x] Main header (`include/hyperliquid.h`)
   - [x] Type definitions (enums, structs, error codes)
   - [x] Function signatures (client, trading, account, market data)
   - [x] Documentation (Doxygen comments)

3. **Example Code**
   - [x] Simple trading example (`examples/simple_trade.c`)
   - [x] Demonstrates full order lifecycle

### 🔄 In Progress

4. **Core Implementation** (TO BE COPIED from listing-v3-c)
   
   **Source Files to Extract:**
   ```
   From listing-v3-c/src/exchanges/hyperliquid_trader.c:
   → hyperliquid-c/src/trading.c       # Order placement/cancellation
   → hyperliquid-c/src/account.c       # Balance/positions/history
   → hyperliquid-c/src/market_data.c   # Market prices/tickers
   
   From listing-v3-c/src/utils/crypto_utils.c:
   → hyperliquid-c/src/crypto/eip712.c      # EIP-712 signing
   → hyperliquid-c/src/crypto/signature.c   # ECDSA with libsecp256k1
   
   From listing-v3-c/src/utils/sha3.c:
   → hyperliquid-c/src/crypto/keccak.c      # Keccak-256 hashing
   
   From listing-v3-c/src/utils/hyperliquid_action.c:
   → hyperliquid-c/src/msgpack/serialize.c   # MessagePack serialization
   → hyperliquid-c/src/msgpack/action_hash.c # Action hash computation
   
   From listing-v3-c/src/utils/http_client.c:
   → hyperliquid-c/src/http/client.c        # HTTP client
   → hyperliquid-c/src/http/endpoints.c     # API endpoints
   ```

   **Headers to Extract:**
   ```
   From listing-v3-c/include/:
   → hyperliquid-c/include/hl_types.h       # Internal types
   → hyperliquid-c/include/hl_crypto.h      # Crypto utilities
   → hyperliquid-c/include/hl_msgpack.h     # MessagePack utilities
   → hyperliquid-c/include/hl_http.h        # HTTP utilities
   ```

### 📋 Next Steps (Phase 1 Completion)

1. **Copy Working Code** (Priority 1)
   - Extract proven crypto implementation
   - Extract MessagePack serialization (with correct field ordering!)
   - Extract HTTP client and endpoint handling
   - Extract trading/account/market functions
   - Adapt to new API surface (hl_* functions)

2. **Client Implementation** (`src/client.c`)
   ```c
   struct hl_client {
       char wallet_address[43];      // 0x + 40 hex
       char private_key[65];          // 64 hex + null
       bool testnet;
       void *http_handle;             // libcurl handle
       uint32_t timeout_ms;
       pthread_mutex_t mutex;         // Thread safety
   };
   
   hl_client_t* hl_client_create(...) { /* ... */ }
   void hl_client_destroy(...) { /* ... */ }
   bool hl_test_connection(...) { /* ... */ }
   ```

3. **Unit Tests** (`tests/`)
   - [x] test_crypto.c (copy from listing-v3-c)
   - [x] test_msgpack.c (copy from listing-v3-c)
   - [ ] test_client.c (new, basic client lifecycle)
   - [ ] test_trading.c (mock HTTP responses)
   - [ ] test_account.c (mock HTTP responses)

4. **Integration Tests** (`tests/`)
   - [ ] test_integration.c (live testnet API calls)
   - [ ] Reuse test patterns from listing-v3-c

## Phase 2: Polish & Documentation

### 📝 Documentation
- [ ] Complete API reference in README
- [ ] Add more examples:
  - [ ] Market making bot
  - [ ] Portfolio tracker
  - [ ] Multi-symbol trading
- [ ] Performance benchmarks
- [ ] Deployment guide

### 🧪 Testing
- [ ] Achieve 90%+ code coverage
- [ ] Stress tests (high-frequency orders)
- [ ] Memory leak detection (valgrind)
- [ ] Thread safety tests (helgrind)

### 🚀 Features
- [ ] WebSocket support (real-time data)
- [ ] Advanced order types (stop-loss, take-profit)
- [ ] Batch operations (place/cancel multiple orders)
- [ ] Rate limiting and retry logic

## Phase 3: Production Hardening

### 🔒 Security
- [ ] Secure private key storage (memory locking)
- [ ] Audit crypto implementation
- [ ] Penetration testing
- [ ] Security documentation

### ⚡ Performance
- [ ] Connection pooling
- [ ] Request batching
- [ ] Zero-copy optimizations
- [ ] SIMD for crypto operations

### 📦 Packaging
- [ ] Create releases (GitHub)
- [ ] Publish to package managers (vcpkg, conan)
- [ ] Docker images
- [ ] Pre-built binaries

## Critical Implementation Notes

### MessagePack Field Ordering (MUST PRESERVE!)

From our debugging in listing-v3-c, we learned that Hyperliquid API requires **exact** field ordering:

**Order object:** `a → b → p → s → r → t`
```c
// NOT alphabetical! This is insertion order from reference SDKs
msgpack_pack_map(pk, 6);
pack_field(pk, "a", asset_id);     // 1st
pack_field(pk, "b", is_buy);       // 2nd
pack_field(pk, "p", price_str);    // 3rd
pack_field(pk, "s", size_str);     // 4th ← CRITICAL: "s" before "r"!
pack_field(pk, "r", reduce_only);  // 5th ← CRITICAL: "r" after "s"!
pack_field(pk, "t", order_type);   // 6th
```

**Order action:** `type → orders → grouping`
```c
msgpack_pack_map(pk, 3);
pack_field(pk, "type", "order");   // 1st ← CRITICAL: type first!
pack_field(pk, "orders", orders);  // 2nd
pack_field(pk, "grouping", "na");  // 3rd
```

**Cancel action:** `type → cancels`
```c
msgpack_pack_map(pk, 2);
pack_field(pk, "type", "cancel");  // 1st ← CRITICAL: type first!
pack_field(pk, "cancels", cancels);// 2nd
```

**Why this matters:**
- Changing order causes different MessagePack bytes
- Different bytes → different action hash (connection_id)
- Different hash → different EIP-712 signature
- Different signature → API rejects with "User or API Wallet does not exist"

This was discovered after extensive debugging and comparison with Go/Rust/Python SDKs.

### EIP-712 Signing (MUST USE libsecp256k1!)

**Working implementation** (from listing-v3-c):
```c
// 1. Compute signing hash
uint8_t domain_hash[32];
eip712_domain_hash("Exchange", 1337, domain_hash);

uint8_t agent_struct_hash[32];
eip712_agent_struct_hash(source, connection_id, agent_struct_hash);

uint8_t signing_hash[32];
eip712_signing_hash(domain_hash, agent_struct_hash, signing_hash);

// 2. Sign with libsecp256k1
secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
secp256k1_ecdsa_recoverable_signature sig;
secp256k1_ecdsa_sign_recoverable(ctx, &sig, signing_hash, priv_key, NULL, NULL);

// 3. Get correct recovery ID by comparing addresses
// (see listing-v3-c/src/utils/crypto_utils.c for full implementation)
```

**Critical:** The recovery ID (v value) MUST be calculated correctly by:
1. Deriving public key from private key
2. Computing expected Ethereum address
3. Trying all 4 recovery IDs (0-3)
4. Finding which one recovers to the correct address
5. Setting v = recovery_id + 27

## Known Working Configuration

From listing-v3-c integration tests:
- ✅ Orders placed successfully on testnet
- ✅ Orders canceled successfully on testnet
- ✅ Balance retrieval works
- ✅ Position queries work
- ✅ Trade history works
- ✅ EIP-712 signatures verified correct (matches Go SDK)
- ✅ MessagePack bytes identical to Go SDK
- ✅ Connection ID identical to Go SDK (for same nonce)

**Test Environment:**
- Platform: macOS (M1)
- Dependencies: libsecp256k1, msgpack-c, curl, cjson, openssl
- Testnet API: https://api.hyperliquid-testnet.xyz
- Validated against: Go SDK, Rust SDK, CCXT Python

## Timeline

- **Week 1** (Current): Project setup, API design ✅
- **Week 2**: Copy working code, adapt to new API
- **Week 3**: Unit tests, integration tests
- **Week 4**: Documentation, examples, polish
- **Week 5**: Security audit, performance testing
- **Week 6**: Release v1.0.0

## Success Criteria

1. ✅ Project structure complete
2. ✅ Public API defined
3. ✅ Documentation written
4. ⏳ All functions implemented (copy from listing-v3-c)
5. ⏳ Tests passing (90%+ coverage)
6. ⏳ Examples working
7. ⏳ Integration tests on testnet passing
8. ⏳ Ready for production use

## Resources

- **Source Code**: `/Users/suenot/projects/listing-v3/listing-v3-c/`
- **Reference SDKs**: `/Users/suenot/projects/listing-v3/listing-v3-c/hyperliqui/`
  - Go SDK: `go-hyperliquid/`
  - Rust SDK: `hyperliquid-rust-sdk/`
  - C++ SDK: `hyperliquid-cpp/`

## Contact

Questions? Issues? Open a GitHub issue or discussion!

---

**Status:** 🟢 Phase 4 Active (Day 1 COMPLETED)

**Last Updated:** 2025-10-06

**Overall Progress:** 25/51 methods (49.0%)

**Phase 4 Progress:**
- ✅ **Day 1-2: fetch_balance** - COMPLETED
- ✅ **Day 2-3: fetch_positions** - COMPLETED
- ✅ **Day 4-5: fetch_markets** - COMPLETED (CRITICAL!)

**Phase 4: 100% ✅ COMPLETED**

**Phase 5 Progress:**
- ✅ **Day 6: fetch_ticker** - COMPLETED
- ✅ **Day 7: fetch_order_book** - COMPLETED
- ✅ **Days 8-10: fetch_ohlcv** - COMPLETED

**Phase 5: 100% ✅ COMPLETED**

**Phase 6 Progress:**
- ✅ **Trading functions** - place/cancel orders implemented and tested
- ✅ **Basic integration tests** - limit orders, error handling, lifecycle
- ✅ **Multi-symbol support** - BTC, ETH, SOL, DOGE
- ✅ **Comprehensive test suite** - test_trading_comprehensive added

**Phase 6: 100% ✅ COMPLETED**

**Phase 7: CCXT-Compatible Architecture ✅ COMPLETED**

**CCXT Compatibility Implementation:**
- ✅ Exchange describe() function - CCXT compatible configuration
- ✅ Capabilities (has) structure - 51+ method support mapping
- ✅ URL configurations - mainnet/testnet, REST/WebSocket
- ✅ Timeframes support - all standard intervals
- ✅ Exchange properties - certified, pro, dex flags
- ✅ CCXT compatibility test suite - validates all features

**Phase 8: Base API Completion ✅ COMPLETED**

**New API Methods Implemented:**
- ✅ `hl_fetch_open_orders()` - get user's open orders
- ✅ `hl_fetch_closed_orders()` - get user's closed orders (historicalOrders)
- ✅ `hl_fetch_order()` - get specific order by ID
- ✅ `hl_fetch_my_trades()` - get user's trade history
- ✅ `hl_fetch_trades()` - get public trades for symbol
- ✅ `hl_fetch_tickers()` - get multiple ticker prices
- ✅ `hl_set_leverage()` - set leverage (framework ready)
- ✅ CCXT-compatible data structures (hl_order_t, hl_trade_t, etc.)
- ✅ Memory management functions (hl_free_orders, hl_free_trades)
- ✅ Extended test suite for new methods

**Phase 9: Extended API Methods ✅ COMPLETED**

**Additional CCXT Methods Implemented:**
- ✅ `fetch_currencies` - currency metadata (framework)
- ✅ `fetch_funding_rates` - funding rates for all symbols (framework)
- ✅ Advanced order management (fetch_closed_orders fully working)
- ✅ Trade history (fetch_my_trades, fetch_trades)
- ✅ Market data extensions (fetch_tickers)
- ✅ CCXT compatibility testing (5/5 tests passed)

**Current Implementation Status:**
- **Methods:** 25/51 (49.0%)
- **Core Trading:** ✅ Complete (create/cancel orders)
- **Order Management:** ✅ Complete (open/closed orders)
- **Market Data:** ✅ Complete (tickers, orderbook, OHLCV)
- **Account:** ✅ Complete (balance, positions, trades)
- **CCXT Compatibility:** ✅ 100% (describe, has, URLs, timeframes)

**Next Milestone:** Phase 10 - WebSocket/Pro API Implementation (watch_ticker, watch_orders, etc.)

