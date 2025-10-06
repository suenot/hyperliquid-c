# Private Key Management

<cite>
**Referenced Files in This Document**   
- [client.c](file://src/client.c)
- [client_new.c](file://src/client_new.c)
- [hl_client.h](file://include/hl_client.h)
- [hyperliquid.h](file://include/hyperliquid.h)
- [eip712.c](file://src/crypto/eip712.c)
- [hl_crypto_internal.h](file://include/hl_crypto_internal.h)
</cite>

## Table of Contents
1. [Private Key Ingestion and Validation](#private-key-ingestion-and-validation)
2. [Secure Memory Handling](#secure-memory-handling)
3. [Thread Safety and Access Control](#thread-safety-and-access-control)
4. [Best Practices for External Key Storage](#best-practices-for-external-key-storage)
5. [Security Pitfalls and Mitigations](#security-pitfalls-and-mitigations)
6. [Secure Deployment and Key Lifecycle](#secure-deployment-and-key-lifecycle)

## Private Key Ingestion and Validation

The private key ingestion process begins with the `hl_client_create()` function, which serves as the entry point for client initialization and credential handling. During this phase, both the wallet address and private key undergo strict validation to ensure correct formatting and integrity.

The wallet address is validated to confirm it is at least 42 characters long and begins with the "0x" prefix, consistent with Ethereum address standards. The private key is checked for length compliance, accepting either 64 characters (raw hex) or 66 characters (including "0x" prefix). Inputs that do not meet these criteria result in immediate function termination and return of a NULL pointer, preventing the creation of an improperly initialized client.

If validation passes, the "0x" prefix is stripped from both the wallet address and private key before storage. This normalization ensures consistent internal representation and avoids redundancy in subsequent cryptographic operations. The cleaned credentials are then copied into fixed-size buffers within the `hl_client_t` structure using `strncpy()` to prevent buffer overflows.

**Section sources**
- [client.c](file://src/client.c#L34-L87)

## Secure Memory Handling

Private keys are stored in memory using fixed-size character arrays within the `hl_client_t` structure: `private_key[65]` for legacy implementation and `secret[128]` in the CCXT-compatible version. These buffers are allocated via `calloc()`, which ensures zero-initialization and prevents leakage of residual memory contents.

Upon client destruction via `hl_client_destroy()` or `hl_client_free()`, the private key buffer is explicitly zeroed using `memset()` before the client structure is freed. This practice ensures that sensitive cryptographic material is not left in memory after use, reducing the risk of exposure through memory dumps or inspection.

The library avoids logging or printing private key values at any point in its execution. Debugging output, if enabled, is carefully controlled to exclude sensitive data. Additionally, temporary buffers used during cryptographic operations—such as those in `ecdsa_sign_secp256k1()`—are stack-allocated and automatically cleared upon function return, minimizing exposure window.

While the current implementation does not explicitly allocate non-pageable memory (e.g., using `mlock()`), doing so would further protect against swap-based leaks on systems where virtual memory is used. Future enhancements could incorporate such mechanisms for environments requiring higher security assurance.

**Section sources**
- [client.c](file://src/client.c#L170-L175)
- [client_new.c](file://src/client_new.c#L25-L35)
- [client_new.c](file://src/client_new.c#L47-L52)

## Thread Safety and Access Control

To ensure thread-safe access to client credentials and internal state, the library employs `pthread_mutex_t` for synchronization. In the legacy client implementation (`client.c`), a mutex is initialized during `hl_client_create()` and destroyed during `hl_client_destroy()`. This mutex protects all operations that access or modify the client’s state, including HTTP requests and internal data updates.

The mutex is explicitly locked before any critical section and unlocked afterward. For example, during connection testing in `hl_test_connection()`, the mutex is acquired before making the HTTP request and released afterward, ensuring that concurrent calls do not interfere with each other or expose inconsistent state.

In the newer `client_new.c` implementation, while the direct mutex is not exposed, thread safety is maintained through internal wrappers and design patterns that prevent race conditions during HTTP and WebSocket operations. However, external synchronization may still be required when multiple threads access the same client instance, especially during signing operations that involve the private key.

This use of mutexes prevents concurrent access to sensitive data and ensures atomicity of operations involving credentials, thereby maintaining integrity and confidentiality in multi-threaded environments.

**Section sources**
- [client.c](file://src/client.c#L70-L75)
- [client.c](file://src/client.c#L115-L137)

## Best Practices for External Key Storage

While the library securely handles private keys once provided, it is critical that keys are sourced securely from external systems. Storing private keys in plaintext configuration files or environment variables is discouraged due to risks of exposure through logs, version control, or process inspection.

Preferred methods include:
- **Hardware Security Modules (HSMs)**: Offload key storage and signing operations to dedicated hardware, ensuring private keys never reside in application memory.
- **Encrypted Key Stores**: Use encrypted storage backends (e.g., AWS KMS, Hashicorp Vault) to retrieve keys at runtime, decrypting them only when needed and zeroing memory immediately after use.
- **Environment-Based Injection**: Inject keys via secure orchestration systems (e.g., Kubernetes secrets) without persisting them on disk.

The library supports dependency injection of credentials via function parameters, enabling integration with external key management systems. Applications should avoid hardcoding keys and instead retrieve them through secure channels at initialization time.

Additionally, consider using ephemeral key derivation or session tokens where supported by the exchange, reducing the lifetime and scope of long-term private key exposure.

**Section sources**
- [client.c](file://src/client.c#L34-L87)
- [client_new.c](file://src/client_new.c#L25-L35)

## Security Pitfalls and Mitigations

Several common security pitfalls could compromise private key integrity if not properly addressed:

- **Memory Dumps**: Process memory containing private keys can be exposed through crash dumps or debugging tools. Mitigation includes zeroing buffers after use and avoiding unnecessary logging.
- **Debugging Exposure**: Debug builds or verbose logging may inadvertently print sensitive data. The library disables debug output by default and avoids logging credentials.
- **Improper Error Handling**: Errors during key parsing or cryptographic operations must not leak information about the key. The library returns generic error codes and avoids detailed diagnostic messages that could aid attackers.
- **Stack Overflow and Buffer Overruns**: Fixed-size buffers and use of `strncpy()` prevent overflows, but input validation remains essential to reject malformed inputs early.

Cryptographic functions such as `ecdsa_sign_secp256k1()` perform internal validation of the private key format and reject invalid inputs, preventing the use of weak or malformed keys. The use of deterministic signing (RFC 6979) eliminates risks associated with poor random number generation during signature creation.

**Section sources**
- [eip712.c](file://src/crypto/eip712.c#L88-L180)
- [hl_crypto_internal.h](file://include/hl_crypto_internal.h#L79-L83)

## Secure Deployment and Key Lifecycle

For secure deployment, applications should follow a zero-trust model:
- Keys should be injected at startup and never written to disk.
- Processes should run with minimal privileges to limit access to memory and system resources.
- Audit logging should record key usage (e.g., signing events) without capturing the key material itself.

Key rotation should be supported by the application layer, allowing clients to be reinitialized with new credentials without restarting the entire system. The library’s stateless design facilitates this by encapsulating credentials within the client object, enabling multiple clients with different keys to coexist.

Audit trails should log metadata such as timestamp, operation type, and outcome, enabling detection of unauthorized access or misuse. Integration with centralized logging systems (with sensitive fields redacted) supports compliance and forensic analysis.

By combining secure coding practices with robust operational procedures, developers can ensure that private key management remains resilient against both technical and procedural threats.

**Section sources**
- [client.c](file://src/client.c#L34-L87)
- [client_new.c](file://src/client_new.c#L25-L52)