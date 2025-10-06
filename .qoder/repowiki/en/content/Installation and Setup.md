# Installation and Setup

<cite>
**Referenced Files in This Document**   
- [Makefile](file://Makefile)
- [fix_includes.sh](file://fix_includes.sh)
- [README.md](file://README.md)
</cite>

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Build Process](#build-process)
3. [External Dependencies](#external-dependencies)
4. [Library Installation](#library-installation)
5. [Linking with Client Applications](#linking-with-client-applications)
6. [Cross-Compilation](#cross-compilation)
7. [Integration with CMake](#integration-with-cmake)
8. [Installation Verification](#installation-verification)
9. [Header File Management](#header-file-management)
10. [Troubleshooting](#troubleshooting)

## Prerequisites

Before building the Hyperliquid C SDK, ensure your system has the following prerequisites installed:

- **C Compiler**: GCC or Clang with C11 support
- **Build System**: GNU Make
- **Development Tools**: Standard build tools (autoconf, automake, libtool)
- **Package Manager**: apt (Ubuntu/Debian) or Homebrew (macOS)

The SDK requires specific external libraries that will be detailed in the External Dependencies section.

**Section sources**
- [README.md](file://README.md#L216-L230)

## Build Process

The Hyperliquid C SDK uses a comprehensive Makefile-based build system that supports both static and shared library compilation. The build process is controlled through various Make targets that handle different aspects of the build workflow.

### Static and Shared Library Compilation

The default build target compiles both static and shared library variants of the SDK:

```bash
# Build both static and shared libraries (default)
make

# Clean build artifacts before building
make clean all
```

The Makefile defines the following key variables for compilation:
- `CC = gcc`: Compiler used for compilation
- `CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -O3 -fPIC`: Compilation flags
- `AR = ar`: Archiver for static libraries
- `LIBS = -lcurl -lcjson -lssl -lcrypto -lmsgpackc -lsecp256k1 -lm -lpthread`: Required external libraries

The build system automatically compiles all source files from the `src/` directory and places object files in the `build/obj/` directory before creating the final libraries in `build/lib/`.

### Debug Build

For development and debugging purposes, a debug build target is available that includes debug symbols and sanitizers:

```bash
# Build with debug symbols and sanitizers
make debug
```

This target modifies the `CFLAGS` to include `-g -O0 -DDEBUG` and enables AddressSanitizer and UndefinedBehaviorSanitizer for comprehensive runtime error detection.

**Section sources**
- [Makefile](file://Makefile#L10-L216)

## External Dependencies

The Hyperliquid C SDK depends on several external libraries that must be installed on your system before compilation. These dependencies provide essential functionality for HTTP communication, JSON parsing, cryptographic operations, and message serialization.

### Linux Installation (Ubuntu/Debian)

On Ubuntu or Debian-based systems, install the required dependencies using apt:

```bash
# Install required development libraries
sudo apt-get update
sudo apt-get install -y \
    libcurl4-openssl-dev \
    libcjson-dev \
    libssl-dev \
    libmsgpack-dev \
    libsecp256k1-dev \
    uuid-dev
```

### macOS Installation (Homebrew)

On macOS with Homebrew installed, use the following commands to install dependencies:

```bash
# Install required libraries via Homebrew
brew install curl
brew install jansson
brew install openssl@1.1
brew install msgpack
brew install libsecp256k1
brew install ossp-uuid
```

Note that on macOS, the Makefile includes `-I/opt/homebrew/include` in the `INCLUDES` variable to ensure the compiler can find headers installed by Homebrew on Apple Silicon Macs.

**Section sources**
- [README.md](file://README.md#L216-L230)
- [Makefile](file://Makefile#L13)

## Library Installation

The SDK provides a convenient installation target that copies the compiled libraries and header files to system directories, making them available for use by other applications.

### Installing the Library

After building the libraries, install them system-wide using:

```bash
# Build and install the library
make install
```

This command performs the following actions:
1. Builds both static and shared libraries if they haven't been built
2. Copies `libhyperliquid.a` and `libhyperliquid.so` to `/usr/local/lib/`
3. Copies all header files from the `include/` directory to `/usr/local/include/`
4. Runs `ldconfig` to update the shared library cache

### Uninstalling the Library

To remove the installed library files:

```bash
# Remove installed library files
make uninstall
```

This command removes the library files from `/usr/local/lib/` and header files from `/usr/local/include/`.

### Custom Installation Directory

For installations to custom directories, modify the Makefile or use environment variables:

```bash
# Example: Install to a custom prefix
sudo cp build/lib/libhyperliquid.* /opt/hyperliquid/lib/
sudo cp include/*.h /opt/hyperliquid/include/
```

**Section sources**
- [Makefile](file://Makefile#L134-L145)

## Linking with Client Applications

To use the Hyperliquid C SDK in your applications, you need to link against the compiled libraries and include the appropriate header files.

### Compiler and Linker Flags

When compiling client applications, use the following flags:

```bash
# Compilation flags for including SDK headers
-I/usr/local/include

# Linking flags for static library
-L/usr/local/lib -lhyperliquid -lcurl -lcjson -lssl -lcrypto -lmsgpackc -lsecp256k1 -lm -lpthread

# Or for shared library (same linking flags)
-L/usr/local/lib -lhyperliquid -lcurl -lcjson -lssl -lcrypto -lmsgpackc -lsecp256k1 -lm -lpthread
```

### Example Compilation

Here's an example of compiling a client application that uses the SDK:

```bash
# Compile a client application
gcc -std=c11 -O3 \
    -I/usr/local/include \
    client_app.c \
    -o client_app \
    -L/usr/local/lib \
    -lhyperliquid \
    -lcurl -lcjson -lssl -lcrypto -lmsgpackc -lsecp256k1 -lm -lpthread
```

### Basic Usage Example

```c
#include "hyperliquid.h"

int main() {
    // Create client instance
    hl_client_t* client = hl_client_create(
        "your_wallet_address",
        "your_private_key",
        true  // testnet
    );
    
    if (!client) {
        return 1;
    }
    
    // Use SDK functions
    hl_balance_t balance = {0};
    if (hl_fetch_balance(client, HL_ACCOUNT_PERPETUAL, &balance) == HL_SUCCESS) {
        printf("Account value: %.2f USDC\n", balance.account_value);
    }
    
    // Cleanup
    hl_client_destroy(client);
    return 0;
}
```

**Section sources**
- [Makefile](file://Makefile#L13)
- [README.md](file://README.md#L276-L288)

## Cross-Compilation

The Hyperliquid C SDK can be cross-compiled for different architectures and platforms by modifying the compiler and flags in the Makefile.

### Cross-Compilation Setup

To cross-compile, modify the `CC` variable to point to the cross-compiler:

```bash
# Example: Cross-compiling for ARM64
make CC=aarch64-linux-gnu-gcc

# Example: Cross-compiling for Windows using MinGW
make CC=x86_64-w64-mingw32-gcc
```

### Architecture-Specific Considerations

When cross-compiling, ensure that the required dependencies are available for the target architecture. You may need to:

1. Install cross-compilation toolchains
2. Build dependencies for the target architecture
3. Set appropriate include and library paths

```bash
# Example: Cross-compiling with custom paths
make CC=aarch64-linux-gnu-gcc \
     CFLAGS="-std=c11 -O3 -fPIC -I/path/to/cross/include" \
     LIBS="-L/path/to/cross/lib -lcurl -lcjson -lssl -lcrypto -lmsgpackc -lsecp256k1 -lm -lpthread"
```

### Target Platform Examples

#### Windows (MinGW)
```bash
# Install MinGW dependencies on Ubuntu
sudo apt-get install mingw-w64

# Cross-compile for Windows
make CC=x86_64-w64-mingw32-gcc \
     AR=x86_64-w64-mingw32-ar \
     LIBS="-lcurl -ljansson -lssl -lcrypto -lmsgpackc -lsecp256k1 -lm -lpthread"
```

#### ARM64 (Linux)
```bash
# Cross-compile for ARM64
make CC=aarch64-linux-gnu-gcc \
     AR=aarch64-linux-gnu-ar \
     CFLAGS="-std=c11 -O3 -fPIC -I/usr/aarch64-linux-gnu/include" \
     LIBS="-L/usr/aarch64-linux-gnu/lib -lcurl -lcjson -lssl -lcrypto -lmsgpackc -lsecp256k1 -lm -lpthread"
```

**Section sources**
- [Makefile](file://Makefile#L10-L13)

## Integration with CMake

The Hyperliquid C SDK can be integrated into projects using CMake as the build system through several approaches.

### Using find_package

If the SDK is installed system-wide, use `find_package` in your CMakeLists.txt:

```cmake
# Find the Hyperliquid package
find_package(PkgConfig REQUIRED)
pkg_check_modules(HYPERLIQUID REQUIRED hyperliquid)

# Include directories
include_directories(${HYPERLIQUID_INCLUDE_DIRS})

# Link libraries
target_link_libraries(your_target ${HYPERLIQUID_LIBRARIES})
target_link_directories(your_target ${HYPERLIQUID_LIBRARY_DIRS})
```

### Manual Integration

For manual integration without pkg-config:

```cmake
# Set Hyperliquid SDK paths
set(HYPERLIQUID_ROOT "/path/to/hyperliquid-c")
set(HYPERLIQUID_INCLUDE_DIR "${HYPERLIQUID_ROOT}/include")
set(HYPERLIQUID_LIB_DIR "${HYPERLIQUID_ROOT}/build/lib")

# Include directories
include_directories(${HYPERLIQUID_INCLUDE_DIR})

# Link libraries
target_link_libraries(your_target 
    "${HYPERLIQUID_LIB_DIR}/libhyperliquid.a"
    curl cjson ssl crypto msgpackc secp256k1 m pthread)
```

### FetchContent Integration

For modern CMake projects, use FetchContent to include the SDK as a subproject:

```cmake
# CMake 3.14+ - Fetch and build Hyperliquid SDK
include(FetchContent)

FetchContent_Declare(
    hyperliquid-c
    GIT_REPOSITORY https://github.com/hyperliquid/hyperliquid-c.git
    GIT_TAG v1.0.0
)

FetchContent_MakeAvailable(hyperliquid-c)

# Link against the built target
target_link_libraries(your_target hyperliquid-c)
```

### Creating a Config File

Create a `hyperliquid-config.cmake` file for seamless integration:

```cmake
# hyperliquid-config.cmake
set(HYPERLIQUID_INCLUDE_DIRS "/usr/local/include")
set(HYPERLIQUID_LIBRARIES 
    "/usr/local/lib/libhyperliquid.a"
    curl cjson ssl crypto msgpackc secp256k1 m pthread)
set(HYPERLIQUID_FOUND TRUE)
```

**Section sources**
- [Makefile](file://Makefile#L13)

## Installation Verification

After installation, verify that the Hyperliquid C SDK is properly installed and functional through test compilation and basic functionality checks.

### Test Compilation

Create a simple test program to verify the installation:

```c
// test_hyperliquid.c
#include <stdio.h>
#include "hyperliquid.h"

int main() {
    printf("Hyperliquid SDK version 1.0.0\n");
    
    // Test client creation (without actual credentials)
    hl_client_t* client = hl_client_create(NULL, NULL, true);
    if (client) {
        printf("✓ Client creation successful\n");
        hl_client_destroy(client);
    } else {
        printf("✗ Client creation failed\n");
        return 1;
    }
    
    printf("✓ Hyperliquid SDK installation verified\n");
    return 0;
}
```

Compile and run the test:

```bash
# Compile the test program
gcc -o test_hyperliquid test_hyperliquid.c -lhyperliquid -lcurl -lcjson -lssl -lcrypto -lmsgpackc -lsecp256k1 -lm -lpthread

# Run the test
./test_hyperliquid
```

### Running Provided Examples

The SDK includes example programs that demonstrate various functionalities:

```bash
# Build all examples
make examples

# Run a specific example
./build/bin/simple_balance
./build/bin/simple_trade
./build/bin/websocket_demo
```

### Running Tests

The SDK includes a comprehensive test suite to verify functionality:

```bash
# Build and run all tests
make test

# Build tests only
make tests

# Run memory checks with valgrind
make memcheck
```

### Build Information

Display build configuration information:

```bash
# Show build configuration
make info
```

This command displays compiler settings, flags, library versions, and source file counts to verify the build environment.

**Section sources**
- [Makefile](file://Makefile#L147-L214)
- [examples/simple_balance.c](file://examples/simple_balance.c)
- [examples/simple_trade.c](file://examples/simple_trade.c)

## Header File Management

The SDK includes a script to manage header file includes, ensuring consistency across the codebase.

### fix_includes.sh Script

The `fix_includes.sh` script automatically corrects include statements in source files to use the proper header names:

```bash
# Run the include fixing script
./fix_includes.sh
```

This script performs the following operations:

1. **Crypto Files**: Updates includes in `src/crypto/*.c`:
   - Changes `#include "crypto_utils.h"` to `#include "hl_crypto_internal.h"`
   - Changes `#include "logger.h"` to `#include "hl_logger.h"`
   - Changes `#include "sha3.h"` to `#include "hl_crypto_internal.h"`

2. **Msgpack Files**: Updates includes in `src/msgpack/*.c`:
   - Changes `#include "hyperliquid_action.h"` to `#include "hl_msgpack.h"`
   - Changes `#include "crypto_utils.h"` to `#include "hl_crypto_internal.h"`

3. **HTTP Files**: Updates includes in `src/http/*.c`:
   - Changes `#include "http_client.h"` to `#include "hl_http.h"`
   - Changes `#include "logger.h"` to `#include "hl_logger.h"`
   - Replaces `#include "memory.h"` with a comment

### When to Use fix_includes.sh

Run the `fix_includes.sh` script in the following scenarios:

- After copying files from external sources
- When adding new source files that may have incorrect includes
- Before committing changes to ensure include consistency
- After merging code from different branches that might have different include conventions

The script should be run from the repository root directory and requires `sed` to be available on the system. On macOS, the script uses `sed -i ''` for in-place editing, which is compatible with BSD sed.

**Section sources**
- [fix_includes.sh](file://fix_includes.sh)
- [Makefile](file://Makefile#L10)

## Troubleshooting

This section addresses common build errors and their solutions, focusing on issues related to missing symbols and library paths.

### Missing Dependencies

**Error**: `fatal error: curl/curl.h: No such file or directory`
**Solution**: Install the missing development package:
```bash
# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev

# macOS
brew install curl
```

### Library Not Found

**Error**: `cannot find -lcurl`
**Solution**: Ensure development packages are installed and libraries are in the library path:
```bash
# Verify library locations
find /usr -name "libcurl*" 2>/dev/null
find /opt/homebrew -name "libcurl*" 2>/dev/null

# Update library cache
sudo ldconfig
```

### Undefined Symbols

**Error**: Linker errors about undefined symbols from secp256k1 or other libraries
**Solution**: Ensure all required libraries are specified in the correct order:
```bash
# Link with all required libraries
-lhyperliquid -lcurl -lcjson -lssl -lcrypto -lmsgpackc -lsecp256k1 -lm -lpthread
```

### Header File Issues

**Error**: `fatal error: hyperliquid.h: No such file or directory`
**Solution**: Verify installation and include paths:
```bash
# Check if header exists
ls /usr/local/include/hyperliquid.h

# Specify include path explicitly
-I/usr/local/include
```

### Architecture Mismatch

**Error**: `cannot execute binary file: Exec format error` during cross-compilation
**Solution**: Ensure the cross-compiler is properly installed and target architecture matches:
```bash
# Verify compiler target
x86_64-w64-mingw32-gcc -v
aarch64-linux-gnu-gcc -v
```

### Permission Issues

**Error**: `Permission denied` during installation
**Solution**: Use sudo for system-wide installation:
```bash
# Install with elevated privileges
sudo make install
```

Or install to a user-writable directory:
```bash
# Install to home directory
make install prefix=$HOME/local
```

### Debugging Build Issues

Use the `info` target to display build configuration:
```bash
# Show build configuration
make info
```

This displays the compiler, flags, include paths, and library dependencies being used in the build process, helping to identify configuration issues.

**Section sources**
- [Makefile](file://Makefile#L13)
- [README.md](file://README.md#L216-L230)
- [fix_includes.sh](file://fix_includes.sh)