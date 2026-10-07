# Switch SDL input bridge prototype

This directory contains an SDL2 host to SDL3 guest event adapter extracted
from the NxHalo Switch host. It translates the pointer-free SDL events needed
by the guest, including keyboard, mouse, and gamepad input. Gamepad IDs are
made nonzero for SDL3, and the bridge maps SDL2 controller types and button
positions to their SDL3 equivalents.

The SDL bridge remains an input integration prototype, not a complete OpenCE
Switch port. OpenCE does not currently build or launch this adapter. Linux already supports keyboard, mouse and gamepads. The Linux patch selects one
physical gamepad for player 1, gives directly connected PlayStation controllers
priority over built-in Deck controls, preserves joystick identities across SDL
reordering, and retains existing split-player slots when new devices appear.
Keyboard/mouse port 0 stays connected during controller replacement. Held menu
navigation repeats at 350 ms. The creator confirmed PS5 gameplay and menu input
on the Deck with the corresponding e09 controller build.

The guest event buffer is 128 bytes. Event IDs and fields use SDL3's event
ABI; the host writes scalar fields at their ABI offsets with `memcpy`, so it
does not rely on guest alignment. SDL2 joystick instance IDs are translated
to nonzero guest IDs because SDL3 reserves ID zero. Gamepad events retain
physical button positions across Xbox, PlayStation, and Nintendo controllers.

The focused test in `tests/test_sdl_input_bridge.c` checks keyboard, mouse,
and gamepad event serialization. Run it with the SDL2 headers used by the
Switch host:

```sh
cc -std=c11 -Wall -Wextra -Werror -I"$SDL2_INCLUDE_DIR" -Iport/switch \
  port/switch/tests/test_sdl_input_bridge.c -o /tmp/test_sdl_input_bridge
/tmp/test_sdl_input_bridge
```

Set `SDL2_INCLUDE_DIR` to the directory that contains `SDL2/SDL.h`.

The profile33 Switch host used a separate devkitPro/libnx NRO build, with
SDL2, Switch Mesa, GLESv2, and EGL (`-march=armv8-a+crc+crypto`,
`-mtune=cortex-a57`, `-fPIE`). It required a separately built guest ELF.
That packaging path is not included here. No game files, signing keys, private
credentials, or device-specific files are part of this prototype. This change
does not claim a complete Switch port. The creator confirmed PS5 gameplay and menu controls on the Deck. Mocked-SDL
regressions through the production XInput device-change path cover reordering,
delayed virtual controllers, disconnects, split ports and held-source replacement.
Switch hardware acceptance and full platform integration remain unverified.
