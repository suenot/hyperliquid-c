#include "hl_crypto_internal.h"
#include "hl_logger.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

// Use libsecp256k1 for production-grade ECDSA with correct recovery ID
#include <secp256k1.h>
#include <secp256k1_recovery.h>

// Keccak256 using SHA3IUF library
#include "hl_crypto_internal.h"

int keccak256(const uint8_t *input, size_t input_len, uint8_t output[32]) {
    // Use SHA3_FLAGS_KECCAK for Ethereum-style Keccak (not NIST SHA3)
    sha3_return_t result = sha3_HashBuffer(256, SHA3_FLAGS_KECCAK, 
                                           input, input_len, 
                                           output, 32);
    return (result == SHA3_RETURN_OK) ? 0 : -1;
}

int hex_to_bytes(const char *hex, uint8_t *bytes_out, size_t max_out_len) {
    if (!hex || !bytes_out) {
        return -1;
    }
    
    // Skip "0x" prefix if present
    if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
        hex += 2;
    }
    
    size_t hex_len = strlen(hex);
    if (hex_len % 2 != 0) {
        fprintf(stderr, "Hex string must have even length\n");
        return -1;
    }
    
    size_t byte_len = hex_len / 2;
    if (byte_len > max_out_len) {
        fprintf(stderr, "Output buffer too small for hex string\n");
        return -1;
    }
    
    for (size_t i = 0; i < byte_len; i++) {
        char high = hex[i * 2];
        char low = hex[i * 2 + 1];
        
        int h = (high >= '0' && high <= '9') ? (high - '0') :
                (high >= 'a' && high <= 'f') ? (high - 'a' + 10) :
                (high >= 'A' && high <= 'F') ? (high - 'A' + 10) : -1;
        
        int l = (low >= '0' && low <= '9') ? (low - '0') :
                (low >= 'a' && low <= 'f') ? (low - 'a' + 10) :
                (low >= 'A' && low <= 'F') ? (low - 'A' + 10) : -1;
        
        if (h < 0 || l < 0) {
            fprintf(stderr, "Invalid hex character\n");
            return -1;
        }
        
        bytes_out[i] = (h << 4) | l;
    }
    
    return (int)byte_len;
}

void bytes_to_hex(const uint8_t *bytes, size_t len, char *hex_out, bool add_prefix) {
    const char hex_chars[] = "0123456789abcdef";
    size_t offset = 0;
    
    if (add_prefix) {
        hex_out[0] = '0';
        hex_out[1] = 'x';
        offset = 2;
    }
    
    for (size_t i = 0; i < len; i++) {
        hex_out[offset + i * 2] = hex_chars[bytes[i] >> 4];
        hex_out[offset + i * 2 + 1] = hex_chars[bytes[i] & 0x0F];
    }
    hex_out[offset + len * 2] = '\0';
}

int parse_eth_address(const char *address_hex, uint8_t address_out[20]) {
    return hex_to_bytes(address_hex, address_out, 20) == 20 ? 0 : -1;
}

int ecdsa_sign_secp256k1(const uint8_t hash[32], 
                         const char *private_key_hex,
                         uint8_t signature_out[65]) {
    // Parse private key
    uint8_t priv_bytes[32];
    if (hex_to_bytes(private_key_hex, priv_bytes, 32) != 32) {
        fprintf(stderr, "Failed to parse private key\n");
        return -1;
    }
    
    // Create secp256k1 context
    secp256k1_context *ctx = secp256k1_context_create(
        SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY
    );
    if (!ctx) {
        fprintf(stderr, "Failed to create secp256k1 context\n");
        return -1;
    }
    
    // Sign with recovery ID (RFC 6979 deterministic)
    secp256k1_ecdsa_recoverable_signature sig;
    if (!secp256k1_ecdsa_sign_recoverable(ctx, &sig, hash, priv_bytes, NULL, NULL)) {
        fprintf(stderr, "Failed to sign with secp256k1\n");
        secp256k1_context_destroy(ctx);
        return -1;
    }
    
    // Serialize signature
    int recovery_id;
    uint8_t compact_sig[64];
    secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, compact_sig, &recovery_id, &sig);
    
    // Copy r and s
    memcpy(signature_out, compact_sig, 64);
    
    // Get the actual public key from private key
    secp256k1_pubkey actual_pubkey;
    if (!secp256k1_ec_pubkey_create(ctx, &actual_pubkey, priv_bytes)) {
        fprintf(stderr, "Failed to compute public key\n");
        secp256k1_context_destroy(ctx);
        return -1;
    }
    
    // Convert to uncompressed format to get Ethereum address
    uint8_t actual_pubkey_bytes[65];
    size_t actual_pubkey_len = 65;
    secp256k1_ec_pubkey_serialize(ctx, actual_pubkey_bytes, &actual_pubkey_len, 
                                  &actual_pubkey, SECP256K1_EC_UNCOMPRESSED);
    
    // Compute actual Ethereum address (last 20 bytes of keccak256(pubkey[1:]))
    uint8_t actual_addr_hash[32];
    keccak256(actual_pubkey_bytes + 1, 64, actual_addr_hash);
    
    // Now try all 4 recovery IDs and find which one matches our actual address
    int correct_recovery_id = -1;
    for (int recid = 0; recid < 4; recid++) {
        secp256k1_ecdsa_recoverable_signature test_sig;
        if (!secp256k1_ecdsa_recoverable_signature_parse_compact(ctx, &test_sig, compact_sig, recid)) {
            continue;
        }
        
        secp256k1_pubkey recovered_pubkey;
        if (!secp256k1_ecdsa_recover(ctx, &recovered_pubkey, &test_sig, hash)) {
            continue;
        }
        
        uint8_t recovered_pubkey_bytes[65];
        size_t recovered_pubkey_len = 65;
        secp256k1_ec_pubkey_serialize(ctx, recovered_pubkey_bytes, &recovered_pubkey_len,
                                      &recovered_pubkey, SECP256K1_EC_UNCOMPRESSED);
        
        uint8_t recovered_addr_hash[32];
        keccak256(recovered_pubkey_bytes + 1, 64, recovered_addr_hash);
        
        // Compare last 20 bytes (Ethereum address)
        if (memcmp(actual_addr_hash + 12, recovered_addr_hash + 12, 20) == 0) {
            correct_recovery_id = recid;
            break;
        }
    }
    
    if (correct_recovery_id == -1) {
        fprintf(stderr, "Failed to find correct recovery ID\n");
        secp256k1_context_destroy(ctx);
        return -1;
    }
    
    // Set v = correct_recovery_id + 27 (Ethereum style)
    signature_out[64] = (uint8_t)(correct_recovery_id + 27);
    
    secp256k1_context_destroy(ctx);
    return 0;
}

int eip712_domain_hash(const char *domain_name, 
                       uint64_t chain_id,
                       uint8_t domain_hash_out[32]) {
    // EIP712Domain type hash
    // keccak256("EIP712Domain(string name,string version,uint256 chainId,address verifyingContract)")
    const uint8_t domain_typehash[] = {
        0x8b, 0x73, 0xc3, 0xc6, 0x9b, 0xb8, 0xfe, 0x3d,
        0x51, 0x2e, 0xcc, 0x4c, 0xf7, 0x59, 0xcc, 0x79,
        0x23, 0x9f, 0x7b, 0x17, 0x9b, 0x0f, 0xfa, 0xca,
        0xa9, 0xa7, 0x5d, 0x52, 0x2b, 0x39, 0x40, 0x0f
    };
    
    // Hash domain name
    uint8_t name_hash[32];
    keccak256((const uint8_t *)domain_name, strlen(domain_name), name_hash);
    
    // Hash version "1"
    uint8_t version_hash[32];
    keccak256((const uint8_t *)"1", 1, version_hash);
    
    // Prepare chain_id as 32-byte big-endian
    uint8_t chain_id_bytes[32] = {0};
    for (int i = 0; i < 8; i++) {
        chain_id_bytes[24 + i] = (chain_id >> (56 - i * 8)) & 0xFF;
    }
    
    // Verifying contract (zero address)
    uint8_t verifying_contract[32] = {0};
    
    // Concatenate: typehash || name_hash || version_hash || chain_id || verifying_contract
    uint8_t data[160];
    memcpy(data, domain_typehash, 32);
    memcpy(data + 32, name_hash, 32);
    memcpy(data + 64, version_hash, 32);
    memcpy(data + 96, chain_id_bytes, 32);
    memcpy(data + 128, verifying_contract, 32);
    
    return keccak256(data, 160, domain_hash_out);
}

int eip712_agent_struct_hash(const char *source,
                              const uint8_t connection_id[32],
                              uint8_t struct_hash_out[32]) {
    // Agent type hash
    // keccak256("Agent(string source,bytes32 connectionId)")
    const uint8_t agent_typehash[] = {
        0x26, 0xf0, 0x5c, 0x2f, 0x72, 0x39, 0xb6, 0x98,
        0x30, 0x75, 0xe5, 0x83, 0x21, 0x29, 0x2d, 0x77,
        0xb3, 0xaa, 0x17, 0x3d, 0x19, 0xb2, 0x72, 0x57,
        0xac, 0x96, 0xab, 0x36, 0x25, 0x70, 0xf5, 0x08
    };
    
    // Hash source string
    uint8_t source_hash[32];
    keccak256((const uint8_t *)source, strlen(source), source_hash);
    
    // Concatenate: typehash || source_hash || connection_id
    uint8_t data[96];
    memcpy(data, agent_typehash, 32);
    memcpy(data + 32, source_hash, 32);
    memcpy(data + 64, connection_id, 32);
    
    return keccak256(data, 96, struct_hash_out);
}

int eip712_signing_hash(const uint8_t domain_hash[32],
                        const uint8_t struct_hash[32],
                        uint8_t signing_hash_out[32]) {
    // Construct: 0x19 0x01 || domain_hash || struct_hash
    uint8_t data[66];
    data[0] = 0x19;
    data[1] = 0x01;
    memcpy(data + 2, domain_hash, 32);
    memcpy(data + 34, struct_hash, 32);
    
    return keccak256(data, 66, signing_hash_out);
}

int eip712_sign_agent(const char *domain_name,
                      uint64_t chain_id,
                      const char *source,
                      const uint8_t connection_id[32],
                      const char *private_key_hex,
                      uint8_t signature_out[65]) {
    uint8_t domain_hash[32];
    uint8_t struct_hash[32];
    uint8_t signing_hash[32];
    
    // Compute domain hash
    if (eip712_domain_hash(domain_name, chain_id, domain_hash) != 0) {
        fprintf(stderr, "Failed to compute domain hash\n");
        return -1;
    }
    
    // Compute struct hash
    if (eip712_agent_struct_hash(source, connection_id, struct_hash) != 0) {
        fprintf(stderr, "Failed to compute struct hash\n");
        return -1;
    }
    
    // Compute signing hash
    if (eip712_signing_hash(domain_hash, struct_hash, signing_hash) != 0) {
        fprintf(stderr, "Failed to compute signing hash\n");
        return -1;
    }
    
    // Sign the hash (v is already set to recovery_id + 27 inside ecdsa_sign_secp256k1)
    if (ecdsa_sign_secp256k1(signing_hash, private_key_hex, signature_out) != 0) {
        fprintf(stderr, "Failed to sign hash\n");
        return -1;
    }
    
    return 0;
}

