$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
$workspace = Split-Path -Parent (Split-Path -Parent $project)
$env:PATH = 'C:\msys64\mingw32\bin;' + $env:PATH
$compiler = 'C:\msys64\mingw32\bin\gcc.exe'
$output = Join-Path $project 'build\webm-twitch-volume-tests.exe'
$sources = @('webm_channel_dialog.c', 'webm_twitch.c', 'webm_twitch_auth.c',
             'webm_twitch_chat.c', 'webm_twitch_emotes.c') |
    ForEach-Object { Join-Path $project $_ }
& $compiler -m32 -O2 -static-libgcc -o $output (Join-Path $PSScriptRoot 'twitch-volume.c') @sources `
    -lole32 -luuid -ld3d8 -ld3d11 -lgdi32 -lwinhttp -lcrypt32 -lshell32 -luser32
if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed' }
& $output (Join-Path $workspace 'The Klub 17\Binaries\lua51.dll') `
    (Join-Path $workspace 'The Klub 17\Addons\Script.[NC].TK17 WebM Settings\Scripts\Shared\CcConfigEditorWebM.lua')
if ($LASTEXITCODE -ne 0) { throw 'Twitch volume tests failed' }
