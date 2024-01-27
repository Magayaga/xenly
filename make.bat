@echo off

rem Check for the availability of compilers
where gcc >nul 2>nul && set compiler=gcc
if not defined compiler where tcc >nul 2>nul && set compiler=tcc
if not defined compiler where clang >nul 2>nul && set compiler=clang

if not defined compiler (
    echo Error: No suitable C compiler found (gcc, tcc, or clang)
    exit /b 1
)

rem Compile xenly.c with the selected compiler
%compiler% sources\xenly.c -o xenly

rem Check if compilation was successful
if %errorlevel% equ 0 (
    echo Compilation successful. Running xenly programming language
    xenly.exe
) else (
    echo Compilation failed.
)
