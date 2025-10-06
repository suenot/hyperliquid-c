# Testing

<cite>
**Referenced Files in This Document**   
- [tests/README.md](file://tests/README.md)
- [Makefile](file://Makefile)
- [CONTRIBUTING.md](file://CONTRIBUTING.md)
- [tests/helpers/test_common.c](file://tests/helpers/test_common.c)
- [tests/unit/test_crypto_msgpack.c](file://tests/unit/test_crypto_msgpack.c)
- [tests/integration/02_market_data/test_fetch_markets.c](file://tests/integration/02_market_data/test_fetch_markets.c)
- [tests/integration/03_account/test_fetch_balance.c](file://tests/integration/03_account/test_fetch_balance.c)
- [tests/integration/04_trading/test_create_cancel_order.c](file://tests/integration/04_trading/test_create_cancel_order.c)
- [tests/integration/04_trading/test_trading_comprehensive.c](file://tests/integration/04_trading/test_trading_comprehensive.c)
- [tests/integration/08_websocket/test_websocket.c](file://tests/integration/08_websocket/test_websocket.c)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Test Organization](#test-organization)
3. [Running Tests](#running-tests)
4. [Testing Methodology](#testing-methodology)
5. [Writing New Tests](#writing-new-tests)
6. [Debugging Failed Tests](#debugging-failed-tests)
7. [Test Requirements for Pull Requests](#test-requirements-for-pull-requests)
8. [Test Dependencies and Environment Setup](#test-dependencies-and-environment-setup)
9. [Best Practices](#best-practices)

## Introduction

The Hyperliquid C SDK employs a comprehensive testing framework designed to ensure reliability, correctness, and robustness of its API integration and core cryptographic functionality. The test suite is structured into unit and integration tests, with a clear roadmap toward 100% coverage of all 51 API methods. This document details the organization, execution, and best practices for contributing to and maintaining the test suite.

**Section sources**
- [tests/README.md](file://tests/README.md)

## Test Organization

The test suite is organized under the `tests/` directory with a hierarchical structure that separates unit tests from integration tests and groups related functionality.

### Unit Tests
Located in `tests/unit/`, these tests validate core modules without requiring network connectivity:
- `test_crypto.c`: Keccak-256 hashing and EIP-712 signing
- `test_msgpack.c`: MessagePack serialization
- `test_http.c`: HTTP client functionality
- `test_client.c`: Client creation and configuration

### Integration Tests
Located in `tests/integration/`, these tests are grouped by API category and executed against the Hyperliquid testnet:
- `01_connection/`: Connection verification
- `02_market_data/`: Market data retrieval (markets, tickers, order books, OHLCV)
- `03_account/`: Account balance and position queries
- `04_trading/`: Order placement, cancellation, and management
- `05_margin/`: Leverage and margin mode operations
- `06_transfers/`: Fund transfers and withdrawals
- `07_vault/`: Vault creation and builder fee approval
- `08_advanced/`: Advanced features including funding history and request weight reservation

### Helper and Runner Modules
- `tests/helpers/`: Contains shared utilities including `test_common.h` and `test_common.c` for client creation, assertions, and environment loading
- `tests/runner/`: Executables to run specific test suites (all, unit, integration, priority)

**Section sources**
- [tests/README.md](file://tests/README.md)

## Running Tests

The Makefile provides several targets for executing tests, allowing developers to run specific subsets based on their needs.

### Available Makefile Targets
```bash
# Run all tests
make test

# Run only unit tests
make test-unit

# Run only integration tests
make test-integration

# Run high-priority tests
make test-priority

# Run tests for a specific category
make test-market-data
make test-trading
make test-account

# Run a specific test
make test-fetch-balance
```

The `test` target compiles all test executables and runs them sequentially, providing comprehensive output for each test suite. Integration tests require valid testnet credentials in a `.env` file or environment variables.

**Section sources**
- [Makefile](file://Makefile)
- [tests/README.md](file://tests/README.md)

## Testing Methodology

The testing approach combines unit testing for isolated components with integration testing against the live testnet to validate end-to-end functionality.

### Test Coverage Goals
The project aims for 100% coverage of all 51 API methods, organized into phases:
- **Phase 1-3**: Foundation and core trading functionality (completed)
- **Phase 4**: High-priority methods (20 methods)
- **Phase 5**: Medium-priority methods (18 methods)
- **Phase 6**: Low-priority and advanced methods (13 methods)

As of the latest update, 7 of 51 methods are covered (13.7%), with ongoing development to achieve full coverage.

### Assertion Patterns
Tests use a consistent assertion pattern through helper functions in `test_common.h`:
- `test_assert(condition, message)`: General boolean assertion
- `test_assert_success(err, context)`: Verify success error code
- `test_assert_not_null(ptr, context)`: Check for null pointers
- `test_assert_equals_int()`, `test_assert_equals_str()`: Value comparison
- `test_assert_greater()`: Threshold validation

These assertions provide clear failure messages and terminate the test on failure to prevent cascading errors.

**Section sources**
- [tests/README.md](file://tests/README.md)
- [tests/helpers/test_common.c](file://tests/helpers/test_common.c)

## Writing New Tests

New tests should follow established conventions and patterns to maintain consistency across the codebase.

### Naming Convention
- Unit tests: `test_<module>_<function>` (e.g., `test_crypto_keccak256`)
- Integration tests: `test_<action>_<resource>` (e.g., `test_fetch_balance`)

### Test Structure
Integration tests follow a standard Arrange-Act-Assert pattern:
1. **Setup**: Create client using `test_create_client(true)` for testnet
2. **Execute**: Call the API function under test
3. **Verify**: Use assertion functions to validate results
4. **Cleanup**: Destroy client with `test_destroy_client()`

Each test file typically contains multiple test functions grouped in a suite using `test_run_suite()`.

### Example Test Template
```c
bool test_fetch_balance_perpetual(void) {
    // Setup
    hl_client_t* client = test_create_client(true);
    test_assert_not_null(client, "Client creation");
    
    // Execute
    hl_balance_t balance = {0};
    hl_error_t err = hl_fetch_balance(client, HL_ACCOUNT_PERPETUAL, &balance);
    
    // Verify
    test_assert_success(err, "fetch_balance");
    test_assert(balance.total_usdc > 0, "Balance has USDC");
    
    // Cleanup
    test_destroy_client(client);
    
    return true;
}
```

**Section sources**
- [tests/README.md](file://tests/README.md)
- [tests/integration/03_account/test_fetch_balance.c](file://tests/integration/03_account/test_fetch_balance.c)

## Debugging Failed Tests

When tests fail, several tools and techniques are available for diagnosis.

### Environment Setup
Ensure testnet credentials are properly configured in `.env`:
```
HYPERLIQUID_TESTNET_WALLET_ADDRESS=your_wallet
HYPERLIQUID_TESTNET_PRIVATE_KEY=your_private_key
```

The `test_load_env()` function automatically loads these variables.

### Debugging Tools
The Makefile includes several diagnostic targets:
```bash
# Build with debug symbols and sanitizers
make debug test

# Run memory checks with valgrind
make memcheck

# Generate code coverage report
make coverage
```

Failed assertions provide detailed error messages including the context and expected vs. actual values, helping to quickly identify the source of failures.

**Section sources**
- [Makefile](file://Makefile)
- [tests/helpers/test_common.c](file://tests/helpers/test_common.c)

## Test Requirements for Pull Requests

Contributions must adhere to the testing standards outlined in CONTRIBUTING.md.

### Required Test Coverage
- All new features must include corresponding unit and integration tests
- Bug fixes must include regression tests
- Test coverage should not decrease

### Code Review Requirements
Pull requests are evaluated on:
- Correctness of test implementation
- Adequate coverage of success and error cases
- Proper cleanup of resources
- Adherence to naming and structural conventions
- Performance considerations (e.g., rate limiting)

Automated CI/CD runs unit tests on every push and integration tests on pull requests, requiring all tests to pass before merging.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md)

## Test Dependencies and Environment Setup

The test suite requires several external dependencies and proper environment configuration.

### Dependencies
Install required libraries:
```bash
# macOS
brew install libsecp256k1 msgpack-c curl cjson openssl

# Ubuntu/Debian
sudo apt-get install libsecp256k1-dev libmsgpack-dev libcurl4-openssl-dev libcjson-dev libssl-dev
```

### Environment Variables
Integration tests require testnet credentials:
- `HYPERLIQUID_TESTNET_WALLET_ADDRESS`
- `HYPERLIQUID_TESTNET_PRIVATE_KEY`

These can be provided via `.env` file or directly in the environment. The test helpers automatically load these values.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md)
- [tests/helpers/test_common.c](file://tests/helpers/test_common.c)

## Best Practices

Follow these best practices when writing and maintaining tests.

### General Guidelines
- One test file per API method
- Test both success and error cases
- Use small amounts on testnet to avoid significant financial impact
- Clean up after tests (cancel orders, close positions)
- Test with realistic data
- Include rate limit handling
- Test thread safety where applicable

### Edge Case Testing
Specific attention should be paid to:
- Invalid parameters and error handling
- Network timeouts and connectivity issues
- Rate limiting scenarios
- Concurrent access to shared resources
- Memory management and resource cleanup

### Thread Safety
The client implementation uses mutexes to protect shared state. Tests should verify thread safety by simulating concurrent operations, particularly for order placement and cancellation scenarios.

**Section sources**
- [tests/README.md](file://tests/README.md)
- [CONTRIBUTING.md](file://CONTRIBUTING.md)