# Contributing

<cite>
**Referenced Files in This Document**   
- [CONTRIBUTING.md](file://CONTRIBUTING.md)
- [check-modularity.sh](file://scripts/check-modularity.sh)
- [Makefile](file://Makefile)
- [hyperliquid.h](file://include/hyperliquid.h)
- [trading_api.c](file://src/trading_api.c)
- [simple_test.c](file://tests/simple_test.c)
</cite>

## Table of Contents
1. [Code Style Guidelines](#code-style-guidelines)
2. [Pull Request Process and Code Review](#pull-request-process-and-code-review)
3. [Testing Requirements](#testing-requirements)
4. [Modularity Maintenance](#modularity-maintenance)
5. [Documentation Requirements](#documentation-requirements)
6. [Issue Reporting and Feature Requests](#issue-reporting-and-feature-requests)
7. [Versioning and Release Cycle](#versioning-and-release-cycle)

## Code Style Guidelines

The Hyperliquid C SDK follows strict code style conventions to ensure consistency, readability, and maintainability across the codebase.

### C Code Style
The project uses a consistent C coding standard with the following rules:
- **Function names**: snake_case (e.g., `hl_place_order`)
- **Struct names**: snake_case with _t suffix (e.g., `hl_client_t`)
- **Constants**: UPPER_CASE (e.g., `HL_MAX_SYMBOL_LENGTH`)
- **Variables**: snake_case (e.g., `connection_timeout`)
- **Indentation**: 4 spaces (no tabs)
- **Braces**: K&R style with opening brace on the same line as control statements
- **Pointer declaration**: asterisk with type (e.g., `char* string` preferred over `char *string`)

### Documentation Style
All public functions must include comprehensive Doxygen-style comments that document parameters, return values, and behavior. Comments should explain the "why" rather than the "what" when possible. The documentation should include:
- `@brief` description of function purpose
- `@param` for each parameter with description
- `@return` for return value description
- `@note` for important implementation details or constraints

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
```

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md#L192-L222)

## Pull Request Process and Code Review

The pull request process follows a structured workflow to ensure code quality and maintain project standards.

### Development Workflow
1. **Fork and Clone**: Fork the repository on GitHub and clone your fork locally
2. **Create Feature Branch**: Use descriptive branch names with prefixes like `feature/` or `fix/`
3. **Make Changes**: Implement your changes following the code style guidelines
4. **Test Changes**: Run all relevant tests and ensure they pass
5. **Commit**: Use conventional commit messages with prefixes
6. **Push and Create PR**: Push to your fork and create a pull request on GitHub

### Commit Message Format
The project uses conventional commit prefixes to categorize changes:
- `feat:` - New feature implementation
- `fix:` - Bug fix
- `docs:` - Documentation changes
- `test:` - Test additions or modifications
- `refactor:` - Code refactoring without behavior changes
- `perf:` - Performance improvements

All pull requests undergo a comprehensive code review process that includes automated checks and manual review. Automated checks verify compilation without warnings, test passing, and code style compliance. Manual review evaluates code quality, documentation completeness, test coverage, and performance implications.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md#L192-L222)

## Testing Requirements

The project maintains rigorous testing requirements to ensure reliability and correctness of all functionality.

### Test Structure
Tests follow a consistent structure with clear arrangement, action, and assertion phases. Each test file includes:
- Necessary header inclusions
- Test function definitions with descriptive names
- Arrange phase: setup of test conditions
- Act phase: execution of the function under test
- Assert phase: verification of expected outcomes
- Cleanup phase: resource deallocation
- Success reporting

### Test Coverage
The project aims for comprehensive test coverage across multiple levels:
- **Unit Tests**: Isolated testing of individual functions
- **Integration Tests**: Testing interactions between modules
- **End-to-End Tests**: Full workflow testing using the Hyperliquid testnet

All new contributions must include appropriate tests that verify the implemented functionality. Tests should cover both success and error conditions, and edge cases should be considered. The build system supports running tests with `make test`, and memory checking can be performed with `make memcheck` using valgrind.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md#L192-L222)
- [simple_test.c](file://tests/simple_test.c#L1-L35)

## Modularity Maintenance

The project enforces modularity through the `check-modularity.sh` script to prevent overly large files and promote maintainable code organization.

### Modularity Check Script
The `scripts/check-modularity.sh` script analyzes all source and header files in the `src` and `include` directories, checking for files exceeding 1000 lines. The script provides detailed analysis including:
- Line count for each file
- File status (OK or TOO LARGE)
- Overall statistics (total files, total lines, average lines per file)
- Largest file identification
- Count of files over the 1000-line threshold

Files exceeding 1000 lines are flagged with a warning, and the script exits with code 1 if critical modularity issues are detected. This prevents the codebase from developing monolithic files that are difficult to maintain and understand.

The script also outputs metrics to GitHub Actions environment variables for CI/CD integration, including modularity grade, color coding, and specific file information. This enables automated enforcement of modularity standards in the continuous integration pipeline.

**Section sources**
- [check-modularity.sh](file://scripts/check-modularity.sh#L1-L91)

## Documentation Requirements

Comprehensive documentation is required for all new features to ensure usability and maintainability.

### README Updates
When adding new features, the following documentation updates are required:
1. Update the feature list to include the new functionality
2. Add usage examples demonstrating the feature
3. Update the API reference section with new functions and structures
4. Update the project roadmap to reflect completed work

### Code Comments
Source code must include meaningful comments that explain the rationale behind complex implementations. Comments should focus on explaining "why" decisions were made rather than simply stating what the code does. For example, comments should explain protocol requirements, security considerations, or performance optimizations rather than restating obvious operations.

All public APIs must be thoroughly documented with Doxygen comments that describe function parameters, return values, error conditions, and any important behavioral notes. Internal functions should also be commented when their purpose or implementation is not immediately obvious.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md#L192-L222)

## Issue Reporting and Feature Requests

The project uses GitHub's issue tracking and discussion features for communication and collaboration.

### Bug Reports
When reporting bugs, include the following information:
- **Description**: Clear explanation of the issue
- **Reproduction**: Step-by-step instructions to reproduce the problem
- **Expected Behavior**: What should happen under normal circumstances
- **Actual Behavior**: What actually occurs when the issue manifests
- **Environment**: Operating system, compiler version, and library versions
- **Logs**: Relevant error messages or log output

### Communication Channels
The project uses different GitHub features for different types of communication:
- **GitHub Issues**: For bug reports and specific feature requests
- **GitHub Discussions**: For general questions, ideas, and help requests
- **Pull Requests**: For code contributions and associated discussions

This separation ensures that different types of conversations are organized appropriately and can be addressed by the right contributors.

**Section sources**
- [CONTRIBUTING.md](file://CONTRIBUTING.md#L192-L222)

## Versioning and Release Cycle

The project follows a structured versioning and release process to manage development and deployment.

### Versioning Scheme
The project uses semantic versioning (MAJOR.MINOR.PATCH) to indicate the nature of changes in each release:
- **MAJOR**: Incompatible API changes
- **MINOR**: Backward-compatible functionality additions
- **PATCH**: Backward-compatible bug fixes

The current version is specified in the Makefile and is used for library naming and distribution.

### Release Cycle
The development follows a phased approach with clear milestones:
- **Phase 1**: Foundation and project structure
- **Phase 2**: Core module implementation and testing
- **Phase 3**: Full integration and adaptation
- **Phase 4**: Market data implementation
- **Phase 5**: Historical data and analytics
- **Phase 6**: Trading functionality
- **Phase 7**: CCXT-compatible architecture
- **Phase 8**: Base API completion
- **Phase 9**: Extended API methods
- **Phase 10**: WebSocket/Pro API framework
- **Phase 11**: Production deployment and documentation

Each phase includes specific deliverables and completion criteria. The project has achieved production-ready status with 84.3% CCXT method implementation (43/51 methods), representing comprehensive functionality for algorithmic trading applications.

The release process includes comprehensive testing, documentation generation, and packaging for distribution. The build system supports installation via `make install` and provides targets for generating documentation, running static analysis, and producing code coverage reports.

**Section sources**
- [STATUS.md](file://STATUS.md#L435-L490)
- [Makefile](file://Makefile#L1-L226)