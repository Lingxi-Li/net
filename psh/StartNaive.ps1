Import-Module -Name .\psh\Utility.psm1

$AppPath = "naive\naive.exe"
$CfgPath = "naive\config.json"

Require (Test-Path $AppPath -PathType Leaf) "'$($AppPath)' not found"
Require (Test-Path $CfgPath -PathType Leaf) "'$($CfgPath)' not found"

$Listen = (Get-Content $CfgPath -Raw | ConvertFrom-Json).listen.Replace("0.0.0.0", "localhost")
EnableSysProxy $Listen "<local>"
Start-Process -FilePath $AppPath -ArgumentList $CfgPath -WindowStyle Minimized -Wait
DisableSysProxy
