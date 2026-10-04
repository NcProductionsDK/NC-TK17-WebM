$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
$workspace = Split-Path -Parent (Split-Path -Parent $project)
$env:PATH = 'C:\msys64\mingw32\bin;' + $env:PATH
$output = Join-Path $project 'build\webm-h5m-tests.exe'
$sources = @('webm_channel_dialog.c', 'webm_twitch.c', 'webm_twitch_auth.c',
             'webm_twitch_chat.c', 'webm_twitch_emotes.c') |
    ForEach-Object { Join-Path $project $_ }
& 'C:\msys64\mingw32\bin\gcc.exe' -m32 -O2 -static-libgcc -o $output `
    (Join-Path $PSScriptRoot 'h5m.c') @sources `
    -lole32 -luuid -ld3d8 -ld3d11 -lgdi32 -lwinhttp -lcrypt32 -lshell32 -luser32
if ($LASTEXITCODE -ne 0) { throw 'H5M test compilation failed' }
& $output (Join-Path $workspace 'The Klub 17\Mod\ActiveMod\_hook5data\objects\CRT TV\NcToy7_TV_Screen.png')
if ($LASTEXITCODE -ne 0) { throw 'H5M tests failed' }
