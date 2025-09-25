# NinjaTrader API Library

A lightweight, cross-platform C library for accessing NinjaTrader REST API endpoints.

## Features

- **Pure C implementation** - C99 standard, minimal dependencies, maximum compatibility
- **Cross-platform** - Works on Linux, macOS, and Windows
- **REST API endpoints** - Authentication, orders, accounts, positions, contracts
- **Clean API** - Simple, consistent interface
- **Minimal dependencies** - Only libcurl required (cJSON submodule)
- **Memory safe** - Proper error handling and resource management

## API Coverage

### ✅ Implemented
- **Authentication** - Login, token renewal
- **Account Management** - Get accounts, account details
- **Order Management** - Place, cancel, modify, query orders
- **Position Tracking** - Get positions by account
- **Contract Lookup** - Search contracts by symbol/ID

### 🚧 Future (WebSocket)
- Real-time market data
- Real-time order updates
- Streaming quotes and charts

## Dependencies

### Required
- **libcurl** - HTTP/HTTPS client (standard on most systems)

### Submodule
- **cJSON** - JSON parsing (v1.7.19 via git submodule)

## Building

### Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt-get install libcurl4-openssl-dev cmake build-essential
```

**macOS:**
```bash
brew install curl cmake
```

**Windows:**
```cmd
# Install Visual Studio with C++ support
# Install vcpkg and libcurl
vcpkg install curl
```

### Build Steps

**Unix-like systems (Linux/macOS):**
```bash
# Clone the repository
git clone https://github.com/zachgenius/NinjaTraderApi.git
cd NinjaTraderApi

# Initialize submodules
git submodule update --init --recursive

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Optional: Install system-wide
sudo cmake --install .
```

**Windows (Visual Studio):**
```cmd
REM Clone the repository
git clone <repository-url>
cd NinjaTraderApi

REM Initialize submodules
git submodule update --init --recursive

REM Create build directory
mkdir build
cd build

REM Configure (Visual Studio 2022)
cmake -G "Visual Studio 17 2022" -A x64 ..

REM Build
cmake --build . --config Release

REM Optional: Install system-wide (run as Administrator)
cmake --install .
```

**Windows (MinGW):**
```cmd
REM Clone the repository
git clone <repository-url>
cd NinjaTraderApi

REM Initialize submodules
git submodule update --init --recursive

REM Create build directory
mkdir build
cd build

REM Configure
cmake -G "MinGW Makefiles" ..

REM Build
cmake --build .
```

### CMake Options

```bash
# Build examples
cmake -DBUILD_EXAMPLES=ON ..

# Build tests
cmake -DBUILD_TESTS=ON ..

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (default)
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Cross-Platform Build Verification

To verify builds work correctly across platforms, use the provided scripts:

**Unix-like systems (Linux/macOS):**
```bash
./scripts/build_all_platforms.sh
```

**Windows:**
```cmd
scripts\build_all_platforms.bat
```

These scripts will:
- Test both Debug and Release builds
- Run all unit tests
- Verify cross-platform compatibility
- Provide platform-specific guidance

## Quick Start

### 1. Basic Usage

```c
#include <ninja/ninja_api.h>

int main() {
    // Create client (demo environment)
    ninja_client_t* client = ninja_client_create(NINJA_ENV_DEMO);

    // Authenticate
    ninja_auth_response_t auth;
    ninja_error_t result = ninja_authenticate(client,
                                              "username",
                                              "password",
                                              "MyApp",
                                              "1.0",
                                              &auth);

    if (result != NINJA_OK) {
        printf("Auth failed: %s\n", ninja_error_string(result));
        ninja_client_destroy(client);
        return 1;
    }

    // Get accounts
    ninja_account_t* accounts;
    size_t count;
    result = ninja_get_accounts(client, &accounts, &count);

    if (result == NINJA_OK) {
        printf("Found %zu accounts\n", count);
        for (size_t i = 0; i < count; i++) {
            printf("Account: %s, Balance: $%.2f\n",
                   accounts[i].name, accounts[i].balance);
        }
        ninja_free_array(accounts);
    }

    ninja_client_destroy(client);
    return 0;
}
```

### 2. Place Orders

```c
// Place limit buy order
ninja_order_t order;
ninja_error_t result = ninja_place_order(client,
                                         "MyAccount",      // account spec
                                         12345,            // account ID
                                         "ESM4",           // symbol
                                         NINJA_SIDE_BUY,   // side
                                         NINJA_ORDER_LIMIT, // type
                                         1,                // quantity
                                         4200.0,           // limit price
                                         0.0,              // stop price
                                         true,             // is_automated
                                         &order);

if (result == NINJA_OK) {
    printf("Order placed: %s\n", order.order_id);
} else {
    printf("Order failed: %s\n", ninja_error_string(result));
}
```

### 3. Error Handling

```c
ninja_error_t result = ninja_get_orders(client, &orders, &count);

switch (result) {
    case NINJA_OK:
        printf("Success: %zu orders\n", count);
        break;
    case NINJA_ERROR_AUTH:
        printf("Authentication expired\n");
        // Try token renewal
        break;
    case NINJA_ERROR_CONNECTION:
        printf("Network error\n");
        break;
    default:
        printf("Error: %s\n", ninja_error_string(result));
        break;
}
```

## API Reference

### Core Functions

```c
// Client management
ninja_client_t* ninja_client_create(ninja_env_t env);
void ninja_client_destroy(ninja_client_t* client);

// Authentication
ninja_error_t ninja_authenticate(client, username, password, app_id, app_version, auth_response);
ninja_error_t ninja_renew_token(client, auth_response);

// Error handling
const char* ninja_error_string(ninja_error_t error);
void ninja_free_array(void* array);
```

### Account Operations

```c
// Get all accounts
ninja_error_t ninja_get_accounts(client, accounts, count);

// Get specific account
ninja_error_t ninja_get_account_by_id(client, account_id, account);
```

### Order Operations

```c
// Place order
ninja_error_t ninja_place_order(client, account_spec, account_id, symbol,
                               side, type, quantity, price, stop_price,
                               is_automated, order_out);

// Cancel order
ninja_error_t ninja_cancel_order(client, order_id);

// Modify order
ninja_error_t ninja_modify_order(client, order_id, new_quantity, new_price);

// Query orders
ninja_error_t ninja_get_orders(client, orders, count);
ninja_error_t ninja_get_order_by_id(client, order_id, order);
```

### Position Operations

```c
// Get all positions
ninja_error_t ninja_get_positions(client, positions, count);

// Get positions by account
ninja_error_t ninja_get_positions_by_account(client, account_id, positions, count);
```

### Contract Operations

```c
// Lookup by symbol
ninja_error_t ninja_get_contract_by_symbol(client, symbol, contract);

// Lookup by ID
ninja_error_t ninja_get_contract_by_id(client, contract_id, contract);

// Search contracts
ninja_error_t ninja_find_contracts(client, search_term, contracts, count);
```

## Data Structures

### ninja_order_t
```c
typedef struct {
    char order_id[64];
    int account_id;
    char symbol[32];
    ninja_order_side_t side;        // NINJA_SIDE_BUY/SELL
    ninja_order_type_t type;        // NINJA_ORDER_MARKET/LIMIT/STOP
    ninja_order_status_t status;    // NINJA_ORDER_WORKING/FILLED/etc
    int quantity;
    double price;
    double stop_price;
    double filled_price;
    int filled_quantity;
    char timestamp[32];
    bool is_automated;
    char error_text[256];
} ninja_order_t;
```

### ninja_account_t
```c
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
```

## Environment Setup

### Demo Environment
```c
ninja_client_t* client = ninja_client_create(NINJA_ENV_DEMO);
// Uses: https://demo.tradovateapi.com/v1
```

### Live Environment
```c
ninja_client_t* client = ninja_client_create(NINJA_ENV_LIVE);
// Uses: https://live.tradovateapi.com/v1
```

## Thread Safety

- **Not thread-safe** - Use separate client instances per thread
- **Single-threaded per client** - Don't share clients across threads
- **Concurrent clients OK** - Multiple clients can be used simultaneously

## Cross-Platform Notes

### Linux
- Uses standard POSIX APIs
- Requires `libcurl4-openssl-dev` or `libcurl4-gnutls-dev`
- Tested on Ubuntu 20.04+ and CentOS 8+

### macOS
- Uses standard BSD/POSIX APIs
- libcurl included with Xcode Command Line Tools
- Tested on macOS 11+ (Big Sur and later)

### Windows
- Uses WinSock2 for networking
- Requires Visual Studio 2019+ or MinGW-w64
- Supports both MSVC and MinGW builds
- Links against `ws2_32.lib` automatically

#### Windows Build Requirements:
- **Visual Studio**: Recommended for production builds
- **vcpkg**: For libcurl dependency management
- **MinGW**: Alternative compiler (may have some compatibility issues)

#### Windows-Specific Considerations:
- Uses `Sleep()` instead of `sleep()` for delays
- Handles secure string function warnings automatically
- Network initialization handled transparently

## Examples

The `examples/` directory contains:
- `simple_trading.c` - Basic authentication and account listing
- `place_order.c` - Order placement and cancellation

Build examples:
```bash
cmake -DBUILD_EXAMPLES=ON ..
cmake --build .
./examples/simple_trading
```

## Error Codes

```c
NINJA_OK = 0                    // Success
NINJA_ERROR_AUTH = -1           // Authentication failed
NINJA_ERROR_CONNECTION = -2     // Network/HTTP error
NINJA_ERROR_INVALID_PARAM = -3  // Invalid parameters
NINJA_ERROR_JSON_PARSE = -4     // JSON parsing failed
NINJA_ERROR_TIMEOUT = -5        // Request timeout
NINJA_ERROR_ORDER_REJECTED = -6 // Order rejected by server
NINJA_ERROR_HTTP = -7           // HTTP error (4xx/5xx)
NINJA_ERROR_MEMORY = -8         // Memory allocation failed
NINJA_ERROR_NOT_FOUND = -9      // Resource not found
```

## License

This library is provided as-is for educational and development purposes.

## Support

- Check examples for common usage patterns
- Review error messages and status codes
- Ensure proper authentication and account setup
- Use demo environment for testing