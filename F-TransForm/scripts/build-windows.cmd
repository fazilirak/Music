@echo off
REM Gelistirici: Fazil Irak - Olusturma / son duzenleme: 27.10.2026
REM Dosyanin gorevi: Visual Studio'yu bulup x64 Release derlemesi ve CTest dogrulamasi yapmak.
REM SPDX-License-Identifier: AGPL-3.0-or-later
setlocal
for /f "usebackq delims=" %%V in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "FT_VSROOT=%%V"
if not defined FT_VSROOT exit /b 1
REM Derleyici ortamini yalniz bu komut penceresi icin etkinlestirir.
call "%FT_VSROOT%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
set "PATH=%FT_VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%FT_VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
cd /d "%~dp0.."
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1
cmake --build build --parallel 4
if errorlevel 1 exit /b 1
ctest --test-dir build --output-on-failure --timeout 180
