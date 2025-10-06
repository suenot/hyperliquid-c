#include "hyperliquid_action.h"
#include "crypto_utils.h"
#include <msgpack.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Pack order limit type
static void pack_limit(msgpack_packer *pk, const hl_limit_t *limit) {
    // {"limit": {"tif": "Gtc"}}
    msgpack_pack_map(pk, 1);
    msgpack_pack_str(pk, 5);
    msgpack_pack_str_body(pk, "limit", 5);
    
    msgpack_pack_map(pk, 1);
    msgpack_pack_str(pk, 3);
    msgpack_pack_str_body(pk, "tif", 3);
    
    size_t tif_len = strlen(limit->tif);
    msgpack_pack_str(pk, tif_len);
    msgpack_pack_str_body(pk, limit->tif, tif_len);
}

// Pack single order
// Keys must be in alphabetical order: a, b, p, r, s, t
static void pack_order(msgpack_packer *pk, const hl_order_t *order) {
    msgpack_pack_map(pk, 6);
    
    // "a": asset_id
    msgpack_pack_str(pk, 1);
    msgpack_pack_str_body(pk, "a", 1);
    msgpack_pack_uint32(pk, order->a);
    
    // "b": is_buy
    msgpack_pack_str(pk, 1);
    msgpack_pack_str_body(pk, "b", 1);
    if (order->b) {
        msgpack_pack_true(pk);
    } else {
        msgpack_pack_false(pk);
    }
    
    // "p": price (string)
    msgpack_pack_str(pk, 1);
    msgpack_pack_str_body(pk, "p", 1);
    size_t p_len = strlen(order->p);
    msgpack_pack_str(pk, p_len);
    msgpack_pack_str_body(pk, order->p, p_len);
    
    // "s": size (string) - BEFORE "r" to match Go SDK!
    msgpack_pack_str(pk, 1);
    msgpack_pack_str_body(pk, "s", 1);
    size_t s_len = strlen(order->s);
    msgpack_pack_str(pk, s_len);
    msgpack_pack_str_body(pk, order->s, s_len);
    
    // "r": reduce_only - AFTER "s" to match Go SDK!
    msgpack_pack_str(pk, 1);
    msgpack_pack_str_body(pk, "r", 1);
    if (order->r) {
        msgpack_pack_true(pk);
    } else {
        msgpack_pack_false(pk);
    }
    
    // "t": order type (limit)
    msgpack_pack_str(pk, 1);
    msgpack_pack_str_body(pk, "t", 1);
    pack_limit(pk, &order->limit);
}

// Pack order action  
// CCXT format: flat map {type, orders, grouping} (in dict insertion order)
static int pack_order_action(msgpack_packer *pk, const hl_order_action_t *action) {
    msgpack_pack_map(pk, 3);
    
    // "type": "order" (first!)
    msgpack_pack_str(pk, 4);
    msgpack_pack_str_body(pk, "type", 4);
    msgpack_pack_str(pk, 5);
    msgpack_pack_str_body(pk, "order", 5);
    
    // "orders": [...] (second!)
    msgpack_pack_str(pk, 6);
    msgpack_pack_str_body(pk, "orders", 6);
    msgpack_pack_array(pk, action->orders_count);
    for (size_t i = 0; i < action->orders_count; i++) {
        pack_order(pk, &action->orders[i]);
    }
    
    // "grouping": "na" (third!)
    msgpack_pack_str(pk, 8);
    msgpack_pack_str_body(pk, "grouping", 8);
    size_t g_len = strlen(action->grouping);
    msgpack_pack_str(pk, g_len);
    msgpack_pack_str_body(pk, action->grouping, g_len);
    
    return 0;
}

// Pack cancel action
// CCXT format: flat map {type, cancels} (in dict insertion order)
static int pack_cancel_action(msgpack_packer *pk, const hl_cancel_action_t *action) {
    msgpack_pack_map(pk, 2);
    
    // "type": "cancel" (first!)
    msgpack_pack_str(pk, 4);
    msgpack_pack_str_body(pk, "type", 4);
    msgpack_pack_str(pk, 6);
    msgpack_pack_str_body(pk, "cancel", 6);
    
    // "cancels": [...] (second!)
    msgpack_pack_str(pk, 7);
    msgpack_pack_str_body(pk, "cancels", 7);
    msgpack_pack_array(pk, action->cancels_count);
    for (size_t i = 0; i < action->cancels_count; i++) {
        // Each cancel has 2 fields: a, o (already alphabetical)
        msgpack_pack_map(pk, 2);
        
        // "a": asset_id
        msgpack_pack_str(pk, 1);
        msgpack_pack_str_body(pk, "a", 1);
        msgpack_pack_uint32(pk, action->cancels[i].a);
        
        // "o": order_id
        msgpack_pack_str(pk, 1);
        msgpack_pack_str_body(pk, "o", 1);
        msgpack_pack_uint64(pk, action->cancels[i].o);
    }
    
    return 0;
}

int hl_build_action_hash(const char *action_type,
                         const void *action_data,
                         uint64_t nonce,
                         const char *vault_address,
                         uint8_t connection_id_out[32]) {
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    int result = -1;
    
    // Initialize MessagePack buffer
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
    
    // Pack action based on type
    if (strcmp(action_type, "order") == 0) {
        if (pack_order_action(&pk, (const hl_order_action_t *)action_data) != 0) {
            goto cleanup;
        }
    } else if (strcmp(action_type, "cancel") == 0) {
        if (pack_cancel_action(&pk, (const hl_cancel_action_t *)action_data) != 0) {
            goto cleanup;
        }
    } else {
        fprintf(stderr, "Unknown action type: %s\n", action_type);
        goto cleanup;
    }
    
    // Append nonce (timestamp) as big-endian uint64
    size_t data_len = sbuf.size + 8;
    uint8_t *data = malloc(data_len + (vault_address ? 21 : 1));
    if (!data) {
        fprintf(stderr, "Failed to allocate memory for action hash\n");
        goto cleanup;
    }
    
    memcpy(data, sbuf.data, sbuf.size);
    
    // Append nonce in big-endian
    for (int i = 0; i < 8; i++) {
        data[sbuf.size + i] = (nonce >> (56 - i * 8)) & 0xFF;
    }
    
    // Append vault address or 0x00
    if (vault_address && vault_address[0] != '\0') {
        data[data_len] = 0x01;
        uint8_t addr_bytes[20];
        if (parse_eth_address(vault_address, addr_bytes) != 0) {
            fprintf(stderr, "Failed to parse vault address\n");
            free(data);
            goto cleanup;
        }
        memcpy(data + data_len + 1, addr_bytes, 20);
        data_len += 21;
    } else {
        data[data_len] = 0x00;
        data_len += 1;
    }
    
    // Compute Keccak256 hash
    if (keccak256(data, data_len, connection_id_out) != 0) {
        fprintf(stderr, "Failed to compute Keccak256\n");
        free(data);
        goto cleanup;
    }
    
    free(data);
    result = 0;
    
cleanup:
    msgpack_sbuffer_destroy(&sbuf);
    return result;
}

int hl_build_order_hash(const hl_order_t *orders,
                        size_t orders_count,
                        const char *grouping,
                        uint64_t nonce,
                        const char *vault_address,
                        uint8_t connection_id_out[32]) {
    hl_order_action_t action = {
        .orders = (hl_order_t *)orders,
        .orders_count = orders_count,
        .grouping = grouping
    };
    
    return hl_build_action_hash("order", &action, nonce, vault_address, connection_id_out);
}

int hl_build_cancel_hash(const hl_cancel_t *cancels,
                         size_t cancels_count,
                         uint64_t nonce,
                         const char *vault_address,
                         uint8_t connection_id_out[32]) {
    hl_cancel_action_t action = {
        .cancels = (hl_cancel_t *)cancels,
        .cancels_count = cancels_count
    };
    
    return hl_build_action_hash("cancel", &action, nonce, vault_address, connection_id_out);
}

