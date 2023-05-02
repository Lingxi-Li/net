function Require([bool]$Condition, [string]$ErrorMessage) {
    if (-not $Condition) {
        throw $ErrorMessage
    }
}

function AsAdmin {
    ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator
        )
}

function VpnConnected {
    foreach ($VpnConn in (Get-VpnConnection)) {
        if ($VpnConn.ConnectionStatus -eq "Connected") {
            return $true
        }
    }
    $false
}

function EnableSysProxy([string]$ServerUrl, [string]$Bypass) {
    Require (-not (VpnConnected)) "VPN is connected"
    $RegPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Internet Settings"
    if ($ServerUrl) { $null = Set-ItemProperty -Path $RegPath -Name "ProxyServer" -Value $ServerUrl -Type String -Force }
    if ($Bypass) { $null = Set-ItemProperty -Path $RegPath -Name "ProxyOverride" -Value $Bypass -Type String -Force }
    $null = Set-ItemProperty -Path $RegPath -Name "ProxyEnable" -Value 1 -Type DWord -Force
}

function DisableSysProxy() {
    $RegPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Internet Settings"
    $null = Set-ItemProperty -Path $RegPath -Name "ProxyEnable" -Value 0 -Type DWord -Force
}
