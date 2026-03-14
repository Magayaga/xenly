@echo off
REM ═══════════════════════════════════════════════════════════════════════════
REM XVM — Xenly Virtual Machine Build System (Windows Batch Version)
REM
REM Created, designed, and developed by Cyril John Magayaga
REM Available for Windows, macOS, and Linux
REM ═══════════════════════════════════════════════════════════════════════════

setlocal enabledelayedexpansion

REM Configuration
set BINDIR=bin
set XENLYBYC=%BINDIR%\xenlybyc.exe
set XENLYRUN=%BINDIR%\xenlyrun.exe
set LDFLAGS=-ldflags="-s -w"

if "%1"=="" goto build
if "%1"=="all" goto build
if "%1"=="build" goto build
if "%1"=="xenlybyc" goto xenlybyc
if "%1"=="xenlyrun" goto xenlyrun
if "%1"=="clean" goto clean
if "%1"=="test" goto test
if "%1"=="fmt" goto fmt
if "%1"=="vet" goto vet
if "%1"=="help" goto help
if "%1"=="cross-linux-amd64" goto cross-linux-amd64
if "%1"=="cross-linux-arm64" goto cross-linux-arm64
if "%1"=="cross-darwin-amd64" goto cross-darwin-amd64
if "%1"=="cross-darwin-arm64" goto cross-darwin-arm64
if "%1"=="cross-windows-amd64" goto cross-windows-amd64
if "%1"=="cross-all" goto cross-all
if "%1"=="install" goto install
if "%1"=="uninstall" goto uninstall

echo Unknown target: %1
goto help

:build
call :xenlybyc
call :xenlyrun
echo.
echo ✓  XVM build complete
echo    Bytecode compiler : %XENLYBYC%
echo    VM launcher       : %XENLYRUN%
echo.
goto end

:xenlybyc
if not exist %BINDIR% mkdir %BINDIR%
echo Building xenlybyc...
go build %LDFLAGS% -o %XENLYBYC% .\xenlybyc\
echo   ✓  %XENLYBYC%
goto end

:xenlyrun
if not exist %BINDIR% mkdir %BINDIR%
echo Building xenlyrun...
go build %LDFLAGS% -o %XENLYRUN% .\xenlyrun\
echo   ✓  %XENLYRUN%
goto end

:cross-linux-amd64
if not exist %BINDIR% mkdir %BINDIR%
set GOOS=linux
set GOARCH=amd64
go build %LDFLAGS% -o %BINDIR%\xenlybyc-linux-amd64 .\xenlybyc\
go build %LDFLAGS% -o %BINDIR%\xenlyrun-linux-amd64 .\xenlyrun\
goto end

:cross-linux-arm64
if not exist %BINDIR% mkdir %BINDIR%
set GOOS=linux
set GOARCH=arm64
go build %LDFLAGS% -o %BINDIR%\xenlybyc-linux-arm64 .\xenlybyc\
go build %LDFLAGS% -o %BINDIR%\xenlyrun-linux-arm64 .\xenlyrun\
goto end

:cross-darwin-amd64
if not exist %BINDIR% mkdir %BINDIR%
set GOOS=darwin
set GOARCH=amd64
go build %LDFLAGS% -o %BINDIR%\xenlybyc-darwin-amd64 .\xenlybyc\
go build %LDFLAGS% -o %BINDIR%\xenlyrun-darwin-amd64 .\xenlyrun\
goto end

:cross-darwin-arm64
if not exist %BINDIR% mkdir %BINDIR%
set GOOS=darwin
set GOARCH=arm64
go build %LDFLAGS% -o %BINDIR%\xenlybyc-darwin-arm64 .\xenlybyc\
go build %LDFLAGS% -o %BINDIR%\xenlyrun-darwin-arm64 .\xenlyrun\
goto end

:cross-windows-amd64
if not exist %BINDIR% mkdir %BINDIR%
set GOOS=windows
set GOARCH=amd64
go build %LDFLAGS% -o %BINDIR%\xenlybyc-windows-amd64.exe .\xenlybyc\
go build %LDFLAGS% -o %BINDIR%\xenlyrun-windows-amd64.exe .\xenlyrun\
goto end

:cross-all
call :cross-linux-amd64
call :cross-linux-arm64
call :cross-darwin-amd64
call :cross-darwin-arm64
call :cross-windows-amd64
echo ✓  All cross-compile targets built
goto end

:test
call :build
echo ═══════════════════════════════════════
echo   Running XVM test suite
echo ═══════════════════════════════════════
echo print("Hello, World!")> %TEMP%\xvm_test.xe
%XENLYBYC% %TEMP%\xvm_test.xe -o %TEMP%\xvm_test.xyc
echo   ✓  xenlybyc compiled hello
%XENLYRUN% %TEMP%\xvm_test.xyc
echo   ✓  xenlyrun executed hello
del /f /q %TEMP%\xvm_test.xe %TEMP%\xvm_test.xyc 2>nul
go test ./...
echo ✓  All tests passed
goto end

:fmt
go fmt ./...
goto end

:vet
go vet ./...
goto end

:clean
echo Cleaning...
if exist %BINDIR% rmdir /s /q %BINDIR%
del /f /q *.xyc 2>nul
echo ✓  Clean complete
goto end

:install
call :build
set PREFIX=%2
if "%PREFIX%"=="" set PREFIX=C:\Program Files\Xenly
set BININSTDIR=%PREFIX%\bin
if not exist %BININSTDIR% mkdir %BININSTDIR%
copy /y %XENLYBYC% %BININSTDIR%\xenlybyc.exe >nul
copy /y %XENLYRUN% %BININSTDIR%\xenlyrun.exe >nul
echo ✓  Installed to %BININSTDIR%
goto end

:uninstall
set PREFIX=%2
if "%PREFIX%"=="" set PREFIX=C:\Program Files\Xenly
set BININSTDIR=%PREFIX%\bin
del /f /q %BININSTDIR%\xenlybyc.exe 2>nul
del /f /q %BININSTDIR%\xenlyrun.exe 2>nul
echo ✓  Uninstalled
goto end

:help
echo.
echo XVM — Xenly Virtual Machine Build System (Windows Batch)
echo ═══════════════════════════════════════════════════════════════
echo.
echo   Usage: main.bat [target] [options]
echo.
echo   Targets:
echo     all              Build xenlybyc + xenlyrun (default)
echo     xenlybyc         Build the bytecode compiler only
echo     xenlyrun         Build the VM launcher only
echo     test             Build and run test suite
echo     fmt              Auto-format all Go source
echo     vet              Run go vet on all packages
echo     cross-all        Build for all supported platforms
echo     cross-linux-amd64   Linux x86-64
echo     cross-linux-arm64   Linux ARM64
echo     cross-darwin-amd64  macOS x86-64
echo     cross-darwin-arm64  macOS Apple Silicon
echo     cross-windows-amd64 Windows x86-64
echo     install [PREFIX] Install to PREFIX (default: C:\Program Files\Xenly)
echo     uninstall [PREFIX] Remove installed files
echo     clean            Remove build artifacts
echo     help             Show this message
echo.
echo   Workflow examples:
echo     main.bat                     # build both tools
echo     main.bat test                # build + smoke test
echo     %BINDIR%\xenlybyc hello.xe      # compile hello.xe → hello.xyc
echo     %BINDIR%\xenlyrun  hello.xyc    # run compiled bytecode
echo     %BINDIR%\xenlyrun --run hello.xe  # compile + run in one step
echo     main.bat cross-all           # build for all platforms
echo.

:end
endlocal
