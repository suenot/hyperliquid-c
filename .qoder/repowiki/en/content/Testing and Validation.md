# Testing and Validation

<cite>
**Referenced Files in This Document**   
- [Makefile](file://Makefile)
- [tests/README.md](file://tests/README.md)
- [tests/simple_test.c](file://tests/simple_test.c)
- [tests/helpers/test_common.h](file://tests/helpers/test_common.h)
- [tests/helpers/test_common.c](file://tests/helpers/test_common.c)
- [tests/unit/test_crypto_msgpack.c](file://tests/unit/test_crypto_msgpack.c)
- [tests/integration/04_trading/test_create_cancel_order.c](file://tests/integration/04_trading/test_create_cancel_order.c)
- [scripts/generate-coverage-badge.sh](file://scripts/generate-coverage-badge.sh)
- [scripts/check-modularity.sh](file://scripts/check-modularity.sh)
</cite>

## Table of Contents
1. [Unit Test Framework](#unit-test-framework)
2. [Running Tests with Makefile](#running-tests-with-makefile)
3. [Test Coverage Metrics and Reporting](#test-coverage-metrics-and-reporting)
4. [Testing Network-Dependent Functionality](#testing-network-dependent-functionality)
5. [Writing New Tests](#writing-new-tests)
6. [Integration Testing with Hyperliquid API](#integration-testing-with-hyperliquid-api)
7. [Performance and Cryptographic Testing](#performance-and-cryptographic-testing)

## Unit Test Framework

The hyperliquid-c library implements a comprehensive test framework structured into unit and integration tests. Unit tests validate core modules such as cryptographic operations, MessagePack serialization, HTTP client functionality, and client initialization without requiring network connectivity. These tests are located in the `tests/unit/` directory and include dedicated test files for crypto (keccak, EIP-712), msgpack serialization, HTTP client behavior, and client creation. The framework uses a simple assertion-based approach with helper functions to verify conditions, check for null pointers, validate error codes, and compare expected versus actual values. The test suite is designed to be extensible, with a clear naming convention (`test_<module>_<function>`) that ensures consistency across test cases.

**Section sources**
- [tests/README.md](file://tests/README.md#L1-L387)
- [tests/unit/test_crypto_msgpack.c](file://tests/unit/test_crypto_msgpack.c#L1-L151)

## Running Tests with Makefile

The Makefile provides a comprehensive set of targets for building and executing tests. The `make test` target compiles and runs all available tests, while more specific targets allow for selective execution: `make test-unit` runs only unit tests, `make test-integration` executes integration tests, and `make test-priority` runs high-priority test suites. Developers can also run category-specific tests using targets like `make test-market-data`, `make test-trading`, or `make test-account`. Individual tests can be executed with commands such as `make test-fetch-balance`. The Makefile automatically handles compilation of test executables, linking against the hyperliquid library and required dependencies (curl, cjson, ssl, crypto, msgpackc, secp256k1, m, pthread). Additional utility targets include `make debug` for building with debug symbols and sanitizers, `make memcheck` for running tests with valgrind, and `make analyze` for static analysis with cppcheck.

**Section sources**
- [Makefile](file://Makefile#L1-L227)

## Test Coverage Metrics and Reporting

Test coverage is measured using the `make coverage` target, which compiles the code with coverage instrumentation (`--coverage` flag), runs all tests, and generates a comprehensive coverage report using gcov, lcov, and genhtml. This produces an HTML report in the `coverage/` directory showing line-by-line coverage statistics. The repository includes the `scripts/generate-coverage-badge.sh` script, which takes a coverage percentage as input and generates a Shields.io badge URL with appropriate color coding (brightgreen for ≥90%, yellow for ≥75%, orange for ≥60%, red for <60%). The test framework tracks progress through six phases, with current coverage at 7/51 API methods (13.7%). The README.md in the tests directory maintains a detailed checklist of implemented and pending tests across priority levels, providing transparency into testing progress. The `check-modularity.sh` script also evaluates code quality by identifying source files exceeding 1000 lines, ensuring maintainable, modular code.

**Section sources**
- [Makefile](file://Makefile#L200-L212)
- [scripts/generate-coverage-badge.sh](file://scripts/generate-coverage-badge.sh#L1-L44)
- [scripts/check-modularity.sh](file://scripts/check-modularity.sh#L1-L92)
- [tests/README.md](file://tests/README.md#L1-L387)

## Testing Network-Dependent Functionality

Network-dependent functionality is tested through integration tests that run against the Hyperliquid testnet. The test framework employs mocking strategies for unit testing of network components, allowing HTTP request building and response parsing to be validated without actual network calls. For integration tests, the framework uses real API calls to the testnet environment, with credentials loaded from a `.env` file or environment variables. The `test_common.h` and `test_common.c` helpers provide functions like `test_create_client()` and `test_destroy_client()` to manage client lifecycle, along with assertion utilities that handle testnet-specific error conditions. The framework is designed to test both success and error scenarios, with careful attention to cleanup operations to prevent test state pollution. Rate limit handling and thread safety are also considered in test design.

**Section sources**
- [tests/helpers/test_common.h](file://tests/helpers/test_common.h#L1-L68)
- [tests/helpers/test_common.c](file://tests/helpers/test_common.c#L1-L329)
- [tests/README.md](file://tests/README.md#L1-L387)

## Writing New Tests

New tests should follow the established naming convention and directory structure. Unit tests should be placed in `tests/unit/` with names like `test_crypto_keccak256.c`, while integration tests belong in categorized subdirectories under `tests/integration/` (e.g., `02_market_data/`, `03_account/`). Each test file should include `test_common.h` and use the provided helper functions for setup, assertions, and teardown. Tests should validate both successful execution and proper error handling, with cleanup code to release allocated resources. The framework supports test suites through `test_run_suite()`, allowing multiple related test functions to be grouped together. Developers should implement one test file per API method, ensuring comprehensive coverage of input validation, edge cases, and expected behavior. After implementation, tests should be added to the appropriate Makefile target for automated execution.

**Section sources**
- [tests/README.md](file://tests/README.md#L1-L387)
- [tests/helpers/test_common.h](file://tests/helpers/test_common.h#L1-L68)
- [tests/helpers/test_common.c](file://tests/helpers/test_common.c#L1-L329)

## Integration Testing with Hyperliquid API

Integration testing is performed against the Hyperliquid testnet using real wallet credentials and API calls. The test framework supports both connection testing and comprehensive trading operations, including order placement, cancellation, and position management. Tests in the `tests/integration/` directory are organized by functionality, with examples including `test_fetch_balance.c`, `test_fetch_positions.c`, and `test_create_cancel_order.c`. These tests use actual API endpoints to verify the correctness of authentication, request formatting, and response parsing. The framework includes safeguards such as using small trade amounts on testnet and implementing cleanup routines to cancel orders and close positions after testing. GitHub Actions CI/CD integration automatically runs integration tests on push and pull request events, with testnet credentials securely stored as GitHub secrets. The `test_has_credentials()` function ensures tests only proceed when valid credentials are available.

**Section sources**
- [tests/integration/04_trading/test_create_cancel_order.c](file://tests/integration/04_trading/test_create_cancel_order.c#L1-L110)
- [tests/README.md](file://tests/README.md#L1-L387)
- [tests/helpers/test_common.c](file://tests/helpers/test_common.c#L1-L329)

## Performance and Cryptographic Testing

The test framework includes specific validation for cryptographic correctness, particularly for EIP-712 signing and Keccak-256 hashing, which are critical for secure trading operations. The `test_crypto_msgpack.c` unit test verifies that field ordering in MessagePack serialization matches the expected sequence (price→size→reduceOnly), ensuring compatibility with the Hyperliquid exchange. Performance testing is supported through the `make memcheck` target, which runs tests with valgrind to detect memory leaks and other memory-related issues. The framework also enables timing measurements through helper functions like `test_get_timestamp_ms()` and `test_sleep_ms()`, allowing developers to evaluate the performance of critical code paths. While dedicated performance benchmarks are not yet implemented, the existing test infrastructure provides a foundation for adding latency and throughput measurements for API calls and cryptographic operations.

**Section sources**
- [tests/unit/test_crypto_msgpack.c](file://tests/unit/test_crypto_msgpack.c#L1-L151)
- [tests/helpers/test_common.h](file://tests/helpers/test_common.h#L1-L68)
- [tests/helpers/test_common.c](file://tests/helpers/test_common.c#L1-L329)
- [Makefile](file://Makefile#L185-L198)