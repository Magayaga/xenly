@echo off
setlocal

rem Check for gcc
where gcc >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set "compiler=gcc"
) else (
    rem Check for clang
    where clang >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        set "compiler=clang"
    ) else (
        echo Error: No suitable C compiler found (gcc or clang)
        exit /b 1
    )
)

rem Compile xenly.c with the selected compiler
%compiler% src\main.c src\ast.c src\interpreter.c src\lexer.c src\modules.c src\parser.c -o xenly.exe -lm -mconsole
if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed.
    exit /b 1
)

rem Check if compilation was successful
if exist xenly.exe (
    echo Compilation successful. Running xenly programming language
) else (
    echo Compilation failed.
)

endlocal