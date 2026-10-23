@echo off
REM Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
REM SPDX-License-Identifier: GPL-3.0-only
REM Copyright (c) 2026 Fazıl Irak
setlocal
if not defined SHAPER_VSROOT (
    for /f "usebackq delims=" %%V in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "SHAPER_VSROOT=%%V"
)
if not defined SHAPER_VSROOT exit /b 1
call "%SHAPER_VSROOT%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
set "PATH=%SHAPER_VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%SHAPER_VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
if not defined SHAPER_PYTHON (
    if exist "C:\Users\FAZIL\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe" set "SHAPER_PYTHON=C:\Users\FAZIL\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe"
)
if not defined SHAPER_PYTHON (
    for /f "delims=" %%P in ('where python.exe') do if not defined SHAPER_PYTHON set "SHAPER_PYTHON=%%P"
)
if not defined SHAPER_PYTHON exit /b 1
if not defined SHAPER_BUILD_JOBS set "SHAPER_BUILD_JOBS=4"
cd /d "%~dp0.."
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DPython3_EXECUTABLE="%SHAPER_PYTHON%"
if errorlevel 1 exit /b 1
cmake --build build --parallel %SHAPER_BUILD_JOBS%
if errorlevel 1 exit /b 1
ctest --test-dir build --output-on-failure --timeout 300
