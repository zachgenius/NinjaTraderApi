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

// Helper function to convert order side enum to string
static const char* ninja_order_side_to_string(ninja_order_side_t side) {
    switch (side) {
        case NINJA_SIDE_BUY: return "Buy";
        case NINJA_SIDE_SELL: return "Sell";
        default: return "Buy";
    }
}

// Helper function to convert order type enum to string
static const char* ninja_order_type_to_string(ninja_order_type_t type) {
    switch (type) {
        case NINJA_ORDER_MARKET: return "Market";
        case NINJA_ORDER_LIMIT: return "Limit";
        case NINJA_ORDER_STOP: return "Stop";
        case NINJA_ORDER_STOP_LIMIT: return "StopLimit";
        default: return "Market";
    }
}

// Helper function to parse order status from string
static ninja_order_status_t ninja_parse_order_status(const char* status_str) {
    if (!status_str) return NINJA_ORDER_PENDING;

    if (strcmp(status_str, "Working") == 0) return NINJA_ORDER_WORKING;
    if (strcmp(status_str, "Filled") == 0) return NINJA_ORDER_FILLED;
    if (strcmp(status_str, "Cancelled") == 0) return NINJA_ORDER_CANCELLED;
    if (strcmp(status_str, "Rejected") == 0) return NINJA_ORDER_REJECTED;

    return NINJA_ORDER_PENDING;
}

// Helper function to parse JSON order into ninja_order_t
static ninja_error_t ninja_parse_order(cJSON* order_json, ninja_order_t* order) {
    if (!order_json || !order) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    // Clear order structure
    memset(order, 0, sizeof(ninja_order_t));

    // Parse order fields
    cJSON* id = cJSON_GetObjectItemCaseSensitive(order_json, "id");
    if (id && cJSON_IsNumber(id)) {
        snprintf(order->order_id, sizeof(order->order_id), "%d", (int)cJSON_GetNumberValue(id));
    }

    cJSON* account_id = cJSON_GetObjectItemCaseSensitive(order_json, "accountId");
    if (account_id && cJSON_IsNumber(account_id)) {
        order->account_id = (int)cJSON_GetNumberValue(account_id);
    }

    cJSON* action = cJSON_GetObjectItemCaseSensitive(order_json, "action");
    if (action && cJSON_IsString(action)) {
        const char* action_str = cJSON_GetStringValue(action);
        order->side = (strcmp(action_str, "Buy") == 0) ? NINJA_SIDE_BUY : NINJA_SIDE_SELL;
    }

    cJSON* order_type = cJSON_GetObjectItemCaseSensitive(order_json, "orderType");
    if (order_type && cJSON_IsString(order_type)) {
        const char* type_str = cJSON_GetStringValue(order_type);
        if (strcmp(type_str, "Market") == 0) order->type = NINJA_ORDER_MARKET;
        else if (strcmp(type_str, "Limit") == 0) order->type = NINJA_ORDER_LIMIT;
        else if (strcmp(type_str, "Stop") == 0) order->type = NINJA_ORDER_STOP;
        else order->type = NINJA_ORDER_MARKET;
    }

    cJSON* ord_status = cJSON_GetObjectItemCaseSensitive(order_json, "ordStatus");
    if (ord_status && cJSON_IsString(ord_status)) {
        order->status = ninja_parse_order_status(cJSON_GetStringValue(ord_status));
    }

    cJSON* order_qty = cJSON_GetObjectItemCaseSensitive(order_json, "orderQty");
    if (order_qty && cJSON_IsNumber(order_qty)) {
        order->quantity = (int)cJSON_GetNumberValue(order_qty);
    }

    cJSON* price = cJSON_GetObjectItemCaseSensitive(order_json, "price");
    if (price && cJSON_IsNumber(price)) {
        order->price = cJSON_GetNumberValue(price);
    }

    cJSON* stop_price = cJSON_GetObjectItemCaseSensitive(order_json, "stopPrice");
    if (stop_price && cJSON_IsNumber(stop_price)) {
        order->stop_price = cJSON_GetNumberValue(stop_price);
    }

    cJSON* filled_qty = cJSON_GetObjectItemCaseSensitive(order_json, "filledQty");
    if (filled_qty && cJSON_IsNumber(filled_qty)) {
        order->filled_quantity = (int)cJSON_GetNumberValue(filled_qty);
    }

    cJSON* avg_fill_price = cJSON_GetObjectItemCaseSensitive(order_json, "avgFillPrice");
    if (avg_fill_price && cJSON_IsNumber(avg_fill_price)) {
        order->filled_price = cJSON_GetNumberValue(avg_fill_price);
    }

    cJSON* timestamp = cJSON_GetObjectItemCaseSensitive(order_json, "timestamp");
    if (timestamp && cJSON_IsString(timestamp)) {
        strncpy(order->timestamp, cJSON_GetStringValue(timestamp), sizeof(order->timestamp) - 1);
    }

    cJSON* is_automated = cJSON_GetObjectItemCaseSensitive(order_json, "isAutomated");
    if (is_automated && cJSON_IsBool(is_automated)) {
        order->is_automated = cJSON_IsTrue(is_automated);
    }

    return NINJA_OK;
}

ninja_error_t ninja_place_order(ninja_client_t* client,
                               const char* account_spec,
                               int account_id,
                               const char* symbol,
                               ninja_order_side_t side,
                               ninja_order_type_t type,
                               int quantity,
                               double price,
                               double stop_price,
                               bool is_automated,
                               ninja_order_t* order_out) {
    if (!client || !account_spec || !symbol || !order_out || quantity <= 0) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    // Create JSON request body
    cJSON* json = cJSON_CreateObject();
    if (!json) {
        return NINJA_ERROR_JSON_PARSE;
    }

    cJSON_AddStringToObject(json, "accountSpec", account_spec);
    cJSON_AddNumberToObject(json, "accountId", account_id);
    cJSON_AddStringToObject(json, "symbol", symbol);
    cJSON_AddStringToObject(json, "action", ninja_order_side_to_string(side));
    cJSON_AddStringToObject(json, "orderType", ninja_order_type_to_string(type));
    cJSON_AddNumberToObject(json, "orderQty", quantity);
    cJSON_AddBoolToObject(json, "isAutomated", is_automated);

    // Add price fields based on order type
    if (type == NINJA_ORDER_LIMIT || type == NINJA_ORDER_STOP_LIMIT) {
        cJSON_AddNumberToObject(json, "price", price);
    }
    if (type == NINJA_ORDER_STOP || type == NINJA_ORDER_STOP_LIMIT) {
        cJSON_AddNumberToObject(json, "stopPrice", stop_price);
    }

    char* json_string = cJSON_Print(json);
    cJSON_Delete(json);

    if (!json_string) {
        return NINJA_ERROR_JSON_PARSE;
    }

    // Make HTTP request
    ninja_http_response_t response;
    ninja_error_t result = ninja_http_post(client, "order/placeorder", json_string, &response);
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
        strncpy(order_out->error_text, cJSON_GetStringValue(error_text), sizeof(order_out->error_text) - 1);
        cJSON_Delete(response_json);
        return NINJA_ERROR_ORDER_REJECTED;
    }

    // Parse order data from response
    result = ninja_parse_order(response_json, order_out);
    cJSON_Delete(response_json);

    return result;
}

ninja_error_t ninja_cancel_order(ninja_client_t* client, const char* order_id) {
    if (!client || !order_id) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    // Create JSON request body
    cJSON* json = cJSON_CreateObject();
    if (!json) {
        return NINJA_ERROR_JSON_PARSE;
    }

    cJSON_AddStringToObject(json, "orderId", order_id);

    char* json_string = cJSON_Print(json);
    cJSON_Delete(json);

    if (!json_string) {
        return NINJA_ERROR_JSON_PARSE;
    }

    // Make HTTP request
    ninja_http_response_t response;
    ninja_error_t result = ninja_http_post(client, "order/cancelorder", json_string, &response);
    free(json_string);
    ninja_http_response_free(&response);

    return result;
}

ninja_error_t ninja_modify_order(ninja_client_t* client,
                                const char* order_id,
                                int new_quantity,
                                double new_price) {
    if (!client || !order_id || new_quantity <= 0) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    // Create JSON request body
    cJSON* json = cJSON_CreateObject();
    if (!json) {
        return NINJA_ERROR_JSON_PARSE;
    }

    cJSON_AddStringToObject(json, "orderId", order_id);
    cJSON_AddNumberToObject(json, "orderQty", new_quantity);
    cJSON_AddNumberToObject(json, "price", new_price);

    char* json_string = cJSON_Print(json);
    cJSON_Delete(json);

    if (!json_string) {
        return NINJA_ERROR_JSON_PARSE;
    }

    // Make HTTP request
    ninja_http_response_t response;
    ninja_error_t result = ninja_http_post(client, "order/modifyorder", json_string, &response);
    free(json_string);
    ninja_http_response_free(&response);

    return result;
}

ninja_error_t ninja_get_orders(ninja_client_t* client,
                              ninja_order_t** orders,
                              size_t* count) {
    if (!client || !orders || !count) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    *orders = NULL;
    *count = 0;

    // Make HTTP request
    ninja_http_response_t response;
    ninja_error_t result = ninja_http_get(client, "order/list", &response);

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

    size_t order_count = cJSON_GetArraySize(response_json);
    if (order_count == 0) {
        cJSON_Delete(response_json);
        return NINJA_OK;
    }

    // Allocate memory for orders
    ninja_order_t* order_array = calloc(order_count, sizeof(ninja_order_t));
    if (!order_array) {
        cJSON_Delete(response_json);
        return NINJA_ERROR_MEMORY;
    }

    // Parse each order
    for (size_t i = 0; i < order_count; i++) {
        cJSON* order_json = cJSON_GetArrayItem(response_json, i);
        if (order_json) {
            ninja_parse_order(order_json, &order_array[i]);
        }
    }

    cJSON_Delete(response_json);

    *orders = order_array;
    *count = order_count;

    return NINJA_OK;
}

ninja_error_t ninja_get_order_by_id(ninja_client_t* client,
                                   const char* order_id,
                                   ninja_order_t* order) {
    if (!client || !order_id || !order) {
        return NINJA_ERROR_INVALID_PARAM;
    }

    char endpoint[256];
    snprintf(endpoint, sizeof(endpoint), "order/item?id=%s", order_id);

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

    result = ninja_parse_order(response_json, order);
    cJSON_Delete(response_json);

    return result;
}