@echo off
REM Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
REM SPDX-License-Identifier: AGPL-3.0-or-later
REM Copyright (c) 2026 Fazıl Irak
setlocal
if not defined FSATURATOR_VSROOT (
    for /f "usebackq delims=" %%V in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "FSATURATOR_VSROOT=%%V"
)
if not defined FSATURATOR_VSROOT exit /b 1
call "%FSATURATOR_VSROOT%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
set "PATH=%FSATURATOR_VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%FSATURATOR_VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
if not defined FSATURATOR_BUILD_JOBS set "FSATURATOR_BUILD_JOBS=4"
cd /d "%~dp0.."
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1
cmake --build build --parallel %FSATURATOR_BUILD_JOBS%
if errorlevel 1 exit /b 1
ctest --test-dir build --output-on-failure --timeout 300
