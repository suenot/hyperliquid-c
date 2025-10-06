/**
 * @file simple_client.c
 * @brief Simple HTTP client implementation for hyperliquid-c SDK
 * 
 * Clean implementation with simple API signatures
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "hl_http.h"
#include "hl_internal.h"

struct http_client {
    CURL *curl;
    int timeout_ms;
};

// Curl write callback
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    http_response_t *response = (http_response_t *)userp;
    
    char *ptr = realloc(response->body, response->body_size + realsize + 1);
    if (!ptr) {
        return 0;
    }
    
    response->body = ptr;
    memcpy(&(response->body[response->body_size]), contents, realsize);
    response->body_size += realsize;
    response->body[response->body_size] = 0;
    
    return realsize;
}

/**
 * @brief Create HTTP client
 */
http_client_t* http_client_create(void) {
    static bool curl_initialized = false;
    if (!curl_initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_initialized = true;
    }
    
    http_client_t *client = calloc(1, sizeof(http_client_t));
    if (!client) {
        return NULL;
    }
    
    client->curl = curl_easy_init();
    if (!client->curl) {
        free(client);
        return NULL;
    }
    
    client->timeout_ms = 30000;
    
    // Set default options
    curl_easy_setopt(client->curl, CURLOPT_TIMEOUT_MS, client->timeout_ms);
    curl_easy_setopt(client->curl, CURLOPT_CONNECTTIMEOUT_MS, 10000);
    curl_easy_setopt(client->curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(client->curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(client->curl, CURLOPT_USERAGENT, "Hyperliquid-C-SDK/1.0");
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);
    
    return client;
}

/**
 * @brief Destroy HTTP client
 */
void http_client_destroy(http_client_t *client) {
    if (!client) {
        return;
    }
    
    if (client->curl) {
        curl_easy_cleanup(client->curl);
    }
    
    free(client);
}

/**
 * @brief Make HTTP GET request
 */
lv3_error_t http_client_get(http_client_t *client, const char *url, http_response_t *response) {
    if (!client || !url || !response) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    // Initialize response
    memset(response, 0, sizeof(http_response_t));
    
    // Set URL
    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    curl_easy_setopt(client->curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, response);
    
    // Perform request
    CURLcode res = curl_easy_perform(client->curl);
    if (res != CURLE_OK) {
        http_response_free(response);
        return LV3_ERROR_NETWORK;
    }
    
    // Get status code
    long status_code;
    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, &status_code);
    response->status_code = (int)status_code;
    
    return LV3_SUCCESS;
}

/**
 * @brief Make HTTP POST request
 */
lv3_error_t http_client_post(http_client_t *client, const char *url, 
                             const char *body, const char *headers,
                             http_response_t *response) {
    if (!client || !url || !response) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    // Initialize response
    memset(response, 0, sizeof(http_response_t));
    
    // Set URL
    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    curl_easy_setopt(client->curl, CURLOPT_POST, 1L);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, response);
    
    // Set body if provided
    if (body) {
        curl_easy_setopt(client->curl, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(client->curl, CURLOPT_POSTFIELDSIZE, strlen(body));
    }
    
    // Set headers if provided
    struct curl_slist *header_list = NULL;
    if (headers) {
        header_list = curl_slist_append(NULL, headers);
        curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, header_list);
    }
    
    // Perform request
    CURLcode res = curl_easy_perform(client->curl);
    
    // Cleanup headers
    if (header_list) {
        curl_slist_free_all(header_list);
    }
    
    if (res != CURLE_OK) {
        http_response_free(response);
        return LV3_ERROR_NETWORK;
    }
    
    // Get status code
    long status_code;
    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, &status_code);
    response->status_code = (int)status_code;
    
    return LV3_SUCCESS;
}

/**
 * @brief Free HTTP response
 */
void http_response_free(http_response_t *response) {
    if (!response) {
        return;
    }
    
    if (response->body) {
        free(response->body);
        response->body = NULL;
    }
    
    if (response->headers) {
        free(response->headers);
        response->headers = NULL;
    }
    
    response->body_size = 0;
    response->headers_size = 0;
    response->status_code = 0;
}

/**
 * @brief Set proxy
 */
lv3_error_t http_client_set_proxy(http_client_t *client, const char *proxy_url) {
    if (!client) {
        return LV3_ERROR_INVALID_PARAMS;
    }
    
    if (proxy_url) {
        curl_easy_setopt(client->curl, CURLOPT_PROXY, proxy_url);
    } else {
        curl_easy_setopt(client->curl, CURLOPT_PROXY, NULL);
    }
    
    return LV3_SUCCESS;
}

/**
 * @brief Test connection
 */
bool http_client_test_connection(http_client_t *client, const char *test_url) {
    if (!client) {
        return false;
    }
    
    const char *url = test_url ? test_url : "https://httpbin.org/get";
    
    http_response_t response = {0};
    lv3_error_t err = http_client_get(client, url, &response);
    
    bool success = (err == LV3_SUCCESS && response.status_code == 200);
    
    http_response_free(&response);
    
    return success;
}

