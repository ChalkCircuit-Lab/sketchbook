param(
  [Parameter(Mandatory = $true)]
  [string]$ProjectId,

  [Parameter(Mandatory = $true)]
  [string]$MongoUrl,

  [string]$Region = "us-central1",
  [string]$Repo = "pcd-project"
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
$GcloudCommand = Get-Command gcloud -ErrorAction SilentlyContinue
$Gcloud = if ($GcloudCommand) { "gcloud" } else { Join-Path $PSScriptRoot "gcloud-local.ps1" }

function Invoke-GcloudOptional {
  param([Parameter(ValueFromRemainingArguments = $true)][string[]]$Arguments)
  & $Gcloud @Arguments
  if ($LASTEXITCODE -ne 0) {
    Write-Host "Optional gcloud command returned exit code $LASTEXITCODE; continuing."
  }
}

& $Gcloud config set project $ProjectId
& $Gcloud config set run/region $Region

& $Gcloud services enable `
  run.googleapis.com `
  cloudbuild.googleapis.com `
  cloudfunctions.googleapis.com `
  eventarc.googleapis.com `
  pubsub.googleapis.com `
  firestore.googleapis.com `
  artifactregistry.googleapis.com

Invoke-GcloudOptional artifacts repositories create $Repo `
  --repository-format=docker `
  --location=$Region `
  --description="PCD Project 1 images"

Invoke-GcloudOptional firestore databases create --location=$Region

Invoke-GcloudOptional pubsub topics create movie-views
Invoke-GcloudOptional pubsub topics create analytics-updates
Invoke-GcloudOptional pubsub subscriptions create analytics-updates-gateway-sub `
  --topic=analytics-updates `
  --ack-deadline=30

$ProjectNumber = & $Gcloud projects describe $ProjectId --format='value(projectNumber)'
$ComputeServiceAccount = "$ProjectNumber-compute@developer.gserviceaccount.com"

$ImageRoot = "$Region-docker.pkg.dev/$ProjectId/$Repo"

Push-Location (Join-Path $Root "fast-lazy-bee")
npm install
npm run build
& $Gcloud builds submit --tag "$ImageRoot/fast-lazy-bee:v1"
& $Gcloud run deploy fast-lazy-bee `
  --image "$ImageRoot/fast-lazy-bee:v1" `
  --platform managed `
  --region $Region `
  --allow-unauthenticated `
  --port 3000 `
  --set-env-vars "MONGO_URL=$MongoUrl,MONGO_DB_NAME=sample_mflix,MOVIE_EVENTS_TOPIC=movie-views,NODE_ENV=production" `
  --min-instances 1 `
  --max-instances 3
Pop-Location

& $Gcloud pubsub topics add-iam-policy-binding movie-views `
  --member="serviceAccount:$ComputeServiceAccount" `
  --role="roles/pubsub.publisher"

Push-Location (Join-Path $Root "event-processor")
npm install
& $Gcloud functions deploy process-movie-view `
  --gen2 `
  --runtime=nodejs22 `
  --region=$Region `
  --source=. `
  --entry-point=processMovieView `
  --trigger-topic=movie-views `
  --set-env-vars "UPDATE_TOPIC=analytics-updates,STATS_COLLECTION=movieStats,PROCESSED_COLLECTION=processedMovieViewEvents,RECENT_COLLECTION=recentMovieViews"
Pop-Location

& $Gcloud pubsub topics add-iam-policy-binding analytics-updates `
  --member="serviceAccount:$ComputeServiceAccount" `
  --role="roles/pubsub.publisher"
& $Gcloud projects add-iam-policy-binding $ProjectId `
  --member="serviceAccount:$ComputeServiceAccount" `
  --role="roles/datastore.user"
& $Gcloud projects add-iam-policy-binding $ProjectId `
  --member="serviceAccount:$ComputeServiceAccount" `
  --role="roles/storage.objectViewer"
& $Gcloud projects add-iam-policy-binding $ProjectId `
  --member="serviceAccount:$ComputeServiceAccount" `
  --role="roles/artifactregistry.writer"
& $Gcloud projects add-iam-policy-binding $ProjectId `
  --member="serviceAccount:$ComputeServiceAccount" `
  --role="roles/logging.logWriter"

Push-Location (Join-Path $Root "websocket-gateway")
npm install
& $Gcloud builds submit --tag "$ImageRoot/websocket-gateway:v1"
& $Gcloud run deploy websocket-gateway `
  --image "$ImageRoot/websocket-gateway:v1" `
  --platform managed `
  --region $Region `
  --allow-unauthenticated `
  --port 8080 `
  --set-env-vars "UPDATE_SUBSCRIPTION=analytics-updates-gateway-sub,STATS_COLLECTION=movieStats,RECENT_COLLECTION=recentMovieViews" `
  --min-instances 1 `
  --max-instances 3
Pop-Location

& $Gcloud pubsub subscriptions add-iam-policy-binding analytics-updates-gateway-sub `
  --member="serviceAccount:$ComputeServiceAccount" `
  --role="roles/pubsub.subscriber"

& $Gcloud functions add-invoker-policy-binding process-movie-view `
  --region $Region `
  --member="allUsers"

$ServiceUrl = & $Gcloud run services describe fast-lazy-bee --region $Region --format='value(status.url)'
$DashboardUrl = & $Gcloud run services describe websocket-gateway --region $Region --format='value(status.url)'

Write-Host "SERVICE_URL=$ServiceUrl"
Write-Host "DASHBOARD_URL=$DashboardUrl"
