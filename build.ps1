$ErrorActionPreference = "Stop"

# Keep this script ASCII-only so Windows PowerShell 5.1 can read it without a BOM.
$softwareFolder = [string]([char]0x8F6F) + [char]0x4EF6
$msysRoot = Join-Path "D:\" "$softwareFolder\msys2\mys"
$compiler = Join-Path $msysRoot "ucrt64\bin\g++.exe"

if (-not (Test-Path -LiteralPath $compiler)) {
    throw "g++ was not found: $compiler"
}

$originalPath = $env:PATH
$originalGccExecPrefix = $env:GCC_EXEC_PREFIX
$originalCompilerPath = $env:COMPILER_PATH
$originalLibraryPath = $env:LIBRARY_PATH

# GCC 16 can misread its own non-ASCII installation path. Use a temporary
# ASCII drive mapping and explicitly provide compiler and library search paths.
$driveLetter = @("Z", "Y", "X", "W", "V") |
    Where-Object { -not (Test-Path "$($_):\") } |
    Select-Object -First 1

if (-not $driveLetter) {
    throw "No free drive letter is available for the temporary MSYS2 mapping."
}

$mappedDrive = "${driveLetter}:"
& subst.exe $mappedDrive $msysRoot
if ($LASTEXITCODE -ne 0) {
    throw "Could not map MSYS2 to $mappedDrive"
}

try {
    $toolchainRoot = "$mappedDrive\ucrt64"
    $target = (& $compiler -dumpmachine).Trim()
    $version = (& $compiler -dumpfullversion).Trim()
    $gccLibrary = "$toolchainRoot\lib\gcc\$target\$version"

    $env:PATH = "$toolchainRoot\bin;$env:PATH"
    $env:GCC_EXEC_PREFIX = "$toolchainRoot\lib\gcc\"
    $env:COMPILER_PATH = "$toolchainRoot\bin;$gccLibrary;$toolchainRoot\$target\bin"
    $env:LIBRARY_PATH = "$gccLibrary;$toolchainRoot\$target\lib;$toolchainRoot\lib"

    & $compiler main.cpp HuffmanSystem.cpp Utils.cpp `
        -std=c++11 -Wall -Wextra -pedantic -static -o huffman.exe

    if ($LASTEXITCODE -ne 0) {
        throw "Compilation failed with g++ exit code $LASTEXITCODE"
    }

    Write-Host "Build succeeded: $PSScriptRoot\huffman.exe"
}
finally {
    $env:PATH = $originalPath
    $env:GCC_EXEC_PREFIX = $originalGccExecPrefix
    $env:COMPILER_PATH = $originalCompilerPath
    $env:LIBRARY_PATH = $originalLibraryPath
    & subst.exe $mappedDrive /d
}
