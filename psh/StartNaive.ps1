Import-Module -Name .\psh\Utility.psm1

$AppPath = "naive\naive.exe"
$CfgPath = "naive\config.json"

Require (Test-Path $AppPath -PathType Leaf) "'$($AppPath)' not found"
Require (Test-Path $CfgPath -PathType Leaf) "'$($CfgPath)' not found"

$Listen = (Get-Content $CfgPath -Raw | ConvertFrom-Json).listen.Replace("0.0.0.0", "localhost")

EnableSysProxy $Listen "<local>"
$Process = Start-Process -FilePath $AppPath -ArgumentList $CfgPath -WindowStyle Minimized -PassThru
Start-Sleep -Seconds 1

$Repo = "github.com/klzgrad/naiveproxy"
$Feed = "https://$($Repo)/releases/latest"
$Regex = "(?<=<title>Release ).*?(?= )"
$VerPath = "naive\version.txt"
$ZipPath = "naive\naive.zip"

$Page = curl --proxy $Listen -L $Feed
"Done fetching latest version number"
$LatestVer = [Regex]::Match($Page, $Regex).Value
$AssetName = "naiveproxy-$($LatestVer)-win-x64"
$CurrentVer = Get-Content -Path $VerPath -Raw -ErrorAction SilentlyContinue
$Update = $CurrentVer -ne $LatestVer
if ($Update) {
    curl --proxy $Listen -L "https://$($Repo)/releases/download/$($LatestVer)/$($AssetName).zip" -o $ZipPath
    "Done fetching latest release"
}

Wait-Process -Id $Process.Id
DisableSysProxy

if ($Update) {
    $null = (New-Object -ComObject Wscript.Shell).Popup("About to update naive...", 0, "Auto Updater")
    Expand-Archive -Path $ZipPath -Force
    $AssetPath = "naive\$($AssetName)"
    Copy-Item -Path "$($AssetPath)\naive.exe" -Destination $AppPath -Force
    Remove-Item -Path $AssetPath -Recurse -Force
    $LatestVer | Out-File -FilePath $VerPath -NoNewline -Force
}
