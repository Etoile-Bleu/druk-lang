# Script to build and run Druk benchmarks with LLVM JIT
# Usage: .\run_jit_benchmark.ps1

$ErrorActionPreference = "Stop"

Write-Host "====================================" -ForegroundColor Cyan
Write-Host "  Druk LLVM JIT Benchmark Runner" -ForegroundColor Cyan
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

# Check if LLVM is installed
Write-Host "[1/5] Checking for LLVM..." -ForegroundColor Yellow
$llvmConfig = $null

# Common LLVM installation paths on Windows
$llvmPaths = @(
    "C:\Program Files\LLVM",
    "C:\LLVM",
    "$env:ProgramFiles\LLVM",
    "$env:LOCALAPPDATA\Programs\LLVM"
)

foreach ($path in $llvmPaths) {
    if (Test-Path "$path\bin\llvm-config.exe") {
        $llvmConfig = "$path\bin\llvm-config.exe"
        $env:LLVM_DIR = "$path\lib\cmake\llvm"
        break
    }
}

if ($llvmConfig) {
    Write-Host "  ✓ Found LLVM at: $llvmConfig" -ForegroundColor Green
} else {
    Write-Host "  ✗ LLVM not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "  Please install LLVM 17 or later:" -ForegroundColor Yellow
    Write-Host "  1. Download from: https://github.com/llvm/llvm-project/releases" -ForegroundColor Yellow
    Write-Host "  2. Or use chocolatey: choco install llvm" -ForegroundColor Yellow
    Write-Host ""
    exit 1
}

# Create build directory
Write-Host ""
Write-Host "[2/5] Setting up build directory..." -ForegroundColor Yellow
$buildDir = "build"
if (!(Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}
Set-Location $buildDir

# Configure with CMake
Write-Host ""
Write-Host "[3/5] Configuring CMake..." -ForegroundColor Yellow
cmake .. -DCMAKE_BUILD_TYPE=Release `
         -DLLVM_DIR="$env:LLVM_DIR" `
         -G "Visual Studio 17 2022" `
         -A x64

if ($LASTEXITCODE -ne 0) {
    Write-Host "  ✗ CMake configuration failed!" -ForegroundColor Red
    exit 1
}

# Build the project
Write-Host ""
Write-Host "[4/5] Building Druk with LLVM JIT..." -ForegroundColor Yellow
cmake --build . --config Release --target druk_compare_jit_bench -j

if ($LASTEXITCODE -ne 0) {
    Write-Host "  ✗ Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host "  ✓ Build successful!" -ForegroundColor Green

# Run benchmarks
Write-Host ""
Write-Host "[5/5] Running benchmarks..." -ForegroundColor Yellow
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

$benchExe = ".\bin\Release\druk_compare_jit_bench.exe"
if (!(Test-Path $benchExe)) {
    $benchExe = ".\benchmarks\Release\druk_compare_jit_bench.exe"
}

if (Test-Path $benchExe) {
    & $benchExe --benchmark_filter=.*
} else {
    Write-Host "  ✗ Benchmark executable not found!" -ForegroundColor Red
    Write-Host "  Expected at: $benchExe" -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Benchmark complete!" -ForegroundColor Green
Write-Host ""

Set-Location ..
