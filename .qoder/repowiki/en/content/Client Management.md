# Client Management

<cite>
**Referenced Files in This Document**   
- [client.c](file://src/client.c)
- [client_new.c](file://src/client_new.c)
- [hl_client.h](file://include/hl_client.h)
- [hyperliquid.h](file://include/hyperliquid.h)
- [hl_http.h](file://include/hl_http.h)
</cite>

## Table of Contents
1. [Client Creation and Configuration](#client-creation-and-configuration)
2. [Resource Cleanup and Destruction](#resource-cleanup-and-destruction)
3. [Connection Testing and Health Checks](#connection-testing-and-health-checks)
4. [Testnet vs Mainnet Configuration](#testnet-vs-mainnet-configuration)
5. [Error Handling During Initialization](#error-handling-during-initialization)
6. [Opaque Pointer Pattern and ABI Stability](#opaque-pointer-pattern-and-abi-stability)
7. [Thread-Safety Considerations](#thread-safety-considerations)

## Client Creation and Configuration

The Hyperliquid C SDK provides a robust client creation mechanism through the `hl_client_create()` function, which initializes a client instance with essential configuration parameters for API interaction. This function requires three parameters: a wallet address, a private key, and a boolean flag indicating testnet usage. The wallet address must be a valid Ethereum address in hexadecimal format starting with "0x", and the private key must be either 64 or 66 characters long (with or without the "0x" prefix). The function performs validation on these inputs and returns NULL if validation fails.

The client creation process allocates memory for the `hl_client_t` structure and initializes several critical components. By default, the HTTP request timeout is set to 30,000 milliseconds (30 seconds), providing a reasonable window for network operations. The client automatically configures the appropriate API endpoint based on the testnet parameter: "https://api.hyperliquid-testnet.xyz" for testnet operations and "https://api.hyperliquid.xyz" for mainnet operations. This endpoint selection is handled internally, ensuring that users don't need to manually specify URLs.

The client structure includes fields for storing the wallet address and private key (with the "0x" prefix automatically stripped if present), the testnet flag, timeout configuration, and debugging status. Additionally, a mutex is initialized during client creation to ensure thread safety for subsequent operations. The HTTP client component is also created during this process, establishing the foundation for all API communications. This comprehensive initialization ensures that the client is fully configured and ready for immediate use upon successful creation.

**Section sources**
- [client.c](file://src/client.c#L34-L87)
- [hyperliquid.h](file://include/hyperliquid.h#L154-L156)

## Resource Cleanup and Destruction

Proper resource cleanup is essential for maintaining application stability and preventing memory leaks when using the Hyperliquid C SDK. The `hl_client_destroy()` function provides a comprehensive cleanup mechanism that systematically releases all resources allocated during client creation. This function accepts a pointer to the client instance and safely handles NULL inputs, making it safe to call even if client creation failed.

The destruction process follows a specific sequence to ensure proper cleanup. First, the HTTP client component is destroyed, terminating any active connections and freeing associated network resources. Next, the private key stored in memory is securely zeroed out before the client structure is freed, providing an additional layer of security by preventing sensitive information from remaining in memory after the client is destroyed. The mutex initialized during client creation is then properly destroyed to release any system resources it may be holding.

Finally, the memory allocated for the client structure itself is freed using the standard `free()` function. This complete cleanup process ensures that no resources are left dangling after the client is destroyed. It's important to note that after calling `hl_client_destroy()`, the client pointer becomes invalid and should not be used for any subsequent operations. The function is designed to be idempotent with respect to NULL pointers, allowing for safe error handling in initialization code paths.

**Section sources**
- [client.c](file://src/client.c#L89-L107)
- [hyperliquid.h](file://include/hyperliquid.h#L163-L163)

## Connection Testing and Health Checks

The Hyperliquid C SDK includes a built-in connection testing mechanism through the `hl_test_connection()` function, which allows applications to verify connectivity to the Hyperliquid API before executing critical operations. This function performs a lightweight HTTP GET request to the `/info` endpoint of the Hyperliquid API, which returns basic exchange information without requiring authentication. The function returns a boolean value indicating whether the connection test was successful.

The connection test is implemented with proper thread safety in mind, using the client's mutex to ensure that the test operation is atomic and doesn't interfere with other concurrent operations on the same client instance. The test constructs the appropriate URL based on the client's testnet configuration, ensuring that the connection check is performed against the correct environment (testnet or mainnet). A successful connection test requires both a successful HTTP request and a 200 OK status code from the server.

This health check functionality is particularly valuable in production environments where network conditions may vary, or when implementing retry logic for transient network failures. Applications can use this function to implement connection resilience strategies, such as retrying failed connections or alerting users to network issues before attempting trading operations that require authentication and may have financial implications. The lightweight nature of this check makes it suitable for frequent polling if needed for monitoring purposes.

**Section sources**
- [client.c](file://src/client.c#L109-L140)
- [hyperliquid.h](file://include/hyperliquid.h#L171-L171)

## Testnet vs Mainnet Configuration

The Hyperliquid C SDK provides straightforward configuration for operating in either testnet or mainnet environments through a single boolean parameter in the `hl_client_create()` function. When the `testnet` parameter is set to `true`, the client automatically configures itself to communicate with the Hyperliquid testnet environment, using the base URL "https://api.hyperliquid-testnet.xyz". When set to `false`, the client uses the production mainnet URL "https://api.hyperliquid.xyz".

This configuration affects all subsequent API operations performed through the client instance, ensuring that trading, market data retrieval, and account operations are directed to the appropriate environment. The testnet environment is designed for development, testing, and integration purposes, allowing developers to experiment with the API without risking real funds. In contrast, the mainnet environment handles real trading with actual financial assets.

The client maintains the testnet configuration throughout its lifecycle, and this setting cannot be changed after client creation. This design ensures that operations remain consistent within a single client instance and prevents accidental cross-environment operations. Developers should create separate client instances when they need to interact with both testnet and mainnet environments simultaneously. The clear separation between environments helps prevent common development mistakes, such as accidentally placing real trades while intending to test functionality.

**Section sources**
- [client.c](file://src/client.c#L34-L87)
- [client.c](file://src/client.c#L109-L140)

## Error Handling During Initialization

The client initialization process in the Hyperliquid C SDK incorporates comprehensive error handling to ensure robust operation and provide meaningful feedback when issues occur. The `hl_client_create()` function employs multiple validation checks and returns NULL to indicate failure, allowing callers to handle initialization errors appropriately. The function validates both the wallet address and private key parameters, checking for NULL pointers, proper format (wallet address starting with "0x"), minimum length requirements, and valid private key length (64 or 66 characters).

When validation fails, the function immediately returns NULL without allocating any resources, preventing memory leaks in error conditions. If validation passes but memory allocation fails during client structure creation, the function also returns NULL. Similarly, if the mutex initialization fails, the allocated client structure is freed, and NULL is returned. This systematic approach to error handling ensures that the function is exception-safe and that no resources are leaked when initialization fails.

Applications should always check the return value of `hl_client_create()` before using the client instance. A common pattern is to use the initialization within a conditional statement and provide appropriate error messaging or fallback behavior when client creation fails. This defensive programming approach helps create more resilient applications that can gracefully handle configuration errors, network issues, or system resource constraints during startup.

**Section sources**
- [client.c](file://src/client.c#L34-L87)
- [hyperliquid.h](file://include/hyperliquid.h#L154-L156)

## Opaque Pointer Pattern and ABI Stability

The Hyperliquid C SDK employs the opaque pointer pattern in its API design, which is evident in the declaration of the `hl_client_t` type as a forward-declared struct in the public headers. This design approach hides the internal implementation details of the client structure from users of the library, exposing only a pointer to the structure through the public API. The actual structure definition is contained within the implementation files, preventing direct access to its fields.

This pattern provides significant benefits for ABI (Application Binary Interface) stability, allowing the library maintainers to modify the internal structure of the client without breaking compatibility with existing compiled applications. As long as the public API functions continue to accept and return `hl_client_t*` pointers and maintain their signatures, changes to the internal fields, their order, or additional functionality can be implemented without requiring recompilation of client code.

The opaque pointer pattern also enforces encapsulation, ensuring that all interactions with the client occur through well-defined API functions rather than direct field manipulation. This allows the library to maintain invariants, perform validation, and manage resources properly. For example, when the private key is stored in the client structure, the library can ensure it's properly secured and zeroed out during cleanup, which would be difficult to guarantee if users could directly access the structure fields.

**Section sources**
- [hl_client.h](file://include/hl_client.h#L14-L14)
- [client.c](file://src/client.c#L14-L42)

## Thread-Safety Considerations

The Hyperliquid C SDK incorporates thread-safety features to support concurrent access to client instances from multiple threads. The primary mechanism for ensuring thread safety is the inclusion of a pthread mutex within the client structure, which is initialized during client creation and destroyed during cleanup. This mutex is used to protect critical sections of code that access shared client state, preventing race conditions when multiple threads attempt to use the same client instance simultaneously.

The connection test function (`hl_test_connection()`) demonstrates the use of this mutex by explicitly locking and unlocking it around the HTTP request operation, ensuring that the test is performed atomically. This design allows multiple threads to safely call client functions without external synchronization, as the library handles the necessary locking internally. However, developers should still consider the performance implications of contention when multiple threads frequently access the same client instance.

While the client provides internal synchronization for its own operations, applications should be aware that excessive concurrent usage of a single client instance may lead to performance bottlenecks due to mutex contention. In high-concurrency scenarios, it may be more efficient to use multiple client instances or implement additional application-level queuing mechanisms. The thread-safety features ensure correctness but do not eliminate the need for thoughtful concurrency design in performance-critical applications.

**Section sources**
- [client.c](file://src/client.c#L34-L87)
- [client.c](file://src/client.c#L89-L107)
- [client.c](file://src/client.c#L109-L140)