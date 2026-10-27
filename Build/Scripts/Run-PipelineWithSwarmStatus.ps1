param(
    [string]$Project = "C:\Path\To\MyProject\MyProject.uproject",
    [string]$Script = "C:\Path\To\MyProject\Build\Graph\GauntletPipelineWithPRStatus.xml",
    [string]$Target = "CI_FullPipeline",
    [string]$SwarmStatusEndpoint = "", # Leave empty to skip Swarm notification, or provide your Swarm test callback URL
    [string]$P4Port = "ssl:perforce:1666",
    [string]$P4User = "ci_bot"
)

# Grab the latest pending changelist context from Perforce
$latestChange = (p4 changes -m 1 -u $P4User -s pending -F "%change%").Trim()

if ($SwarmStatusEndpoint) {
    Write-Host "Reporting RUNNING status to Perforce Swarm (Changelist: $latestChange)..." -ForegroundColor Cyan
    $body = @{
        status  = "running"
        message = "BuildGraph pipeline is compiling, cooking, and executing Gauntlet tests..."
    } | ConvertTo-Json

    try {
        Invoke-RestMethod -Uri $SwarmStatusEndpoint -Method Post -Body $body -ContentType "application/json" -ErrorAction Stop
    } catch {
        Write-Warning "Failed to contact Swarm API for running status: $_"
    }
}

try {
    # 2. Execute BuildGraph Pipeline
    & "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\RunUAT.bat" BuildGraph `
        -Script="$Script" `
        -Target="$Target" `
        -set:Project="$Project"

    if ($LASTEXITCODE -ne 0) {
        throw "BuildGraph execution failed with exit code $LASTEXITCODE"
    }

    # 3. Report PASS / SUCCESS to Perforce Swarm
    Write-Host "Pipeline completed successfully! Reporting PASS to Perforce Swarm." -ForegroundColor Green
    if ($SwarmStatusEndpoint) {
        $body = @{
            status  = "pass"
            message = "Build, cook, and Gauntlet test suite passed successfully."
        } | ConvertTo-Json
        Invoke-RestMethod -Uri $SwarmStatusEndpoint -Method Post -Body $body -ContentType "application/json"
    }

} catch {
    # 4. Report FAIL / FAILURE to Perforce Swarm
    Write-Host "Pipeline FAILED: $_" -ForegroundColor Red
    if ($SwarmStatusEndpoint) {
        $body = @{
            status  = "fail"
            message = "Pipeline failed during compilation, cooking, or testing: $_"
        } | ConvertTo-Json
        Invoke-RestMethod -Uri $SwarmStatusEndpoint -Method Post -Body $body -ContentType "application/json"
    }
    exit 1
}