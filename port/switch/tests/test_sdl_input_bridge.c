/* Run with the SDL2 version used by the Switch host toolchain. */
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "../host_sdl_input.h"

static uint32_t event_u32(const uint8_t *event, unsigned offset)
{
    uint32_t value;
    memcpy(&value, event + offset, sizeof(value));
    return value;
}

static float event_float(const uint8_t *event, unsigned offset)
{
    float value;
    memcpy(&value, event + offset, sizeof(value));
    return value;
}

int main(void)
{
    uint8_t guest[GUEST_SDL_EVENT_SIZE];
    SDL_Event host;

    /* SDL3 reserves zero; the SDL2 joystick instance ID zero remains valid. */
    assert(guest_gamepad_id(-1) == 0);
    assert(guest_gamepad_id(0) == 1);
    assert(guest_gamepad_id(37) == 38);
    assert(host_gamepad_id(0) == -1);
    assert(host_gamepad_id(1) == 0);
    assert(host_gamepad_id(38) == 37);

    assert(guest_gamepad_button(SDL_CONTROLLER_BUTTON_A) == 0);
    assert(guest_gamepad_button(SDL_CONTROLLER_BUTTON_B) == 1);
    assert(guest_gamepad_button(SDL_CONTROLLER_BUTTON_X) == 2);
    assert(guest_gamepad_button(SDL_CONTROLLER_BUTTON_Y) == 3);
    assert(guest_gamepad_button(SDL_CONTROLLER_BUTTON_INVALID) == -1);
    assert(guest_gamepad_type(SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO) == 7);

    /* Keyboard events keep SDL3's event offsets and key state fields. */
    memset(&host, 0, sizeof(host));
    host.type = SDL_KEYDOWN;
    host.common.timestamp = 12;
    host.key.keysym.scancode = SDL_SCANCODE_A;
    host.key.keysym.sym = SDLK_a;
    host.key.keysym.mod = KMOD_SHIFT;
    host.key.state = SDL_PRESSED;
    host.key.repeat = 1;
    assert(guest_input_event(guest, &host) == 1);
    assert(event_u32(guest, 0) == 0x300);
    assert(event_u32(guest, 8) == 12000000U);
    assert(event_u32(guest, 24) == SDL_SCANCODE_A);
    assert(event_u32(guest, 28) == SDLK_a);
    assert(guest[36] == 1 && guest[37] == 1);

    /* Mouse motion is preserved as SDL3-position and relative-motion floats. */
    memset(&host, 0, sizeof(host));
    host.type = SDL_MOUSEMOTION;
    host.motion.x = 42;
    host.motion.y = 91;
    host.motion.xrel = -3;
    host.motion.yrel = 5;
    assert(guest_input_event(guest, &host) == 1);
    assert(event_u32(guest, 0) == 0x400);
    assert(event_float(guest, 28) == 42.0f);
    assert(event_float(guest, 32) == 91.0f);
    assert(event_float(guest, 36) == -3.0f);
    assert(event_float(guest, 40) == 5.0f);

    /* A physical SDL2 gamepad button is serialized with its stable guest ID. */
    memset(&host, 0, sizeof(host));
    host.type = SDL_CONTROLLERBUTTONDOWN;
    host.cbutton.which = 7;
    host.cbutton.button = SDL_CONTROLLER_BUTTON_A;
    host.cbutton.state = SDL_PRESSED;
    assert(guest_input_event(guest, &host) == 1);
    assert(event_u32(guest, 0) == 0x651);
    assert(event_u32(guest, 16) == 8);
    assert(guest[20] == 0 && guest[21] == 1);

    /* Unknown SDL events do not leak uninitialized guest data. */
    memset(&host, 0, sizeof(host));
    host.type = SDL_USEREVENT;
    memset(guest, 0xFF, sizeof(guest));
    assert(guest_input_event(guest, &host) == 0);
    assert(guest[0] == 0);
    return 0;
}
