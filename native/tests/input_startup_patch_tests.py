#!/usr/bin/env python3
"""Validate startup logging against pinned input code without compiling it."""
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('input_startup_patch', ROOT / 'native/gx/patch_aurora_input_startup.py')
patcher = importlib.util.module_from_spec(spec)
spec.loader.exec_module(patcher)


class StartupPatchTests(unittest.TestCase):
    def test_actual_pinned_input_call(self):
        source = (Path(sys.argv[1]) / 'lib/input.cpp').read_text()
        changed = patcher.patch(source)
        events = ('input_initialize_enter', 'sdl_input_init_enter', 'sdl_input_init_exit', 'input_initialize_exit')
        offsets = [changed.index('startupEvent("' + e + '")') for e in events]
        self.assertEqual(offsets, sorted(offsets))
        call = source[source.index('  AURORA_ASSERT(SDL_Init(SDL_INIT_HAPTIC'):source.index('\n}', source.index('  AURORA_ASSERT(SDL_Init(SDL_INIT_HAPTIC'))]
        self.assertEqual(changed.count(call), 1)
        self.assertLess(offsets[1], changed.index(call))
        self.assertLess(changed.index(call) + len(call), offsets[2])
        for event in events: self.assertEqual(changed.count('startupEvent("' + event + '")'), 1)
        for bad in (source.replace('void initialize() noexcept {', 'void initialize() {'), source + source):
            with self.assertRaises(ValueError): patcher.patch(bad)


if __name__ == '__main__':
    unittest.main(argv=[sys.argv[0]])
