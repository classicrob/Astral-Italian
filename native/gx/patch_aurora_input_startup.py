#!/usr/bin/env python3
"""Timestamp the pinned one-time SDL input init; never alter pinned sources."""
from pathlib import Path
import sys


def patch(text):
    begin = 'void initialize() noexcept {\n'
    call = '''  AURORA_ASSERT(SDL_Init(SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD | SDL_INIT_SENSOR),
                "Failed to initialize SDL subsystems: {}", SDL_GetError());'''
    for anchor in (begin, call):
        if text.count(anchor) != 1:
            raise ValueError('input startup anchor missing or ambiguous: ' + anchor)
    text = text.replace(begin, begin + '  PetariBootSplash::startupEvent("input_initialize_enter");\n')
    text = text.replace(call,
        '  PetariBootSplash::startupEvent("sdl_input_init_enter");\n' + call + '\n'
        '  PetariBootSplash::startupEvent("sdl_input_init_exit");\n'
        '  PetariBootSplash::startupEvent("input_initialize_exit");')
    return '#include "' + str(Path(__file__).resolve().with_name('boot_splash.h')) + '"\n' + text


if __name__ == '__main__':
    source, output = map(Path, sys.argv[1:])
    text = patch(source.read_text())
    output.parent.mkdir(parents=True, exist_ok=True)
    if not output.exists() or output.read_text() != text:
        output.write_text(text)
