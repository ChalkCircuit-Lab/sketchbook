$SdkRoot = "C:\Users\Dragos\AppData\Local\Google\Cloud SDK\google-cloud-sdk"
$Python = Join-Path $SdkRoot "platform\bundledpython\python.exe"
$Gcloud = Join-Path $SdkRoot "lib\gcloud.py"

& $Python $Gcloud @args
exit $LASTEXITCODE
