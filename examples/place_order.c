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
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #define sleep(x) Sleep((x) * 1000)
#else
    #include <unistd.h>
#endif

int main() {
    printf("NinjaTrader API Place Order Example\n");
    printf("===================================\n\n");

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
                                              "OrderPlacingApp",
                                              "1.0",
                                              &auth_response);

    if (result != NINJA_OK) {
        printf("Error: Authentication failed - %s\n", ninja_error_string(result));
        ninja_client_destroy(client);
        return 1;
    }

    printf("Authentication successful!\n\n");

    // Get first account
    ninja_account_t* accounts = NULL;
    size_t account_count = 0;
    result = ninja_get_accounts(client, &accounts, &account_count);

    if (result != NINJA_OK || account_count == 0) {
        printf("Error: No accounts found\n");
        ninja_client_destroy(client);
        return 1;
    }

    ninja_account_t* account = &accounts[0];
    printf("Using account: %s (ID: %d)\n", account->name, account->account_id);
    printf("Available balance: $%.2f\n\n", account->balance);

    // Place a limit order for ES
    printf("Placing limit buy order for 1 ES contract...\n");

    ninja_order_t order;
    result = ninja_place_order(client,
                               account->account_spec,
                               account->account_id,
                               "ESM4",  // ES March 2024 contract
                               NINJA_SIDE_BUY,
                               NINJA_ORDER_LIMIT,
                               1,       // quantity
                               4200.0,  // limit price
                               0.0,     // stop price (not used for limit orders)
                               true,    // is_automated
                               &order);

    if (result == NINJA_OK) {
        printf("Order placed successfully!\n");
        printf("Order ID: %s\n", order.order_id);
        printf("Symbol: %s\n", order.symbol);
        printf("Side: %s\n", (order.side == NINJA_SIDE_BUY) ? "BUY" : "SELL");
        printf("Quantity: %d\n", order.quantity);
        printf("Price: $%.2f\n", order.price);
        printf("Status: %d\n", order.status);

        // Wait a moment, then try to cancel the order
        printf("\nWaiting 2 seconds...\n");
        #ifdef _WIN32
            Sleep(2000);
        #else
            sleep(2);
        #endif

        printf("Attempting to cancel order %s...\n", order.order_id);
        result = ninja_cancel_order(client, order.order_id);

        if (result == NINJA_OK) {
            printf("Order cancellation request sent successfully\n");
        } else {
            printf("Error cancelling order: %s\n", ninja_error_string(result));
        }
    } else {
        printf("Error placing order: %s\n", ninja_error_string(result));
        if (strlen(order.error_text) > 0) {
            printf("Server error: %s\n", order.error_text);
        }
    }

    // Clean up
    ninja_free_array(accounts);
    ninja_client_destroy(client);

    printf("\nOrder example completed\n");
    return 0;
}