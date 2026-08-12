# NC-TK17-WebM

NC-TK17-WebM is a 32-bit Windows extension for The Klub 17 that adds modern
WebM playback and optional Twitch streaming and chat support.

## Build

Install the MSYS2 MinGW 32-bit toolchain, then run this from the repository
root in PowerShell:

```powershell
C:\msys64\mingw32\bin\gcc.exe -m32 -shared -O2 -s -static-libgcc `
  -o build\NC-TK17-WEBM.dll `
  NC-TK17-WebM.c webm_channel_dialog.c webm_twitch.c `
  webm_twitch_auth.c webm_twitch_chat.c webm_twitch_emotes.c `
  -lole32 -luuid -ld3d8 -ld3d11 -lgdi32 -lwinhttp -lcrypt32 `
  -lshell32 -luser32
```
