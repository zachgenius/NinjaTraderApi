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

// Helper function to parse JSON account into ninja_account_t
static ninja_error_t ninja_parse_account(cJSON* account_json, ninja_account_t* account) {
    if (!account_json || !account) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    // Clear account structure
    memset(account, 0, sizeof(ninja_account_t));

    // Parse account fields
    cJSON* id = cJSON_GetObjectItemCaseSensitive(account_json, "id");
    if (id && cJSON_IsNumber(id)) {
        account->account_id = (int)cJSON_GetNumberValue(id);
    }

    cJSON* name = cJSON_GetObjectItemCaseSensitive(account_json, "name");
    if (name && cJSON_IsString(name)) {
        strncpy(account->name, cJSON_GetStringValue(name), sizeof(account->name) - 1);
        strncpy(account->account_spec, cJSON_GetStringValue(name), sizeof(account->account_spec) - 1);
    }

    cJSON* balance = cJSON_GetObjectItemCaseSensitive(account_json, "cashBalance");
    if (balance && cJSON_IsNumber(balance)) {
        account->balance = cJSON_GetNumberValue(balance);
    }

    cJSON* net_liq = cJSON_GetObjectItemCaseSensitive(account_json, "netLiquidatingValue");
    if (net_liq && cJSON_IsNumber(net_liq)) {
        account->equity = cJSON_GetNumberValue(net_liq);
    }

    cJSON* margin_used = cJSON_GetObjectItemCaseSensitive(account_json, "marginUsed");
    if (margin_used && cJSON_IsNumber(margin_used)) {
        account->margin_used = cJSON_GetNumberValue(margin_used);
    }

    cJSON* margin_available = cJSON_GetObjectItemCaseSensitive(account_json, "marginAvailable");
    if (margin_available && cJSON_IsNumber(margin_available)) {
        account->margin_available = cJSON_GetNumberValue(margin_available);
    }

    cJSON* buying_power = cJSON_GetObjectItemCaseSensitive(account_json, "buyingPower");
    if (buying_power && cJSON_IsNumber(buying_power)) {
        account->buying_power = cJSON_GetNumberValue(buying_power);
    }

    cJSON* currency = cJSON_GetObjectItemCaseSensitive(account_json, "currency");
    if (currency && cJSON_IsString(currency)) {
        strncpy(account->currency, cJSON_GetStringValue(currency), sizeof(account->currency) - 1);
    } else {
        strcpy(account->currency, "USD"); // Default to USD
    }

    cJSON* legal_status = cJSON_GetObjectItemCaseSensitive(account_json, "legalStatus");
    if (legal_status && cJSON_IsString(legal_status)) {
        const char* status_str = cJSON_GetStringValue(legal_status);
        account->is_demo = (strstr(status_str, "Demo") != NULL || strstr(status_str, "Sim") != NULL);
    }

    return NINJA_OK;
}

ninja_error_t ninja_get_accounts(ninja_client_t* client,
                                ninja_account_t** accounts,
                                size_t* count) {
    if (!client || !accounts || !count) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    *accounts = NULL;
    *count = 0;

    // Make HTTP request
    ninja_http_response_t response;
    ninja_error_t result = ninja_http_get(client, "account/list", &response);

    if (result != NINJA_OK) {
        return result;
    }

    // Parse response
    cJSON* response_json = cJSON_Parse(response.data);
    ninja_http_response_free(&response);

    if (!response_json) {
        return NINJA_ERROR_JSON_PARSE;
    }

    if (!cJSON_IsArray(response_json)) {
        cJSON_Delete(response_json);
        return NINJA_ERROR_JSON_PARSE;
    }

    size_t account_count = cJSON_GetArraySize(response_json);
    if (account_count == 0) {
        cJSON_Delete(response_json);
        return NINJA_OK;
    }

    // Allocate memory for accounts
    ninja_account_t* account_array = calloc(account_count, sizeof(ninja_account_t));
    if (!account_array) {
        cJSON_Delete(response_json);
        return NINJA_ERROR_MEMORY;
    }

    // Parse each account
    for (size_t i = 0; i < account_count; i++) {
        cJSON* account_json = cJSON_GetArrayItem(response_json, i);
        if (account_json) {
            ninja_parse_account(account_json, &account_array[i]);
        }
    }

    cJSON_Delete(response_json);

    *accounts = account_array;
    *count = account_count;

    return NINJA_OK;
}

ninja_error_t ninja_get_account_by_id(ninja_client_t* client,
                                     int account_id,
                                     ninja_account_t* account) {
    if (!client || account_id <= 0 || !account) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    char endpoint[256];
    snprintf(endpoint, sizeof(endpoint), "account/item?id=%d", account_id);

    // Make HTTP request
    ninja_http_response_t response;
    ninja_error_t result = ninja_http_get(client, endpoint, &response);

    if (result != NINJA_OK) {
        return result;
    }

    // Parse response
    cJSON* response_json = cJSON_Parse(response.data);
    ninja_http_response_free(&response);

    if (!response_json) {
        return NINJA_ERROR_JSON_PARSE;
    }

    result = ninja_parse_account(response_json, account);
    cJSON_Delete(response_json);

    return result;
}