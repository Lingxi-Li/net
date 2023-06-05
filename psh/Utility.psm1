function Require([bool]$Condition, [string]$ErrorMessage) {
    if (-not $Condition) {
        throw $ErrorMessage
    }
}

function MessageBox([string]$Title, [string]$Message) {
    $null = (New-Object -ComObject Wscript.Shell).Popup($Message, 0, $Title)
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

function CfwPresent {
    if (Get-Process -Name "Clash for Windows" -ErrorAction SilentlyContinue) {
        return $true
    }
    $false
}

function EnableSysProxy([string]$ServerUrl, [string]$Bypass) {
    if (VpnConnected) {
        "VPN Connected. Enable/disable proxy over VPN with CFW"
    }
    elseif (CfwPresent) {
        "CFW is running. Enable/disable proxy with CFW"
    }
    else {
        $RegPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Internet Settings"
        if ($ServerUrl) { $null = Set-ItemProperty -Path $RegPath -Name "ProxyServer" -Value $ServerUrl -Type String -Force }
        if ($Bypass) { $null = Set-ItemProperty -Path $RegPath -Name "ProxyOverride" -Value $Bypass -Type String -Force }
        $null = Set-ItemProperty -Path $RegPath -Name "ProxyEnable" -Value 1 -Type DWord -Force
    }
}

function DisableSysProxy() {
    if ((-not (VpnConnected)) -and (-not (CfwPresent))) {
        $RegPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Internet Settings"
        $null = Set-ItemProperty -Path $RegPath -Name "ProxyEnable" -Value 0 -Type DWord -Force
    }
}

function DisableWpad() {
    $KeyPath = "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Internet Settings\WinHttp"
    $null = Set-ItemProperty -Path $KeyPath -Name "DisableWpad" -Type DWord -Value 1 -Force
}
