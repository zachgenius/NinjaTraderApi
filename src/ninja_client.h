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

#pragma once

#include "../include/ninja/ninja_types.h"
#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Internal client structure
struct ninja_client {
    ninja_env_t env;
    char base_url[256];
    char access_token[256];
    char md_access_token[256];
    int user_id;

    // HTTP client
    CURL* curl;
    struct curl_slist* headers;

    // Configuration
    long timeout_ms;
    bool debug_mode;
};

// Internal HTTP functions
ninja_error_t ninja_http_get(ninja_client_t* client,
                            const char* endpoint,
                            ninja_http_response_t* response);

ninja_error_t ninja_http_post(ninja_client_t* client,
                             const char* endpoint,
                             const char* json_data,
                             ninja_http_response_t* response);

ninja_error_t ninja_http_delete(ninja_client_t* client,
                               const char* endpoint,
                               ninja_http_response_t* response);

void ninja_http_response_free(ninja_http_response_t* response);

// Internal utility functions
ninja_error_t ninja_set_auth_header(ninja_client_t* client);
const char* ninja_get_base_url(ninja_env_t env);

#ifdef __cplusplus
}
#endif