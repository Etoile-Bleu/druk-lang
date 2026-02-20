# build.ps1 - Build specific target or everything
param (
    [string]$Target = "ALL_BUILD",
    [string]$Config = "Release"
)

if (-not (Test-Path build)) {
    Write-Host "Build directory not found. Running setup.ps1 first..." -ForegroundColor Yellow
    .\setup.ps1
}

Write-Host "Building target: $Target ($Config)..." -ForegroundColor Green
cmake --build build --config $Config --target $Target
