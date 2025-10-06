/**
 * @file http_client.c
 * @brief HTTP client implementation using libcurl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "hl_http.h"
#include "hl_logger.h"

struct http_client {
    CURL *curl;
    http_client_config_t config;
    struct curl_slist *headers;
};

/**
 * @brief Response data structure for curl callbacks
 */
typedef struct {
    char *data;
    size_t size;
} curl_response_data_t;

/**
 * @brief Callback for writing response data
 */
static size_t write_callback(void *contents, size_t size, size_t nmemb, curl_response_data_t *response) {
    size_t realsize = size * nmemb;
    char *ptr = lv3_realloc(response->data, response->size + realsize + 1);
    
    if (!ptr) {
        LV3_LOG_ERROR("Failed to reallocate response buffer");
        return 0;
    }
    
    response->data = ptr;
    memcpy(&(response->data[response->size]), contents, realsize);
    response->size += realsize;
    response->data[response->size] = '\0';
    
    return realsize;
}

/**
 * @brief Create HTTP client instance
 */
lv3_error_t http_client_create(const http_client_config_t *config, http_client_t **client_out) {
    if (!client_out) return LV3_ERROR_INVALID_PARAM;
    
    // Initialize curl globally if not done
    static bool curl_initialized = false;
    if (!curl_initialized) {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
            LV3_LOG_ERROR("Failed to initialize libcurl");
            return LV3_ERROR_NETWORK;
        }
        curl_initialized = true;
    }
    
    http_client_t *client = lv3_calloc(1, sizeof(http_client_t));
    if (!client) return LV3_ERROR_MEMORY;
    
    client->curl = curl_easy_init();
    if (!client->curl) {
        lv3_free(client);
        return LV3_ERROR_NETWORK;
    }
    
    // Apply configuration
    if (config) {
        client->config = *config;
    } else {
        // Default configuration
        client->config.timeout_ms = 30000;
        client->config.connect_timeout_ms = 10000;
        client->config.follow_redirects = true;
        client->config.max_redirects = 5;
        client->config.verify_ssl = true;
        lv3_string_copy(client->config.user_agent, 
                       "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36", 
                       sizeof(client->config.user_agent));
    }
    
    // Set basic curl options
    curl_easy_setopt(client->curl, CURLOPT_TIMEOUT_MS, client->config.timeout_ms);
    curl_easy_setopt(client->curl, CURLOPT_CONNECTTIMEOUT_MS, client->config.connect_timeout_ms);
    curl_easy_setopt(client->curl, CURLOPT_FOLLOWLOCATION, client->config.follow_redirects ? 1L : 0L);
    curl_easy_setopt(client->curl, CURLOPT_MAXREDIRS, (long)client->config.max_redirects);
    curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYPEER, client->config.verify_ssl ? 1L : 0L);
    curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYHOST, client->config.verify_ssl ? 2L : 0L);
    curl_easy_setopt(client->curl, CURLOPT_USERAGENT, client->config.user_agent);
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);
    
    *client_out = client;
    return LV3_SUCCESS;
}

/**
 * @brief Destroy HTTP client
 */
void http_client_destroy(http_client_t *client) {
    if (!client) return;
    
    if (client->headers) {
        curl_slist_free_all(client->headers);
    }
    
    if (client->curl) {
        curl_easy_cleanup(client->curl);
    }
    
    lv3_free(client);
}

/**
 * @brief Set proxy for HTTP client
 */
lv3_error_t http_client_set_proxy(http_client_t *client, const char *proxy_url) {
    if (!client) return LV3_ERROR_INVALID_PARAM;
    
    if (proxy_url) {
        curl_easy_setopt(client->curl, CURLOPT_PROXY, proxy_url);
        LV3_LOG_DEBUG("Set proxy: %s", proxy_url);
    } else {
        curl_easy_setopt(client->curl, CURLOPT_PROXY, "");
        LV3_LOG_DEBUG("Disabled proxy");
    }
    
    return LV3_SUCCESS;
}

/**
 * @brief Parse proxy string and create URL
 */
static lv3_error_t parse_proxy_string(const char *proxy_str, char *proxy_url, size_t url_size) {
    if (!proxy_str || !proxy_url) return LV3_ERROR_INVALID_PARAM;
    
    // Parse format: "protocol:host:port:user:pass:status"
    char temp[512];
    lv3_string_copy(temp, proxy_str, sizeof(temp));
    
    char *parts[6];
    int part_count = 0;
    
    char *token = strtok(temp, ":");
    while (token && part_count < 6) {
        parts[part_count++] = token;
        token = strtok(NULL, ":");
    }
    
    if (part_count < 3) {
        return LV3_ERROR_INVALID_PARAM;
    }
    
    const char *protocol = parts[0];
    const char *host = parts[1];
    const char *port = parts[2];
    const char *user = (part_count > 3) ? parts[3] : "";
    const char *pass = (part_count > 4) ? parts[4] : "";
    
    if (strlen(user) > 0 && strlen(pass) > 0) {
        snprintf(proxy_url, url_size, "%s://%s:%s@%s:%s", protocol, user, pass, host, port);
    } else {
        snprintf(proxy_url, url_size, "%s://%s:%s", protocol, host, port);
    }
    
    return LV3_SUCCESS;
}

/**
 * @brief Make HTTP request
 */
static lv3_error_t http_request_internal(http_client_t *client, const http_request_t *request, 
                                        http_response_t *response) {
    if (!client || !request || !response) return LV3_ERROR_INVALID_PARAM;
    
    memset(response, 0, sizeof(http_response_t));
    
    curl_response_data_t curl_response = {0};
    
    // Set URL
    curl_easy_setopt(client->curl, CURLOPT_URL, request->url);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, &curl_response);
    
    // Set method-specific options
    if (strcmp(request->method, "GET") == 0) {
        curl_easy_setopt(client->curl, CURLOPT_HTTPGET, 1L);
    } else if (strcmp(request->method, "POST") == 0) {
        curl_easy_setopt(client->curl, CURLOPT_POST, 1L);
        if (request->body) {
            curl_easy_setopt(client->curl, CURLOPT_POSTFIELDS, request->body);
        }
    }
    
    // Set headers if provided
    struct curl_slist *headers = NULL;
    if (request->headers) {
        headers = curl_slist_append(headers, request->headers);
        curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, headers);
    }
    
    // Set proxy if provided
    if (request->proxy) {
        char proxy_url[512];
        if (parse_proxy_string(request->proxy, proxy_url, sizeof(proxy_url)) == LV3_SUCCESS) {
            curl_easy_setopt(client->curl, CURLOPT_PROXY, proxy_url);
        }
    }
    
    // Set timeout if specified in request
    if (request->timeout_ms > 0) {
        curl_easy_setopt(client->curl, CURLOPT_TIMEOUT_MS, request->timeout_ms);
    }
    
    // Record start time
    int64_t start_time = lv3_current_time_ms();
    
    // Perform request
    CURLcode res = curl_easy_perform(client->curl);
    
    // Record response time
    response->response_time_ms = (int)(lv3_current_time_ms() - start_time);
    
    // Clean up headers
    if (headers) {
        curl_slist_free_all(headers);
        curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, NULL);
    }
    
    if (res != CURLE_OK) {
        LV3_LOG_ERROR("HTTP request failed: %s", curl_easy_strerror(res));
        if (curl_response.data) lv3_free(curl_response.data);
        return LV3_ERROR_NETWORK;
    }
    
    // Get response code
    long response_code;
    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, &response_code);
    response->status_code = (int)response_code;
    
    // Set response data
    response->body = curl_response.data;
    response->content_length = curl_response.size;
    
    LV3_LOG_DEBUG("HTTP %s %s -> %d (%dms)", request->method, request->url, 
                 response->status_code, response->response_time_ms);
    
    return LV3_SUCCESS;
}

/**
 * @brief Make HTTP GET request
 */
lv3_error_t http_client_get(http_client_t *client, const http_request_t *request, http_response_t *response) {
    if (!request) return LV3_ERROR_INVALID_PARAM;
    
    http_request_t get_request = *request;
    lv3_string_copy(get_request.method, "GET", sizeof(get_request.method));
    
    return http_request_internal(client, &get_request, response);
}

/**
 * @brief Make HTTP POST request
 */
lv3_error_t http_client_post(http_client_t *client, const http_request_t *request, http_response_t *response) {
    if (!request) return LV3_ERROR_INVALID_PARAM;
    
    http_request_t post_request = *request;
    lv3_string_copy(post_request.method, "POST", sizeof(post_request.method));
    
    return http_request_internal(client, &post_request, response);
}

/**
 * @brief Free HTTP response memory
 */
void http_response_free(http_response_t *response) {
    if (!response) return;
    
    if (response->headers) {
        lv3_free(response->headers);
        response->headers = NULL;
    }
    
    if (response->body) {
        lv3_free(response->body);
        response->body = NULL;
    }
    
    response->content_length = 0;
    response->status_code = 0;
    response->response_time_ms = 0;
}

/**
 * @brief Test HTTP client connectivity
 */
bool http_client_test_connection(http_client_t *client, const char *test_url) {
    if (!client) return false;
    
    const char *url = test_url ? test_url : "https://httpbin.org/get";
    
    http_request_t request = {0};
    lv3_string_copy(request.url, url, sizeof(request.url));
    request.timeout_ms = 10000;
    
    http_response_t response = {0};
    
    lv3_error_t result = http_client_get(client, &request, &response);
    bool success = (result == LV3_SUCCESS && response.status_code == 200);
    
    http_response_free(&response);
    
    LV3_LOG_DEBUG("HTTP client test: %s -> %s", url, success ? "OK" : "FAILED");
    return success;
}
