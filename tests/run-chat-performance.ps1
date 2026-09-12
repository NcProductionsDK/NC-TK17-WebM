$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
$env:PATH = 'C:\msys64\mingw32\bin;' + $env:PATH
$output = Join-Path $project 'build\webm-chat-performance-tests.exe'
$sources = @('webm_twitch.c', 'webm_twitch_auth.c', 'webm_twitch_emotes.c') |
    ForEach-Object { Join-Path $project $_ }
& 'C:\msys64\mingw32\bin\gcc.exe' -m32 -O2 -static-libgcc -o $output `
    (Join-Path $PSScriptRoot 'chat-performance.c') @sources `
    -lole32 -luuid -lgdi32 -lwinhttp -lcrypt32 -lshell32 -luser32
if ($LASTEXITCODE -ne 0) { throw 'Chat test compilation failed' }
& $output
if ($LASTEXITCODE -ne 0) { throw 'Chat regression tests failed' }
