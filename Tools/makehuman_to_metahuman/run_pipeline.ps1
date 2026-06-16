param(
    [string]$Name = "OldMan",
    [float]$Age = 0.85,
    [float]$Gender = 0.9,
    [float]$Muscle = 0.3,
    [float]$Weight = 0.5,
    [float]$Height = 0.5,
    [string]$UeApiUrl = "http://127.0.0.1:8090"
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $PSCommandPath
$ExportsDir = "$ScriptDir\exports"
$FbxPath = "$ExportsDir\$Name.fbx"
$Null = New-Item -ItemType Directory -Path $ExportsDir -Force

Write-Host "=== MPFB → MetaHuman Pipeline ===" -ForegroundColor Cyan
Write-Host "Name:   $Name"
Write-Host "Age:    $Age"
Write-Host "Gender: $Gender"
Write-Host "FBX:    $FbxPath"

# Step 1: Generate MPFB character in Blender
Write-Host "`n[1/3] Creating MPFB character in Blender..." -ForegroundColor Yellow
$BlenderArgs = @(
    "--background"
    "--python", "$ScriptDir\blender_create_character.py"
    "--"
    "--name", $Name
    "--age", $Age
    "--gender", $Gender
    "--muscle", $Muscle
    "--weight", $Weight
    "--height", $Height
    "--output", $FbxPath
)

$BlenderPath = if (Test-Path "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe") {
    "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe"
} else {
    "blender"
}

& $BlenderPath $BlenderArgs
if ($LASTEXITCODE -ne 0) { throw "Blender failed (exit $LASTEXITCODE)" }
Write-Host "  FBX exported to: $FbxPath" -ForegroundColor Green

# Step 2: Write temporary config for UE5 script
$Config = @{
    name     = $Name
    fbx_path = $FbxPath
    build_path = "/Game/MetaHumans/MPFB"
} | ConvertTo-Json
Set-Content -Path "$ScriptDir\pipeline_config.json" -Value $Config

# Step 3: Import FBX into UE5 via API
Write-Host "`n[2/3] Importing FBX into UE5..." -ForegroundColor Yellow
$ImportBody = @{
    file        = $FbxPath
    destination = "/Game/MetaHumans/MPFB/Source"
} | ConvertTo-Json

try {
    $ImportResult = Invoke-RestMethod -Uri "$UeApiUrl/api/asset/import" -Method Post -Body $ImportBody -ContentType "application/json" -ErrorAction Stop
    Write-Host "  Import result: $($ImportResult | ConvertTo-Json)" -ForegroundColor Green
} catch {
    Write-Warning "  FBX import via API failed: $_"
    Write-Warning "  You may need to import manually in UE5 Content Browser."
}

# Step 4: Run MetaHuman conversion in UE5
Write-Host "`n[3/3] Running MetaHuman conversion in UE5..." -ForegroundColor Yellow
$UeScriptPath = "$ScriptDir\ue5_import_and_convert.py"
$ExecBody = @{ code = $UeScriptPath } | ConvertTo-Json

try {
    $ExecResult = Invoke-RestMethod -Uri "$UeApiUrl/api/exec" -Method Post -Body $ExecBody -ContentType "application/json" -ErrorAction Stop
    Write-Host "  Result: $($ExecResult | ConvertTo-Json)" -ForegroundColor Green
} catch {
    Write-Warning "  UE5 exec failed: $_"
    Write-Warning "  Run the script manually in UE5: py `"$UeScriptPath`""
}

Write-Host "`n=== Pipeline complete ===" -ForegroundColor Cyan
