#!/usr/bin/env python3
"""Run the real handler + opt-in app crash chain and verify real atos output.

Usage: python3 -I native/tests/crash_demo_receipt.py build/crash/petari_crash_demo RECEIPTS
Creates its own .ips files, copies them into receipts, and removes only those
from DiagnosticReports. No game data is read. Keep the binary + dSYM together.
"""
import importlib.util
import json
import os
from pathlib import Path
import plistlib
import re
import shutil
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('symbolicate', ROOT / 'tools/release/symbolicate.py')
symbolicate = importlib.util.module_from_spec(spec)
spec.loader.exec_module(symbolicate)


def ips_body(path):
    text = path.read_text()
    _, end = json.JSONDecoder().raw_decode(text)
    return json.loads(text[end:])


def main():
    binary, receipts = Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve()
    receipts.mkdir(parents=True, exist_ok=True)
    dsym = Path(str(binary) + '.dSYM')
    transcript = []
    def run(args, **kwargs):
        p = subprocess.run([str(x) for x in args], capture_output=True, text=True, **kwargs)
        transcript.append('$ ' + ' '.join(str(x) for x in args) + f'\nexit: {p.returncode}\n' + p.stdout + p.stderr)
        return p
    assert run(['dsymutil', binary, '-o', dsym]).returncode == 0
    assert run(['dwarfdump', '--uuid', binary, dsym]).returncode == 0
    # Exercise CoreFoundation against a real synthetic .app Info.plist.
    with tempfile.TemporaryDirectory(prefix='petari-identity-') as temp:
        app = Path(temp) / 'Identity.app'
        (app / 'Contents/MacOS').mkdir(parents=True)
        shutil.copy2(binary, app / 'Contents/MacOS/Petari')
        expected_uuid = run([binary, '--identity']).stdout.splitlines()[0]
        with (app / 'Contents/Info.plist').open('wb') as f:
            plistlib.dump(dict(CFBundleExecutable='Petari', CFBundleIdentifier='test.petari.identity',
                               CFBundlePackageType='APPL', CFBundleShortVersionString='test-2.3',
                               AstralSourceCommit='synthetic-source', AstralExecutableUUID=expected_uuid), f)
        identity = run([app / 'Contents/MacOS/Petari', '--identity'])
        assert identity.stdout.splitlines()[:4] == [expected_uuid, 'test-2.3', 'synthetic-source', expected_uuid], identity.stdout
    diagnostics = Path.home() / 'Library/Logs/DiagnosticReports'
    source = (ROOT / 'native/app/test_crash.cpp').read_text().splitlines()
    def verify(report, label):
        p = run(['sh', ROOT / 'tools/release/symbolicate.sh', report, '--dsym', dsym, '--json'])
        (receipts / f'{label}.json').write_text(p.stdout)
        assert p.returncode == 0, p.stderr
        frames = json.loads(p.stdout)['frames']
        for name in ['testCrashLeaf', 'testCrashMiddle', 'testCrashOuter']:
            entries = [f for f in frames if name in f['symbol']]
            assert entries and all(Path(f['file']).name == 'test_crash.cpp' and f['line'] for f in entries), entries
            start = next(i + 1 for i, line in enumerate(source) if 'void ' + name + '(' in line)
            end = next(i + 1 for i in range(start, len(source)) if source[i] == '}')
            assert all(start <= f['line'] <= end for f in entries), entries
            transcript.append(f"verified {label}: {name} ({entries[0]['file']}:{entries[0]['line']})")
    created_pids = set()
    try:
        for kind, handler in [('null-call', True), ('segv-write', True), ('abort', True), ('segv-write', False)]:
            label = kind if handler else 'macos-no-handler'
            with tempfile.TemporaryDirectory(prefix='petari-report-') as temp:
                env = dict(os.environ, PETARI_TEST_CRASH=kind)
                args = [str(binary), temp] + ([] if handler else ['--no-handler'])
                p = subprocess.Popen(args, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
                created_pids.add(p.pid)
                out, err = p.communicate(timeout=25)
                transcript.append('$ PETARI_TEST_CRASH=' + kind + ' ' + ' '.join(args) + f'\nexit: {p.returncode}\n')
                assert p.returncode < 0, (p.returncode, err)
                (receipts / f'{label}.stderr').write_text(err)
                if handler:
                    reports = list(Path(temp).glob('petari-crash-*.txt'))
                    assert len(reports) == 1
                    report = receipts / f'{label}.txt'
                    shutil.copy2(reports[0], report)
                    text = report.read_text()
                    assert 'report version: 2' in text and 'Binary Images:\n0x' in text and '#1 0x' in text
                    assert 'host thread: Petari crash demo;' in text and '==== end of report ====' in text
                    assert len(re.findall(r'^#\d+ ', text, re.M)) <= 64
                    verify(report, label)
                deadline = time.monotonic() + (1 if handler else 45)
                owned = None
                while time.monotonic() < deadline:
                    for path in diagnostics.glob('petari_crash_demo*.ips'):
                        try:
                            body = ips_body(path)
                        except (ValueError, OSError):
                            continue
                        if body.get('pid') == p.pid:
                            owned = path
                            break
                    if owned:
                        break
                    time.sleep(0.5)
                if handler and not owned:
                    continue  # Only the no-handler run requires an OS report.
                assert owned, f'macOS did not write an .ips for pid {p.pid}'
                copied = receipts / f'{label}.ips'
                shutil.copy2(owned, copied)
                owned.unlink()  # Only the .ips with this demo process's pid.
                transcript.append(f'copied {owned.name}; removed own DiagnosticReports file (pid {p.pid})')
                if not handler:
                    verify(copied, label)
    finally:
        # Verification failures must not leave our completed OS reports behind.
        for path in diagnostics.glob('petari_crash_demo*.ips'):
            try:
                if ips_body(path).get('pid') in created_pids:
                    shutil.copy2(path, receipts / path.name)
                    path.unlink()
            except (OSError, ValueError):
                pass
        (receipts / 'demo-commands.log').write_text('\n'.join(transcript))
    print('\n'.join(line for line in transcript if line.startswith('verified ') or line.startswith('copied ')))


if __name__ == '__main__':
    main()
