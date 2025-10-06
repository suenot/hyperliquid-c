# Contribution Guidelines

<cite>
**Referenced Files in This Document**   
- [CONTRIBUTING.md](file://CONTRIBUTING.md)
- [README.md](file://README.md)
- [ARCHITECTURE.md](file://ARCHITECTURE.md)
- [STATUS.md](file://STATUS.md)
- [PROJECT_SUMMARY.md](file://PROJECT_SUMMARY.md)
- [hyperliquid.h](file://include/hyperliquid.h)
- [client.c](file://src/client.c)
- [trading_api.c](file://src/trading_api.c)
- [account.c](file://src/account.c)
- [markets.c](file://src/markets.c)
- [ohlcv.c](file://src/ohlcv.c)
- [trades.c](file://src/trades.c)
- [orders.c](file://src/orders.c)
- [market_extended.c](file://src/market_extended.c)
- [leverage.c](file://src/leverage.c)
- [currencies.c](file://src/currencies.c)
- [funding.c](file://src/funding.c)
- [transfers.c](file://src/transfers.c)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Code of Conduct](#code-of-conduct)
3. [Issue Reporting Process](#issue-reporting-process)
4. [Pull Request Requirements](#pull-request-requirements)
5. [Code Review Workflow](#code-review-workflow)
6. [Coding Standards](#coding-standards)
7. [Testing Expectations](#testing-expectations)
8. [Documentation Updates](#documentation-updates)
9. [Release Cycle and Versioning](#release-cycle-and-versioning)
10. [Proposing New Features](#proposing-new-features)

## Introduction

Welcome to the Hyperliquid C SDK contribution guidelines! This document provides comprehensive information for contributors who want to help improve this production-ready C library for the Hyperliquid decentralized exchange. The SDK offers full REST API and WebSocket streaming capabilities with CCXT-compatible interface, featuring complete trading, market data, and account management functionality.

This guide covers everything you need to know to contribute effectively, from setting up your development environment to understanding the code review process. Whether you're fixing a bug, adding a new feature, or improving documentation, these guidelines will help ensure your contributions meet the project's high quality standards while maintaining consistency across the codebase.

The Hyperliquid C SDK has achieved 84.3% CCXT method implementation (43/51 methods), representing the most comprehensive C SDK for any cryptocurrency exchange. With a focus on performance, security, and reliability, this library is designed for high-frequency trading applications and institutional-grade use.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md)
- [README.md](file://README.md)

## Code of Conduct

All contributors are expected to adhere to a professional code of conduct that promotes a positive, inclusive, and respectful environment. This project follows the principles of open collaboration and values constructive feedback, clear communication, and mutual respect among all participants.

Contributors must:
- Treat others with respect and professionalism at all times
- Provide constructive criticism and feedback
- Accept feedback graciously and respond thoughtfully
- Avoid personal attacks, derogatory language, or inflammatory statements
- Be mindful of cultural differences in communication styles
- Focus on technical discussions rather than personal opinions

The project maintains a zero-tolerance policy for harassment, discrimination, or any behavior that creates a hostile environment. This includes but is not limited to: offensive comments related to gender, gender identity, sexual orientation, disability, mental illness, neurotype, physical appearance, body size, race, age, or religion; sexual images in public spaces; deliberate intimidation; stalking; sustained disruption of discussions; and unwelcome sexual attention.

By participating in this project, you agree to uphold these standards and help maintain a welcoming community for all contributors regardless of experience level, background, or identity.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md)

## Issue Reporting Process

When reporting issues, please provide comprehensive information to help maintainers understand and reproduce the problem. Follow this structured approach to ensure your bug reports are actionable and can be addressed efficiently.

Include the following information in your issue report:

**Description**: Provide a clear and concise description of the issue. Explain what you were trying to accomplish and what went wrong.

**Reproduction Steps**: List detailed steps to reproduce the issue:
1. Environment setup requirements
2. Code or commands used
3. Expected sequence of operations
4. Where the issue occurs

**Expected Behavior**: Describe what should have happened under normal circumstances.

**Actual Behavior**: Detail what actually occurred, including any error messages, unexpected outputs, or system states.

**Environment Information**: Include complete environment details:
- Operating system and version (e.g., macOS 14.5, Ubuntu 22.04)
- Compiler and version (e.g., Clang 15.0.0, GCC 11.4.0)
- Library versions (libsecp256k1, msgpack-c, curl, cjson, openssl)
- Hardware specifications if relevant (especially for performance issues)

**Logs and Error Messages**: Provide relevant logs, stack traces, or error messages. For API-related issues, include the complete HTTP request and response if possible, redacting any sensitive information.

Example issue report:
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

For feature requests, clearly articulate the use case, benefits, and potential implementation approaches. This helps maintainers evaluate the request against the project roadmap and technical constraints.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md)

## Pull Request Requirements

To ensure high-quality contributions, all pull requests must meet specific requirements before they can be merged into the main codebase. Follow these guidelines to increase the likelihood of your PR being accepted promptly.

### Development Workflow

1. **Fork and Clone**: Fork the repository on GitHub, then clone your fork locally:
```bash
git clone https://github.com/YOUR_USERNAME/hyperliquid-c.git
cd hyperliquid-c
git remote add upstream https://github.com/suenot/hyperliquid-c.git
```

2. **Create Feature Branch**: Use descriptive branch names with appropriate prefixes:
```bash
git checkout -b feature/your-feature-name
# or
git checkout -b fix/your-bug-fix
```

3. **Make Changes**: Implement your changes while following the project's coding standards and design principles.

4. **Test Your Changes**: Run the complete test suite to ensure your changes don't break existing functionality:
```bash
# Run existing tests
make -f Makefile.test test_minimal

# Run your new tests
make -f Makefile.test test_your_feature

# Check for memory leaks (recommended)
valgrind --leak-check=full ./build/test_your_feature
```

5. **Commit**: Use conventional commit messages that clearly describe the change:
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

6. **Push and Create PR**: Push your branch and create a pull request on GitHub:
```bash
git push origin feature/your-feature-name
```

### PR Content Requirements

Your pull request should include:
- Clear description of the changes and their purpose
- Reference to any related issues (using GitHub's issue linking syntax)
- Explanation of the implementation approach
- Any trade-offs or design decisions made
- Testing strategy and results
- Documentation updates if applicable

Ensure your PR focuses on a single concern. If you have multiple unrelated changes, submit them as separate PRs to make the review process more manageable.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md)

## Code Review Workflow

All contributions undergo a thorough code review process to maintain the project's high quality standards. Understanding this workflow will help you navigate the review process effectively.

### Automated Checks

Every pull request triggers automated checks that verify:
- **Compilation**: Code must compile without warnings using both GCC and Clang
- **Tests**: All unit and integration tests must pass
- **Code Style**: Code must adhere to the project's style guidelines (checked via automated tools)
- **Coverage**: Test coverage must not decrease significantly

### Manual Review Criteria

Maintainers evaluate contributions based on several key criteria:

**Code Quality**:
- Clear, readable code with appropriate comments explaining complex logic
- Proper error handling and edge case management
- Efficient algorithms and data structures
- Memory safety and proper resource management
- Thread safety for public APIs

**Documentation**:
- Comprehensive Doxygen-style comments for all public functions
- Clear explanations of parameters, return values, and error conditions
- Examples in documentation when appropriate
- Updated README and other documentation files

**Test Coverage**:
- Unit tests for individual functions in isolation
- Integration tests for module interactions
- End-to-end tests for complete workflows (especially for testnet interactions)
- Edge case testing for error conditions

**Performance Impact**:
- Minimal impact on latency-critical paths
- Efficient memory usage
- Consideration of rate limiting and API constraints
- No unnecessary allocations or copies

### Approval Process

A pull request requires:
- At least one maintainer approval
- All CI checks passing
- No merge conflicts with the main branch
- Resolution of all review comments

Maintainers may request changes or additional testing before approving a PR. Be responsive to feedback and willing to make improvements. The goal is to ensure the code meets the project's production-ready standards.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md)

## Coding Standards

Adhering to consistent coding standards ensures code readability, maintainability, and compatibility across the codebase. All contributions must follow these standards.

### C Code Style

**Naming Conventions**:
- Function names: snake_case
```c
hl_error_t hl_place_order(...);
```
- Struct names: snake_case with _t suffix
```c
typedef struct hl_client hl_client_t;
```
- Constants: UPPER_CASE
```c
#define HL_MAX_SYMBOL_LENGTH 16
```
- Variables: snake_case
```c
int connection_timeout = 30;
```

**Formatting Rules**:
- Indentation: 4 spaces (not tabs)
```c
if (condition) {
    do_something();
}
```
- Braces: K&R style
```c
if (condition) {
    // code
} else {
    // code
}
```
- Pointer declaration: asterisk with type (preferred)
```c
char* string;  // preferred
char *string;  // acceptable
```

### Documentation Style

All public functions must have comprehensive Doxygen-style comments that explain their purpose, parameters, return values, and any special considerations.

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

### Comment Guidelines

Write comments that explain the "why" rather than the "what":
```c
// Good: Explains WHY, not WHAT
// Use big-endian for network byte order (Hyperliquid API requirement)
uint64_t nonce_be = htobe64(nonce);

// Bad: States the obvious
// Convert nonce to big endian
uint64_t nonce_be = htobe64(nonce);
```

Avoid redundant comments that simply repeat what the code already clearly expresses. Focus on explaining complex algorithms, non-obvious design decisions, or API requirements that influence the implementation.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md)
- [hyperliquid.h](file://include/hyperliquid.h)

## Testing Expectations

Comprehensive testing is essential for maintaining the reliability and stability of the Hyperliquid C SDK. All contributions must include appropriate tests to verify functionality and prevent regressions.

### Test Structure

Follow the established test pattern for consistency across the codebase:

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

### Test Coverage Requirements

Aim for comprehensive test coverage across three levels:

**Unit Tests**: Test individual functions in isolation with various input combinations, including edge cases and error conditions. Mock external dependencies like HTTP responses to ensure tests are reliable and fast.

**Integration Tests**: Test interactions between modules and with external systems. For API-related functionality, create tests that interact with the Hyperliquid testnet to verify real-world behavior.

**End-to-End Tests**: Test complete workflows from client initialization through to result processing. These tests should cover common user scenarios and verify that all components work together correctly.

When adding new functionality, ensure you provide tests that cover:
- Normal operation with valid inputs
- Error handling with invalid parameters
- Edge cases and boundary conditions
- Memory management and resource cleanup
- Thread safety for public APIs

The project targets 90%+ code coverage, with particular emphasis on critical paths like order placement, cancellation, and account balance retrieval.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md)
- [STATUS.md](file://STATUS.md)

## Documentation Updates

Maintaining up-to-date and comprehensive documentation is crucial for both users and contributors. All contributions that affect functionality must be accompanied by appropriate documentation updates.

### README Updates

When adding features, update the README.md with:
1. Feature list: Add the new functionality to the appropriate section
2. Usage examples: Include clear, copy-pasteable code examples demonstrating the new feature
3. API reference: Update the complete method list with input/output parameters
4. Roadmap: Update the project status and future plans if applicable

### Code Comments

Update existing comments and add new ones as needed:
- Add Doxygen-style comments for new public functions
- Update parameter descriptions and return value explanations
- Add notes about thread safety, performance characteristics, or known limitations
- Document any non-obvious implementation details or design decisions

### Additional Documentation

Consider adding or updating:
- Troubleshooting guides for common issues related to your changes
- Performance considerations and optimization tips
- Security implications and best practices
- Migration guides if your changes affect existing functionality

The project values documentation that helps users understand not just how to use the API, but also why certain design decisions were made and how to use the features effectively in real-world scenarios.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md)
- [README.md](file://README.md)

## Release Cycle and Versioning

The Hyperliquid C SDK follows a structured release cycle and semantic versioning scheme to ensure predictable updates and backward compatibility.

### Versioning Scheme

The project uses semantic versioning (MAJOR.MINOR.PATCH) where:
- **MAJOR** version increments for incompatible API changes
- **MINOR** version increments for backward-compatible functionality additions
- **PATCH** version increments for backward-compatible bug fixes

Version numbers are defined in the hyperliquid.h header:
```c
#define HL_VERSION_MAJOR 1
#define HL_VERSION_MINOR 0
#define HL_VERSION_PATCH 0
#define HL_VERSION_STRING "1.0.0"
```

### Release Cycle

The project follows a phased development approach with regular releases:

**Phase 1: Foundation** - Complete (Project structure, API design, documentation)
**Phase 2: Implementation** - Complete (Core modules tested and working)
**Phase 3: Integration** - Optional (Full integration with 8-12 hours of work remaining)

Releases are triggered when:
- All tests pass on multiple platforms (Linux, macOS, Windows)
- Code coverage meets the target threshold
- Documentation is complete and accurate
- No critical bugs are outstanding
- The code has been reviewed and approved by maintainers

### Release Criteria

Before a release is published, it must satisfy these criteria:
- All functions implemented and tested
- All tests passing with 90%+ code coverage
- Examples working and up-to-date
- Integration tests on testnet passing
- No memory leaks detected by valgrind
- No security issues identified
- Documentation complete and accurate

The project maintains a high bar for releases, ensuring that each version is production-ready and suitable for use in high-frequency trading environments.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md)
- [STATUS.md](file://STATUS.md)
- [PROJECT_SUMMARY.md](file://PROJECT_SUMMARY.md)
- [hyperliquid.h](file://include/hyperliquid.h)

## Proposing New Features

The project welcomes proposals for new features that enhance the functionality, performance, or usability of the Hyperliquid C SDK. Follow this process to propose and discuss new features effectively.

### Feature Proposal Process

1. **Open a GitHub Discussion**: Start by opening a discussion in the GitHub Discussions section to outline your proposed feature. This allows for early feedback and helps determine if the feature aligns with the project roadmap.

2. **Provide Detailed Information**: Include:
   - Clear description of the feature and its benefits
   - Use cases and target users
   - Proposed API design and implementation approach
   - Potential impact on existing functionality
   - Any technical challenges or dependencies

3. **Consider Alternatives**: Discuss alternative approaches and their trade-offs to demonstrate thorough consideration of the design space.

### Criteria for Acceptance

Features are evaluated based on several criteria:

**Technical Feasibility**: Can the feature be implemented within the constraints of the C language and existing architecture? Does it require external dependencies that align with the project's goals?

**Performance Impact**: Will the feature maintain the library's low-latency characteristics? Does it introduce unnecessary overhead in critical paths?

**Security Implications**: Does the feature maintain the project's security standards? Are there any new attack vectors or privacy concerns?

**Maintenance Burden**: How much ongoing maintenance will the feature require? Is it likely to become outdated or need frequent updates?

**User Value**: How many users will benefit from this feature? Does it address a common use case or enable new applications?

**Alignment with Roadmap**: Does the feature fit with the project's long-term vision and priorities?

### Current Priority Areas

The project has identified several priority areas for contributions:

**Phase 3 Integration (8-12 hours estimated)**:
- Adapt API signatures in src/trading.c to match the new public API
- Create integration tests that interact with Hyperliquid testnet
- Update and create usage examples

**Advanced Features**:
- WebSocket support for real-time data streaming
- Advanced order types (stop-loss, take-profit, trailing stops, OCO)
- Hardware wallet integration (Ledger, Trezor)

**Documentation Improvements**:
- Usage examples with API snippets and common patterns
- Troubleshooting guide for common compilation and runtime issues
- Performance guide covering memory usage, CPU usage, and network optimization

By focusing on these priority areas, contributors can have the greatest impact on the project's development and help achieve the goal of 100% CCXT compatibility.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md)
- [STATUS.md](file://STATUS.md)
- [PROJECT_SUMMARY.md](file://PROJECT_SUMMARY.md)