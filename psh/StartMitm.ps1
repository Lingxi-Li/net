param(
    [Parameter(Mandatory=$false)][string]$Args
)

Import-Module -Name .\psh\Utility.psm1

Require (-not (VpnConnected)) "VPN is connected"

$TlsKeyLogPath = $env:MITMPROXY_SSLKEYLOGFILE
if ($TlsKeyLogPath) {
    Remove-Item -Path $TlsKeyLogPath -Force -ErrorAction SilentlyContinue
}

"Args: $($Args)"
$Transparent = $Args.ToLower().Contains("transparent")
"Transparent: $($Transparent)"

if ($Transparent) {
    Require (AsAdmin) "Require admin privilege"
    DisableSysProxy
    Start-Process -FilePath "mitmproxy" -ArgumentList $Args -Wait
}
else {
    $Process = Start-Process -FilePath "mitmproxy" -ArgumentList $Args -PassThru
    Start-Sleep -Seconds 1
    EnableSysProxy "http://localhost:8080" "<local>"
    Wait-Process -Id $Process.Id
    DisableSysProxy
}
