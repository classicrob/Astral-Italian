#!/usr/bin/env python3
"""Private runtime regression: a gated neutral tape must get past boot to sync."""
import argparse
import csv
import json
import os
from pathlib import Path
import subprocess
import sys

import headless


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--disc', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    output = headless.outside_repo(args.output)
    output.mkdir(parents=True, exist_ok=False)
    inputs, trace = output / 'synthetic-inputs.jsonl', output / 'native.csv'
    frames = 16
    with inputs.open('x') as stream:
        for row in headless.generate(frames=frames, idle=frames):
            stream.write(json.dumps(row, separators=(',', ':')) + '\n')
    command = [sys.executable, str(Path(headless.__file__).resolve()), 'run',
               '--executable', str(args.executable.resolve()), '--disc', str(args.disc.resolve()),
               '--inputs', str(inputs), '--frames', str(frames), '--epoch-ticks', '0',
               '--user', str(output / 'user'), '--trace', str(trace),
               '--stage', 'RedBlueExGalaxy', '--scenario', '1', '--sync-stage', 'RedBlueExGalaxy',
               '--sync-scenario', '1', '--inputs-from-sync', '--actor-trace']
    with (output / 'run.log').open('x') as log:
        status = subprocess.call(command, stdout=log, stderr=subprocess.STDOUT)
    if status:
        raise RuntimeError(f'gated runtime failed ({status}); see {output / "run.log"}')
    manifest = json.loads(Path(str(trace) + '.manifest.json').read_text())
    origin = manifest['sync_origin']
    with trace.open() as stream:
        rows = list(csv.DictReader(stream))
    if (manifest['status'] != 'complete' or manifest['input_frames'] != frames or
            manifest['trace_frames'] != frames + 1 or len(rows) != frames + 1 or
            origin['source_frame'] <= frames or manifest['input_source_start'] != origin['source_frame'] + 1 or
            rows[0]['stage'] != 'RedBlueExGalaxy' or rows[0]['init_state'] != '6' or
            manifest['actors']['status'] != 'captured'):
        raise RuntimeError('gated runtime did not preserve the post-anchor tape contract')
    result = dict(status='PASS', qualification='synthetic bootstrap regression only', input_frames=frames,
                  trace_frames=len(rows), source_origin=origin['source_frame'],
                  asan_options=os.environ.get('ASAN_OPTIONS'), manifest=Path(str(trace) + '.manifest.json').name)
    (output / 'regression.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result))
    return 0


if __name__ == '__main__':
    sys.exit(main())
