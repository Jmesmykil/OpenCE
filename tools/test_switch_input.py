#!/usr/bin/env python3
"""Compile and run the game-data-free Switch input bridge smoke tests.

The route test is standalone. The event bridge test needs SDL2 headers only;
it does not link SDL or launch the game. SDL2_INCLUDE_DIR may point to the
include root (the directory containing SDL2/SDL.h). Otherwise pkg-config is
used when available.
"""

import os
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent


def run(command):
    print("+", " ".join(str(part) for part in command), flush=True)
    subprocess.run(command, cwd=ROOT, check=True)


def sdl2_flags():
    include_dir = os.environ.get("SDL2_INCLUDE_DIR")
    if include_dir:
        return [f"-I{include_dir}"]
    pkg_config = shutil.which("pkg-config")
    if pkg_config:
        result = subprocess.run(
            [pkg_config, "--cflags", "sdl2"], cwd=ROOT,
            check=False, capture_output=True, text=True,
        )
        if result.returncode == 0:
            return shlex.split(result.stdout)
    raise RuntimeError(
        "SDL2 headers not found; install SDL2 development headers or set SDL2_INCLUDE_DIR"
    )


def main():
    compiler = os.environ.get("CC", "cc")
    common = [compiler, "-std=c11", "-Wall", "-Wextra", "-Werror"]
    with tempfile.TemporaryDirectory(prefix="opence-switch-input-") as temporary:
        tmp = Path(temporary)
        route_test = tmp / "test_gamepad_routes"
        bridge_test = tmp / "test_sdl_input_bridge"
        run(common + ["-Iport/linux/src", "port/switch/tests/test_gamepad_routes.c",
                      "-o", str(route_test)])
        run([str(route_test)])
        run(common + sdl2_flags() + ["-Iport/switch",
                                    "port/switch/tests/test_sdl_input_bridge.c",
                                    "-o", str(bridge_test)])
        run([str(bridge_test)])
    print("PASS: Linux route assignment and SDL2-to-SDL3 event serialization smoke tests")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        sys.exit(1)
