param([string]$BuildDirectory)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
if (!$BuildDirectory) { $BuildDirectory=Join-Path $root 'build-mods' }
$cache=[IO.File]::ReadAllText((Join-Path $BuildDirectory 'CMakeCache.txt'))
$match=[regex]::Match($cache,'(?m)^CMAKE_C_COMPILER:(?:STRING|FILEPATH)=(.+)\r?$')
if (!$match.Success) { throw 'Configure the native CMake build first.' }
$compiler=$match.Groups[1].Value.Trim()
$output=Join-Path $BuildDirectory 'test-pe-mod-tools.exe'
$compileArgs=@('-D__psyz','-DPE_MODS=1','-std=c11','-O1','-fno-strict-aliasing','-Wno-pointer-sign')
foreach ($path in @('include','mods/host','mods/pe_tools','third_party/psyz/psyz/include','third_party/psyz/external/SDL/include')) {
    $compileArgs+='-I'; $compileArgs+=Join-Path $root $path
}
foreach ($file in @('tests/pe_mod_tools_test.c','mods/pe_tools/pe_stats_editor.c','mods/pe_tools/pe_equip_editor.c')) {
    $compileArgs+=Join-Path $root $file
}
$compileArgs+='-o'; $compileArgs+=$output
& $compiler @compileArgs
if ($LASTEXITCODE -ne 0) { throw 'Tools test compilation failed.' }
& $output
if ($LASTEXITCODE -ne 0) { throw 'Tools regression test failed.' }
