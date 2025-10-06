#ifndef HYPERLIQUID_ACTION_H
#define HYPERLIQUID_ACTION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file hyperliquid_action.h
 * @brief Hyperliquid action building and hashing
 * 
 * Implements MessagePack serialization and hashing for Hyperliquid actions
 * following the exchange API specification.
 */

/**
 * @brief Order limit type configuration
 */
typedef struct {
    const char *tif;  /**< Time in force: "Gtc", "Ioc", "Alo" */
} hl_limit_t;

/**
 * @brief Order request structure
 * 
 * Follows Hyperliquid API format with short field names for MessagePack
 */
typedef struct {
    uint32_t a;        /**< Asset ID */
    bool b;            /**< Is buy */
    const char *p;     /**< Limit price (string) */
    const char *s;     /**< Size (string) */
    bool r;            /**< Reduce only */
    hl_limit_t limit;  /**< Limit order config */
} hl_order_t;

/**
 * @brief Order action for placing orders
 */
typedef struct {
    hl_order_t *orders;       /**< Array of orders */
    size_t orders_count;      /**< Number of orders */
    const char *grouping;     /**< Grouping: "na" for normal */
} hl_order_action_t;

/**
 * @brief Cancel request structure
 */
typedef struct {
    uint32_t a;  /**< Asset ID */
    uint64_t o;  /**< Order ID to cancel */
} hl_cancel_t;

/**
 * @brief Cancel action
 */
typedef struct {
    hl_cancel_t *cancels;     /**< Array of cancels */
    size_t cancels_count;     /**< Number of cancels */
} hl_cancel_action_t;

/**
 * @brief Build action hash (connection_id) for Hyperliquid
 * 
 * Serializes action to MessagePack, appends nonce and vault address,
 * then computes Keccak256 hash.
 * 
 * @param action_type "order" or "cancel"
 * @param action_data Pointer to hl_order_action_t or hl_cancel_action_t
 * @param nonce Timestamp in milliseconds
 * @param vault_address Optional vault address (NULL for none)
 * @param connection_id_out Output buffer for 32-byte hash
 * @return 0 on success, -1 on error
 */
int hl_build_action_hash(const char *action_type,
                         const void *action_data,
                         uint64_t nonce,
                         const char *vault_address,
                         uint8_t connection_id_out[32]);

/**
 * @brief Build order action hash
 * 
 * Convenience wrapper for order actions
 * 
 * @param orders Array of orders
 * @param orders_count Number of orders
 * @param grouping Grouping string ("na" for normal)
 * @param nonce Timestamp in milliseconds
 * @param vault_address Optional vault address (NULL for none)
 * @param connection_id_out Output buffer for 32-byte hash
 * @return 0 on success, -1 on error
 */
int hl_build_order_hash(const hl_order_t *orders,
                        size_t orders_count,
                        const char *grouping,
                        uint64_t nonce,
                        const char *vault_address,
                        uint8_t connection_id_out[32]);

/**
 * @brief Build cancel action hash
 * 
 * Convenience wrapper for cancel actions
 * 
 * @param cancels Array of cancels
 * @param cancels_count Number of cancels
 * @param nonce Timestamp in milliseconds
 * @param vault_address Optional vault address (NULL for none)
 * @param connection_id_out Output buffer for 32-byte hash
 * @return 0 on success, -1 on error
 */
int hl_build_cancel_hash(const hl_cancel_t *cancels,
                         size_t cancels_count,
                         uint64_t nonce,
                         const char *vault_address,
                         uint8_t connection_id_out[32]);

#endif /* HYPERLIQUID_ACTION_H */

