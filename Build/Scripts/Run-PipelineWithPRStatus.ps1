param(
    [string]$Project = "C:\Path\To\MyProject\MyProject.uproject",
    [string]$Script = "C:\Path\To\MyProject\Build\Graph\GauntletPipelineWithPRStatus.xml",
    [string]$Target = "CI_FullPipeline"
)

$commitHash = (git rev-parse HEAD).Trim()
$repoOwnerAndName = (git remote get-url origin) -replace '.*[:/]([^/]+/[^/]+)\.git$', '$1'

# 1. Set Status to Pending
Write-Host "Setting GitHub PR status to PENDING for commit $commitHash..." -ForegroundColor Cyan
gh api repos/$repoOwnerAndName/statuses/$commitHash -f state='pending' -f context='Unreal/BuildGraph-Gauntlet' -f description='Pipeline is running...'

try {
    # 2. Execute BuildGraph Pipeline
    & "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\RunUAT.bat" BuildGraph `
        -Script="$Script" `
        -Target="$Target" `
        -set:Project="$Project"

    if ($LASTEXITCODE -ne 0) {
        throw "BuildGraph execution failed with exit code $LASTEXITCODE"
    }

    # 3. Set Status to Success
    Write-Host "Pipeline completed successfully! Updating GitHub status to SUCCESS." -ForegroundColor Green
    gh api repos/$repoOwnerAndName/statuses/$commitHash -f state='success' -f context='Unreal/BuildGraph-Gauntlet' -f description='Build, cook, and Gauntlet tests passed!'

} catch {
    # 4. Set Status to Failure
    Write-Host "Pipeline FAILED: $_" -ForegroundColor Red
    gh api repos/$repoOwnerAndName/statuses/$commitHash -f state='failure' -f context='Unreal/BuildGraph-Gauntlet' -f description='Pipeline failed during compilation, cooking, or testing.'
    exit 1
}