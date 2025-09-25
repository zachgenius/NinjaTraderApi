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

#include "ninja_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Client management
ninja_client_t* ninja_client_create(ninja_env_t env);
void ninja_client_destroy(ninja_client_t* client);

// Authentication
ninja_error_t ninja_authenticate(ninja_client_t* client,
                                const char* username,
                                const char* password,
                                const char* app_id,
                                const char* app_version,
                                ninja_auth_response_t* auth_response);

ninja_error_t ninja_renew_token(ninja_client_t* client, ninja_auth_response_t* auth_response);

// Account operations
ninja_error_t ninja_get_accounts(ninja_client_t* client,
                                ninja_account_t** accounts,
                                size_t* count);

ninja_error_t ninja_get_account_by_id(ninja_client_t* client,
                                     int account_id,
                                     ninja_account_t* account);

// Order operations
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
                               ninja_order_t* order_out);

ninja_error_t ninja_cancel_order(ninja_client_t* client,
                                const char* order_id);

ninja_error_t ninja_modify_order(ninja_client_t* client,
                                const char* order_id,
                                int new_quantity,
                                double new_price);

ninja_error_t ninja_get_orders(ninja_client_t* client,
                              ninja_order_t** orders,
                              size_t* count);

ninja_error_t ninja_get_order_by_id(ninja_client_t* client,
                                   const char* order_id,
                                   ninja_order_t* order);

// Position operations
ninja_error_t ninja_get_positions(ninja_client_t* client,
                                 ninja_position_t** positions,
                                 size_t* count);

ninja_error_t ninja_get_positions_by_account(ninja_client_t* client,
                                            int account_id,
                                            ninja_position_t** positions,
                                            size_t* count);

// Contract operations
ninja_error_t ninja_get_contract_by_symbol(ninja_client_t* client,
                                          const char* symbol,
                                          ninja_contract_t* contract);

ninja_error_t ninja_get_contract_by_id(ninja_client_t* client,
                                      int contract_id,
                                      ninja_contract_t* contract);

ninja_error_t ninja_find_contracts(ninja_client_t* client,
                                  const char* search_term,
                                  ninja_contract_t** contracts,
                                  size_t* count);

// Utility functions
const char* ninja_error_string(ninja_error_t error);
void ninja_free_array(void* array);

#ifdef __cplusplus
}
#endif