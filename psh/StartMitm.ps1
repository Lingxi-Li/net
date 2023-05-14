param(
    [Parameter(Mandatory=$false)][string]$Arg
)

Import-Module -Name .\psh\Utility.psm1

$TlsKeyLogPath = $env:MITMPROXY_SSLKEYLOGFILE
if ($TlsKeyLogPath) {
    Remove-Item -Path $TlsKeyLogPath -Force -ErrorAction SilentlyContinue
    $null = New-Item -Path $TlsKeyLogPath -ItemType File -Force -ErrorAction Stop
}

$ExtraOptions = Read-Host "Extra options"
if ($ExtraOptions) {
    $Arg += " $($ExtraOptions)"
}

"Arg: $($Arg)"
$Transparent = $Arg.ToLower().Contains("transparent")
"Transparent: $($Transparent)"

if ($Transparent) {
    Require (-not (VpnConnected)) "VPN is connected"
    Require (-not (CfwPresent)) "CFW is running"
    Require (AsAdmin) "Require admin privilege"
    DisableSysProxy
    Start-Process -FilePath "mitmproxy" -ArgumentList $Arg -Wait
}
else {
    $Process = Start-Process -FilePath "mitmproxy" -ArgumentList $Arg -PassThru
    Start-Sleep -Seconds 1
    EnableSysProxy "http://localhost:8080" "<local>"
    Wait-Process -Id $Process.Id
    DisableSysProxy
}
