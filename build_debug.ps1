# build_debug.ps1
# This script attempts to build the compiler inside the docker container
# and redirects all output to build.log for detailed analysis.

$logFile = "build.log"
Write-Host "Starting debug build. Output will be saved to $logFile"

# Ensure build directory exists in the container context if needed
# But usually docker compose build handles this. 
# We use docker compose build because it's what's failing.
# To get raw output from docker build, we can use --progress=plain

docker compose build backend --progress=plain 2>&1 | Tee-Object -FilePath $logFile

Write-Host "Build finished. Check $logFile for details."
