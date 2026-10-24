#!/usr/bin/env python3
"""Synthetic parser/tool tests. The sanitized .ips fixture comes from our demo."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
from contextlib import ExitStack

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('symbolicate', ROOT / 'tools/release/symbolicate.py')
s = importlib.util.module_from_spec(spec)
spec.loader.exec_module(s)
FIXTURES = Path(__file__).with_name('crash_fixtures')
UUID = '12345678123412341234123456789ABC'


class SymbolicateTests(unittest.TestCase):
    def test_v2_and_inline(self):
        report, images, locations, lines = s.parse_report((FIXTURES / 'v2.txt').read_text())
        self.assertEqual(report['build']['source_commit'], 'test-source')
        self.assertEqual(report['crash']['fault_address'], '0x0')
        self.assertEqual(report['frames'][0]['address'], '0x0')
        self.assertEqual(len(locations), 2)
        with patch.object(s, 'command', return_value='testCrashLeaf (in Petari) (test_crash.cpp:16)\ttestCrashMiddle (in Petari) (test_crash.cpp:25)'):
            self.assertTrue(s.resolve_frames(report, images, Path('/demo.dSYM/DWARF/Petari')))
        self.assertEqual(report['frames'][1]['line'], 16)
        self.assertEqual(report['frames'][2]['file'], 'test_crash.cpp')
        self.assertTrue(report['frames'][1]['inlined'])
        self.assertFalse(report['frames'][2]['inlined'])

    def test_return_address_uses_call_instruction(self):
        report, images, _, _ = s.parse_report((FIXTURES / 'v2.txt').read_text())
        with patch.object(s, 'command', return_value='f (in Petari) (file.cpp:12)') as command:
            s.resolve_frames(report, images, Path('/demo.dSYM'))
        self.assertEqual(command.call_args.args[0][-1], '0x100001230')
        self.assertEqual(report['frames'][1]['address'], '0x100001234')

    def test_old_crash(self):
        text = """Process: Petari [123]
Exception Type: EXC_BAD_ACCESS (SIGSEGV)
Thread 0 Crashed:
0   Petari  0x100001234 0x100000000 + 4660

Binary Images:
0x100000000 - 0x10000ffff +Petari arm64 <12345678-1234-1234-1234-123456789ABC> /synthetic/Petari
"""
        report, images, locations, _ = s.parse_report(text)
        self.assertEqual(report['frames'][0]['offset'], '0x1234')
        self.assertEqual(s.uuid_key(report['build']['executable_uuid']), UUID)
        self.assertEqual(len(locations), 1)

    def test_ips(self):
        report, images, _, _ = s.parse_report((FIXTURES / 'demo.ips').read_text())
        self.assertTrue(report['build']['executable_uuid'])
        self.assertTrue(any('petari_crash_demo' == f['image'] for f in report['frames']))
        self.assertEqual(report['crash']['signal'], 'SIGSEGV')
        self.assertEqual(report['crash']['fault_address'], '0x0000000000000000')

    def test_mismatch_refused(self):
        with tempfile.TemporaryDirectory() as temp, ExitStack() as stack:
            bundle = Path(temp) / 'Stale.dSYM/Contents/Resources/DWARF'
            bundle.mkdir(parents=True)
            (bundle / 'Petari').write_text('synthetic')
            diagnostics = []
            with patch.object(s, 'command', side_effect=lambda args: 'UUID: AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA (arm64) /stale' if args[0] == 'dwarfdump' else ''):
                self.assertIsNone(s.find_dsym(UUID, [temp], stack, diagnostics))
            self.assertIn('Refusing mismatched dSYM', diagnostics[0])
            self.assertIn(UUID, diagnostics[0])

    def test_matching_and_zip_cleanup(self):
        import zipfile
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'release.dSYM.zip'
            with zipfile.ZipFile(path, 'w') as archive:
                archive.writestr('Petari.dSYM/Contents/Resources/DWARF/Petari', 'synthetic')
            with ExitStack() as stack, patch.object(s, 'command', return_value='UUID: 12345678-1234-1234-1234-123456789ABC (arm64) /test'):
                dwarf = s.find_dsym(UUID, [path], stack, [])
                self.assertTrue(dwarf.is_file())
            self.assertFalse(dwarf.exists())

    def test_wrong_image_never_uses_dsym(self):
        report, images, _, _ = s.parse_report((FIXTURES / 'stale.txt').read_text())
        calls = []
        with patch.object(s, 'command', side_effect=lambda args: calls.append(args) or ''):
            self.assertFalse(s.resolve_frames(report, images, None))
        self.assertFalse(any(args[0] == 'atos' for args in calls))

    def test_no_symbols_fails(self):
        report, images, _, _ = s.parse_report((FIXTURES / 'v2.txt').read_text())
        with patch.object(s, 'command', return_value='0x100001234'):
            self.assertFalse(s.resolve_frames(report, images, Path('/demo.dSYM')))
        self.assertEqual(report['frames'][1]['symbol'], '')

    def test_zip_traversal(self):
        import zipfile
        with tempfile.TemporaryDirectory() as temp, ExitStack() as stack:
            path = Path(temp) / 'bad.zip'
            with zipfile.ZipFile(path, 'w') as archive:
                archive.writestr('../escape', 'bad')
            with self.assertRaisesRegex(ValueError, 'unsafe path'):
                s.find_dsym(UUID, [path], stack, [])


if __name__ == '__main__':
    unittest.main()
