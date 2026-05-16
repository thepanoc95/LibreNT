# PROJECT:     LibreNT
# LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
# PURPOSE:     Project maintenance script
# COPYRIGHT:   Copyright 2026 Panoc95

[CmdletBinding(SupportsShouldProcess=$true)]
param(
    [Parameter(Mandatory=$false)]
    [string]$WindowsRoot = "$env:SystemRoot",

    [Parameter(Mandatory=$false)]
    [switch]$Restore
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Resolve-WindowsRoot {
    param([string]$Root)

    if ([string]::IsNullOrWhiteSpace($Root)) {
        throw "Windows root path is empty."
    }

    $resolved = Resolve-Path -Path $Root -ErrorAction Stop
    return $resolved.Path
}

function Disable-Binary {
    param([string]$Path)

    $disabledPath = "$Path.libre.disabled"
    if (Test-Path -LiteralPath $disabledPath) {
        Write-Host "Already disabled: $Path"
        return
    }

    if (-not (Test-Path -LiteralPath $Path)) {
        Write-Warning "Not found (skipped): $Path"
        return
    }

    if ($PSCmdlet.ShouldProcess($Path, "Rename to $disabledPath")) {
        Rename-Item -LiteralPath $Path -NewName ([System.IO.Path]::GetFileName($disabledPath)) -ErrorAction Stop
        Write-Host "Disabled: $Path"
    }
}

function Restore-Binary {
    param([string]$Path)

    $disabledPath = "$Path.libre.disabled"
    if (-not (Test-Path -LiteralPath $disabledPath)) {
        Write-Host "Already restored or missing backup: $Path"
        return
    }

    if ($PSCmdlet.ShouldProcess($disabledPath, "Restore to $Path")) {
        Rename-Item -LiteralPath $disabledPath -NewName ([System.IO.Path]::GetFileName($Path)) -ErrorAction Stop
        Write-Host "Restored: $Path"
    }
}

$root = Resolve-WindowsRoot -Root $WindowsRoot
$targets = @(
    (Join-Path $root "System32\authui.dll"),
    (Join-Path $root "SysWOW64\authui.dll")
)

Write-Host "Windows root: $root"
if ($Restore) {
    foreach ($target in $targets) {
        Restore-Binary -Path $target
    }
} else {
    foreach ($target in $targets) {
        Disable-Binary -Path $target
    }
    Write-Host ""
    Write-Host "Console logon toggle applied. Reboot required."
    Write-Host "To revert: .\Enable-ConsoleLogon.ps1 -WindowsRoot '$root' -Restore"
}
