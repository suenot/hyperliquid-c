/**
 * @file hl_logger.h
 * @brief Simple logging macros for Hyperliquid SDK
 */

#ifndef HL_LOGGER_H
#define HL_LOGGER_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Simple logging macros (can be replaced with more sophisticated logging later)
#ifdef DEBUG
#define HL_LOG_DEBUG(fmt, ...) fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define HL_LOG_DEBUG(fmt, ...) ((void)0)
#endif

#define HL_LOG_INFO(fmt, ...) fprintf(stdout, "[INFO] " fmt "\n", ##__VA_ARGS__)
#define HL_LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)
#define HL_LOG_WARN(fmt, ...) fprintf(stderr, "[WARN] " fmt "\n", ##__VA_ARGS__)

// Compatibility macros for old code
#define LV3_LOG_ERROR HL_LOG_ERROR
#define LV3_LOG_DEBUG HL_LOG_DEBUG
#define LV3_LOG_INFO HL_LOG_INFO

#ifdef __cplusplus
}
#endif

#endif // HL_LOGGER_H

