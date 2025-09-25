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

// Helper function to parse JSON contract into ninja_contract_t
static ninja_error_t ninja_parse_contract(cJSON* contract_json, ninja_contract_t* contract) {
    if (!contract_json || !contract) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    // Clear contract structure
    memset(contract, 0, sizeof(ninja_contract_t));

    // Parse contract fields
    cJSON* id = cJSON_GetObjectItemCaseSensitive(contract_json, "id");
    if (id && cJSON_IsNumber(id)) {
        contract->contract_id = (int)cJSON_GetNumberValue(id);
    }

    cJSON* symbol = cJSON_GetObjectItemCaseSensitive(contract_json, "name");
    if (symbol && cJSON_IsString(symbol)) {
        strncpy(contract->symbol, cJSON_GetStringValue(symbol), sizeof(contract->symbol) - 1);
        strncpy(contract->name, cJSON_GetStringValue(symbol), sizeof(contract->name) - 1);
    }

    cJSON* full_name = cJSON_GetObjectItemCaseSensitive(contract_json, "fullName");
    if (full_name && cJSON_IsString(full_name)) {
        strncpy(contract->full_name, cJSON_GetStringValue(full_name), sizeof(contract->full_name) - 1);
    }

    cJSON* exchange = cJSON_GetObjectItemCaseSensitive(contract_json, "exchange");
    if (exchange && cJSON_IsString(exchange)) {
        strncpy(contract->exchange, cJSON_GetStringValue(exchange), sizeof(contract->exchange) - 1);
    }

    cJSON* currency = cJSON_GetObjectItemCaseSensitive(contract_json, "currency");
    if (currency && cJSON_IsString(currency)) {
        strncpy(contract->currency, cJSON_GetStringValue(currency), sizeof(contract->currency) - 1);
    } else {
        strcpy(contract->currency, "USD"); // Default to USD
    }

    cJSON* tick_size = cJSON_GetObjectItemCaseSensitive(contract_json, "tickSize");
    if (tick_size && cJSON_IsNumber(tick_size)) {
        contract->tick_size = cJSON_GetNumberValue(tick_size);
    }

    cJSON* tick_value = cJSON_GetObjectItemCaseSensitive(contract_json, "tickValue");
    if (tick_value && cJSON_IsNumber(tick_value)) {
        contract->tick_value = cJSON_GetNumberValue(tick_value);
    }

    cJSON* contract_size = cJSON_GetObjectItemCaseSensitive(contract_json, "contractSize");
    if (contract_size && cJSON_IsNumber(contract_size)) {
        contract->contract_multiplier = (int)cJSON_GetNumberValue(contract_size);
    } else {
        contract->contract_multiplier = 1; // Default multiplier
    }

    cJSON* expiry_date = cJSON_GetObjectItemCaseSensitive(contract_json, "expirationDate");
    if (expiry_date && cJSON_IsString(expiry_date)) {
        strncpy(contract->expiry_date, cJSON_GetStringValue(expiry_date), sizeof(contract->expiry_date) - 1);
    }

    cJSON* tradable = cJSON_GetObjectItemCaseSensitive(contract_json, "isTradable");
    if (tradable && cJSON_IsBool(tradable)) {
        contract->is_tradable = cJSON_IsTrue(tradable);
    } else {
        contract->is_tradable = true; // Assume tradable by default
    }

    return NINJA_OK;
}

ninja_error_t ninja_get_contract_by_symbol(ninja_client_t* client,
                                          const char* symbol,
                                          ninja_contract_t* contract) {
    if (!client || !symbol || !contract) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    char endpoint[256];
    snprintf(endpoint, sizeof(endpoint), "contract/find?name=%s", symbol);

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

    result = ninja_parse_contract(response_json, contract);
    cJSON_Delete(response_json);

    return result;
}

ninja_error_t ninja_get_contract_by_id(ninja_client_t* client,
                                      int contract_id,
                                      ninja_contract_t* contract) {
    if (!client || contract_id <= 0 || !contract) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    char endpoint[256];
    snprintf(endpoint, sizeof(endpoint), "contract/item?id=%d", contract_id);

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

    result = ninja_parse_contract(response_json, contract);
    cJSON_Delete(response_json);

    return result;
}

ninja_error_t ninja_find_contracts(ninja_client_t* client,
                                  const char* search_term,
                                  ninja_contract_t** contracts,
                                  size_t* count) {
    if (!client || !search_term || !contracts || !count) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    *contracts = NULL;
    *count = 0;

    char endpoint[256];
    snprintf(endpoint, sizeof(endpoint), "contract/suggest?t=%s", search_term);

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

    if (!cJSON_IsArray(response_json)) {
        cJSON_Delete(response_json);
        return NINJA_ERROR_JSON_PARSE;
    }

    size_t contract_count = cJSON_GetArraySize(response_json);
    if (contract_count == 0) {
        cJSON_Delete(response_json);
        return NINJA_OK;
    }

    // Allocate memory for contracts
    ninja_contract_t* contract_array = calloc(contract_count, sizeof(ninja_contract_t));
    if (!contract_array) {
        cJSON_Delete(response_json);
        return NINJA_ERROR_MEMORY;
    }

    // Parse each contract
    for (size_t i = 0; i < contract_count; i++) {
        cJSON* contract_json = cJSON_GetArrayItem(response_json, i);
        if (contract_json) {
            ninja_parse_contract(contract_json, &contract_array[i]);
        }
    }

    cJSON_Delete(response_json);

    *contracts = contract_array;
    *count = contract_count;

    return NINJA_OK;
}