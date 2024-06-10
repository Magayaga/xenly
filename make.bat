@echo off

rem Directory structure
set SOURCE_DIR=src
set OBJ_DIR=obj
set OUTPUT_FILE=xenly.exe

rem Check if GCC is installed
gcc --version >nul 2>&1
if %errorlevel% neq 0 (
    echo GCC not found. Please install GCC and add it to your system PATH.
    exit /b 1
)

rem Create the obj directory if it doesn't exist
if not exist %OBJ_DIR% (
    mkdir %OBJ_DIR%
)

rem Compile each source file separately into object files
gcc -c %SOURCE_DIR%\xenly.c -o %OBJ_DIR%\xenly.o
gcc -c %SOURCE_DIR%\print_info.c -o %OBJ_DIR%\print_info.o
gcc -c %SOURCE_DIR%\color.c -o %OBJ_DIR%\color.o
gcc -c %SOURCE_DIR%\project.c -o %OBJ_DIR%\project.o
gcc -c %SOURCE_DIR%\math_binary.c -o %OBJ_DIR%\math_binary.o
gcc -c %SOURCE_DIR%\error.c -o %OBJ_DIR%\error.o

rem Check if compilation was successful
if %errorlevel% equ 0 (
    rem Link object files to create the executable
    gcc -o %OUTPUT_FILE% %OBJ_DIR%\xenly.o %OBJ_DIR%\print_info.o %OBJ_DIR%\color.o %OBJ_DIR%\project.o %OBJ_DIR%\math_binary.o %OBJ_DIR%\error.o
    if %errorlevel% equ 0 (
        echo Compilation successful. Running xenly programming language.
    ) else (
        echo Linking failed.
    )
) else (
    echo Compilation of source files failed.
)

exit /b %errorlevel%
