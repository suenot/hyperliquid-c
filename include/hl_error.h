/**
 * @file hl_error.h
 * @brief Error codes for Hyperliquid C SDK
 */

#ifndef HL_ERROR_H
#define HL_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HL_SUCCESS = 0,
    HL_ERROR_INVALID_PARAMS = -1,
    HL_ERROR_NETWORK = -2,
    HL_ERROR_API = -3,
    HL_ERROR_AUTH = -4,
    HL_ERROR_INSUFFICIENT_BALANCE = -5,
    HL_ERROR_INVALID_SYMBOL = -6,
    HL_ERROR_ORDER_REJECTED = -7,
    HL_ERROR_SIGNATURE = -8,
    HL_ERROR_MSGPACK = -9,
    HL_ERROR_JSON = -10,
    HL_ERROR_MEMORY = -11,
    HL_ERROR_TIMEOUT = -12,
    HL_ERROR_NOT_IMPLEMENTED = -13,
    HL_ERROR_NOT_FOUND = -14,
    HL_ERROR_PARSE = -15
} hl_error_t;

#define HL_ERROR_T_DEFINED

#ifdef __cplusplus
}
#endif

#endif // HL_ERROR_H