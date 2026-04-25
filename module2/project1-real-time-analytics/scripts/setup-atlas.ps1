param(
  [Parameter(Mandatory = $true)]
  [string]$AtlasProjectId,

  [Parameter(Mandatory = $true)]
  [string]$DbPassword,

  [string]$ClusterName = "pcd-project1",
  [string]$DbUser = "pcduser",
  [string]$Provider = "GCP",
  [string]$Region = "CENTRAL_US",
  [string]$Tier = "M0"
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
$Atlas = Join-Path $Root ".tools\atlas-cli\bin\atlas.exe"

if (-not (Test-Path $Atlas)) {
  throw "Atlas CLI not found at $Atlas. Run the Atlas CLI download step from README first."
}

& $Atlas auth whoami | Out-Host

& $Atlas clusters create $ClusterName `
  --projectId $AtlasProjectId `
  --provider $Provider `
  --region $Region `
  --tier $Tier `
  --watch

& $Atlas dbusers create readWriteAnyDatabase `
  --username $DbUser `
  --password $DbPassword `
  --projectId $AtlasProjectId

& $Atlas accessLists create "0.0.0.0/0" `
  --type cidrBlock `
  --projectId $AtlasProjectId `
  --comment "PCD lab Cloud Run access"

$raw = & $Atlas clusters connectionStrings describe $ClusterName --projectId $AtlasProjectId --output json
$connectionStrings = $raw | ConvertFrom-Json
$srv = $connectionStrings.standardSrv
if (-not $srv) {
  $srv = (($raw | Select-String -Pattern "mongodb\+srv://[^`"'\s,]+" -AllMatches).Matches | Select-Object -First 1).Value
}

if (-not $srv) {
  throw "Could not extract SRV connection string. Raw output: $raw"
}

$escapedPassword = [uri]::EscapeDataString($DbPassword)
$mongoUrl = $srv -replace "mongodb\+srv://", "mongodb+srv://$DbUser`:$escapedPassword@"
if ($mongoUrl -notmatch "/sample_mflix(\?|$)") {
  $mongoUrl = $mongoUrl -replace "/\?", "/sample_mflix?"
  if ($mongoUrl -notmatch "/sample_mflix") {
    $mongoUrl = "$mongoUrl/sample_mflix"
  }
}

Write-Host "MONGO_URL=$mongoUrl"
