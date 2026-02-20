# setup.ps1 - Configure CMake with vcpkg toolchain
if (Test-Path build) { 
    Write-Host "Removing existing build directory..." -ForegroundColor Cyan
    Remove-Item -Recurse -Force build 
}

Write-Host "Configuring CMake with vcpkg toolchain..." -ForegroundColor Green
cmake -S . -B build -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
