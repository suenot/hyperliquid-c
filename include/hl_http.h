/**
 * @file http_client.h
 * @brief HTTP client interface for API requests
 * 
 * Provides HTTP client functionality using libcurl with support for
 * SSL, proxies, connection pooling, and retry logic.
 */

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "listing_v3.h"

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
 * @param config Client configuration (NULL for defaults)
 * @param client_out Pointer to store the created client
 * @return LV3_SUCCESS on success, error code on failure
 */
lv3_error_t http_client_create(const http_client_config_t *config, http_client_t **client_out);

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
 * @param request Request parameters
 * @param response Response object to populate
 * @return LV3_SUCCESS on success, error code on failure
 */
lv3_error_t http_client_get(http_client_t *client, const http_request_t *request, http_response_t *response);

/**
 * @brief Make HTTP POST request
 * 
 * @param client HTTP client instance
 * @param request Request parameters
 * @param response Response object to populate
 * @return LV3_SUCCESS on success, error code on failure
 */
lv3_error_t http_client_post(http_client_t *client, const http_request_t *request, http_response_t *response);

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
