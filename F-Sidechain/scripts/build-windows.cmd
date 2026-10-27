REM Gelistirici: Fazil Irak - Son duzenleme: 27.10.2026
REM Dosyanin gorevi: Urun derleme, kaynak hazirlama ve dagitim yardimcilari.
REM Katki kapsami: F-Sidechain bakimi ve uyarlamalari. GATE-12/Shaper ozgun telifleri korunur.
@echo off
REM Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
REM SPDX-License-Identifier: GPL-3.0-only
REM Copyright (c) 2026 Fazıl Irak
setlocal
if not defined FSIDECHAIN_VSROOT (
    for /f "usebackq delims=" %%V in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "FSIDECHAIN_VSROOT=%%V"
)
if not defined FSIDECHAIN_VSROOT exit /b 1
call "%FSIDECHAIN_VSROOT%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
set "PATH=%FSIDECHAIN_VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%FSIDECHAIN_VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
if not defined FSIDECHAIN_PYTHON (
    if exist "C:\Users\FAZIL\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe" set "FSIDECHAIN_PYTHON=C:\Users\FAZIL\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe"
)
if not defined FSIDECHAIN_PYTHON (
    for /f "delims=" %%P in ('where python.exe') do if not defined FSIDECHAIN_PYTHON set "FSIDECHAIN_PYTHON=%%P"
)
if not defined FSIDECHAIN_PYTHON exit /b 1
if not defined FSIDECHAIN_BUILD_JOBS set "FSIDECHAIN_BUILD_JOBS=4"
cd /d "%~dp0.."
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release 
if errorlevel 1 exit /b 1
cmake --build build --parallel %FSIDECHAIN_BUILD_JOBS%
if errorlevel 1 exit /b 1
ctest --test-dir build --output-on-failure --timeout 300
