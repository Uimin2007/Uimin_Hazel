$files = Get-ChildItem -Path "c:\dev\Uimin_Hazel\Uimin_Hazel\src", "c:\dev\Uimin_Hazel\Sandbox\src" -Recurse -File -Include *.h, *.cpp

$replacements = @{
    '"Uimin_Hazel/Application.h"' = '"Uimin_Hazel/Core/Application.h"'
    '"Uimin_Hazel/Core.h"' = '"Uimin_Hazel/Core/Core.h"'
    '"Uimin_Hazel/EntryPoint.h"' = '"Uimin_Hazel/Core/EntryPoint.h"'
    '"Uimin_Hazel/Input.h"' = '"Uimin_Hazel/Core/Input.h"'
    '"Uimin_Hazel/KeyCodes.h"' = '"Uimin_Hazel/Core/KeyCodes.h"'
    '"Uimin_Hazel/Layer.h"' = '"Uimin_Hazel/Core/Layer.h"'
    '"Uimin_Hazel/LayerStack.h"' = '"Uimin_Hazel/Core/LayerStack.h"'
    '"Uimin_Hazel/Log.h"' = '"Uimin_Hazel/Core/Log.h"'
    '"Uimin_Hazel/MouseButtonCodes.h"' = '"Uimin_Hazel/Core/MouseButtonCodes.h"'
    '"Uimin_Hazel/Window.h"' = '"Uimin_Hazel/Core/Window.h"'
    '"Uimin_Hazel/OrthographicCameraController.h"' = '"Uimin_Hazel/Renderer/OrthographicCameraController.h"'
}

foreach ($f in $files) {
    $content = Get-Content -Path $f.FullName -Raw
    $modified = $false
    foreach ($key in $replacements.Keys) {
        if ($content -match [regex]::Escape($key)) {
            $content = $content -replace [regex]::Escape($key), $replacements[$key]
            $modified = $true
        }
    }
    if ($modified) {
        Write-Host "Updating $($f.FullName)"
        Set-Content -Path $f.FullName -Value $content -NoNewline
    }
}
