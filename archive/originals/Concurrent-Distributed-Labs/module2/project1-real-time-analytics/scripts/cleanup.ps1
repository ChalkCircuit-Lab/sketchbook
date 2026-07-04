param(
  [Parameter(Mandatory = $true)]
  [string]$ProjectId,

  [string]$Region = "us-central1"
)

$ErrorActionPreference = "Stop"
$GcloudCommand = Get-Command gcloud -ErrorAction SilentlyContinue
$Gcloud = if ($GcloudCommand) { "gcloud" } else { Join-Path $PSScriptRoot "gcloud-local.ps1" }

& $Gcloud config set project $ProjectId
& $Gcloud pubsub subscriptions delete analytics-updates-gateway-sub --quiet 2>$null
& $Gcloud pubsub topics delete analytics-updates --quiet 2>$null
& $Gcloud pubsub topics delete movie-views --quiet 2>$null
& $Gcloud run services delete websocket-gateway --region $Region --quiet 2>$null
& $Gcloud run services delete fast-lazy-bee --region $Region --quiet 2>$null
& $Gcloud functions delete process-movie-view --gen2 --region $Region --quiet 2>$null
