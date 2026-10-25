#!/usr/bin/env python3
"""Regenerate the shipped native membership from retained source evidence + v14."""
import copy
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'native/tools'))
import build_frontend_membership as builder
import seedpack

class Frontend(unittest.TestCase):
    def setUp(self): self.source = json.loads(builder.SOURCE.read_text())
    def test_exact_reproduction(self):
        # No preserved application/environment dependency: expand the committed pack.
        with tempfile.TemporaryDirectory() as d:
            subprocess.run([sys.executable, '-B', str(ROOT/'native/tools/seedpack.py'), 'expand',
                            str(ROOT/'native/data/pipeline-seeds.v14.xz'), d], check=True, capture_output=True)
            self.assertEqual(builder.render(self.source, Path(d)), builder.HEADER.read_text())
            stages = builder.membership(self.source)
            self.assertEqual((len(stages['Unknown']), len(stages['FileSelect'])), (13,75))
            self.assertIn('3fc41d69ec1e1681', stages['Unknown'])
    def test_pair_disagreement(self):
        self.source['arms'][1]['events_before_quit'].append(['Unknown','0000000000000001'])
        with self.assertRaises(ValueError): builder.membership(self.source)
    def test_ambiguous_quit(self):
        self.source['arms'][0]['quit_markers'] = 2
        with self.assertRaises(ValueError): builder.membership(self.source)
    def test_post_quit_disagreement(self):
        self.source['arms'][1]['events_after_quit'].append(['Unknown','0000000000000001'])
        with self.assertRaises(ValueError): builder.membership(self.source)
    def test_duplicate_observation_dedup(self):
        expected=builder.membership(self.source)
        self.source['arms'][0]['events_before_quit'] *= 2
        self.assertEqual(expected,builder.membership(self.source))

if __name__=='__main__': unittest.main()
