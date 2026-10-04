# WebM performance regression checks

Native H5M texture integration checks use the Windows D3D11 WARP renderer and
the installed CRT TV screen PNG:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File tests/run-h5m.ps1
```

These check real D3DX11 loading, native target registration, texture upload and
pixel readback (including orientation), selective binding, Twitch/fallback
selection, override matching, settings reload, idle playback cleanup, and COM
reference release. They do not launch TK17 or connect to Twitch.

Spatial checks use real dynamic D3D11 constant buffers with a synthetic verified
Hook5 module layout. They cover object movement, camera translation/rotation,
nearest shared-texture instance selection, shadow-camera rejection, unrelated
buffer/material isolation, stale/invalid matrices, both projection handednesses,
and source-relative OpenAL updates for local and Twitch audio. Actual audible
direction/falloff and the live Hook5 capture still need an in-game check.

The Twitch master-volume checks can be run with:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File tests/run-twitch-volume.ps1
```

They exercise production stereo/spatial audio queuing, neutral volume, mute,
attenuation, amplification with clipping, override isolation, live slider values,
and saved settings without advancing the playback restart generation. They also
execute the settings-page generator with the game's Lua runtime to check slider
placement, range, saved defaults, and unique parameter IDs.

Run from the repository root with the existing 32-bit MSYS2 compiler:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File tests/run-performance.ps1
```

The executable includes the production routines directly. It does not load
the DLL into the game, install hooks, connect to Twitch, or read/write the
game configuration. The test runner builds only `build/webm-performance-tests.exe`.

Checks include:

- 2,148 byte-exact comparisons with the retained original scalar converter,
  covering all six output formats, odd dimensions, up/downscaling, mip sizes,
  single-pixel inputs, unaligned destinations, row padding and trailing guards.
- Local publication and Twitch frame selection compared with frozen routines
  from commit `9292122306fa1898a1a99f5f57e77bdfb4a71e7d`. Cases include repeated
  frames, loop index reuse, configuration/cache changes, changing dimensions,
  full queues, wraparound and skipped frames.
- Injected spare-buffer allocation failure to verify the locked-copy fallback.
- Concurrent frame publication/cache readers and concurrent Twitch queue
  production/presentation, including buffer ownership and pixel consistency.
- Isolated BGRA conversion benchmarks using the same `-m32 -O2` optimization
  flags as the plugin. These measure CPU conversion only, not game FPS.

The frozen reference header is test-only and is not part of the DLL build.

## Chat rendering and composition

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File tests/run-chat-performance.ps1
```

This separate executable includes production chat code and the pre-chat
render/compositor/mip routines in `reference_chat.h`. It uses synthetic messages
and decoded emote pixels with a fixed clock; no Twitch account or network is
used. Baseline chat source SHA256 starts with `24f22e97` and is also backed up
in `build/backups`.

It compares background rounding exhaustively across 1,001 opacity levels,
1,082 full-compositor cases, 160 concurrent frames, and 1,800 mip cases. Cases
cover RGB/BGR/RGB565, padding, odd sizes, flipping, overlay/side-panel layouts,
fonts, wrapping, animated emotes, cache invalidation and allocation fallback.
GDI resource counts are checked after all sessions are destroyed.

The benchmark reports CPU composition with cached and redrawn overlays plus
half-size BGRA mip preparation. Composition medians alternate original and
optimized execution order over three trials. These are not in-game FPS tests.
