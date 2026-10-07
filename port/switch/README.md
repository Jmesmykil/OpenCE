# Switch SDL input bridge prototype

This directory contains an SDL2 host to SDL3 guest event adapter extracted
from the NxHalo Switch host. It translates the pointer-free SDL events needed
by the guest, including keyboard, mouse, and gamepad input. Gamepad IDs are
made nonzero for SDL3, and the bridge maps SDL2 controller types and button
positions to their SDL3 equivalents.

This is an input integration prototype, not a complete OpenCE Switch port.
OpenCE does not currently build or launch this adapter. The upstream Linux
build continues to use its existing SDL input path, which already supports
keyboard, mouse, and gamepads. The Steam Deck runs through that Linux path.

The guest event buffer is 128 bytes. Event IDs and fields use SDL3's event
ABI; the host writes scalar fields at their ABI offsets with `memcpy`, so it
does not rely on guest alignment. SDL2 joystick instance IDs are translated
to nonzero guest IDs because SDL3 reserves ID zero. Gamepad events retain
physical button positions across Xbox, PlayStation, and Nintendo controllers.

The focused test in `tests/test_sdl_input_bridge.c` checks keyboard, mouse,
and gamepad event serialization. Run it with the SDL2 development headers used
by the Switch host:

```sh
cc -std=c11 $(pkg-config --cflags sdl2) -Iport/switch \
  port/switch/tests/test_sdl_input_bridge.c -o /tmp/test_sdl_input_bridge
/tmp/test_sdl_input_bridge
```

The profile33 Switch host used a separate devkitPro/libnx NRO build, with
SDL2, Switch Mesa, GLESv2, and EGL (`-march=armv8-a+crc+crypto`,
`-mtune=cortex-a57`, `-fPIE`). It required a separately built guest ELF.
That packaging path is not included here. No game files, signing keys, private
credentials, or device-specific files are part of this prototype. Neither
Switch nor Steam Deck hardware support is claimed as verified by this change.

Before integrating this adapter into an OpenCE Switch target, the project
needs a supported guest build, a clean platform build target, and runtime input
verification on the device. Linux mouse, keyboard, and gamepad behavior should
remain on its existing path.
