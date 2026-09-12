# Playback efficiency changes

Baseline source: `9292122306fa1898a1a99f5f57e77bdfb4a71e7d`.

## Changes

- Cache conversion computes horizontal sample offsets once per output width
  instead of repeating coordinate progression for every row. Separate format
  loops remove per-pixel format checks, and repeated source rows during
  upscaling reuse the already converted row. The original scalar converter is
  retained for widths beyond the optimized path's 4096-pixel bound.
- Local/unbuffered frame publication skips redundant RGB copies when the
  decoded frame is unchanged. Changed frames are copied into a worker-owned
  spare before briefly locking to exchange buffers. Cache refresh and loop
  handling retain their previous behavior.
- Buffered Twitch playback transfers the selected queue buffer to presentation
  by exchanging pointers under the queue lock. The consumed slot receives the
  spare buffer for reuse. This removes one full RGB copy per presented frame;
  the FFmpeg-to-queue copy remains.
- OpenGL cached-frame copying uses the existing cache reader pinning mechanism,
  so the shared publication lock is released before the large copy.

No configuration values or defaults, video timestamps, audio code, mip upload
schedule, filtering, UV mapping, chat composition or GPU upload APIs were changed.

The local publication spare can retain one additional RGB frame per decoder
(about 5.9 MiB at 1920x1080 or 23.7 MiB at 3840x2160). If allocating it fails,
publication falls back to the original locked-copy approach. Twitch exchanges
existing queue/presentation buffers rather than adding another frame buffer.

## Validation

The 32-bit regression executable passed 2,148 byte-exact conversion cases,
comparison of publication and queue behavior against the baseline, injected
allocation failure, concurrent cache reads and concurrent Twitch queue use.
See [test instructions](tests/README.md).

One local benchmark run produced the following times for a single BGRA base
texture conversion, excluding decoding, other mip levels, uploads and rendering:

| Source -> target | Original | Optimized | Speedup |
| --- | ---: | ---: | ---: |
| 3840x2160 -> 2048x1152 | 6.448 ms | 2.401 ms | 2.69x |
| 1920x1080 -> 2048x1024 | 5.572 ms | 1.741 ms | 3.20x |
| 1280x720 -> 2048x1152 | 6.164 ms | 1.312 ms | 4.70x |
| 1920x1080 -> 1920x1080 | 5.529 ms | 1.765 ms | 3.13x |

These are CPU microbenchmarks, not FPS predictions; timings vary with system
load. In-game visual/audio equivalence and FPS have not been measured. For an
FPS comparison use the original DLL and optimized DLL with the same local
video, room, camera, configuration and warm-up interval. Live Twitch content
varies between runs, so local playback is the more repeatable comparison.

## Twitch chat optimization

This pass builds on the user-tested playback optimization. It changes only
chat rendering/composition and chat mip preparation; configuration, network
handling, message selection, layout, animation timing and audio are unchanged.

- Background dimming uses per-session lookup tables instead of repeating
  floating-point calculations for every video pixel. The original 32-bit
  scalar rounding and RGB565 expansion/packing are preserved. GCC vectorization
  is disabled for this helper to prevent SSE rounding from changing pixels.
- Reused overlays cache the occupied range of each row to skip blank regions.
  Bounds are invalidated on redraw and built only after reuse; failure to
  allocate the optional bounds retains the original rectangular scan.
- BGRA composition copies opaque text/emote pixels directly and uses the
  original arithmetic for translucent emotes. Other formats keep the generic
  pixel compositor.
- Render-cache entries reuse their Arial font handles, releasing them when
  replaced or when the session is destroyed.
- Ordinary 32-bit half-size mip preparation uses a specialized loop with the
  same four samples, alpha handling and rounding. Edge-clamped cases and other
  formats retain the original loop.

Extra storage is approximately 352 bytes of dimming tables plus eight bytes
per overlay row per render-cache entry, and at most five cached font handles
per session. All shared state remains protected by the existing session lock.

Validation passed: 1,082 byte-exact compositor cases, 160 concurrent compositor
frames, 1,800 mip comparisons, all 65,536 RGB565 values and all 256 channel
values at 1,001 opacity levels, optional-allocation fallback, and GDI object
cleanup. The previous playback regression suite also passed with the chat
changes. The compositor cases include both layouts, flips, RGB/BGR/RGB565,
padding, text wrapping, animated emotes and render-cache invalidation.

Final local CPU benchmark (2048x1152 BGRA, eight fixture messages; compositor
times are medians of three trials with alternating baseline/optimized order):

| Chat operation | Before | After | Speedup |
| --- | ---: | ---: | ---: |
| Cached overlay composition | 4.140 ms | 1.156 ms | 3.58x |
| Overlay composition with redraw | 10.007 ms | 7.416 ms | 1.35x |
| Cached side-panel composition | 2.298 ms | 2.291 ms | 1.00x |
| Side-panel composition with redraw | 9.434 ms | 9.368 ms | 1.01x |
| First chat mip, 2048x1152 -> 1024x576 | 2.974 ms | 1.925 ms | 1.54x |

Side-panel composition is effectively unchanged in this run; it still benefits
from faster mip preparation. These timings exclude video decoding, GPU uploads
and game rendering, and do not predict game FPS. Live in-game chat validation
remains necessary. The pre-chat DLL is saved as
`build/backups/NC-TK17-WebM-before-chat-60e134dc.dll`.
