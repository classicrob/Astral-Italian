#!/usr/bin/env python3
"""Prepare a new owned title perf copy from an immutable candidate + BUILD.json."""
import argparse
import json
from pathlib import Path
import plistlib
import re
import stat
import subprocess
import common


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate-dir', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--bundle-id', required=True)
    args = parser.parse_args()
    source, output = args.candidate_dir.expanduser().resolve(), args.out.expanduser().resolve()
    if not re.fullmatch(r'dev\.petari\.Petari\.perf[A-Za-z0-9_-]+', args.bundle_id):
        parser.error('dedicated perf bundle ID required')
    if output.exists() or not output.is_relative_to(common.ROOT.resolve()):
        parser.error('output must be a new directory under port53/m1g')
    common.disk()
    build = json.loads((source / 'BUILD.json').read_text())
    original = source / 'Petari.app'
    assert re.fullmatch(r'[0-9a-f]{40}', build['source'])
    assert common.sha(original / 'Contents/MacOS/Petari') == build['binary_sha256']
    before = common.tree(original)
    resources = common.tree(original / 'Contents/Resources')
    output.mkdir(parents=True); app = output / 'Petari.app'
    subprocess.run(['/usr/bin/ditto', str(original), str(app)], check=True)
    # ditto preserves the immutable candidate's read-only modes. Only the owned
    # copy becomes writable; keep original permissions and bytes untouched.
    for path in (app, *app.rglob('*')):
        if not path.is_symlink(): path.chmod(path.stat().st_mode | stat.S_IWUSR)
    subprocess.run(['/usr/libexec/PlistBuddy', '-c', 'Set :CFBundleIdentifier ' + args.bundle_id,
                    str(app / 'Contents/Info.plist')], check=True)
    subprocess.run(['/usr/bin/codesign', '--force', '--deep', '-s', '-', str(app)], check=True, capture_output=True)
    with (app / 'Contents/Info.plist').open('rb') as file:
        assert plistlib.load(file)['CFBundleIdentifier'] == args.bundle_id
    assert common.tree(original) == before
    assert common.tree(app / 'Contents/Resources') == resources
    common.write(output / 'PREP.json', {'build_kind':'server-candidate', 'bundle_id':args.bundle_id,
        'source_commit':build['source'], 'original_app':str(original), 'app':str(app), 'build':build,
        'original_exe_sha256':build['binary_sha256'], 'perf_sha256':common.sha(app / 'Contents/MacOS/Petari'),
        'resource_tree':resources, 'source_app_unchanged':True})
    print(output / 'PREP.json')


if __name__ == '__main__': main()
