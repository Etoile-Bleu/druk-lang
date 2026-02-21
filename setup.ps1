# setup.ps1 - Configure CMake with Ninja (Clang)
if (Test-Path build) { 
    Write-Host "Removing existing build directory..." -ForegroundColor Cyan
    Remove-Item -Recurse -Force build 
}

Write-Host "Configuring CMake with Ninja (Clang)..." -ForegroundColor Green
$env:CC = "clang"
$env:CXX = "clang++"
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
