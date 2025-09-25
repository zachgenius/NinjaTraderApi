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

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
typedef enum {
    NINJA_OK = 0,
    NINJA_ERROR_AUTH = -1,
    NINJA_ERROR_CONNECTION = -2,
    NINJA_ERROR_INVALID_PARAM = -3,
    NINJA_ERROR_JSON_PARSE = -4,
    NINJA_ERROR_TIMEOUT = -5,
    NINJA_ERROR_ORDER_REJECTED = -6,
    NINJA_ERROR_HTTP = -7,
    NINJA_ERROR_MEMORY = -8,
    NINJA_ERROR_NOT_FOUND = -9
} ninja_error_t;

// Environment types
typedef enum {
    NINJA_ENV_DEMO,
    NINJA_ENV_LIVE
} ninja_env_t;

// Order side
typedef enum {
    NINJA_SIDE_BUY,
    NINJA_SIDE_SELL
} ninja_order_side_t;

// Order type
typedef enum {
    NINJA_ORDER_MARKET,
    NINJA_ORDER_LIMIT,
    NINJA_ORDER_STOP,
    NINJA_ORDER_STOP_LIMIT
} ninja_order_type_t;

// Order status
typedef enum {
    NINJA_ORDER_PENDING,
    NINJA_ORDER_WORKING,
    NINJA_ORDER_FILLED,
    NINJA_ORDER_CANCELLED,
    NINJA_ORDER_REJECTED
} ninja_order_status_t;

// Forward declarations
typedef struct ninja_client ninja_client_t;

// Authentication response
typedef struct {
    char access_token[256];
    char md_access_token[256];
    char name[64];
    int user_id;
    int expires_in;
} ninja_auth_response_t;

// Order structure
typedef struct {
    char order_id[64];
    int account_id;
    char symbol[32];
    ninja_order_side_t side;
    ninja_order_type_t type;
    ninja_order_status_t status;
    int quantity;
    double price;
    double stop_price;
    double filled_price;
    int filled_quantity;
    char timestamp[32];
    bool is_automated;
    char error_text[256];
} ninja_order_t;

// Position structure
typedef struct {
    int account_id;
    char symbol[32];
    int net_position;
    double average_price;
    double unrealized_pnl;
    double realized_pnl;
    char timestamp[32];
} ninja_position_t;

// Account structure
typedef struct {
    int account_id;
    char name[64];
    char account_spec[64];
    double balance;
    double equity;
    double margin_used;
    double margin_available;
    double buying_power;
    char currency[8];
    bool is_demo;
} ninja_account_t;

// Contract structure
typedef struct {
    int contract_id;
    char symbol[32];
    char name[64];
    char full_name[128];
    char exchange[32];
    char currency[8];
    double tick_size;
    double tick_value;
    int contract_multiplier;
    char expiry_date[32];
    bool is_tradable;
} ninja_contract_t;

// HTTP response structure (internal)
typedef struct {
    char* data;
    size_t size;
    long status_code;
} ninja_http_response_t;

#ifdef __cplusplus
}
#endif