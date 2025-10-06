/**
 * @file hl_http.h
 * @brief HTTP client interface for API requests
 * 
 * Provides HTTP client functionality using libcurl with support for
 * SSL, proxies, connection pooling, and retry logic.
 */

#ifndef HL_HTTP_H
#define HL_HTTP_H

#include <stdbool.h>
#include <stddef.h>

// Forward declarations
typedef struct http_client http_client_t;

// Error codes (compatibility with lv3_error_t)
typedef enum {
    LV3_SUCCESS = 0,
    LV3_ERROR_INVALID_PARAMS = -1,
    LV3_ERROR_NETWORK = -2,
    LV3_ERROR_HTTP = -3,
    LV3_ERROR_JSON = -4,
    LV3_ERROR_EXCHANGE = -5,
    LV3_ERROR_MEMORY = -6,
    LV3_ERROR_TIMEOUT = -7,
} lv3_error_t;

// HTTP response
typedef struct {
    int status_code;
    char *body;
    size_t body_size;
    char *headers;
    size_t headers_size;
} http_response_t;

/**
 * @brief HTTP client configuration
 */
typedef struct {
    int timeout_ms;                         /**< Request timeout in milliseconds */
    int connect_timeout_ms;                 /**< Connection timeout in milliseconds */
    bool follow_redirects;                  /**< Whether to follow redirects */
    int max_redirects;                      /**< Maximum number of redirects */
    bool verify_ssl;                        /**< Whether to verify SSL certificates */
    char user_agent[256];                   /**< User-Agent header */
} http_client_config_t;

/**
 * @brief Create HTTP client instance
 * 
 * @return HTTP client handle, or NULL on failure
 */
http_client_t* http_client_create(void);

/**
 * @brief Destroy HTTP client and cleanup resources
 * 
 * @param client Client to destroy (can be NULL)
 */
void http_client_destroy(http_client_t *client);

/**
 * @brief Make HTTP GET request
 * 
 * @param client HTTP client instance
 * @param url URL to request
 * @param response Response object to populate
 * @return LV3_SUCCESS on success, error code on failure
 */
lv3_error_t http_client_get(http_client_t *client, const char *url, http_response_t *response);

/**
 * @brief Make HTTP POST request
 * 
 * @param client HTTP client instance
 * @param url URL to request
 * @param body Request body (can be NULL)
 * @param headers Additional headers (can be NULL)
 * @param response Response object to populate
 * @return LV3_SUCCESS on success, error code on failure
 */
lv3_error_t http_client_post(http_client_t *client, const char *url, const char *body, const char *headers, http_response_t *response);

/**
 * @brief Free HTTP response memory
 * 
 * @param response Response to free (can be NULL)
 */
void http_response_free(http_response_t *response);

/**
 * @brief Set proxy for HTTP client
 * 
 * @param client HTTP client instance
 * @param proxy_url Proxy URL (NULL to disable proxy)
 * @return LV3_SUCCESS on success, error code on failure
 */
lv3_error_t http_client_set_proxy(http_client_t *client, const char *proxy_url);

/**
 * @brief Test HTTP client connectivity
 * 
 * @param client HTTP client instance
 * @param test_url URL to test (NULL for default)
 * @return true if connection successful, false otherwise
 */
bool http_client_test_connection(http_client_t *client, const char *test_url);

#endif // HTTP_CLIENT_H
