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
#include <assert.h>
#include <string.h>

// Simple test framework
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s - %s\n", __func__, message); \
            return 0; \
        } \
    } while(0)

#define TEST_PASS() \
    do { \
        printf("PASS: %s\n", __func__); \
        return 1; \
    } while(0)

// Test client creation and destruction
int test_client_lifecycle() {
    ninja_client_t* client = ninja_client_create(NINJA_ENV_DEMO);
    TEST_ASSERT(client != NULL, "Client creation failed");

    ninja_client_destroy(client);
    TEST_PASS();
}

// Test error string function
int test_error_strings() {
    const char* error_str = ninja_error_string(NINJA_OK);
    TEST_ASSERT(error_str != NULL, "Error string is NULL");
    TEST_ASSERT(strlen(error_str) > 0, "Error string is empty");

    error_str = ninja_error_string(NINJA_ERROR_AUTH);
    TEST_ASSERT(error_str != NULL, "Auth error string is NULL");
    TEST_ASSERT(strstr(error_str, "Authentication") != NULL, "Auth error string is incorrect");

    TEST_PASS();
}

// Test invalid parameters
int test_invalid_parameters() {
    // Test NULL client
    ninja_auth_response_t auth;
    ninja_error_t result = ninja_authenticate(NULL, "user", "pass", "app", "1.0", &auth);
    TEST_ASSERT(result == NINJA_ERROR_INVALID_PARAM, "Should reject NULL client");

    // Test NULL parameters
    ninja_client_t* client = ninja_client_create(NINJA_ENV_DEMO);
    TEST_ASSERT(client != NULL, "Client creation failed");

    result = ninja_authenticate(client, NULL, "pass", "app", "1.0", &auth);
    TEST_ASSERT(result == NINJA_ERROR_INVALID_PARAM, "Should reject NULL username");

    result = ninja_authenticate(client, "user", NULL, "app", "1.0", &auth);
    TEST_ASSERT(result == NINJA_ERROR_INVALID_PARAM, "Should reject NULL password");

    result = ninja_authenticate(client, "user", "pass", "app", "1.0", NULL);
    TEST_ASSERT(result == NINJA_ERROR_INVALID_PARAM, "Should reject NULL auth response");

    ninja_client_destroy(client);
    TEST_PASS();
}

// Test order parameter validation
int test_order_parameters() {
    ninja_client_t* client = ninja_client_create(NINJA_ENV_DEMO);
    TEST_ASSERT(client != NULL, "Client creation failed");

    ninja_order_t order;
    ninja_error_t result;

    // Test invalid quantity
    result = ninja_place_order(client, "account", 123, "ES", NINJA_SIDE_BUY,
                              NINJA_ORDER_LIMIT, 0, 4200.0, 0.0, true, &order);
    TEST_ASSERT(result == NINJA_ERROR_INVALID_PARAM, "Should reject zero quantity");

    // Test negative quantity
    result = ninja_place_order(client, "account", 123, "ES", NINJA_SIDE_BUY,
                              NINJA_ORDER_LIMIT, -1, 4200.0, 0.0, true, &order);
    TEST_ASSERT(result == NINJA_ERROR_INVALID_PARAM, "Should reject negative quantity");

    // Test NULL parameters
    result = ninja_place_order(NULL, "account", 123, "ES", NINJA_SIDE_BUY,
                              NINJA_ORDER_LIMIT, 1, 4200.0, 0.0, true, &order);
    TEST_ASSERT(result == NINJA_ERROR_INVALID_PARAM, "Should reject NULL client");

    result = ninja_place_order(client, NULL, 123, "ES", NINJA_SIDE_BUY,
                              NINJA_ORDER_LIMIT, 1, 4200.0, 0.0, true, &order);
    TEST_ASSERT(result == NINJA_ERROR_INVALID_PARAM, "Should reject NULL account spec");

    result = ninja_place_order(client, "account", 123, NULL, NINJA_SIDE_BUY,
                              NINJA_ORDER_LIMIT, 1, 4200.0, 0.0, true, &order);
    TEST_ASSERT(result == NINJA_ERROR_INVALID_PARAM, "Should reject NULL symbol");

    ninja_client_destroy(client);
    TEST_PASS();
}

// Test memory management
int test_memory_management() {
    // Test free_array with NULL
    ninja_free_array(NULL); // Should not crash

    TEST_PASS();
}

int main() {
    printf("Running NinjaTrader API Basic Tests\n");
    printf("==================================\n\n");

    int tests_run = 0;
    int tests_passed = 0;

    // Run tests
    tests_run++; if (test_client_lifecycle()) tests_passed++;
    tests_run++; if (test_error_strings()) tests_passed++;
    tests_run++; if (test_invalid_parameters()) tests_passed++;
    tests_run++; if (test_order_parameters()) tests_passed++;
    tests_run++; if (test_memory_management()) tests_passed++;

    printf("\nTest Results: %d/%d passed\n", tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}