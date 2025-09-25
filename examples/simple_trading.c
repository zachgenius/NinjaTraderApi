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

#include <ninja/ninja_api.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("NinjaTrader API Simple Trading Example\n");
    printf("======================================\n\n");

    // Create client for demo environment
    ninja_client_t* client = ninja_client_create(NINJA_ENV_DEMO);
    if (!client) {
        printf("Error: Failed to create client\n");
        return 1;
    }

    // Authenticate (replace with your credentials)
    ninja_auth_response_t auth_response;
    ninja_error_t result = ninja_authenticate(client,
                                              "your_username",
                                              "your_password",
                                              "MyTradingApp",
                                              "1.0",
                                              &auth_response);

    if (result != NINJA_OK) {
        printf("Error: Authentication failed - %s\n", ninja_error_string(result));
        ninja_client_destroy(client);
        return 1;
    }

    printf("Authentication successful!\n");
    printf("User: %s\n", auth_response.name);
    printf("Access token expires in: %d seconds\n\n", auth_response.expires_in);

    // Get accounts
    ninja_account_t* accounts = NULL;
    size_t account_count = 0;
    result = ninja_get_accounts(client, &accounts, &account_count);

    if (result == NINJA_OK && account_count > 0) {
        printf("Found %zu account(s):\n", account_count);
        for (size_t i = 0; i < account_count; i++) {
            printf("  Account %d: %s (Balance: $%.2f, Demo: %s)\n",
                   accounts[i].account_id,
                   accounts[i].name,
                   accounts[i].balance,
                   accounts[i].is_demo ? "Yes" : "No");
        }
        printf("\n");

        // Get positions for first account
        ninja_position_t* positions = NULL;
        size_t position_count = 0;
        result = ninja_get_positions_by_account(client, accounts[0].account_id, &positions, &position_count);

        if (result == NINJA_OK) {
            if (position_count > 0) {
                printf("Current positions for account %s:\n", accounts[0].name);
                for (size_t i = 0; i < position_count; i++) {
                    printf("  %s: %d shares @ $%.2f (Unrealized P&L: $%.2f)\n",
                           positions[i].symbol,
                           positions[i].net_position,
                           positions[i].average_price,
                           positions[i].unrealized_pnl);
                }
            } else {
                printf("No open positions\n");
            }
            ninja_free_array(positions);
        }

        // Get orders
        ninja_order_t* orders = NULL;
        size_t order_count = 0;
        result = ninja_get_orders(client, &orders, &order_count);

        if (result == NINJA_OK) {
            if (order_count > 0) {
                printf("\nOpen orders:\n");
                for (size_t i = 0; i < order_count; i++) {
                    printf("  Order %s: %s %d %s @ $%.2f (Status: %d)\n",
                           orders[i].order_id,
                           (orders[i].side == NINJA_SIDE_BUY) ? "BUY" : "SELL",
                           orders[i].quantity,
                           orders[i].symbol,
                           orders[i].price,
                           orders[i].status);
                }
            } else {
                printf("\nNo open orders\n");
            }
            ninja_free_array(orders);
        }

        // Example: Search for ES contract
        ninja_contract_t contract;
        result = ninja_get_contract_by_symbol(client, "ES", &contract);
        if (result == NINJA_OK) {
            printf("\nFound contract ES:\n");
            printf("  Name: %s\n", contract.name);
            printf("  Exchange: %s\n", contract.exchange);
            printf("  Tick Size: %.4f\n", contract.tick_size);
            printf("  Tick Value: $%.2f\n", contract.tick_value);
        }

        ninja_free_array(accounts);
    } else {
        printf("Error getting accounts: %s\n", ninja_error_string(result));
    }

    // Clean up
    ninja_client_destroy(client);

    printf("\nExample completed successfully!\n");
    return 0;
}