Import-Module -Name .\psh\Utility.psm1

$AppPath = "naive\naive.exe"
$CfgPath = "naive\config.json"

Require (Test-Path $AppPath -PathType Leaf) "'$($AppPath)' not found"
Require (Test-Path $CfgPath -PathType Leaf) "'$($CfgPath)' not found"

$Cfg = Get-Content $CfgPath -Raw | ConvertFrom-Json -AsHashtable
$Listen = $Cfg["listen"].Replace("0.0.0.0", "localhost")
$TlsKeyLogPath = $Cfg["ssl-key-log-file"]

if ($TlsKeyLogPath) {
    Remove-Item -Path $TlsKeyLogPath -Force -ErrorAction SilentlyContinue
    $null = New-Item -Path $TlsKeyLogPath -ItemType File -Force -ErrorAction Stop
}

try {
    $Process = Start-Process -FilePath $AppPath -ArgumentList $CfgPath -PassThru
    Start-Sleep -Seconds 1
    EnableSysProxy $Listen "<local>"

    $Repo = "github.com/klzgrad/naiveproxy"
    $Feed = "https://$($Repo)/releases/latest"
    $Regex = "(?<=<title>Release ).*?(?= )"
    $VerPath = "naive\version.txt"
    $ZipPath = "naive\naive.zip"

    $Page = curl --proxy $Listen -sL --show-error $Feed
    Require $? "Failed to fetch latest version number, LASTEXITCODE $($LASTEXITCODE)"
    "Done fetching latest version number"
    $LatestVer = [Regex]::Match($Page, $Regex).Value
    $AssetName = "naiveproxy-$($LatestVer)-win-x64"
    $CurrentVer = Get-Content -Path $VerPath -Raw -ErrorAction SilentlyContinue
    $Update = $CurrentVer -ne $LatestVer
    if ($Update) {
        curl --proxy $Listen -sL --show-error "https://$($Repo)/releases/download/$($LatestVer)/$($AssetName).zip" -o $ZipPath
        Require $? "Failed to fetch latest release, LASTEXITCODE $($LASTEXITCODE)"
        "Done fetching latest release"
    }

    Wait-Process -Id $Process.Id
}
finally {
    if (-not $Process.HasExited) {
        Stop-Process -Id $Process.Id -Force
    }
    DisableSysProxy
}

if ($Update) {
    "Updating naive..."
    Expand-Archive -Path $ZipPath -Force
    $AssetPath = "naive\$($AssetName)"
    Copy-Item -Path "$($AssetPath)\naive.exe" -Destination $AppPath -Force
    Remove-Item -Path $AssetPath -Recurse -Force
    $LatestVer | Out-File -FilePath $VerPath -NoNewline -Force
}
