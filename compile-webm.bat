@echo off
setlocal

set "GCC=C:\msys64\mingw32\bin\gcc.exe"
set "PATH=C:\msys64\mingw32\bin;%PATH%"
set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build"

if not exist "%GCC%" (
  echo MinGW GCC was not found at "%GCC%".
  exit /b 1
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

"%GCC%" ^
  -m32 ^
  -shared ^
  -O2 ^
  -s ^
  -static-libgcc ^
  -o "%BUILD_DIR%\NC-TK17-WEBM.dll" ^
  "%PROJECT_DIR%NC-TK17-WebM.c" ^
  "%PROJECT_DIR%webm_channel_dialog.c" ^
  "%PROJECT_DIR%webm_twitch.c" ^
  "%PROJECT_DIR%webm_twitch_auth.c" ^
  "%PROJECT_DIR%webm_twitch_chat.c" ^
  "%PROJECT_DIR%webm_twitch_emotes.c" ^
  -lole32 ^
  -luuid ^
  -ld3d8 ^
  -ld3d11 ^
  -lgdi32 ^
  -lwinhttp ^
  -lcrypt32 ^
  -lshell32 ^
  -luser32

if errorlevel 1 (
  echo Compilation failed.
  exit /b 1
)

echo Built "%BUILD_DIR%\NC-TK17-WEBM.dll".
