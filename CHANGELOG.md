# Changelog

## Build 104

- Switch and Android (Vulkan): Custom Edition maps and mods draw their
  armor, visors and HUD meters as on PC. Halo PC keeps a multipurpose
  map's masks in other channels than the Xbox, and these renderers read
  them in the Xbox's order: a modded cyborg's visor did not reflect, and
  team colors and glow came out wrong. (thelinkin3000)
- From OpenCE: a text chat for network games: T to everyone and Y to the
  team on a keyboard, D-pad Up opens an on-screen keyboard for a
  controller, Tab sends to everyone, the team or the vehicle; the host can
  turn it off and mute players, players can mute one another from the
  scoreboard, and Network Setup's new MULTIPLAYER HUD page holds its
  settings (Jonas Volman, MrBruh). Streamer mode, on Network Setup's
  STREAMER MODE page, hides slurs, swearing and insults in other players'
  names and text, and in maps and mods (MrBruh). A host shares maps at up
  to 20 MB/s, and a player helping it at up to 10 MB/s (MrBruh).

## Build 103

- A level no longer shows a frame of itself before its intro's fade and
  cinematic: with interpolation, no frame of a map is drawn before its
  first tick. (thelinkin3000)
- Switch: a level started in the first seconds after launch waits under
  the loading screen for its shaders, as later ones do, instead of starting
  with draws missing for a second or two (the intro's fade and letterbox
  bars among them). (thelinkin3000)
- From OpenCE: mods can replace a map's models whose nodes, markers and
  regions match (MrBruh); Audio Setup's new SOUND PROCESSING screen holds
  Reverb and the resampler, SINC (the default) or LINEAR, which is brighter
  (audio.resampler), and turning reverb off no longer muffles every sound
  (WhiteBulL, MrBruh).

## Build 102

- Switch: the game's GPU commands are handed to the GPU on a thread of
  their own, so the game goes on with the next frame meanwhile; a full
  40-player match ran without a fault. debug.render_thread in config.toml
  turns it off. (thelinkin3000)
- Switch: cutscene blurs (Pillar of Autumn, Two Betrayals) are drawn on a
  small copy of the screen, as the zoom's are: from 18-39 ms of the GPU's
  frame to about 2.6 ms. (thelinkin3000)
- Switch: dynamic lights (the flashlight, explosions, plasma) are drawn
  only within their reach, and object shadows only within their box on
  the screen: shadows cost about half as much, lights about a tenth less.
  (thelinkin3000)
- Switch: transparent draws (particles, decals, glass) skip the pixels
  their blend leaves as they were. (thelinkin3000)
- Object shadows are limited to the largest 16 on screen
  (display.shadow_limit), the others fading out, for crowded games.
  (thelinkin3000)
- Crowded games spend less time on bodies settling onto the ground: their
  collision tests pass by objects they cannot hit before looking them up.
  (thelinkin3000)
- Switch: draws given a vertex at a time send only what each vertex sets:
  a decal's vertices are a fifth of the data they were. (thelinkin3000)
- Switch: 900p is a choice of Video Setup's resolution. (thelinkin3000)
- New Game starts the level from its beginning, not from the profile's
  last checkpoint. (thelinkin3000)
- Settings > Gameplay: how players' and the AI's bodies fall (the death
  animations, or a ragdoll), and the main menu's background keeps playing
  on the settings screens. (WataNoge)
- Settings > Gameplay > Terminal output: the game's messages at the bottom
  left are shown in the main menu only by default, or never, or always
  (game.terminal_output). (thelinkin3000)
- From OpenCE: client-side mods, with a Mods menu, load order and checks
  for mods that cannot be played together (MrBruh); the complete
  high-resolution Xbox HUD, with Classic, Gems and Menu Gems button
  styles, the pause menu's artwork and timer digits restored, and HUD and
  text settings applied without reloading the map (chestahh); a vehicle
  nobody drives follows the host's positions, and screen fog stays on the
  canopy at a wider field of view (justinfilip); voice chat drops damaged
  packets (Paul Hontz).

Thanks to puginatux for testing the performance work so thoroughly: the
profiles of cutscenes, flashlights, snow, explosions and crowded matches
they recorded showed where each of these improvements was needed, and
proved each one.

Network version 29

## Build 101

- Switch: the maps the game plays are kept unpacked in memory instead of
  in cache files on the SD card: maps load in a second or two, the game no
  longer writes some 730 MB of cache files to the card, or a map to it each
  time a cache file takes a new one, and the old cache files are deleted.
  (thelinkin3000)
- Switch: zooming in keeps the middle of the view sharp, where you aim; the
  blur stays at the edges. (thelinkin3000)
- Switch: gyroscope aiming turns 50% further at every sensitivity.
  (thelinkin3000)

Network version 29

## Build 100

- Switch: zooming in costs about 3.7 ms less of the GPU's frame at 720p:
  the zoom's blur is drawn on a small copy of the screen and blended over
  it once, with the same look. debug.original_zoom in config.toml draws it
  as before. (thelinkin3000)

Network version 29

## Build 99

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
