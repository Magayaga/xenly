@echo off
:: ============================================================================
:: Xenly Language — Windows Build Script (src-windows\build.bat)
::
:: Builds two executables for the Windows operating system:
::
::   xenly.exe   — Xenly bytecode runner   (runs .xe source OR .xbc bytecode)
::   xenlyc.exe  — Xenly bytecode compiler (compiles .xe -> .xbc bytecode)
::
:: The interpreter and native machine-code compiler are available on
:: Linux and macOS only (see src/ and the top-level makefile).
::
:: Requirements:
::   MinGW-w64 GCC  (recommended: MSYS2 + mingw-w64-ucrt-x86_64-gcc)
::     OR
::   LLVM Clang targeting Windows (clang --target=x86_64-pc-windows-gnu)
::
:: Usage (from src-windows\ directory):
::   build.bat              Build both xenly.exe and xenlyc.exe
::   build.bat xenly        Build only xenly.exe
::   build.bat xenlyc       Build only xenlyc.exe
::   build.bat clean        Remove build artefacts
::   build.bat help         Show this message
::
:: Override compiler:   set CC=clang && build.bat
:: ============================================================================

setlocal EnableDelayedExpansion

:: ── Compiler ────────────────────────────────────────────────────────────────
if "%CC%"=="" set CC=D:\mingw_gcc\bin\gcc

:: ── Source directory (where this .bat lives) ─────────────────────────────
set SRCDIR=%~dp0
:: Strip trailing backslash
if "%SRCDIR:~-1%"=="\" set SRCDIR=%SRCDIR:~0,-1%

:: ── Compiler flags ───────────────────────────────────────────────────────
set CFLAGS=-Wall -Wextra -O2 -std=c11 -I"%SRCDIR%" -DPLATFORM_WINDOWS -DXLY_PLATFORM_WINDOWS

:: ── Linker flags ──────────────────────────────────────────────────────────
:: -lws2_32 : Winsock2 (required for type definitions even in stub form)
set LDFLAGS=-lm -lws2_32

:: ── Shared sources (compiled into both executables) ───────────────────────
:: NOTE: xly_rt.c is intentionally excluded here.
::   xly_rt.c is the native-compiler (AOT) runtime library used by executables
::   produced by xenlyc on Linux/macOS.  On Windows, the XVM interpreter
::   (interpreter.c) provides all value_* and related symbols directly.
::   Including xly_rt.c would cause duplicate-symbol linker errors.
set SHARED_SRCS=^
  "%SRCDIR%\lexer.c" ^
  "%SRCDIR%\parser.c" ^
  "%SRCDIR%\ast.c" ^
  "%SRCDIR%\interpreter.c" ^
  "%SRCDIR%\typecheck.c" ^
  "%SRCDIR%\modules.c" ^
  "%SRCDIR%\multiproc.c" ^
  "%SRCDIR%\multiproc_builtins.c" ^
  "%SRCDIR%\unicode.c"

:: ── Dispatch on argument ──────────────────────────────────────────────────
if /I "%~1"=="clean"  goto :do_clean
if /I "%~1"=="xenly"  goto :do_xenly
if /I "%~1"=="xenlyc" goto :do_xenlyc
if /I "%~1"=="help"   goto :do_help
if /I "%~1"=="/?"     goto :do_help
goto :do_all

:: ── Build both ────────────────────────────────────────────────────────────
:do_all
call :build_xenly
if errorlevel 1 exit /b 1
call :build_xenlyc
if errorlevel 1 exit /b 1
echo.
echo   [OK] Both executables built successfully.
echo.
goto :eof

:: ── Build xenly.exe only ──────────────────────────────────────────────────
:do_xenly
call :build_xenly
if errorlevel 1 exit /b 1
goto :eof

:: ── Build xenlyc.exe only ─────────────────────────────────────────────────
:do_xenlyc
call :build_xenlyc
if errorlevel 1 exit /b 1
goto :eof

:: ── Clean ─────────────────────────────────────────────────────────────────
:do_clean
echo   Cleaning build artefacts...
if exist "%SRCDIR%\xenly.exe"  del /Q "%SRCDIR%\xenly.exe"
if exist "%SRCDIR%\xenlyc.exe" del /Q "%SRCDIR%\xenlyc.exe"
echo   Done.
goto :eof

:: ── Help ──────────────────────────────────────────────────────────────────
:do_help
echo.
echo   Xenly Windows Build  (src-windows\build.bat)
echo.
echo   Usage:  build.bat [target]
echo.
echo   Targets:
echo     (none)    Build xenly.exe and xenlyc.exe  (default)
echo     xenly     Build xenly.exe  (bytecode runner)
echo     xenlyc    Build xenlyc.exe (bytecode compiler)
echo     clean     Remove build artefacts
echo     help      Show this message
echo.
echo   Compiler:  CC=%CC%
echo   Override:  set CC=clang ^&^& build.bat
echo.
echo   Platform note:
echo     This directory (src-windows) targets Windows ONLY.
echo     The interpreter and native compiler are in src/ (Linux/macOS).
echo.
goto :eof

:: ══════════════════════════════════════════════════════════════════════════════
:: SUBROUTINES
:: ══════════════════════════════════════════════════════════════════════════════

:build_xenly
echo.
echo   Building xenly.exe ...
%CC% %CFLAGS% -o "%SRCDIR%\xenly.exe" ^
  "%SRCDIR%\main.c" ^
  %SHARED_SRCS% ^
  %LDFLAGS%
if errorlevel 1 (
    echo.
    echo   [FAILED] xenly.exe build failed.
    echo.
    exit /b 1
)
echo   [OK] xenly.exe  (Xenly bytecode runner — Windows^)
exit /b 0

:build_xenlyc
echo.
echo   Building xenlyc.exe ...
%CC% %CFLAGS% -o "%SRCDIR%\xenlyc.exe" ^
  "%SRCDIR%\xenlyc_main.c" ^
  %SHARED_SRCS% ^
  %LDFLAGS%
if errorlevel 1 (
    echo.
    echo   [FAILED] xenlyc.exe build failed.
    echo.
    exit /b 1
)
echo   [OK] xenlyc.exe  (Xenly bytecode compiler — Windows^)
exit /b 0
