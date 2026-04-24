$header = "res.h"

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$writer = New-Object System.IO.StreamWriter($header, $false, $utf8NoBom)

$writer.WriteLine("#pragma once")
$writer.WriteLine("")
$writer.WriteLine("#include <string>")
$writer.WriteLine("")
$writer.WriteLine("namespace RoseAuraResources {")
$writer.WriteLine("")

$dirs = Get-ChildItem -Directory

foreach ($d in $dirs) {
    Write-Host "--- Directory: $($d.Name) ---" -ForegroundColor Cyan
    
    $files = Get-ChildItem -Path $d.FullName -File

    foreach ($f in $files) {
        Write-Host "$($f.Name)" -ForegroundColor Cyan

        $content = Get-Content -Path $f.FullName -Raw

        $writer.WriteLine("/////////////////////////////////")
        $writer.WriteLine("inline const std::string Res_$($f.BaseName) = R`"(" )

        $writer.Write($content)

        $writer.WriteLine("")
        $writer.WriteLine(")`";" )
        $writer.WriteLine("")
    }
}

$writer.WriteLine("}")

$writer.Close()
