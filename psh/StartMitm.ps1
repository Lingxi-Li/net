param(
    [Parameter(Mandatory=$false)][string]$Args
)

Import-Module -Name .\psh\Utility.psm1

$TlsKeyLogPath = $env:MITMPROXY_SSLKEYLOGFILE
if ($TlsKeyLogPath) {
    Remove-Item -Path $TlsKeyLogPath -Force -ErrorAction SilentlyContinue
    $null = New-Item -Path $TlsKeyLogPath -ItemType File -Force -ErrorAction Stop
}

"Args: $($Args)"
$Transparent = $Args.ToLower().Contains("transparent")
"Transparent: $($Transparent)"

if ($Transparent) {
    Require (-not (VpnConnected)) "VPN is connected"
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
