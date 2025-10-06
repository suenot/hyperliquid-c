# Build System

<cite>
**Referenced Files in This Document**   
- [Makefile](file://Makefile)
- [check_ready.sh](file://check_ready.sh)
- [fix_includes.sh](file://fix_includes.sh)
- [CONTRIBUTING.md](file://CONTRIBUTING.md)
- [QUICKSTART.md](file://QUICKSTART.md)
- [README.md](file://README.md)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Build Targets Overview](#build-targets-overview)
3. [Dependency Requirements](#dependency-requirements)
4. [Helper Scripts](#helper-scripts)
5. [Platform-Specific Build Instructions](#platform-specific-build-instructions)
6. [Common Build Commands and Output](#common-build-commands-and-output)
7. [Troubleshooting Common Build Failures](#troubleshooting-common-build-failures)
8. [Build System Integration with Development Workflow](#build-system-integration-with-development-workflow)
9. [Conclusion](#conclusion)

## Introduction

The Hyperliquid C SDK features a production-ready build system designed for reliability, consistency, and ease of use. The system is centered around a comprehensive Makefile that orchestrates compilation, testing, installation, and quality assurance tasks. This documentation provides a complete guide to the build system, covering all available targets, required dependencies, helper scripts, platform-specific instructions, and integration with the development workflow. The build system ensures that developers can compile the library, run tests, and deploy the SDK across different environments with minimal configuration.

**Section sources**
- [Makefile](file://Makefile#L1-L226)
- [README.md](file://README.md#L1-L737)

## Build Targets Overview

The Makefile provides a comprehensive set of targets for building, testing, and maintaining the Hyperliquid C SDK. The default target builds both static and shared libraries, while additional targets support testing, examples, debugging, installation, and code quality checks.

### Core Build Targets

The primary targets for building the library are:

- **all**: Default target that builds both static and shared libraries
- **static**: Builds only the static library (libhyperliquid.a)
- **shared**: Builds only the shared library (libhyperliquid.so)
- **dirs**: Creates necessary build directories (build/obj, build/lib, build/bin)

The build process compiles all source files from the src directory and its subdirectories, organizing object files in a parallel directory structure under build/obj. The static library is created using the ar archiver, while the shared library is built with gcc's -shared flag.

### Testing Targets

The build system includes comprehensive testing capabilities:

- **tests**: Builds all test executables
- **test**: Builds and runs all tests
- **memcheck**: Runs tests with Valgrind for memory leak detection
- **coverage**: Generates code coverage reports using gcov and lcov

The testing framework supports both unit and integration tests, with the ability to run memory checks and generate detailed coverage reports. Tests are located in the tests directory and are compiled as separate executables.

### Development and Quality Assurance Targets

Additional targets support development and code quality:

- **debug**: Builds with debug symbols and sanitizers enabled
- **format**: Formats code using clang-format
- **analyze**: Runs static analysis with cppcheck
- **docs**: Generates documentation using Doxygen
- **info**: Displays build configuration information

These targets help maintain code quality, ensure consistent formatting, and provide detailed build information for troubleshooting.

### Installation and Cleanup Targets

The build system includes targets for installation and cleanup:

- **install**: Installs the library to /usr/local
- **uninstall**: Removes the library from /usr/local
- **clean**: Removes all build artifacts

The installation process copies both static and shared libraries to /usr/local/lib, header files to /usr/local/include, and runs ldconfig to update the library cache.

### Examples Target

- **examples**: Builds all example programs from the examples directory

This target compiles demonstration programs that showcase the SDK's capabilities, including trading bots, market data retrieval, and account management.

**Section sources**
- [Makefile](file://Makefile#L1-L226)

## Dependency Requirements

The Hyperliquid C SDK requires several external libraries to function properly. These dependencies are specified in the Makefile and must be installed before building the library.

### Required Libraries

The build system depends on the following libraries:

- **libcurl**: For HTTP client functionality
- **cJSON**: For JSON parsing and generation
- **libsecp256k1**: For ECDSA cryptographic operations and EIP-712 signing
- **libuuid**: For UUID generation
- **pthread**: For thread synchronization and concurrency

These libraries are linked during the build process using the LIBS variable in the Makefile, which specifies the required libraries with the -l flag.

### Platform-Specific Installation

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get install libcurl4-openssl-dev libcjson-dev libsecp256k1-dev uuid-dev
```

#### macOS (with Homebrew)
```bash
brew install curl cjson libsecp256k1 ossp-uuid
```

#### Arch Linux
```bash
sudo pacman -S curl cjson libsecp256k1 uuid
```

The Makefile includes platform detection logic to handle differences between operating systems, particularly for shared library naming conventions (.so on Linux, .dll on Windows).

### Header File Inclusion

The build system includes header files from both the local include directory and system locations:

- **Local includes**: -Iinclude (for project-specific headers)
- **System includes**: -I/opt/homebrew/include (for Homebrew-installed libraries on macOS)

This ensures that the compiler can locate both project headers and external library headers during compilation.

**Section sources**
- [Makefile](file://Makefile#L9)
- [CONTRIBUTING.md](file://CONTRIBUTING.md#L22)
- [QUICKSTART.md](file://QUICKSTART.md#L10-L25)

## Helper Scripts

The build system includes two helper scripts that maintain build integrity and ensure project readiness: fix_includes.sh and check_ready.sh.

### fix_includes.sh

This script automates the process of correcting include statements in source files. It processes files in the crypto, msgpack, and http subdirectories, replacing outdated or incorrect include directives with the correct ones.

The script performs the following operations:
- Updates crypto files to include hl_crypto_internal.h instead of crypto_utils.h or sha3.h
- Updates msgpack files to include hl_msgpack.h instead of hyperliquid_action.h
- Updates http files to include hl_http.h instead of http_client.h
- Removes unnecessary includes like memory.h

This ensures consistency in include statements across the codebase and prevents compilation errors due to missing or incorrect headers.

```bash
#!/bin/bash
# Fix includes in all copied files

echo "Fixing includes in copied files..."

# Fix crypto files
for f in src/crypto/*.c; do
    echo "  Processing $f..."
    sed -i '' 's|#include "crypto_utils.h"|#include "hl_crypto_internal.h"|g' "$f"
    sed -i '' 's|#include "logger.h"|#include "hl_logger.h"|g' "$f"
    sed -i '' 's|#include "sha3.h"|#include "hl_crypto_internal.h"|g' "$f"
done

# Fix msgpack files
for f in src/msgpack/*.c; do
    echo "  Processing $f..."
    sed -i '' 's|#include "hyperliquid_action.h"|#include "hl_msgpack.h"|g' "$f"
    sed -i '' 's|#include "crypto_utils.h"|#include "hl_crypto_internal.h"|g' "$f"
done

# Fix http files
for f in src/http/*.c; do
    echo "  Processing $f..."
    sed -i '' 's|#include "http_client.h"|#include "hl_http.h"|g' "$f"
    sed -i '' 's|#include "logger.h"|#include "hl_logger.h"|g' "$f"
    sed -i '' 's|#include "memory.h"|// memory.h not needed|g' "$f"
done

echo "✅ Includes fixed!"
```

**Diagram sources**
- [fix_includes.sh](file://fix_includes.sh#L1-L30)

**Section sources**
- [fix_includes.sh](file://fix_includes.sh#L1-L30)

### check_ready.sh

This comprehensive readiness check script verifies that the project is complete and ready for publication. It performs a series of checks on documentation, directory structure, source files, tests, examples, and the build system itself.

The script checks for the presence of essential files including:
- Documentation files (README.md, LICENSE, CONTRIBUTING.md, etc.)
- Directory structure (include, src, tests, examples, docs)
- Critical source files (client.c, trading.c, crypto files, etc.)
- Test files (test_minimal.c)
- Example files (simple_trade.c)
- Build files (Makefile, Makefile.test)

After verifying file presence, the script runs the minimal test suite to ensure core functionality works. It then displays project statistics including total files, size, and code composition. The script concludes with a final assessment: if all checks pass, it indicates the project is ready for publication; otherwise, it lists the failed checks that need to be addressed.

This script serves as a quality gate before releasing new versions of the SDK, ensuring that all components are present and functional.

**Diagram sources**
- [check_ready.sh](file://check_ready.sh#L1-L163)

**Section sources**
- [check_ready.sh](file://check_ready.sh#L1-L163)

## Platform-Specific Build Instructions

The build system supports multiple platforms with specific instructions for each environment.

### Linux Build Instructions

On Linux systems, follow these steps to build the SDK:

1. Install dependencies:
```bash
sudo apt-get update
sudo apt-get install -y libcurl4-openssl-dev libcjson-dev libsecp256k1-dev uuid-dev build-essential
```

2. Clone and build:
```bash
git clone https://github.com/suenot/hyperliquid-c.git
cd hyperliquid-c
make
```

3. Install (optional):
```bash
sudo make install
```

The build process will create both static (libhyperliquid.a) and shared (libhyperliquid.so) libraries in the build/lib directory.

### macOS Build Instructions

On macOS with Homebrew, follow these steps:

1. Install dependencies:
```bash
brew install curl cjson libsecp256k1 ossp-uuid
```

2. Clone and build:
```bash
git clone https://github.com/suenot/hyperliquid-c.git
cd hyperliquid-c
make
```

3. Install (optional):
```bash
sudo make install
```

The build system automatically detects the macOS platform and adjusts include paths accordingly, using /opt/homebrew/include for Homebrew-installed libraries on Apple Silicon Macs.

### Windows Build Instructions

For Windows systems using MSYS2 or similar environments:

1. Install dependencies:
```bash
pacman -S mingw-w64-x86_64-curl mingw-w64-x86_64-cjson mingw-w64-x86_64-libsecp256k1 mingw-w64-x86_64-ossp-uuid
```

2. Build with MinGW:
```bash
make CC=gcc
```

The Makefile includes platform detection logic that sets the appropriate shared library extension (.dll on Windows, .so on Unix-like systems).

**Section sources**
- [QUICKSTART.md](file://QUICKSTART.md#L10-L42)
- [CONTRIBUTING.md](file://CONTRIBUTING.md#L22)
- [Makefile](file://Makefile#L13-L18)

## Common Build Commands and Output

The build system provides a consistent interface for common operations, with predictable output that helps users verify successful execution.

### Basic Build Commands

```bash
# Build static and shared libraries (default)
make

# Build and run tests
make test

# Build with debug symbols and sanitizers
make debug

# Install the library
make install

# Clean build artifacts
make clean

# Show help
make help
```

### Expected Output

When building the libraries, the expected output includes:

```
Compiling src/client.c
Compiling src/trading.c
Compiling src/account.c
...
Building static library build/lib/libhyperliquid.a
Static library built successfully
Building shared library build/lib/libhyperliquid.so
Shared library built successfully
```

When running tests:

```
Building tests...
Building test_minimal
Tests built successfully
Running tests...

=== Running build/bin/test_minimal ===
✅ ALL TESTS PASSED!
```

When installing:

```
Installing library to /usr/local...
Library installed successfully
```

When cleaning:

```
Cleaning build artifacts...
Clean complete
```

The build system uses colored output and emoji indicators (✅, ❌) to make status clear at a glance. The info target provides detailed build configuration:

```
Build configuration:
  CC:       gcc
  CFLAGS:   -std=c11 -Wall -Wextra -Werror -pedantic -O3 -fPIC
  INCLUDES: -Iinclude -I/opt/homebrew/include
  LIBS:     -lcurl -lcjson -lssl -lcrypto -lmsgpackc -lsecp256k1 -lm -lpthread
  VERSION:  1.0.0
  SOURCES:  25 files
  TESTS:    1 files
  EXAMPLES: 10 files
```

**Section sources**
- [Makefile](file://Makefile#L200-L226)
- [QUICKSTART.md](file://QUICKSTART.md#L26-L38)

## Troubleshooting Common Build Failures

This section addresses common build failures and their solutions.

### Missing Dependencies

**Error**: "fatal error: curl/curl.h: No such file or directory"

**Solution**: Install the missing library development package:
- Ubuntu/Debian: `sudo apt-get install libcurl4-openssl-dev`
- macOS: `brew install curl`
- Arch Linux: `sudo pacman -S curl`

### Incorrect Library Paths

**Error**: "ld: library not found for -lcurl"

**Solution**: Ensure the library is installed and the linker can find it. On macOS with Homebrew, the Makefile already includes -I/opt/homebrew/include, but if using a different installation method, you may need to modify the INCLUDES variable in the Makefile.

### Architecture Mismatch

**Error**: "ld: symbol(s) not found for architecture x86_64"

**Solution**: Ensure all libraries are compiled for the same architecture. On Apple Silicon Macs, use Homebrew to install libraries that are compatible with arm64 architecture.

### Permission Issues

**Error**: "Permission denied" when installing

**Solution**: The install target requires sudo privileges to write to /usr/local. Run `sudo make install` or install to a user-writable location by modifying the Makefile.

### Header File Conflicts

**Error**: "error: unknown type name 'hl_client_t'"

**Solution**: Run `./fix_includes.sh` to ensure all include statements are correct, then clean and rebuild:
```bash
./fix_includes.sh
make clean
make
```

### Test Failures

**Error**: "Test failed: Connection refused"

**Solution**: Verify internet connectivity and ensure the Hyperliquid API is accessible:
```bash
curl https://api.hyperliquid-testnet.xyz/info
```

If the API is accessible but tests still fail, check firewall settings and ensure outgoing connections on port 443 are allowed.

**Section sources**
- [QUICKSTART.md](file://QUICKSTART.md#L230-L270)
- [Makefile](file://Makefile#L1-L226)

## Build System Integration with Development Workflow

The build system is tightly integrated with the overall development workflow and CI/CD pipeline, ensuring consistent quality and reliability.

### Development Workflow Integration

The build system supports a comprehensive development workflow:

1. **Code Formatting**: The format target ensures consistent code style using clang-format
2. **Static Analysis**: The analyze target runs cppcheck for code quality analysis
3. **Testing**: The test target runs all tests, while memcheck and coverage provide deeper analysis
4. **Documentation**: The docs target generates API documentation with Doxygen

Developers are expected to run `make format` before committing code, and `make test` to verify functionality. The debug target with sanitizers helps catch memory errors and undefined behavior during development.

### CI/CD Pipeline Integration

The build system is designed to work seamlessly with CI/CD pipelines:

- **Cross-Platform Builds**: The Makefile works on Linux, macOS, and Windows
- **Automated Testing**: The test target runs all tests in a reproducible environment
- **Code Coverage**: The coverage target generates reports for quality metrics
- **Memory Safety**: The memcheck target uses Valgrind to detect memory leaks
- **Static Analysis**: The analyze target identifies potential code issues

The CI/CD pipeline typically executes the following sequence:
1. Install dependencies
2. Run `make info` to verify build configuration
3. Run `make` to build the library
4. Run `make test` to verify functionality
5. Run `make memcheck` on Linux for memory leak detection
6. Run `make coverage` to generate coverage reports
7. Run `make analyze` for static analysis
8. Deploy artifacts if all checks pass

### Quality Gates

The build system implements several quality gates:

- **File Size Limits**: The project monitors file sizes to maintain modularity
- **Test Coverage**: A minimum coverage threshold is enforced
- **Static Analysis**: Code must pass cppcheck without critical warnings
- **Memory Safety**: No memory leaks detected by Valgrind
- **Build Completeness**: The check_ready.sh script verifies all components are present

These quality gates ensure that only high-quality, well-tested code is released, maintaining the reliability and performance of the Hyperliquid C SDK.

**Section sources**
- [Makefile](file://Makefile#L1-L226)
- [check_ready.sh](file://check_ready.sh#L1-L163)
- [README.md](file://README.md#L1-L737)

## Conclusion

The Hyperliquid C SDK build system provides a robust, comprehensive framework for compiling, testing, and deploying the library across multiple platforms. With its well-organized Makefile, comprehensive testing infrastructure, and helpful utility scripts, the build system ensures consistent, reliable builds that meet production standards. The integration of quality assurance tools like static analysis, memory checking, and code coverage reporting helps maintain high code quality throughout the development lifecycle. By following the documented procedures and addressing common issues, developers can successfully build and use the SDK for trading applications on the Hyperliquid exchange.

**Section sources**
- [Makefile](file://Makefile#L1-L226)
- [README.md](file://README.md#L1-L737)
- [CONTRIBUTING.md](file://CONTRIBUTING.md#L1-L446)