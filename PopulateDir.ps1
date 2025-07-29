foreach ($fileName in $args) {
    Write-Host $fileName
    $baseName = $fileName.Trim()

    # Создаем .h файл
    $headerFile = "$baseName.h"
    $headerContent = @"
#ifndef $($baseName.ToUpper())_H_
#define $($baseName.ToUpper())_H_

#endif // $($baseName.ToUpper())_H_
"@
    Set-Content -Path $headerFile -Value $headerContent

    # Создаем .c файл с #include
    $sourceFile = "$baseName.c"
    $sourceContent = "#include `"$headerFile`""
    Set-Content -Path $sourceFile -Value $sourceContent
}

Write-Host "Файлы .c и .h успешно созданы."
