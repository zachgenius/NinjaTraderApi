/*
 * Copyright (c) 2025 Zachary Wang and NinjaTrader API Library contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "../include/ninja/ninja_api.h"
#include "ninja_client.h"
#include <curl/curl.h>
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// HTTP response callback for libcurl
static size_t write_callback(void* contents, size_t size, size_t nmemb, ninja_http_response_t* response) {
    size_t total_size = size * nmemb;
    char* new_data = realloc(response->data, response->size + total_size + 1);

    if (new_data == NULL) {
        return 0; // Out of memory
    }

    response->data = new_data;
    memcpy(&(response->data[response->size]), contents, total_size);
    response->size += total_size;
    response->data[response->size] = '\0';

    return total_size;
}

ninja_client_t* ninja_client_create(ninja_env_t env) {
    ninja_client_t* client = calloc(1, sizeof(ninja_client_t));
    if (!client) {
        return NULL;
    }

    client->env = env;
    client->timeout_ms = 30000; // 30 seconds default timeout
    client->debug_mode = false;

    // Set base URL
    const char* base_url = ninja_get_base_url(env);
    strncpy(client->base_url, base_url, sizeof(client->base_url) - 1);

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    client->curl = curl_easy_init();
    if (!client->curl) {
        free(client);
        return NULL;
    }

    // Set common curl options
    curl_easy_setopt(client->curl, CURLOPT_TIMEOUT_MS, client->timeout_ms);
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(client->curl, CURLOPT_USERAGENT, "NinjaTrader-API-Client/1.0");
    curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYHOST, 2L);

    return client;
}

void ninja_client_destroy(ninja_client_t* client) {
    if (!client) {
        return;
    }

    if (client->curl) {
        curl_easy_cleanup(client->curl);
    }

    if (client->headers) {
        curl_slist_free_all(client->headers);
    }

    curl_global_cleanup();
    free(client);
}

const char* ninja_get_base_url(ninja_env_t env) {
    switch (env) {
        case NINJA_ENV_DEMO:
            return "https://demo.tradovateapi.com/v1";
        case NINJA_ENV_LIVE:
            return "https://live.tradovateapi.com/v1";
        default:
            return "https://demo.tradovateapi.com/v1";
    }
}

ninja_error_t ninja_set_auth_header(ninja_client_t* client) {
    if (!client || strlen(client->access_token) == 0) {
        return NINJA_ERROR_AUTH;
    }

    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", client->access_token);

    // Free existing headers
    if (client->headers) {
        curl_slist_free_all(client->headers);
    }

    // Set new headers
    client->headers = curl_slist_append(NULL, "Content-Type: application/json");
    client->headers = curl_slist_append(client->headers, auth_header);

    return NINJA_OK;
}

ninja_error_t ninja_http_get(ninja_client_t* client, const char* endpoint, ninja_http_response_t* response) {
    if (!client || !endpoint || !response) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    // Initialize response
    response->data = malloc(1);
    response->size = 0;
    response->status_code = 0;

    char url[512];
    snprintf(url, sizeof(url), "%s/%s", client->base_url, endpoint);

    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, client->headers);
    curl_easy_setopt(client->curl, CURLOPT_HTTPGET, 1L);

    CURLcode res = curl_easy_perform(client->curl);
    if (res != CURLE_OK) {
        ninja_http_response_free(response);
        return NINJA_ERROR_CONNECTION;
    }

    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, &response->status_code);

    if (response->status_code >= 400) {
        return NINJA_ERROR_HTTP;
    }

    return NINJA_OK;
}

ninja_error_t ninja_http_post(ninja_client_t* client, const char* endpoint, const char* json_data, ninja_http_response_t* response) {
    if (!client || !endpoint || !response) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    // Initialize response
    response->data = malloc(1);
    response->size = 0;
    response->status_code = 0;

    char url[512];
    snprintf(url, sizeof(url), "%s/%s", client->base_url, endpoint);

    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, client->headers);
    curl_easy_setopt(client->curl, CURLOPT_POST, 1L);

    if (json_data) {
        curl_easy_setopt(client->curl, CURLOPT_POSTFIELDS, json_data);
        curl_easy_setopt(client->curl, CURLOPT_POSTFIELDSIZE, (long)strlen(json_data));
    }

    CURLcode res = curl_easy_perform(client->curl);
    if (res != CURLE_OK) {
        ninja_http_response_free(response);
        return NINJA_ERROR_CONNECTION;
    }

    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, &response->status_code);

    if (response->status_code >= 400) {
        return NINJA_ERROR_HTTP;
    }

    return NINJA_OK;
}

ninja_error_t ninja_http_delete(ninja_client_t* client, const char* endpoint, ninja_http_response_t* response) {
    if (!client || !endpoint || !response) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    // Initialize response
    response->data = malloc(1);
    response->size = 0;
    response->status_code = 0;

    char url[512];
    snprintf(url, sizeof(url), "%s/%s", client->base_url, endpoint);

    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, client->headers);
    curl_easy_setopt(client->curl, CURLOPT_CUSTOMREQUEST, "DELETE");

    CURLcode res = curl_easy_perform(client->curl);
    if (res != CURLE_OK) {
        ninja_http_response_free(response);
        return NINJA_ERROR_CONNECTION;
    }

    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, &response->status_code);

    if (response->status_code >= 400) {
        return NINJA_ERROR_HTTP;
    }

    return NINJA_OK;
}

void ninja_http_response_free(ninja_http_response_t* response) {
    if (response && response->data) {
        free(response->data);
        response->data = NULL;
        response->size = 0;
    }
}

const char* ninja_error_string(ninja_error_t error) {
    switch (error) {
        case NINJA_OK: return "Success";
        case NINJA_ERROR_AUTH: return "Authentication error";
        case NINJA_ERROR_CONNECTION: return "Connection error";
        case NINJA_ERROR_INVALID_PARAM: return "Invalid parameter";
        case NINJA_ERROR_JSON_PARSE: return "JSON parsing error";
        case NINJA_ERROR_TIMEOUT: return "Timeout error";
        case NINJA_ERROR_ORDER_REJECTED: return "Order rejected";
        case NINJA_ERROR_HTTP: return "HTTP error";
        case NINJA_ERROR_MEMORY: return "Memory allocation error";
        case NINJA_ERROR_NOT_FOUND: return "Not found";
        default: return "Unknown error";
    }
}

void ninja_free_array(void* array) {
    if (array) {
        free(array);
    }
}