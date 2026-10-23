#!/usr/bin/env python3
"""Bounded actual no-pad/other-USB checks; no game build or hardware simulation."""
import argparse
from pathlib import Path
import re
import subprocess
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', required=True, type=Path)
    parser.add_argument('--receipts', required=True, type=Path)
    parser.add_argument('--all-usb', action='store_true')
    args = parser.parse_args()
    binary = args.binary.resolve(strict=True)
    receipts = args.receipts.resolve()
    receipts.mkdir(parents=True, exist_ok=True)
    command = [str(binary), '-t', '5'] + (['-a'] if args.all_usb else [])
    begin = time.monotonic()
    run = subprocess.run(command, cwd=receipts, env={}, text=True,
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=8)
    elapsed = time.monotonic() - begin
    (receipts / 'stdout.txt').write_text(run.stdout)
    (receipts / 'stderr.txt').write_text(run.stderr)
    report = receipts / 'padprobe-report.txt'
    assert run.returncode == 0, f'exit {run.returncode}: {run.stderr}'
    assert 4.8 <= elapsed < 6, f'expected ~5 s total, got {elapsed:.3f}'
    assert report.stat().st_size < 30000, 'report exceeded 30 KB'
    assert report.read_text() == run.stdout, 'stdout/report mismatch'
    summary = run.stdout.split('FINAL SUMMARY', 1)[1]
    expected = {
        'libusb load': 'PASS', 'USB enumerate': 'FAIL', 'USB open': 'SKIP',
        'bulk interface claim': 'SKIP', 'IOHID open': 'SKIP',
        'SDL driver attached': 'FAIL', 'SDL mapping': 'SKIP',
        'gamepad event flow': 'SKIP', 'mapping of extras': 'SKIP',
        'sensors': 'SKIP', 'rumble': 'SKIP',
    }
    for layer, status in expected.items():
        assert f'\n{status} {layer}:' in summary, f'wrong/missing summary: {layer}'
    assert 'VERDICT: NO_PAD:' in summary, 'unexpected controller evidence; unplug gamepads'
    assert 'Plug the pad in now' in run.stdout
    assert 'SDL phase dyld libusb image (own dlopen already closed): /' in run.stdout
    rows = re.findall(r'^USB ([0-9a-f]{4}):([0-9a-f]{4}) ', run.stdout, re.M)
    non_pad = [row for row in rows if row[0] != '057e']
    hardware = ('PASS non-pad USB device present; other vendors never opened/claimed'
                if non_pad else 'SKIP non-pad robustness hardware: no non-Nintendo USB device attached')
    validation = (
        f'Command (empty environment): {" ".join(command)}\n'
        f'PASS exit=0 elapsed={elapsed:.3f}s (<6s, requested -t 5)\n'
        f'PASS report={report.stat().st_size} bytes (<30000), identical to stdout\n'
        'PASS all eleven no-pad layer statuses and NO_PAD verdict\n'
        'PASS SDL-phase dyld image reports libusb after the independent probe handle closed\n'
        f'USB rows: {len(rows)}; non-Nintendo rows: {len(non_pad)}\n'
        + (hardware + '\n' if args.all_usb else '')
    )
    (receipts / 'validation.txt').write_text(validation)
    print(validation, end='')


if __name__ == '__main__':
    main()
