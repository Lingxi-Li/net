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
