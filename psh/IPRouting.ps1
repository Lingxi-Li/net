# Ref: https://docs.mitmproxy.org/stable/howto-transparent/#windows
param(
    [Parameter(Mandatory=$true)][string]$Action
)

Import-Module -Name .\psh\Utility.psm1

$IPEnableRouterPath = "HKLM:\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters"
$IPEnableRouterName = "IPEnableRouter"
$RemoteAccessName = "RemoteAccess"
$BlockICMPRedirectName = "Block ICMP redirect outbound"

switch ($Action) {
    "Query" {
        $IPEnableRouterValue = Get-ItemPropertyValue -Path $IPEnableRouterPath -Name $IPEnableRouterName
        $RemoteAccessStatus = (Get-Service -Name $RemoteAccessName).StartupType
        $BlockICMPRedirect = Get-NetFirewallRule -DisplayName $BlockICMPRedirectName -ErrorAction SilentlyContinue
        "IPEnableRouter   : $($IPEnableRouterValue)"
        "RemoteAccess     : $($RemoteAccessStatus)"
        "BlockICMPRedirect: $($BlockICMPRedirect ? 'Found' : 'Not Found')"
    }
    "Enable" {
        Require (AsAdmin) "Require admin privilege"

        $null = Set-ItemProperty -Path $IPEnableRouterPath -Name $IPEnableRouterName -Value 1 -Type DWord -Force
        $null = Set-Service -Name $RemoteAccessName -StartupType Automatic
        $null = New-NetFirewallRule -DisplayName $BlockICMPRedirectName -Protocol ICMPv4 -ICMPType 5 -Direction Outbound -Action Block
        "Enabled. Reboot to take effect."
    }
    "Disable" {
        Require (AsAdmin) "Require admin privilege"
        
        $null = Set-ItemProperty -Path $IPEnableRouterPath -Name $IPEnableRouterName -Value 0 -Type DWord -Force
        $null = Set-Service -Name $RemoteAccessName -StartupType Disabled
        $null = Remove-NetFirewallRule -DisplayName $BlockICMPRedirectName
        "Disabled. Reboot to take effect."
    }
    default {
        Write-Error "Unknown action '$($Action)'. Specify 'Query', 'Enable', or 'Disable'."
    }
}
