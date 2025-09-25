@echo off
REM Cross-platform build verification script for NinjaTrader API (Windows)

echo =========================================
echo NinjaTrader API Cross-Platform Build Test
echo =========================================
echo Platform: Windows

REM Check for CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: cmake not found. Please install CMake.
    exit /b 1
)

REM Function to test build
:test_build
set BUILD_TYPE=%1
set BUILD_DIR=build_%BUILD_TYPE%

echo.
echo Testing %BUILD_TYPE% build...
echo -------------------------

REM Clean previous build
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

REM Configure
echo Configuring...
if "%BUILD_TYPE%"=="debug" (
    cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON ..
) else (
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON ..
)

if errorlevel 1 (
    echo Configuration failed!
    exit /b 1
)

REM Build
echo Building...
cmake --build .

if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

REM Test
echo Running tests...
if exist "tests\Debug\test_basic.exe" (
    tests\Debug\test_basic.exe
) else if exist "tests\Release\test_basic.exe" (
    tests\Release\test_basic.exe
) else if exist "tests\test_basic.exe" (
    tests\test_basic.exe
)

cd ..
echo %BUILD_TYPE% build: SUCCESS
goto :eof

REM Main execution
call :test_build debug
call :test_build release

echo.
echo =========================================
echo All builds completed successfully!
echo Platform: Windows
echo =========================================

echo.
echo Windows Notes:
echo - For Visual Studio builds, use: cmake -G "Visual Studio 17 2022" ..
echo - For MinGW builds, use: cmake -G "MinGW Makefiles" ..
echo - Ensure libcurl is installed via vcpkg or system package