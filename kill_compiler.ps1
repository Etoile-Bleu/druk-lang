# Kill all cl.exe (MSVC compiler) processes
# Useful when files are locked during compilation

Write-Host "Searching for cl.exe processes..." -ForegroundColor Yellow

$processes = Get-Process -Name "cl" -ErrorAction SilentlyContinue

if ($processes) {
    Write-Host "Found $($processes.Count) cl.exe process(es)" -ForegroundColor Cyan
    
    foreach ($proc in $processes) {
        Write-Host "Killing process: PID $($proc.Id)" -ForegroundColor Red
        Stop-Process -Id $proc.Id -Force
    }
    
    Write-Host "All cl.exe processes terminated." -ForegroundColor Green
} else {
    Write-Host "No cl.exe processes found." -ForegroundColor Green
}

# Also check for mspdbsrv.exe (related to cl.exe)
$mspdb = Get-Process -Name "mspdbsrv" -ErrorAction SilentlyContinue
if ($mspdb) {
    Write-Host "Found mspdbsrv.exe, killing..." -ForegroundColor Cyan
    Stop-Process -Name "mspdbsrv" -Force
    Write-Host "mspdbsrv.exe terminated." -ForegroundColor Green
}

Write-Host "Done." -ForegroundColor Green
