param($SourceDir, $DestZip)

$staging = "$SourceDir/tools/installer/staging"
if (Test-Path $staging) {
    Remove-Item -Recurse -Force $staging
}

New-Item -ItemType Directory -Path "$staging/bin" -Force | Out-Null
New-Item -ItemType Directory -Path "$staging/lib" -Force | Out-Null

# MSBuild creates Release subfolder, Ninja does not. Try both.
$exePath = if (Test-Path "$SourceDir/build/Release/druk.exe") { "$SourceDir/build/Release/druk.exe" } else { "$SourceDir/build/druk.exe" }
$libPath = if (Test-Path "$SourceDir/build/Release/druk.exe") { "$SourceDir/build/Release/*.lib" } else { "$SourceDir/build/*.lib" }

if (Test-Path $exePath) { Copy-Item -Path $exePath -Destination "$staging/bin" -Force }

$libDir = if (Test-Path "$SourceDir/build/Release/druk.exe") { "$SourceDir/build/Release" } else { "$SourceDir/build" }
if (Test-Path "$libDir\*.lib") {
    Copy-Item -Path "$libDir\*.lib" -Destination "$staging/lib" -Force -ErrorAction SilentlyContinue 
}
if (Test-Path "$SourceDir/include") {
    Copy-Item -Path "$SourceDir/include" -Destination "$staging/" -Recurse -Force
}

Compress-Archive -Path "$staging/*" -DestinationPath $DestZip -Force -ErrorAction Stop
