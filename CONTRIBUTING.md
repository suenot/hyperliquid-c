# Contributing to Hyperliquid C SDK

Thank you for your interest in contributing! This document provides guidelines and information for contributors.

## 🎯 Current State

**Phase 1**: ✅ Complete (Foundation)  
**Phase 2**: ✅ Complete (Core Modules Tested)  
**Phase 3**: ⏳ Optional (Full Integration - 8-12 hours)

The core cryptographic modules are **working and tested**. The remaining work is primarily API adaptation and integration.

## 🚀 How to Contribute

### Quick Start for Contributors

```bash
# Clone the repository
git clone https://github.com/suenot/hyperliquid-c.git
cd hyperliquid-c

# Install dependencies (macOS)
brew install libsecp256k1 msgpack-c curl cjson openssl

# Run tests to verify setup
make -f Makefile.test test_minimal

# You should see:
# ✅ ALL TESTS PASSED!
```

## 📋 Areas for Contribution

### Priority 1: Phase 3 Integration (Est: 8-12 hours)

#### 1.1 API Adaptation Layer (4-6 hours)

**Task**: Adapt `src/trading.c` to use new public API signatures.

**Current State**:
- `src/trading.c` exists with full Hyperliquid implementation (812 lines)
- Uses old `listing-v3-c` function signatures
- Needs wrapper functions to match `include/hyperliquid.h`

**What Needs to Be Done**:
```c
// OLD (listing-v3-c):
lv3_error_t hyperliquid_place_order(
    exchange_trader_t* trader,
    const char* symbol,
    double price,
    double quantity,
    bool is_buy,
    char* order_id_out
);

// NEW (hyperliquid-c):
hl_error_t hl_place_order(
    hl_client_t* client,
    const char* symbol,
    double price,
    double size,
    bool is_buy,
    hl_order_result_t* result
);
```

**Files to Modify**:
- `src/trading.c` - Adapt function signatures
- `src/client.c` - Implement wrapper functions
- `include/hyperliquid.h` - Already has correct signatures

**Skills Needed**: C programming, API design

#### 1.2 Integration Tests (2-3 hours)

**Task**: Create tests that interact with Hyperliquid testnet.

**What Needs to Be Done**:
- Copy `tests/test_minimal.c` as template
- Add testnet configuration loading
- Test order placement
- Test order cancellation
- Test balance queries
- Test position queries

**Files to Create**:
- `tests/test_integration.c`
- `tests/config.test.json`

**Skills Needed**: C programming, testing

#### 1.3 Example Updates (1-2 hours)

**Task**: Update and create usage examples.

**Current State**:
- `examples/simple_trade.c` exists but uses placeholder API

**What Needs to Be Done**:
- Update `simple_trade.c` with real API calls
- Create `market_maker.c` example
- Create `portfolio_tracker.c` example

**Skills Needed**: C programming, Hyperliquid knowledge

### Priority 2: Documentation Improvements

#### 2.1 Usage Examples

Add more code examples to documentation:
- API usage snippets
- Common patterns
- Error handling examples
- Best practices

#### 2.2 Troubleshooting Guide

Document common issues:
- Compilation errors
- Linking errors
- Runtime errors
- API errors

#### 2.3 Performance Guide

Document performance considerations:
- Memory usage
- CPU usage
- Network optimization
- Rate limiting

**Skills Needed**: Technical writing

### Priority 3: Advanced Features

#### 3.1 WebSocket Support

Add real-time data streaming:
- Subscribe to price updates
- Subscribe to order updates
- Subscribe to fill updates

**Skills Needed**: C programming, WebSocket protocol

#### 3.2 Advanced Order Types

Implement additional order types:
- Stop-loss orders
- Take-profit orders
- Trailing stops
- OCO (One-Cancels-Other)

**Skills Needed**: C programming, trading knowledge

#### 3.3 Hardware Wallet Integration

Add support for hardware wallets:
- Ledger integration
- Trezor integration
- Secure key storage

**Skills Needed**: C programming, hardware integration

## 🏗️ Development Workflow

### 1. Fork and Clone

```bash
# Fork on GitHub, then:
git clone https://github.com/YOUR_USERNAME/hyperliquid-c.git
cd hyperliquid-c
git remote add upstream https://github.com/suenot/hyperliquid-c.git
```

### 2. Create Feature Branch

```bash
git checkout -b feature/your-feature-name
# or
git checkout -b fix/your-bug-fix
```

### 3. Make Changes

- Follow existing code style
- Add tests for new functionality
- Update documentation

### 4. Test Your Changes

```bash
# Run existing tests
make -f Makefile.test test_minimal

# Run your new tests
make -f Makefile.test test_your_feature

# Check for memory leaks (optional but recommended)
valgrind --leak-check=full ./build/test_your_feature
```

### 5. Commit

```bash
git add .
git commit -m "feat: add your feature description"
# or
git commit -m "fix: fix bug description"
```

**Commit Message Format**:
- `feat:` - New feature
- `fix:` - Bug fix
- `docs:` - Documentation changes
- `test:` - Test additions/changes
- `refactor:` - Code refactoring
- `perf:` - Performance improvements

### 6. Push and Create PR

```bash
git push origin feature/your-feature-name
```

Then create a Pull Request on GitHub.

## 📏 Code Style

### C Code Style

```c
// Function names: snake_case
hl_error_t hl_place_order(...);

// Struct names: snake_case with _t suffix
typedef struct hl_client hl_client_t;

// Constants: UPPER_CASE
#define HL_MAX_SYMBOL_LENGTH 16

// Variables: snake_case
int connection_timeout = 30;

// Indentation: 4 spaces (not tabs)
if (condition) {
    do_something();
}

// Braces: K&R style
if (condition) {
    // code
} else {
    // code
}

// Pointer declaration: asterisk with type
char* string;  // preferred
char *string;  // acceptable
```

### Documentation Style

```c
/**
 * @brief Place a limit order on Hyperliquid
 * 
 * @param client Initialized client instance
 * @param symbol Trading pair (e.g., "BTC")
 * @param price Limit price
 * @param size Order size
 * @param is_buy true for buy, false for sell
 * @param result Output parameter for order result
 * 
 * @return HL_SUCCESS on success, error code otherwise
 * 
 * @note This function is thread-safe
 */
hl_error_t hl_place_order(
    hl_client_t* client,
    const char* symbol,
    double price,
    double size,
    bool is_buy,
    hl_order_result_t* result
);
```

## 🧪 Testing Guidelines

### Test Structure

```c
// Test file: tests/test_feature.c

#include <stdio.h>
#include <assert.h>
#include "hyperliquid.h"

static void test_feature_basic(void) {
    // Arrange
    hl_client_t* client = hl_client_create(...);
    
    // Act
    hl_error_t err = hl_some_function(client);
    
    // Assert
    assert(err == HL_SUCCESS);
    
    // Cleanup
    hl_client_destroy(client);
    
    printf("✅ test_feature_basic PASSED\n");
}

int main(void) {
    printf("Running feature tests...\n\n");
    
    test_feature_basic();
    // Add more tests...
    
    printf("\n✅ ALL TESTS PASSED!\n");
    return 0;
}
```

### Test Coverage

Aim for:
- **Unit Tests**: Test individual functions in isolation
- **Integration Tests**: Test module interactions
- **End-to-End Tests**: Test full workflows (testnet)

## 📚 Documentation Guidelines

### README Updates

When adding features:
1. Update feature list
2. Add usage examples
3. Update API reference
4. Update roadmap

### Code Comments

```c
// Good: Explains WHY, not WHAT
// Use big-endian for network byte order (Hyperliquid API requirement)
uint64_t nonce_be = htobe64(nonce);

// Bad: States the obvious
// Convert nonce to big endian
uint64_t nonce_be = htobe64(nonce);
```

## 🔍 Code Review Process

All contributions go through code review:

1. **Automated Checks**:
   - Compilation (no warnings)
   - Tests (all passing)
   - Code style (linting)

2. **Manual Review**:
   - Code quality
   - Documentation
   - Test coverage
   - Performance impact

3. **Approval**:
   - At least one maintainer approval
   - All CI checks passing
   - No merge conflicts

## 🐛 Bug Reports

When reporting bugs, include:

1. **Description**: Clear description of the issue
2. **Reproduction**: Steps to reproduce
3. **Expected**: What should happen
4. **Actual**: What actually happens
5. **Environment**:
   - OS and version
   - Compiler and version
   - Library versions
6. **Logs**: Relevant error messages/logs

Example:
```markdown
### Description
Order placement fails with "User does not exist" error

### Reproduction
1. Create client with API wallet credentials
2. Call hl_place_order() with BTC/USD
3. Observe error

### Expected
Order should be placed successfully

### Actual
Error: "User or API Wallet 0x... does not exist"

### Environment
- macOS 14.5
- Clang 15.0.0
- libsecp256k1 0.4.0

### Logs
```
[ERROR] HTTP 400: {"error": "User does not exist"}
```
```

## 💬 Communication

- **GitHub Issues**: Bug reports, feature requests
- **GitHub Discussions**: Questions, ideas, help
- **Pull Requests**: Code contributions

## ⚖️ License

By contributing, you agree that your contributions will be licensed under the MIT License.

## 🙏 Recognition

Contributors will be:
- Listed in CONTRIBUTORS.md
- Mentioned in release notes
- Credited in relevant documentation

## 📞 Questions?

Feel free to:
- Open a GitHub Discussion
- Comment on existing issues
- Ask in your Pull Request

---

**Thank you for contributing to Hyperliquid C SDK!** 🎉

Every contribution, no matter how small, is valuable and appreciated.

