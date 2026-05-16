# PROJECT:     LibreNT
# LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
# PURPOSE:     Project maintenance script
# COPYRIGHT:   Copyright 2026 Panoc95

$zipUrl = "https://github.com"
$tempPath = "$env:TEMP\SFWindows"
$zipFile = "$tempPath\fonts.zip"

if (!(Test-Path $tempPath)) { New-Item -ItemType Directory -Path $tempPath | Out-Null }
Invoke-WebRequest -Uri $zipUrl -OutFile $zipFile
Expand-Archive -Path $zipFile -DestinationPath $tempPath -Force

$fontFiles = Get-ChildItem -Path "$tempPath\SFWindows-master" -Include *.otf, *.ttf -Recurse
$fontFolder = (New-Object -ComObject Shell.Application).Namespace(0x14)

foreach ($font in $fontFiles) {
    if (!(Test-Path "C:\Windows\Fonts\$($font.Name)")) {
        $fontFolder.CopyHere($font.FullName)
    }
}

$regPath = "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts"
$subPath = "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\FontSubstitutes"

$fontsToBlank = @(
    "Segoe UI (TrueType)", "Segoe UI Bold (TrueType)", "Segoe UI Bold Italic (TrueType)",
    "Segoe UI Italic (TrueType)", "Segoe UI Light (TrueType)", "Segoe UI Semibold (TrueType)",
    "Segoe UI Symbol (TrueType)"
)

foreach ($f in $fontsToBlank) {
    Set-ItemProperty -Path $regPath -Name $f -Value "" -ErrorAction SilentlyContinue
}

Set-ItemProperty -Path $subPath -Name "Segoe UI" -Value "SF Pro Display" -ErrorAction SilentlyContinue

Remove-Item $tempPath -Recurse -Force