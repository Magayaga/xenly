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
%compiler% src\main.c src\xenly.c src\color.c src\error.c src\print_info.c src\project.c -o xenly.exe -lm -mconsole
if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed.
    exit /b 1
)

%compiler% src\libm\math\xenly_math.c -shared -o math.dll -fPIC -lm
%compiler% src\libm\binary_math\xenly_binary_math.c -shared -o binary_math.dll -fPIC -lm
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