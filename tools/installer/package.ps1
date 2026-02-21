param($SourceDir, $DestZip)

$staging = "$SourceDir/tools/installer/staging"
if (Test-Path $staging) {
    Remove-Item -Recurse -Force $staging
}

New-Item -ItemType Directory -Path "$staging/bin" -Force | Out-Null
New-Item -ItemType Directory -Path "$staging/lib" -Force | Out-Null

# MSBuild creates Release subfolder, Ninja does not. Pick the newest one.
$allExes = Get-ChildItem -Path "$SourceDir/build/**/druk.exe" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
$exePath = if ($allExes) { $allExes[0].FullName } else { "$SourceDir/build/druk.exe" }

$allLibs = Get-ChildItem -Path "$SourceDir/build/**/*.lib" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
$libDir = if ($allLibs) { $allLibs[0].DirectoryName } else { "$SourceDir/build" }

if (Test-Path $exePath) { Copy-Item -Path $exePath -Destination "$staging/bin" -Force }

$libDir = if (Test-Path "$SourceDir/build/Release/druk.exe") { "$SourceDir/build/Release" } else { "$SourceDir/build" }
if (Test-Path "$libDir\*.lib") {
    Copy-Item -Path "$libDir\*.lib" -Destination "$staging/lib" -Force -ErrorAction SilentlyContinue 
}
if (Test-Path "$SourceDir/include") {
    Copy-Item -Path "$SourceDir/include" -Destination "$staging/" -Recurse -Force
}

Compress-Archive -Path "$staging/*" -DestinationPath $DestZip -Force -ErrorAction Stop
