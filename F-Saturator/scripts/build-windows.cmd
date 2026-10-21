@echo off
setlocal
rem Visual Studio C++ araçlarının kurulu olduğu yol. Başka sürüm/yerde kuruluysa yalnız bu yolu uyarlayın.
set "VSROOT=C:\Program Files\Microsoft Visual Studio\18\Community"
rem 64-bit MSVC ortamını kurar; bu adım başarısızsa derlemeye devam edilmez.
call "%VSROOT%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
rem Visual Studio ile gelen CMake ve Ninja kullanılır; sistem PATH değişikliği yalnız bu betik süresincedir.
set "PATH=%VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
rem Proje köküne geçer; derleme çıktıları build altına yazılır.
cd /d "%~dp0.."
rem Release yapılandırmasını üretir. Farklı JUCE yolu gerekiyorsa CMake’e -DIRAK_JUCE_PATH=... eklenebilir.
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1
rem Dört paralel derleme işi kullanır; bu sayı ürünün ses işleme kanalı sayısı değildir.
cmake --build build --parallel 4
if errorlevel 1 exit /b 1
rem DSP, preset, gecikme ve GUI kontrollerini çalıştırır. Hata varsa test raporu terminale yazılır.
ctest --test-dir build --output-on-failure
