param(
    [Parameter(Mandatory = $true)]
    [string]$SourceFile
)

$ErrorActionPreference = "Stop"

$projectRoot = $PSScriptRoot
$compiler = "C:\msys64\ucrt64\bin\g++.exe"
$freeglutRoot = Join-Path $projectRoot ".deps\freeglut\ucrt64"
$glewRoot = "C:\msys64\ucrt64"
$buildDirectory = Join-Path $projectRoot "build"
$sourcePath = [System.IO.Path]::GetFullPath($SourceFile)
$chairSourcePath = Join-Path $projectRoot "src\objects\chair.cpp"

if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf)) {
    throw "Source file not found: $sourcePath"
}

if ([System.IO.Path]::GetExtension($sourcePath) -ne ".cpp") {
    throw "The source file must be a .cpp file: $sourcePath"
}

$sourceName = [System.IO.Path]::GetFileNameWithoutExtension($sourcePath)
$output = Join-Path $buildDirectory "$sourceName.exe"

# GCC launches helper programs (cc1plus, assembler, linker) from this directory.
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"

New-Item -ItemType Directory -Force -Path $buildDirectory | Out-Null

$hudSourcePath = Join-Path $projectRoot "src\ui\hud.cpp"
$textureSourcePath = Join-Path $projectRoot "src\rendering\texture.cpp"

& $compiler `
    $sourcePath `
    $hudSourcePath `
    $textureSourcePath `
    $chairSourcePath `
    "-I$(Join-Path $freeglutRoot 'include')" `
    "-I$(Join-Path $glewRoot 'include')" `
    "-L$(Join-Path $freeglutRoot 'lib')" `
    "-L$(Join-Path $glewRoot 'lib')" `
    -std=c++17 -Wall -Wextra -g `
    -lfreeglut -lglew32 -lopengl32 -lglu32 -lgdi32 -lwinmm `
    -o $output

if ($LASTEXITCODE -ne 0) {
    throw "Compilation failed with exit code $LASTEXITCODE."
}

Copy-Item -Force (Join-Path $freeglutRoot "bin\libfreeglut.dll") $buildDirectory
Copy-Item -Force "C:\msys64\ucrt64\bin\libgcc_s_seh-1.dll" $buildDirectory
Copy-Item -Force "C:\msys64\ucrt64\bin\libstdc++-6.dll" $buildDirectory
Copy-Item -Force "C:\msys64\ucrt64\bin\libwinpthread-1.dll" $buildDirectory
Copy-Item -Force "C:\msys64\ucrt64\bin\glew32.dll" $buildDirectory

Write-Host "Built: $output"
