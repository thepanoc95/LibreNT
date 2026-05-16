# PROJECT:     LibreNT
# LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
# PURPOSE:     Project maintenance script
# COPYRIGHT:   Copyright 2026 Panoc95

$AppsToRemove = @(
    "*CandyCrush*", "*LinkedIn*", "*Clipchamp*", "*Solitaire*", 
    "*Weather*", "*News*", "*Paint3D*", "*MixedRealityPortal*", 
    "*Skype*", "*People*", "*WindowsTips*", "*Xbox*", "*Teams*",
    "*ZuneVideo*", "*ZuneMusic*", "*OfficeHub*", "*FeedbackHub*"
)

foreach ($App in $AppsToRemove) {
    Get-AppxPackage -Name $App | Remove-AppxPackage -ErrorAction SilentlyContinue
}

foreach ($App in $AppsToRemove) {
    Get-AppxProvisionedPackage -Online | Where-Object { $_.DisplayName -like $App } | Remove-AppxProvisionedPackage -Online -ErrorAction SilentlyContinue
}
