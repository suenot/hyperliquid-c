# Cryptographic Operations

<cite>
**Referenced Files in This Document**   
- [eip712.c](file://src/crypto/eip712.c)
- [keccak.c](file://src/crypto/keccak.c)
- [hl_crypto_internal.h](file://include/hl_crypto_internal.h)
- [test_crypto_msgpack.c](file://tests/unit/test_crypto_msgpack.c)
- [serialize.c](file://src/msgpack/serialize.c)
</cite>

## Table of Contents
1. [EIP-712 Signing Implementation](#eip-712-signing-implementation)
2. [Keccak-256 Hash Computation](#keccak-256-hash-computation)
3. [ECDSA Signature Generation](#ecdsa-signature-generation)
4. [Private Key Handling](#private-key-handling)
5. [Signature Verification](#signature-verification)
6. [Security Considerations](#security-considerations)
7. [Test Vectors](#test-vectors)

## EIP-712 Signing Implementation

The EIP-712 signing implementation provides order authentication through structured data hashing and signing. The domain separator is constructed using the EIP-712 standard with specific parameters for the Hyperliquid exchange. The domain hash incorporates the domain name "Exchange", chain ID 1337, version "1", and a zero verifying contract address. This creates a unique context for all signatures, preventing replay attacks across different domains.

The Agent struct hashing follows the EIP-712 specification with the type definition "Agent(string source,bytes32 connectionId)". The source parameter identifies the trading environment (e.g., "a" for mainnet, "b" for testnet), while the connectionId is derived from the action hash. The struct hash is computed by concatenating the Agent typehash, keccak256 hash of the source string, and the connectionId, then applying keccak256 to the concatenated data.

The complete signing hash is constructed according to EIP-191 by prefixing the concatenation of domain hash and struct hash with the bytes 0x19 and 0x01. This ensures compatibility with Ethereum's signing standards while providing domain separation.

**Section sources**
- [eip712.c](file://src/crypto/eip712.c#L296-L297)
- [hl_crypto_internal.h](file://include/hl_crypto_internal.h#L85-L86)

## Keccak-256 Hash Computation

Keccak-256 hash computation is implemented using the SHA3IUF library with the SHA3_FLAGS_KECCAK flag to ensure Ethereum-style hashing rather than NIST SHA-3. This implementation produces identical results to the Go SDK and other Ethereum-compatible systems. The hash function is used throughout the protocol for various purposes including address derivation, action hashing, and message integrity verification.

The implementation in keccak.c provides a canonical 64-bit optimized version of the Keccak algorithm with proper padding rules. The hash function processes input data in blocks, maintaining internal state that is transformed through the Keccak-f permutation function. The finalization step applies domain separation padding (0x01 followed by 0x00* and ending with 0x80) to distinguish Keccak instances.

**Section sources**
- [keccak.c](file://src/crypto/keccak.c#L322-L323)
- [hl_crypto_internal.h](file://include/hl_crypto_internal.h#L78-L79)

## ECDSA Signature Generation

ECDSA signature generation is implemented using the libsecp256k1 library, which provides production-grade elliptic curve cryptography. The implementation uses deterministic signatures according to RFC 6979, ensuring that the same message and private key always produce the same signature, eliminating the risk of private key exposure due to poor random number generation.

The signature process involves creating a secp256k1 context with both signing and verification capabilities, then using the ecdsa_sign_recoverable function to generate a recoverable signature. The compact signature format is serialized to extract the r and s components, which are copied to the output buffer. The recovery ID (v value) is calculated by comparing the recovered public key against the actual public key derived from the private key.

**Section sources**
- [eip712.c](file://src/crypto/eip712.c#L127-L144)
- [hl_crypto_internal.h](file://include/hl_crypto_internal.h#L83-L84)

## Private Key Handling

Private key import follows the standard hexadecimal format, with optional "0x" prefix support. The hex_to_bytes function converts the hexadecimal string representation to binary format, performing validation on the input length and character set. The function ensures the private key is exactly 32 bytes, which is the required length for secp256k1 private keys.

Secure handling practices include immediate parsing of the private key into binary format upon receipt, minimizing the time the key exists in string form. The implementation avoids storing private keys in global variables or logging them to any output stream. Memory management ensures that private key material is properly zeroed after use to prevent exposure through memory dumps.

**Section sources**
- [eip712.c](file://src/crypto/eip712.c#L56-L87)
- [hl_crypto_internal.h](file://include/hl_crypto_internal.h#L80-L81)

## Signature Verification

Manual signature verification for debugging purposes can be performed by reconstructing the signing process and comparing results. The process involves computing the domain hash, struct hash, and signing hash exactly as done during signature generation. The recovered public key can be compared against the expected public key derived from the private key.

The recovery ID calculation is critical for proper signature verification. The implementation tries all four possible recovery IDs (0-3) and selects the one that recovers to the correct Ethereum address. The v value in the final signature is set to recovery_id + 27, following Ethereum conventions. This ensures compatibility with Ethereum wallet implementations and verification tools.

**Section sources**
- [eip712.c](file://src/crypto/eip712.c#L144-L175)
- [test_crypto_msgpack.c](file://tests/unit/test_crypto_msgpack.c#L85-L118)

## Security Considerations

Side-channel attack mitigations are implemented through the use of the libsecp256k1 library, which includes protections against timing attacks and other side-channel vulnerabilities. The deterministic signature generation (RFC 6979) eliminates the risk of private key exposure due to entropy failure.

Secure memory management for sensitive data follows best practices by zeroing private key material after use and avoiding unnecessary copies of sensitive data in memory. The secp256k1 context is properly destroyed after use to ensure any sensitive material within the library's internal state is cleared.

The implementation avoids using OpenSSL's ECDSA implementation, which has different signature formatting and potential security issues, in favor of the well-audited libsecp256k1 library used by Bitcoin Core.

**Section sources**
- [eip712.c](file://src/crypto/eip712.c#L88-L175)
- [keccak.c](file://src/crypto/keccak.c#L1-L323)

## Test Vectors

Test vectors are provided in the unit test suite to validate cryptographic correctness. These include tests for Keccak-256 hashing, EIP-712 domain hashing, MessagePack serialization, and full EIP-712 signature generation. The test vectors ensure compatibility with the Go SDK by verifying byte-for-byte identical output.

The test suite verifies that field ordering in MessagePack serialization matches the reference implementation, with critical ordering requirements such as price before size before reduce_only in order objects, and type as the first field in action objects. The tests also validate that the recovery ID calculation produces the correct v value that recovers to the expected Ethereum address.

**Section sources**
- [test_crypto_msgpack.c](file://tests/unit/test_crypto_msgpack.c#L17-L150)
- [serialize.c](file://src/msgpack/serialize.c#L23-L69)