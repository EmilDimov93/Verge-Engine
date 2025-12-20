<#

Build & Run file for Verge Engine Alpha

Warning: Verge Engine is still in alpha, there is nothing to see.
If you want to run it anyway:

Requirements:

- Windows
- PowerShell
- Vulkan SDK (VULKAN_SDK environment variable must be set)
- CMake >= 3.25
- Microsoft Visual Studio 2022 (C++23 workload)
- GLFW 3.4(change path in CMakeLists.txt)
- "models" folder with car.obj and wheel.obj in /build/Release

Usage:

- Open PowerShell in the repo root
- Run: ./run.ps1

#>

$glslang = "$env:VULKAN_SDK\Bin\glslangValidator.exe"
$config = "Release"

if (-not (Test-Path $glslang)) {
    Write-Host "glslangValidator not found. Install Vulkan SDK." -ForegroundColor Red
    exit 1
}

if (-not (Test-Path -Path "build")) {
    Write-Host "Build folder not found. Creating..." -ForegroundColor Cyan
    mkdir build
    Set-Location build
    cmake ..
    Set-Location ..
}

New-Item -ItemType Directory -Force -Path "build/$config/shaders" | Out-Null

& $glslang -V "src/rendering/shaders/shader.vert" -o "build/$config/shaders/vert.spv"
if ($LASTEXITCODE -ne 0) { exit 1 }

& $glslang -V "src/rendering/shaders/shader.frag" -o "build/$config/shaders/frag.spv"
if ($LASTEXITCODE -ne 0) { exit 1 }

Write-Host "Shaders compiled" -ForegroundColor Blue

Write-Host "Building..." -ForegroundColor Blue
$buildOutput = cmake --build build --config $config 2>&1
$issues = $buildOutput | Select-String -Pattern "error","warning" -SimpleMatch

if ($issues.Count -eq 0) {
    Write-Host "Running..." -ForegroundColor Green
    cd build\$config
    ./VergeEngine.exe
    cd ../..
    Write-Host ""
    Write-Host "Finished" -ForegroundColor Magenta
} else {
    Write-Host "Build failed with warnings or errors:" -ForegroundColor Red
    $issues | ForEach-Object { Write-Host $_ -ForegroundColor Yellow }
}