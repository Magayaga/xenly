@echo off

set SOURCE_FILE=sources\xenly.c
set OUTPUT_FILE=xenly.exe

rem Check if GCC is installed
gcc --version >nul 2>&1
if %errorlevel% neq 0 (
    echo GCC not found. Please install GCC and add it to your system PATH.
    exit /b 1
)

rem Compile the C code
gcc -o %OUTPUT_FILE% %SOURCE_FILE%

rem Check if compilation was successful
if %errorlevel% equ 0 (
    echo Compilation successful. Running xenly programming language %OUTPUT_FILE%
) else (
    echo Compilation failed.
)

exit /b %errorlevel%