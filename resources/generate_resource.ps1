$header = "res.h"

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$writer = New-Object System.IO.StreamWriter($header, $false, $utf8NoBom)

$content = Get-Content -Path "template_header.txt" -Raw
$writer.Write($content)

$dirs = Get-ChildItem -Directory

foreach ($d in $dirs) {
    Write-Host "--- Directory: $($d.Name) ---" -ForegroundColor Cyan
    
    $files = Get-ChildItem -Path $d.FullName -File

    foreach ($f in $files) {
        Write-Host "$($f.Name)" -ForegroundColor Cyan

        $content = Get-Content -Path $f.FullName -Raw

        $writer.WriteLine("/////////////////////////////////")
        $writer.WriteLine("    std::string Res_$($f.BaseName) = R`"(" )

        $writer.Write($content)

        $writer.WriteLine("")
        $writer.WriteLine(")`";" )
        $writer.WriteLine("")
    }
}

$content = Get-Content -Path "template_footer.txt" -Raw
$writer.Write($content)

$writer.Close()
