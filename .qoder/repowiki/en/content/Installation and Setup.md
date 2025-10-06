# Installation and Setup

<cite>
**Referenced Files in This Document**   
- [README.md](file://README.md)
- [Makefile](file://Makefile)
- [QUICKSTART.md](file://QUICKSTART.md)
- [simple_balance.c](file://examples/simple_balance.c)
- [simple_test.c](file://tests/simple_test.c)
</cite>

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Platform-Specific Installation](#platform-specific-installation)
3. [Building the Library](#building-the-library)
4. [Environment Configuration](#environment-configuration)
5. [Linking and Usage](#linking-and-usage)
6. [Troubleshooting Common Issues](#troubleshooting-common-issues)
7. [Verification and Testing](#verification-and-testing)
8. [Configuration Options](#configuration-options)

## Prerequisites

Before installing the hyperliquid-c library, ensure your system has the following dependencies:

- **C Compiler**: GCC or Clang with C99/C11 support
- **Build System**: GNU Make
- **Required Libraries**:
  - `libcurl` - HTTP client library
  - `cJSON` - JSON parsing library
  - `libsecp256k1` - Elliptic curve cryptography for EIP-712 signing
  - `msgpack-c` - MessagePack serialization
  - `pthread` - POSIX threads for concurrency
  - `uuid-dev` - UUID generation (Linux) or `ossp-uuid` (macOS)

These dependencies are essential for the library's core functionality, including HTTP communication, cryptographic operations, data serialization, and thread safety.

**Section sources**
- [README.md](file://README.md#L230-L235)
- [QUICKSTART.md](file://QUICKSTART.md#L10-L25)

## Platform-Specific Installation

### Ubuntu/Debian

Install the required dependencies using APT package manager:

```bash
sudo apt-get update
sudo apt-get install -y \
    libcurl4-openssl-dev \
    libcjson-dev \
    libsecp256k1-dev \
    libmsgpack-dev \
    uuid-dev \
    build-essential
```

The `build-essential` package includes GCC, Make, and other essential build tools. The development packages (ending in `-dev`) provide header files needed for compilation.

### macOS

Using Homebrew, install the dependencies:

```bash
brew install curl cjson libsecp256k1 msgpack ossp-uuid
```

Note that on macOS, the UUID library is provided by `ossp-uuid` rather than the Linux `uuid-dev` package.

### Other Unix-like Systems

#### Arch Linux
```bash
sudo pacman -S curl cjson libsecp256k1 msgpack-c base-devel
```

#### Fedora/RHEL
```bash
sudo dnf install curl-devel json-c-devel libsecp256k1-devel msgpack-devel libuuid-devel gcc make
```

#### Alpine Linux
```bash
apk add curl-dev json-c-dev libsecp256k1-dev msgpack-dev util-linux-dev gcc make musl-dev
```

**Section sources**
- [README.md](file://README.md#L230-L245)
- [QUICKSTART.md](file://QUICKSTART.md#L10-L25)

## Building the Library

The hyperliquid-c library uses a standard Makefile-based build system with multiple targets for different build scenarios.

### Basic Build

Clone the repository and build the library:

```bash
git clone https://github.com/suenot/hyperliquid-c.git
cd hyperliquid-c
make
```

This default target builds both static and shared libraries:
- Static library: `build/lib/libhyperliquid.a`
- Shared library: `build/lib/libhyperliquid.so` (Linux) or `build/lib/libhyperliquid.dll` (Windows)

### Build Targets

The Makefile provides several targets for different purposes:

```bash
make all           # Build static and shared libraries (default)
make debug         # Build with debug symbols and sanitizers
make examples      # Build example programs
make tests         # Build test executables
make test          # Build and run all tests
make install       # Install to /usr/local
make clean         # Remove build artifacts
```

### Debug Build

For development and debugging, use the debug target which includes additional flags:

```bash
make clean debug
```

This enables:
- Debug symbols (`-g`)
- AddressSanitizer and UndefinedBehaviorSanitizer
- Debug assertions
- No optimization (`-O0`)

### Building Examples

The library includes several example programs demonstrating various use cases:

```bash
make examples
```

This compiles all examples in the `examples/` directory to the `build/bin/` directory. Examples include:
- `simple_balance.c` - Account balance retrieval
- `simple_trade.c` - Basic trading operations
- `websocket_demo.c` - WebSocket streaming

**Section sources**
- [Makefile](file://Makefile#L1-L227)
- [README.md](file://README.md#L247-L252)

## Environment Configuration

### Environment Variables

The library supports environment variables for configuration:

```bash
# Testnet credentials
export HYPERLIQUID_TESTNET_WALLET_ADDRESS="0xYourWalletAddress"
export HYPERLIQUID_TESTNET_PRIVATE_KEY="your64charprivatekey"

# Mainnet credentials
export HYPERLIQUID_MAINNET_WALLET_ADDRESS="0xYourWalletAddress" 
export HYPERLIQUID_MAINNET_PRIVATE_KEY="your64charprivatekey"
```

These variables are used when creating a client instance without explicitly passing credentials.

### Client Configuration

When creating a client programmatically, you can specify configuration options:

```c
hl_client_t* client = hl_client_create(
    wallet_address,
    private_key,
    true  // testnet flag
);
```

The third parameter determines whether to use the testnet (`true`) or mainnet (`false`).

**Section sources**
- [QUICKSTART.md](file://QUICKSTART.md#L55-L65)
- [README.md](file://README.md#L550-L565)

## Linking and Usage

### Linking the Library

When compiling applications that use hyperliquid-c, link against the required libraries:

```bash
gcc -o my_app my_app.c \
    -I hyperliquid-c/include \
    -L hyperliquid-c/build/lib \
    -lhyperliquid \
    -lcurl -lcjson -lsecp256k1 -lmsgpackc -lpthread -luuid \
    -lssl -lcrypto
```

### Installing System-Wide

To install the library system-wide for easier linking:

```bash
sudo make install
```

This copies:
- Header files to `/usr/local/include/`
- Libraries to `/usr/local/lib/`
- Updates the dynamic linker cache

After installation, you can link more simply:

```bash
gcc -o my_app my_app.c -lhyperliquid -lcurl -lcjson -lsecp256k1 -lmsgpackc
```

### Basic Usage Example

```c
#include "hyperliquid.h"

int main() {
    // Create client with testnet
    hl_client_t* client = hl_client_create(
        "0xYourWalletAddress",
        "your64charprivatekey",
        true
    );

    if (!client) {
        printf("Failed to create client\n");
        return 1;
    }

    // Use the client for API calls
    // ...

    // Cleanup
    hl_client_destroy(client);
    return 0;
}
```

**Section sources**
- [QUICKSTART.md](file://QUICKSTART.md#L100-L130)
- [Makefile](file://Makefile#L10-L15)

## Troubleshooting Common Issues

### Missing Dependencies

**Error**: `fatal error: curl/curl.h: No such file or directory`

**Solution**: Install the development package for the missing library:
- Ubuntu/Debian: `sudo apt-get install libcurl4-openssl-dev`
- macOS: `brew install curl`

### Linker Errors

**Error**: `undefined reference to 'curl_easy_init'`

**Solution**: Ensure all required libraries are linked in the correct order:
```bash
-lhyperliquid -lcurl -lcjson -lsecp256k1 -lmsgpackc -lpthread -luuid -lssl -lcrypto
```

### Compilation Warnings/Errors

**Error**: Compiler warnings treated as errors (`-Werror`)

**Solution**: Check for strict C99 compliance issues or modify CFLAGS in Makefile if necessary.

### Cryptographic Issues

**Error**: Signature verification failures

**Solution**: 
- Verify private key is 64 hexadecimal characters (without `0x` prefix)
- Ensure `libsecp256k1` is properly installed and linked
- Check wallet address format (must start with `0x`)

### Platform-Specific Issues

**On macOS**: If encountering UUID-related errors:
```bash
brew install ossp-uuid
```

**On Windows (MSYS2)**: Use the MinGW-w64 toolchain:
```bash
pacman -S mingw-w64-x86_64-{curl,jansson,libsecp256k1}
make CC=gcc
```

**Section sources**
- [QUICKSTART.md](file://QUICKSTART.md#L200-L240)
- [README.md](file://README.md#L230-L245)

## Verification and Testing

### Running the Connection Test

Verify the installation by running the simple balance example:

```bash
make examples
./build/bin/example_balance
```

Expected output:
```
🔍 Hyperliquid Balance Example
============================
✅ Client created successfully
📊 Fetching perpetual balance...
✅ Perpetual Balance:
   Account Value: 1000.00 USDC
   Margin Used: 0.00 USDC
   Withdrawable: 1000.00 USDC
   Total Notional: 0.00 USDC
```

### Running Unit Tests

Execute the full test suite to verify library functionality:

```bash
make test
```

This runs all tests and reports:
- Compilation success
- Runtime behavior
- Memory safety (if using debug build)

### Integration Testing

For comprehensive verification, run integration tests that connect to the Hyperliquid API:

```bash
make test-integration
```

Note: This requires valid API credentials in environment variables.

**Section sources**
- [simple_balance.c](file://examples/simple_balance.c#L1-L88)
- [simple_test.c](file://tests/simple_test.c#L1-L36)

## Configuration Options

### Testnet vs Mainnet

The library supports both testnet and mainnet environments:

```c
// Testnet (recommended for development)
hl_client_t* testnet_client = hl_client_create(wallet, key, true);

// Mainnet (for live trading)
hl_client_t* mainnet_client = hl_client_create(wallet, key, false);
```

**Important**: Always test thoroughly on testnet before using mainnet credentials.

### Client Options

Additional client configuration can be managed through environment variables or direct parameters:

- **Timeout**: Default 30 seconds, configurable via client structure
- **Debug Mode**: Enabled in debug builds
- **Rate Limiting**: Built-in to respect API limits

### Build Configuration

The Makefile supports platform detection and conditional compilation:

- Automatic shared library extension selection (`.so` vs `.dll`)
- Compiler flag optimization
- Dependency path configuration via `INCLUDES` and `LIBS` variables

**Section sources**
- [QUICKSTART.md](file://QUICKSTART.md#L55-L65)
- [README.md](file://README.md#L299-L304)