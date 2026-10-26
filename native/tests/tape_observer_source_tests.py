#!/usr/bin/env python3
"""The observer-off SDK/replay implementation is byte-identical to 00221126."""
import hashlib
from pathlib import Path
import unittest
ROOT = Path(__file__).resolve().parents[2]


class OriginalPath(unittest.TestCase):
    def test_original_core_unchanged(self):
        text = (ROOT/'native/input/wpad_host.cpp').read_text()
        start = text.index('{', text.index('tapeReadCore(')); depth = 0
        for end in range(start, len(text)):
            depth += (text[end] == '{') - (text[end] == '}')
            if depth == 0:
                original = text[start:end+1]; break
        # Remove only the new opt-in replay block. Everything else (including
        # the live path and null-provider headless replay) remains byte exact.
        first = original.index('    // replay-only synchronous connection begin\n')
        last = original.index('    // replay-only synchronous connection end\n', first)
        last += len('    // replay-only synchronous connection end\n')
        original = original[:first] + original[last:]
        self.assertEqual(hashlib.sha256(original.encode()).hexdigest(),
                         'b0a78a556d91b355c46be71a91859e8f46cd8d8c1b68b7f917ee3890f1787d7d')

    def test_cold_off_branch_precedes_every_observer(self):
        text = (ROOT/'native/include/petari/tape_input_observer.hpp').read_text()
        off = text.index('if (!enabled) return core();')
        self.assertLess(off, text.index('if (hooks.before)'))
        self.assertLess(off, text.index('const auto state = connection();'))
        self.assertNotIn('getenv', text)
        self.assertNotIn('chrono', text)


if __name__ == '__main__': unittest.main()
