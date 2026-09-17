# Engine string fallback memory-query reduction

`copy_engine_string_a` previously called `ptr_readable` for every character
when the engine-length prefix could not be used. Each check invoked Windows
`VirtualQuery`. The fallback now validates a memory region once and copies
within its bounds, stopping at NUL or the output limit. Crossing a region
requires another query. No permissions are cached across calls.

The length-prefix path, truncation, accepted byte values, return values, and
partially written output on failure retain their existing behavior. This
helper is used by WebM's configuration/customizer integration. A fallback
hotspot in actual gameplay has not been established; no FPS gain is claimed.

Run `python Development/NC-TK17-WebM/tests/run-string-copy.py` from the workspace.
It extracts the production functions and compares the old implementation's
return value and entire output buffer. Coverage includes arbitrary byte
values, embedded NULs, engine-length prefixes, output limits, null inputs,
cross-region strings, read-only/guard/execute-only/inaccessible/uncommitted
pages, and permissions changed between calls. All checks passed.

A 255-character fallback copy uses 3 queries instead of 258. The isolated
10,000-copy benchmark measured 1,147.146 ms and 2,580,000 queries before versus
14.318 ms and 30,000 queries after. This deliberately exercises the fallback;
it does not predict normal engine-string or whole-game performance.

The full `compile-webm.bat` build and `git diff --check` passed. The installed
DLL was available for replacement and the new DLL was installed and verified:

- Candidate/installed SHA256:
  `A311AB4745AFB510BFDAD488F71FD813E7800085FF702AAFA770E2FB98F54F8E`.
- Backup: `build/before-string-copy-20260914/NC-TK17-WebM.dll`.
- Backup SHA256:
  `2B4A963D21CD5830943FDEDE99BAF8139E4BE6B57DCB56ACEC96EF2BA11B3ACF`.

No extension configuration was changed. Gameplay validation is pending;
check WebM playback and its configuration/customizer controls after launch.
