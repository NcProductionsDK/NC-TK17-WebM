# NC-TK17-WebM

NC-TK17-WebM is a 32-bit Windows extension for The Klub 17 that adds modern
WebM playback and optional Twitch streaming and chat support.

## Build

Install the MSYS2 MinGW 32-bit toolchain, then run:

```powershell
.\compile-webm.bat
```

The compiled DLL is written to `build\NC-TK17-WEBM.dll`.

## Hook5 direct upload

`directx_d3d11_upload=true` enables a guarded fast path when Hook5 is active.
The plugin correlates a TK17 D3D8 video texture with Hook5's underlying D3D11
texture and uploads the prepared video frame directly, avoiding Hook5's second
copy during `UnlockRect`.

The fast path is used only after an exact texture match and format validation,
including the smaller runtime proxy used for oversized video textures. If
Hook5 is absent, a match is ambiguous, or the texture format is unsupported,
the plugin automatically retains the normal D3D8 upload. OpenGL playback is
unchanged. `async_decoding=true` is required for the prepared-frame cache used
by the direct path.

When an oversized texture uses that runtime proxy, WebM asks
NC-TK17-Hook5-Extended to keep the original Hook5 texture bound while
temporarily redirecting only its D3D11 shader-resource view to the video
proxy. Hook5 therefore retains the original texture's complete `_pass.txt`
identity, auxiliary stages, and values such as `glow_intensity`. The borrowed
view is reference-counted and restored immediately after Hook5's deferred
EndScene render. If Hook5 Extended or its guarded bridge is unavailable, WebM
keeps its previous proxy behavior.

## FFmpeg diagnostics

FFmpeg warnings and errors remain visible, except for its harmless repeated
`Found duplicated MOOV Atom` warning. Some live MP4 streams periodically
repeat valid metadata; FFmpeg already skips it safely, so WebM suppresses only
that exact message to prevent console-log spam.

## Sidecar UV mapping

Either `[NC-TK17-WebM]` or `[NC-TK17-WebM:Twitch]` may opt into full-video UV
mapping:

```ini
[NC-TK17-WebM:Twitch]
uv_mode=full_texture
```

`off` preserves the original UV mapping and is the default. `full_texture`
stretches a uniquely detected simple four-corner plane's UV rectangle across
the complete video. It is safely disabled if the target is not a unique simple
quad. A Twitch value overrides the regular WebM value; otherwise it inherits
that value. OpenGL and DirectX are supported, and all resizing or remapping is
runtime-only—the add-on's files are not modified.

## Sidecar game-audio muting

Either `[NC-TK17-WebM]` or `[NC-TK17-WebM:Twitch]` may list TK17 sound
resources that should be muted while that sidecar's plugin video is actually
displayed:

```ini
[NC-TK17-WebM:Twitch]
mute_game_audio=NcRoom4_TV, NcRoom4_TV2
```

Names are case-insensitive; the `.ogg` extension is optional. Paths such as
`Shared/Effect/NcRoom4_TV` are also accepted. The native sound keeps playing
at TK17's silent volume threshold (`-100`) so its timeline is preserved. Its
previous volume is restored when the plugin falls back to the original TK17
texture or the sidecar unloads.

This is a local-sidecar option, not a global setting. If both supported
sections specify it, their comma-separated target lists are combined.
