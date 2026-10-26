<# Native costume animation sweep: every pe_skins folder (model.bin with its day and room rows),
   or one -ModelContainer. Room packs need their outfit's model, extracted from the user's disc:
   -RoomPackModels @{outfit_evening='bank10-model.bin'; outfit_tshirt='bank13-model.bin'} #>
param(
    [string]$SkinRoot = (Join-Path $env:APPDATA 'Parasite Eve/pe_skins'),
    [string]$BuildDirectory,
    [string]$OutputDirectory,
    [string]$ModelContainer,
    [string[]]$AnimationContainers,
    [hashtable]$RoomPackModels = @{},
    [switch]$GeometryCorrection
)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
if(!$BuildDirectory){$BuildDirectory=Join-Path $root 'build-mods'}
if(!$OutputDirectory){$OutputDirectory=Join-Path $BuildDirectory 'animation-sweep'}
$exe=Join-Path $BuildDirectory 'pe-animation-sweep.exe'
if(!(Test-Path -LiteralPath $exe)){throw 'Build pe-animation-sweep with PE_ANIMATION_SWEEP=ON first.'}
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$jobs=@()
if($ModelContainer){
    if(!$AnimationContainers){$AnimationContainers=@($ModelContainer)}
    $jobs+=@{Key='custom';Model=$ModelContainer;Animations=$AnimationContainers}
}else{
    foreach($dir in Get-ChildItem -LiteralPath $SkinRoot -Directory){
        $model=Join-Path $dir.FullName 'model.bin'
        if(!(Test-Path -LiteralPath $model)){
            # A room pack (rooms.bin alone) is swept against the game outfit it is for.
            $rooms=Join-Path $dir.FullName 'rooms.bin'
            if($RoomPackModels.ContainsKey($dir.Name) -and (Test-Path -LiteralPath $rooms)){$jobs+=@{Key=$dir.Name;Model=$RoomPackModels[$dir.Name];Animations=@($rooms)}}
            continue
        }
        $animations=@($model)
        foreach($name in @('day1.bin','day2.bin','day3.bin','day4.bin','day5.bin','day6.bin','day7.bin','day8.bin','rooms.bin')){
            $path=Join-Path $dir.FullName $name
            if(Test-Path -LiteralPath $path){$animations+=$path}
        }
        $jobs+=@{Key=$dir.Name;Model=$model;Animations=$animations}
    }
}
$oldGeometry=$env:PE_SWEEP_PGXP
try {
    if($GeometryCorrection){$env:PE_SWEEP_PGXP='1'}else{Remove-Item Env:PE_SWEEP_PGXP -ErrorAction SilentlyContinue}
    $summary=foreach($job in $jobs){
        foreach($animation in $job.Animations){
            $name=$job.Key+'-'+[IO.Path]::GetFileNameWithoutExtension($animation)+'.csv'
            $out=Join-Path $OutputDirectory $name
            $details=& $exe $job.Model $animation $out
            $result=$LASTEXITCODE
            $details | Add-Content -LiteralPath (Join-Path $OutputDirectory 'run.log')
            if($result -ne 0){[PSCustomObject]@{Costume=$job.Key;Animation=$animation;Samples=0;Warnings=0;Result='invalid input'};continue}
            $samples=@(Import-Csv -LiteralPath $out)
            $bad=@($samples | Where-Object status -ne 'ok')
            [PSCustomObject]@{Costume=$job.Key;Animation=$animation;Samples=$samples.Count;Warnings=$bad.Count;Result='completed'}
        }
    }
    $summary | Export-Csv -NoTypeInformation -LiteralPath (Join-Path $OutputDirectory 'summary.csv')
    $summary | Group-Object Costume | ForEach-Object {
        [PSCustomObject]@{Costume=$_.Name;Samples=($_.Group | Measure-Object Samples -Sum).Sum;Warnings=($_.Group | Measure-Object Warnings -Sum).Sum}
    } | Format-Table
}finally{
    if($null -eq $oldGeometry){Remove-Item Env:PE_SWEEP_PGXP -ErrorAction SilentlyContinue}else{$env:PE_SWEEP_PGXP=$oldGeometry}
}
