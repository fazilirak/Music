REM Gelistirici: Fazil Irak - Son duzenleme: 27.10.2026
REM Dosyanin gorevi: Urun derleme, kaynak hazirlama ve dagitim yardimcilari.
REM Copyright (c) 2026 Fazil Irak - Ozgun F-Kick uygulama kodu.
@echo off
REM Fazil Irak - 27.10.2026 - MSVC build and verification.
setlocal
for /f "usebackq delims=" %%V in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "FKICK_VSROOT=%%V"
if not defined FKICK_VSROOT exit /b 1
call "%FKICK_VSROOT%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
set "PATH=%FKICK_VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%FKICK_VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
cd /d "%~dp0.."
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1
cmake --build build --parallel 4
if errorlevel 1 exit /b 1
ctest --test-dir build --output-on-failure --timeout 180
