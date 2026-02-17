# Test all Druk examples
# Usage: .\test_examples.ps1

$ErrorActionPreference = "Continue"
$druk = "C:\dev\druk-lang2\build\Release\druk.exe"
$examplesDir = "C:\dev\druk-lang2\examples"

Write-Host ""
Write-Host "=== Testing Druk Examples ===" -ForegroundColor Cyan
Write-Host "Druk compiler: $druk" -ForegroundColor Gray
Write-Host ""

$examples = Get-ChildItem -Path $examplesDir -Filter "*.druk" | Sort-Object Name

$passed = 0
$failed = 0
$total = 0

foreach ($example in $examples) {
    $total++
    $name = $example.Name
    $displayNum = "[$total/$($examples.Count)]"
    Write-Host "$displayNum Testing: $name" -ForegroundColor Yellow
    
    try {
        # Run the example
        $output = & $druk $example.FullName 2>&1
        $exitCode = $LASTEXITCODE
        
        if ($exitCode -eq 0) {
            Write-Host "  PASS" -ForegroundColor Green
            $passed++
            
            # Show first few lines of output
            $outputLines = $output | Select-Object -First 5
            foreach ($line in $outputLines) {
                Write-Host "    $line" -ForegroundColor Gray
            }
            if ($output.Count -gt 5) {
                Write-Host "    ... (truncated)" -ForegroundColor DarkGray
            }
        } else {
            Write-Host "  FAIL (exit code: $exitCode)" -ForegroundColor Red
            $failed++
            
            # Show error output
            $errorLines = $output | Select-Object -Last 10
            foreach ($line in $errorLines) {
                Write-Host "    $line" -ForegroundColor Red
            }
        }
    }
    catch {
        Write-Host "  ERROR: $_" -ForegroundColor Red
        $failed++
    }
    
    Write-Host ""
}

Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Cyan
Write-Host "Total:  $total" -ForegroundColor White
Write-Host "Passed: $passed" -ForegroundColor Green
Write-Host "Failed: $failed" -ForegroundColor $(if ($failed -eq 0) { "Green" } else { "Red" })

if ($failed -eq 0) {
    Write-Host ""
    Write-Host "All tests passed!" -ForegroundColor Green
    exit 0
} else {
    Write-Host ""
    Write-Host "Some tests failed" -ForegroundColor Red
    exit 1
}
