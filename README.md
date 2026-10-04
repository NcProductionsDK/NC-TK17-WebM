# NC-TK17-WebM

NC-TK17-WebM is a 32-bit Windows extension for The Klub 17 that adds modern
WebM playback and optional Twitch streaming and chat support.

## Build

Install the MSYS2 MinGW 32-bit toolchain, then run:

```powershell
.\compile-webm.bat
```

The compiled DLL is written to `build\NC-TK17-WEBM.dll`.

The RGB24 conversion worker uses SSSE3 automatically on supported CPUs for
eligible BGRA/RGBA textures, preserving identical output and existing settings.
See [conversion measurements and validation](SIMD-CONVERSION-20260920.md).

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

## Hook5 H5M object screens

Native Hook5 object textures anywhere under `Mod/ActiveMod` (including all
subfolders) can use
the same sibling-file convention as TK17 textures: `Screen.png`, `Screen.webm`,
and optionally `Screen.ini`. PNG, DDS, JPG, and TGA textures loaded through
Hook5's synchronous D3DX11 file loader are supported. The H5M model stays
unchanged. Video replaces only the matching shader-resource view; the original
image is retained for fallback. A sidecar with a `[NC-TK17-WebM:Twitch]` section
also enables Twitch-only screens without a local WebM.

The WebM settings target list includes **H5M: folder - texture** entries.
Select the screen, enter a channel, and enable the override to use the existing
Twitch quality, chat, fallback and master-volume controls. **Auto assign (Rooms
only)** still applies only to TK17 room sidecars; choose an H5M target explicitly.
For a screen's own Twitch stream, set `channel` in its Twitch sidecar section.
An empty channel uses local WebM unless the global override selects that screen.

The native path reuses the existing decoder/audio player and supports both
asynchronous and synchronous frame conversion. It uploads a single BGRA texture,
preserves the H5M's UV coordinates/material settings, and releases playback
resources after the screen has not been bound for 1.5 seconds. It shares the
16-player pool with regular DirectX textures and tracks up to 32 eligible native
views until the rendering device changes. Images beyond that limit stay static.
Set `texture_audio_3d=true` in the screen INI for directional OpenAL sound that
follows the H5M object's origin and the camera. This works for local WebM,
the screen's own Twitch stream, and the global Twitch override; its master
volume still applies. The supplied CRT TV enables this with the same defaults
as toys: `audio_3d_min_distance=150`, `audio_3d_max_distance=1200` (centimeters),
and `audio_3d_rolloff=3`. Set `texture_audio_3d=false` for ordinary stereo.
Native objects use OpenAL instead of TK17 sound nodes; no `audio_node` is needed.

Spatial capture supports the verified Hook5 2021 builds (PE timestamps
`603CF2B9` / `603CF2C4`, image size `245000`). It observes CPU constant-buffer
uploads and eligible diffuse texture bindings, with no GPU readback. Unknown
builds or unavailable transforms retain centered sound. Shadow cameras are
excluded, and mirror rendering uses the original object's position. Repeated
instances of one screen texture share one player/emitter at the nearest
rendered instance. The existing inactivity timeout still applies when Hook5
stops submitting the screen. Surround output depends on the OpenAL device;
headphones/stereo speakers receive directional panning.

At startup the plugin scans native object texture sidecars and refreshes
`Scripts/Shared/NCWebMH5MTargets.txt` in the WebM settings add-on, where the Lua
sandbox can read it through the add-on index. Restart the game after adding
screens to refresh this static target list. Tests cover real D3DX11 loading and
D3D11 WARP texture readback; in-game validation is still needed.

## Twitch override master volume

The **Twitch Master Volume** slider below **Override Enabled** controls only
the selected override's live Twitch audio. Center (1.0) preserves the existing
volume, left reduces it to mute (0), and right boosts it to twice the sample
amplitude (2.0). The setting is saved as `master_volume` in
`[NC-TK17-WebM:TwitchOverride]` and restored when the settings page opens.
Dragging applies to newly queued audio without reconnecting the stream; the
saved value is written after a short pause. Regular WebM fallback audio and
Twitch sources outside the override keep their existing volume. Boosted samples
are clipped to the PCM range to prevent integer wraparound; loud material may
distort at high boost.

## Twitch override offline fallback

The in-game Twitch override stores `channel_offline_fallback=fallback` or
`channel_offline_fallback=random` in `[NC-TK17-WebM:TwitchOverride]`.
`fallback` uses the regular WebM when available, otherwise TK17's original
texture. `random` reuses the existing Twitch discovery path when the configured
channel is unavailable. While a random stream is playing, the plugin continues
checking the configured channel and switches back when it becomes live.

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
