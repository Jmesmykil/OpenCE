# Changelog

## Build 98

- Switch: crowded games no longer freeze the picture with a GPU fault.
  deko3d wrote a few words past the end of a piece of command memory, and
  the next command overwrote them before the GPU read them. (thelinkin3000)
- Switch: the level is drawn about a quarter faster: the pixel shaders
  leave out the clamps their values cannot need, with the same picture.
  (thelinkin3000)
- Switch: shadows cast on the level cost about half as much on the GPU,
  each one shaded only over its own map. (thelinkin3000)
- Switch: drawing into a render target waits for the GPU only when that
  target was sampled, not after every shadow. (thelinkin3000)
- Switch: the shader cache drops the shaders of earlier versions, which
  could fill it and leave the game drawing nothing. (thelinkin3000)
- Switch: GPU diagnostics for bug reports, all off unless set in
  config.toml's [debug]: gpu_trace (the last commands before a GPU fault),
  gpu_profile and gpu_capture (where the GPU's time goes). (thelinkin3000)

Network version 29

## Build 97

- Custom Edition maps keep their own names; a game names its map's
  version, which a client finds its copy by, and maps an earlier build
  renamed get their names back. A download beside another version of its
  name is listed as "(2)". (MrBruh)
- Map sharing and Custom Edition map loading are hardened against damaged
  or hostile files. (MrBruh)
- custom_maps subfolders work with the above: a client in a network game
  finds its copy of the game's map in a subfolder too. (thelinkin3000)

Network version 29

## Build 96

- Switch: the program's main thread sleeps instead of spinning on the
  game's core, which kept the CPU busy for nothing. (nx-mod)
- Network games catch up at most 4 ticks a frame, so one slow frame no
  longer snowballs into frames of several seconds. (nx-mod)

Network version 28

## Build 95

- Switch: a GPU fault no longer ends the game; it is logged and the
  picture stops. (thelinkin3000)
- Android and Switch: text that starts with an underscore keeps it, so
  shared Custom Edition maps are named once, not again at every start.
  (thelinkin3000)
- A config.toml line that cannot be read is set aside, instead of every
  setting going back to its default. (thelinkin3000)
- Switch: CPU clock in Video Setup, Performance: 1.7 GHz (default), 1 GHz
  or stock. (thelinkin3000)
- Switch: Resolution Limit in Video Setup: 480p (default), 720p or 1080p,
  shown at 720p in handheld and 1080p docked. (thelinkin3000)

Network version 28

## Build 94

- Bodies in multiplayer fall as ragdolls: game.ragdoll "woody" (default),
  "halo2" or "none". A host keeps stock body physics, so it plays with
  builds without ragdolls. (thelinkin3000)
- Gyro aim on the Switch's controllers, in a profile's Gamepads, Gyro Aim.
  (WataNoge)
- Gyro aim ONLY ON ZOOM, and the phone's gyroscope set in Gyro Aim on
  Android. (thelinkin3000)
- custom_maps subfolders, listed under singleplayer or multiplayer.
  (WataNoge)
- Split screen on the Switch: each view's map, clears and pause menu drawn
  correctly. (WataNoge, thelinkin3000)
- A lobby with everyone on one machine starts, and START NOW works.
  (WataNoge)
- Switch settings are saved to the SD card when the game exits. (WataNoge)
- Fixes to profile editing in the pause menu and Back after a game.
  (WataNoge)

Network version 28
