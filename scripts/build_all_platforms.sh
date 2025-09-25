#!/bin/bash
# Cross-platform build verification script for NinjaTrader API

set -e

echo "========================================="
echo "NinjaTrader API Cross-Platform Build Test"
echo "========================================="

# Function to test build on current platform
test_build() {
    local build_type=$1
    local build_dir="build_${build_type}"

    echo ""
    echo "Testing ${build_type} build..."
    echo "-------------------------"

    # Clean previous build
    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"
    cd "${build_dir}"

    # Configure
    echo "Configuring..."
    if [[ "${build_type}" == "debug" ]]; then
        cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON ..
    else
        cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON ..
    fi

    # Build
    echo "Building..."
    cmake --build .

    # Test
    echo "Running tests..."
    if [[ -f "./tests/test_basic" ]]; then
        ./tests/test_basic
    elif [[ -f "./tests/test_basic.exe" ]]; then
        ./tests/test_basic.exe
    fi

    cd ..
    echo "${build_type} build: SUCCESS"
}

# Detect platform
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Platform: Linux"
    PLATFORM="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Platform: macOS"
    PLATFORM="macos"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "win32" ]]; then
    echo "Platform: Windows"
    PLATFORM="windows"
else
    echo "Platform: Unknown ($OSTYPE)"
    PLATFORM="unknown"
fi

# Check dependencies
echo "Checking dependencies..."
if ! command -v cmake &> /dev/null; then
    echo "ERROR: cmake not found. Please install CMake."
    exit 1
fi

if ! command -v curl-config &> /dev/null && ! pkg-config --exists libcurl; then
    echo "WARNING: libcurl development files may not be installed."
    echo "On Ubuntu/Debian: sudo apt-get install libcurl4-openssl-dev"
    echo "On macOS: brew install curl"
    echo "On Windows: vcpkg install curl"
fi

# Test builds
test_build "debug"
test_build "release"

echo ""
echo "========================================="
echo "All builds completed successfully!"
echo "Platform: ${PLATFORM}"
echo "========================================="

# Platform-specific notes
if [[ "$PLATFORM" == "windows" ]]; then
    echo ""
    echo "Windows Notes:"
    echo "- For Visual Studio builds, use: cmake -G \"Visual Studio 17 2022\" .."
    echo "- For MinGW builds, use: cmake -G \"MinGW Makefiles\" .."
    echo "- Ensure libcurl is installed via vcpkg or system package"
elif [[ "$PLATFORM" == "linux" ]]; then
    echo ""
    echo "Linux Notes:"
    echo "- Install libcurl: sudo apt-get install libcurl4-openssl-dev"
    echo "- Or for CentOS/RHEL: sudo yum install libcurl-devel"
elif [[ "$PLATFORM" == "macos" ]]; then
    echo ""
    echo "macOS Notes:"
    echo "- libcurl is included with Xcode Command Line Tools"
    echo "- For Homebrew: brew install curl cmake"
fi