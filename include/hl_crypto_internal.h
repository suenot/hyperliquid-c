#ifndef SHA3_H
#define SHA3_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Works when compiled for either 32-bit or 64-bit targets, optimized for 
 * 64 bit.
 *
 * Canonical implementation of Init/Update/Finalize for SHA-3 byte input. 
 *
 * SHA3-256, SHA3-384, SHA-512 are implemented. SHA-224 can easily be added.
 *
 * Based on code from http://keccak.noekeon.org/ .
 *
 * I place the code that I wrote into public domain, free to use. 
 *
 * I would appreciate if you give credits to this work if you used it to 
 * write or test * your code.
 *
 * Aug 2015. Andrey Jivsov. crypto@brainhub.org
 * ---------------------------------------------------------------------- */

/* 'Words' here refers to uint64_t */
#define SHA3_KECCAK_SPONGE_WORDS \
	(((1600)/8/*bits to byte*/)/sizeof(uint64_t))
typedef struct sha3_context_ {
    uint64_t saved;             /* the portion of the input message that we
                                 * didn't consume yet */
    union {                     /* Keccak's state */
        uint64_t s[SHA3_KECCAK_SPONGE_WORDS];
        uint8_t sb[SHA3_KECCAK_SPONGE_WORDS * 8];
    } u;
    unsigned byteIndex;         /* 0..7--the next byte after the set one
                                 * (starts from 0; 0--none are buffered) */
    unsigned wordIndex;         /* 0..24--the next word to integrate input
                                 * (starts from 0) */
    unsigned capacityWords;     /* the double size of the hash output in
                                 * words (e.g. 16 for Keccak 512) */
} sha3_context;

enum SHA3_FLAGS {
    SHA3_FLAGS_NONE=0,
    SHA3_FLAGS_KECCAK=1
};

enum SHA3_RETURN {
    SHA3_RETURN_OK=0,
    SHA3_RETURN_BAD_PARAMS=1
};
typedef enum SHA3_RETURN sha3_return_t;

/* For Init or Reset call these: */
sha3_return_t sha3_Init(void *priv, unsigned bitSize);

void sha3_Init256(void *priv);
void sha3_Init384(void *priv);
void sha3_Init512(void *priv);

enum SHA3_FLAGS sha3_SetFlags(void *priv, enum SHA3_FLAGS);

void sha3_Update(void *priv, void const *bufIn, size_t len);

void const *sha3_Finalize(void *priv);

/* Single-call hashing */
sha3_return_t sha3_HashBuffer( 
    unsigned bitSize,   /* 256, 384, 512 */
    enum SHA3_FLAGS flags, /* SHA3_FLAGS_NONE or SHA3_FLAGS_KECCAK */
    const void *in, unsigned inBytes, 
    void *out, unsigned outBytes );     /* up to bitSize/8; truncation OK */

/* Additional crypto functions from crypto_utils.c */
int keccak256(const uint8_t *input, size_t input_len, uint8_t output[32]);
int hex_to_bytes(const char *hex, uint8_t *bytes_out, size_t max_out_len);
void bytes_to_hex(const uint8_t *bytes, size_t len, char *hex_out, bool add_prefix);
int parse_eth_address(const char *address_hex, uint8_t address_out[20]);
int ecdsa_sign_secp256k1(const uint8_t hash[32], const char *private_key_hex, uint8_t signature_out[65]);
int eip712_domain_hash(const char *domain_name, uint64_t chain_id, uint8_t domain_hash_out[32]);
int eip712_agent_struct_hash(const char *source, const uint8_t connection_id[32], uint8_t struct_hash_out[32]);
int eip712_signing_hash(const uint8_t domain_hash[32], const uint8_t struct_hash[32], uint8_t signing_hash_out[32]);
int eip712_sign_agent(const char *domain_name, uint64_t chain_id, const char *source, const uint8_t connection_id[32], const char *private_key_hex, uint8_t signature_out[65]);

#endif
