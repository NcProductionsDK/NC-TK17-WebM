# NC-TK17-WebM

## Automatic room Twitch target

The in-game WebM settings target list includes **Auto assign (Rooms only)**. It is
stored in `Binaries/NC-TK17-WebM.ini` as:

```ini
[NC-TK17-WebM:TwitchOverride]
target=auto_room
```

When the Twitch override is enabled, Auto mode selects the first compatible
sidecar detected while the current room loads. A sidecar is compatible when it
contains `[NC-TK17-WebM]`, `[NC-TK17-WebM:Twitch]`, or both. The outer add-on
folder name is not used for classification; room scope comes from the sidecar's
internal `Luder/Room/<room>` path.

Only one room sidecar is overridden at a time. The temporary target is cleared
when its texture unloads, allowing the next room to select its own first
compatible sidecar. Rooms without a compatible sidecar receive no override, and
sidecars belonging to toys, items, tools, or other non-room add-ons are ignored
by Auto mode.

Selecting an explicit target path retains the existing fixed-target behavior.
Disabling the override restores the sidecar's normal Twitch, WebM, or image
behavior.
