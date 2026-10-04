# RGB24 conversion optimization

The user observed higher game FPS with WebM disabled and requested an
optimization without configuration changes. Earlier CPU captures identified
`convert_rgb24_to_d3d8` on the video frame preparation worker. This candidate
reduces that work; it does not establish the cause of the entire FPS loss.

`webm_rgb_simd.h` adds a runtime-dispatched SSSE3 path for RGB24 to BGRA/RGBA
conversion at equal or larger output widths. Four output pixels share one
16-byte input load and byte shuffle. Sampling masks are prepared once per
conversion and reused across rows. Coordinates, vertical orientation, alpha,
channel order, row padding, and repeated-row reuse remain identical.

All loads stay inside the source row, including its final three-byte pixel.
Unaligned buffers and partial final blocks are supported. CPUs without SSSE3,
downscaling, small images, other formats, and widths above 4096 retain the
existing converter. SSSE3 is enabled only on the isolated helper, not for
the whole DLL. No GPU decoding or shader conversion is added in this change.

## Validation

`tests/run-performance.ps1` passed:

- 2,148 existing byte-exact cases against scalar and previous production code.
- Additional full-size video cases and 4096-pixel textures in all six formats.
- Inaccessible guard pages at source and destination ends, covering unaligned
  rows, all four-pixel tails, and source widths down to six pixels.
- Forced unsupported-CPU fallback.
- Frame publication, Twitch buffering, allocation-failure fallback, and
  concurrent frame/cache ownership tests.

The benchmark compares the immediately preceding production converter,
preserved in `tests/reference_conversion_pre_simd.h`, rather than an older
scalar implementation. Each result is the median of seven batches, with
alternating measurement order and 40 conversions per batch.

| Source -> destination | Previous | Candidate | Speedup |
| --- | ---: | ---: | ---: |
| 3840x2160 -> 2048x1152 | 3.373 ms | 3.240 ms | 1.04x |
| 1920x1080 -> 2048x1024 | 2.521 ms | 1.025 ms | 2.46x |
| 1280x720 -> 2048x1152 | 1.724 ms | 0.445 ms | 3.88x |
| 1920x1080 -> 1920x1080 | 2.309 ms | 0.625 ms | 3.69x |
| 1280x720 -> 2048x1024 | 1.401 ms | 0.317 ms | 4.41x |
| 1280x720 -> 1024x512 | 0.424 ms | 0.415 ms | 1.02x |

The downscaling rows use unchanged conversion loops; small timing differences
there are not claimed improvements. These measurements cover conversion,
not decoding, chat composition, GPU upload, or whole-game frame time. An
in-game FPS gain remains unverified. Output is recorded in
`build/simd-conversion-tests-20260920.txt`.

## Deployment and rollback

The normal `compile-webm.bat` build passed. Candidate SHA256:
`14F54E5922EC1EE1A175DF9CC4835B6FB4F79816EE1145C18DDE1853FB2C5A68`.

Source, tests and the preceding installed DLL are backed up in
`build/before-simd-conversion-20260920/`. To roll back with TK17 closed, copy
that directory's `NC-TK17-WebM.dll` to the game's `Binaries` directory.
The optimization requires no INI changes and leaves plugin enablement alone.

For an in-game comparison, use the same scene, camera and video source and
compare this candidate against the backed-up DLL with WebM enabled in both.
Disabling WebM removes playback work altogether and cannot by itself measure
the candidate's benefit.
