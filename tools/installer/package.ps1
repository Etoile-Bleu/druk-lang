param($SourceDir, $DestZip)

$staging = "$SourceDir/tools/installer/staging"
if (Test-Path $staging) {
    Remove-Item -Recurse -Force $staging
}

New-Item -ItemType Directory -Path "$staging/bin" -Force | Out-Null
New-Item -ItemType Directory -Path "$staging/lib" -Force | Out-Null

Copy-Item -Path "$SourceDir/build/Release/druk.exe" -Destination "$staging/bin" -Force
Copy-Item -Path "$SourceDir/build/Release/*.lib" -Destination "$staging/lib" -Force

if (Test-Path "$SourceDir/include") {
    Copy-Item -Path "$SourceDir/include" -Destination "$staging/" -Recurse -Force
}

Compress-Archive -Path "$staging/*" -DestinationPath $DestZip -Force -ErrorAction Stop
