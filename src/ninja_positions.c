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

// Helper function to parse JSON position into ninja_position_t
static ninja_error_t ninja_parse_position(cJSON* position_json, ninja_position_t* position) {
    if (!position_json || !position) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    // Clear position structure
    memset(position, 0, sizeof(ninja_position_t));

    // Parse position fields
    cJSON* account_id = cJSON_GetObjectItemCaseSensitive(position_json, "accountId");
    if (account_id && cJSON_IsNumber(account_id)) {
        position->account_id = (int)cJSON_GetNumberValue(account_id);
    }

    cJSON* net_pos = cJSON_GetObjectItemCaseSensitive(position_json, "netPos");
    if (net_pos && cJSON_IsNumber(net_pos)) {
        position->net_position = (int)cJSON_GetNumberValue(net_pos);
    }

    cJSON* avg_price = cJSON_GetObjectItemCaseSensitive(position_json, "avgPrice");
    if (avg_price && cJSON_IsNumber(avg_price)) {
        position->average_price = cJSON_GetNumberValue(avg_price);
    }

    cJSON* unrealized_pnl = cJSON_GetObjectItemCaseSensitive(position_json, "unrealizedPnL");
    if (unrealized_pnl && cJSON_IsNumber(unrealized_pnl)) {
        position->unrealized_pnl = cJSON_GetNumberValue(unrealized_pnl);
    }

    cJSON* realized_pnl = cJSON_GetObjectItemCaseSensitive(position_json, "realizedPnL");
    if (realized_pnl && cJSON_IsNumber(realized_pnl)) {
        position->realized_pnl = cJSON_GetNumberValue(realized_pnl);
    }

    cJSON* timestamp = cJSON_GetObjectItemCaseSensitive(position_json, "timestamp");
    if (timestamp && cJSON_IsString(timestamp)) {
        strncpy(position->timestamp, cJSON_GetStringValue(timestamp), sizeof(position->timestamp) - 1);
    }

    // For symbol, we need to look up the contract
    cJSON* contract_id = cJSON_GetObjectItemCaseSensitive(position_json, "contractId");
    if (contract_id && cJSON_IsNumber(contract_id)) {
        // We'd need to make another API call to get the symbol from contract ID
        // For now, just store a placeholder
        snprintf(position->symbol, sizeof(position->symbol), "CONTRACT_%d", (int)cJSON_GetNumberValue(contract_id));
    }

    return NINJA_OK;
}

ninja_error_t ninja_get_positions(ninja_client_t* client,
                                 ninja_position_t** positions,
                                 size_t* count) {
    if (!client || !positions || !count) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    *positions = NULL;
    *count = 0;

    // Make HTTP request
    ninja_http_response_t response;
    ninja_error_t result = ninja_http_get(client, "position/list", &response);

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

    size_t position_count = cJSON_GetArraySize(response_json);
    if (position_count == 0) {
        cJSON_Delete(response_json);
        return NINJA_OK;
    }

    // Allocate memory for positions
    ninja_position_t* position_array = calloc(position_count, sizeof(ninja_position_t));
    if (!position_array) {
        cJSON_Delete(response_json);
        return NINJA_ERROR_MEMORY;
    }

    // Parse each position
    for (size_t i = 0; i < position_count; i++) {
        cJSON* position_json = cJSON_GetArrayItem(response_json, i);
        if (position_json) {
            ninja_parse_position(position_json, &position_array[i]);
        }
    }

    cJSON_Delete(response_json);

    *positions = position_array;
    *count = position_count;

    return NINJA_OK;
}

ninja_error_t ninja_get_positions_by_account(ninja_client_t* client,
                                            int account_id,
                                            ninja_position_t** positions,
                                            size_t* count) {
    if (!client || account_id <= 0 || !positions || !count) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    *positions = NULL;
    *count = 0;

    char endpoint[256];
    snprintf(endpoint, sizeof(endpoint), "position/deps?masterid=%d", account_id);

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

    size_t position_count = cJSON_GetArraySize(response_json);
    if (position_count == 0) {
        cJSON_Delete(response_json);
        return NINJA_OK;
    }

    // Allocate memory for positions
    ninja_position_t* position_array = calloc(position_count, sizeof(ninja_position_t));
    if (!position_array) {
        cJSON_Delete(response_json);
        return NINJA_ERROR_MEMORY;
    }

    // Parse each position
    for (size_t i = 0; i < position_count; i++) {
        cJSON* position_json = cJSON_GetArrayItem(response_json, i);
        if (position_json) {
            ninja_parse_position(position_json, &position_array[i]);
        }
    }

    cJSON_Delete(response_json);

    *positions = position_array;
    *count = position_count;

    return NINJA_OK;
}