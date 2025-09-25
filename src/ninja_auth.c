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
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ninja_error_t ninja_authenticate(ninja_client_t* client,
                                const char* username,
                                const char* password,
                                const char* app_id,
                                const char* app_version,
                                ninja_auth_response_t* auth_response) {
    if (!client || !username || !password || !auth_response) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    // Create JSON request body
    cJSON* json = cJSON_CreateObject();
    if (!json) {
        return NINJA_ERROR_JSON_PARSE;
    }

    cJSON_AddStringToObject(json, "name", username);
    cJSON_AddStringToObject(json, "password", password);
    cJSON_AddStringToObject(json, "appId", app_id ? app_id : "NinjaTraderAPI");
    cJSON_AddStringToObject(json, "appVersion", app_version ? app_version : "1.0");
    cJSON_AddNumberToObject(json, "cid", 1); // Client ID

    char* json_string = cJSON_Print(json);
    cJSON_Delete(json);

    if (!json_string) {
        return NINJA_ERROR_JSON_PARSE;
    }

    // Set Content-Type header for auth request
    struct curl_slist* auth_headers = NULL;
    auth_headers = curl_slist_append(auth_headers, "Content-Type: application/json");

    // Temporarily set headers (no auth token needed for login)
    struct curl_slist* old_headers = client->headers;
    client->headers = auth_headers;

    // Make HTTP request
    ninja_http_response_t response;
    ninja_error_t result = ninja_http_post(client, "auth/accesstokenrequest", json_string, &response);

    // Restore old headers
    curl_slist_free_all(client->headers);
    client->headers = old_headers;
    free(json_string);

    if (result != NINJA_OK) {
        return result;
    }

    // Parse response
    cJSON* response_json = cJSON_Parse(response.data);
    ninja_http_response_free(&response);

    if (!response_json) {
        return NINJA_ERROR_JSON_PARSE;
    }

    // Check for error in response
    cJSON* error_text = cJSON_GetObjectItemCaseSensitive(response_json, "errorText");
    if (error_text && cJSON_IsString(error_text)) {
        cJSON_Delete(response_json);
        return NINJA_ERROR_AUTH;
    }

    // Extract authentication data
    cJSON* access_token = cJSON_GetObjectItemCaseSensitive(response_json, "accessToken");
    cJSON* md_access_token = cJSON_GetObjectItemCaseSensitive(response_json, "mdAccessToken");
    cJSON* name = cJSON_GetObjectItemCaseSensitive(response_json, "name");
    cJSON* user_id = cJSON_GetObjectItemCaseSensitive(response_json, "userId");
    cJSON* expires_in = cJSON_GetObjectItemCaseSensitive(response_json, "expirationTime");

    if (!access_token || !cJSON_IsString(access_token)) {
        cJSON_Delete(response_json);
        return NINJA_ERROR_JSON_PARSE;
    }

    // Store tokens in client
    strncpy(client->access_token, cJSON_GetStringValue(access_token), sizeof(client->access_token) - 1);
    strncpy(auth_response->access_token, cJSON_GetStringValue(access_token), sizeof(auth_response->access_token) - 1);

    if (md_access_token && cJSON_IsString(md_access_token)) {
        strncpy(client->md_access_token, cJSON_GetStringValue(md_access_token), sizeof(client->md_access_token) - 1);
        strncpy(auth_response->md_access_token, cJSON_GetStringValue(md_access_token), sizeof(auth_response->md_access_token) - 1);
    }

    if (name && cJSON_IsString(name)) {
        strncpy(auth_response->name, cJSON_GetStringValue(name), sizeof(auth_response->name) - 1);
    }

    if (user_id && cJSON_IsNumber(user_id)) {
        client->user_id = cJSON_GetNumberValue(user_id);
        auth_response->user_id = client->user_id;
    }

    if (expires_in && cJSON_IsNumber(expires_in)) {
        auth_response->expires_in = cJSON_GetNumberValue(expires_in);
    }

    cJSON_Delete(response_json);

    // Set authorization header for future requests
    return ninja_set_auth_header(client);
}

ninja_error_t ninja_renew_token(ninja_client_t* client, ninja_auth_response_t* auth_response) {
    if (!client || strlen(client->access_token) == 0 || !auth_response) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    // Make HTTP request to renew token
    ninja_http_response_t response;
    ninja_error_t result = ninja_http_post(client, "auth/renewAccessToken", NULL, &response);

    if (result != NINJA_OK) {
        return result;
    }

    // Parse response
    cJSON* response_json = cJSON_Parse(response.data);
    ninja_http_response_free(&response);

    if (!response_json) {
        return NINJA_ERROR_JSON_PARSE;
    }

    // Check for error in response
    cJSON* error_text = cJSON_GetObjectItemCaseSensitive(response_json, "errorText");
    if (error_text && cJSON_IsString(error_text)) {
        cJSON_Delete(response_json);
        return NINJA_ERROR_AUTH;
    }

    // Extract new token data
    cJSON* access_token = cJSON_GetObjectItemCaseSensitive(response_json, "accessToken");
    cJSON* md_access_token = cJSON_GetObjectItemCaseSensitive(response_json, "mdAccessToken");
    cJSON* expires_in = cJSON_GetObjectItemCaseSensitive(response_json, "expirationTime");

    if (!access_token || !cJSON_IsString(access_token)) {
        cJSON_Delete(response_json);
        return NINJA_ERROR_JSON_PARSE;
    }

    // Update tokens
    strncpy(client->access_token, cJSON_GetStringValue(access_token), sizeof(client->access_token) - 1);
    strncpy(auth_response->access_token, cJSON_GetStringValue(access_token), sizeof(auth_response->access_token) - 1);

    if (md_access_token && cJSON_IsString(md_access_token)) {
        strncpy(client->md_access_token, cJSON_GetStringValue(md_access_token), sizeof(client->md_access_token) - 1);
        strncpy(auth_response->md_access_token, cJSON_GetStringValue(md_access_token), sizeof(auth_response->md_access_token) - 1);
    }

    if (expires_in && cJSON_IsNumber(expires_in)) {
        auth_response->expires_in = cJSON_GetNumberValue(expires_in);
    }

    cJSON_Delete(response_json);

    // Update authorization header
    return ninja_set_auth_header(client);
}