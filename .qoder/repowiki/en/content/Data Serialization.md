# Data Serialization

<cite>
**Referenced Files in This Document**   
- [hl_msgpack.h](file://include/hl_msgpack.h)
- [serialize.c](file://src/msgpack/serialize.c)
- [hl_crypto_internal.h](file://include/hl_crypto_internal.h)
</cite>

## Table of Contents
1. [Field Ordering Requirements](#field-ordering-requirements)
2. [Serialization Process](#serialization-process)
3. [Type Mapping](#type-mapping)
4. [Manual Serialization Examples](#manual-serialization-examples)
5. [Endianness and Cross-Platform Compatibility](#endianness-and-cross-platform-compatibility)
6. [Optional Fields and Versioning](#optional-fields-and-versioning)
7. [Performance Characteristics](#performance-characteristics)

## Field Ordering Requirements

MessagePack serialization in the Hyperliquid C client requires strict field ordering to ensure byte-perfect compatibility with official SDKs. The order of keys in maps must match exactly across implementations to produce identical binary outputs, which is critical for cryptographic hashing and signature validation.

For order payloads, fields must be serialized in alphabetical order: `"a"` (asset ID), `"b"` (is buy), `"p"` (price), `"s"` (size), `"r"` (reduce only), `"t"` (order type). Notably, `"s"` must precede `"r"` despite their proximity in the alphabet, as confirmed by code comments and STATUS.md documentation. This ordering ensures compatibility with the Go SDK's serialization behavior.

Action envelopes follow a fixed insertion order: `"type"` must be first, followed by `"orders"` or `"cancels"`, then `"grouping"` for order actions. This sequence is enforced in `pack_order_action` and `pack_cancel_action` functions to maintain consistency with CCXT formatting expectations.

**Section sources**
- [hl_msgpack.h](file://include/hl_msgpack.h#L27-L34)
- [serialize.c](file://src/msgpack/serialize.c#L45-L131)
- [STATUS.md](file://STATUS.md#L159-L178)

## Serialization Process

The serialization process for Hyperliquid actions involves multiple layers: struct packing, MessagePack encoding, nonce and vault address appending, and Keccak256 hashing. The core functions `hl_build_action_hash`, `hl_build_order_hash`, and `hl_build_cancel_hash` orchestrate this workflow.

Order payloads are serialized through nested packing functions. The `pack_order` function handles individual orders, writing each field in the required order using the MessagePack C library. Limit order parameters are encapsulated in a nested map under the `"limit"` key, containing the `"tif"` (time-in-force) field.

Action envelopes are constructed as flat maps. Order actions contain three top-level keys: `"type"`, `"orders"` (an array of serialized orders), and `"grouping"`. Cancel actions contain `"type"` and `"cancels"` (an array of asset/order ID pairs). After MessagePack serialization, the binary output is combined with a big-endian nonce and optional vault address before hashing.

**Section sources**
- [serialize.c](file://src/msgpack/serialize.c#L45-L234)
- [hl_msgpack.h](file://include/hl_msgpack.h#L74-L116)

## Type Mapping

The C implementation maps Hyperliquid API types to MessagePack data types through a structured type system defined in `hl_msgpack.h`. Field names are abbreviated to single characters for efficiency, mirroring the exchange's wire format.

| C Type | MessagePack Type | Field Description |
|--------|------------------|-------------------|
| `uint32_t` | integer (unsigned) | Asset ID (`a`) |
| `bool` | boolean | Buy/sell direction (`b`), reduce-only flag (`r`) |
| `const char*` | string | Price (`p`), size (`s`) as decimal strings |
| `hl_limit_t` | map | Nested limit order configuration |
| `hl_order_t[]` | array of maps | Collection of orders |
| `uint64_t` | integer (unsigned) | Order ID (`o`) in cancel requests |

Price and size values are transmitted as strings rather than floating-point numbers to avoid precision loss and ensure exact decimal representation. Boolean values are encoded as MessagePack's native true/false types. The `limit` field is a nested map containing the `"tif"` string value.

**Section sources**
- [hl_msgpack.h](file://include/hl_msgpack.h#L27-L51)
- [serialize.c](file://src/msgpack/serialize.c#L10-L43)

## Manual Serialization Examples

Manual serialization is supported through the public API for debugging and testing purposes. The `hl_build_order_hash` and `hl_build_cancel_hash` functions provide convenient entry points for generating action hashes without direct MessagePack manipulation.

To manually serialize an order action:
1. Populate an `hl_order_t` array with trading parameters
2. Set grouping to `"na"` for normal orders
3. Call `hl_build_order_hash` with the order array, count, grouping, nonce, and vault address
4. The function returns a 32-byte Keccak256 hash in the output buffer

For cancel actions:
1. Create an `hl_cancel_t` array with asset IDs and order IDs
2. Call `hl_build_cancel_hash` with the cancel array, count, nonce, and vault address
3. Verify the returned hash matches expected values

These functions handle all serialization details internally, allowing developers to focus on business logic while ensuring protocol compliance.

**Section sources**
- [hl_msgpack.h](file://include/hl_msgpack.h#L93-L116)
- [serialize.c](file://src/msgpack/serialize.c#L206-L232)

## Endianness and Cross-Platform Compatibility

The serialization layer ensures cross-platform compatibility through explicit endianness handling. While MessagePack itself is endian-neutral for most types, the nonce (timestamp) is appended as a big-endian uint64_t to maintain consistency across architectures.

The `hl_build_action_hash` function explicitly converts the 64-bit nonce to big-endian byte order using bit shifting:
```c
for (int i = 0; i < 8; i++) {
    data[sbuf.size + i] = (nonce >> (56 - i * 8)) & 0xFF;
}
```
This ensures the same binary representation on both little-endian (x86) and big-endian systems.

String fields (price, size, TIF) are encoded with their length prefix followed by UTF-8 bytes, which is platform-independent. Integer fields use MessagePack's variable-length encoding that is endian-neutral by design. The final Keccak256 hash computation operates on the byte stream, producing identical output regardless of host architecture.

**Section sources**
- [serialize.c](file://src/msgpack/serialize.c#L160-L170)
- [hl_crypto_internal.h](file://include/hl_crypto_internal.h#L80)

## Optional Fields and Versioning

The current implementation handles optional fields through conditional logic in the serialization layer. The vault address is the primary optional field, indicated by a null pointer or empty string. When present, it is prefixed with `0x01` and included in the hash input; otherwise, `0x00` is appended.

Versioning is managed implicitly through the action structure definitions. The fixed field ordering and naming convention provide backward compatibility, while new features can be introduced through:
1. Additional fields in existing structs (appended to maintain ordering)
2. New action types beyond "order" and "cancel"
3. Extended grouping strategies

The `hl_build_action_hash` function supports extensibility by accepting a string-based action type parameter, allowing new action types to be added without changing the core interface. However, any new fields must be integrated into the packing functions with attention to the required ordering constraints.

**Section sources**
- [serialize.c](file://src/msgpack/serialize.c#L180-L195)
- [hl_msgpack.h](file://include/hl_msgpack.h#L74-L78)

## Performance Characteristics

The serialization layer is optimized for performance with several key characteristics:

- **Memory Usage**: Temporary buffers are allocated on the heap for the combined MessagePack + nonce + address data. The buffer size is calculated as `sbuf.size + 8 + (vault_address ? 21 : 1)`, minimizing over-allocation.
- **Time Complexity**: O(n) for the number of orders or cancels, with constant-time operations for field packing.
- **Allocation Pattern**: Single malloc/free pair per hash operation, reducing fragmentation.
- **Zero-Copy Optimization**: MessagePack uses direct writes to the sbuffer, avoiding intermediate copies.

The use of abbreviated field names and string-based numeric values reduces serialized size compared to verbose JSON formats. The entire process is synchronous and blocking, designed for predictable performance in trading applications where latency matters.

**Section sources**
- [serialize.c](file://src/msgpack/serialize.c#L145-L155)
- [hl_msgpack.h](file://include/hl_msgpack.h#L74-L116)