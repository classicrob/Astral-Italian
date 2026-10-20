#!/usr/bin/env python3
"""Extract the two production native write sites; test actual canonical PartsModel layout.

This is a typed-storage test with inert constructor/virtual-method fixtures, not a
whole actor/lifecycle execution claim. ASan/UBSan are additional checks; the old
intra-object wrong-field writes are rejected by explicit byte/value assertions.
"""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('compiler', nargs='?', default='clang++')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    source = (ROOT/'src/Game/Enemy/PukupukuWaterSurface.cpp').read_text()
    sites = re.findall(r'#ifdef GALAXY_NATIVE\n\s*(mPartsModel->mCheckHiddenHost = (?:true|false);)\n#else\n\s*(reinterpret_cast< u8\* >\(mPartsModel\)\[0x9D\] = [01];)\n#endif', source)
    assert len(sites) == 2 and sites[0][0].endswith('true;') and sites[1][0].endswith('false;'), 'Production site shape changed'
    template = (ROOT/'native/tests/pukupuku_parts_offset_tests.cpp').read_text()
    flags = [args.compiler, '-std=c++17', '-O1', '-DGALAXY_NATIVE=1', '-fno-rtti',
             '-Wno-register', '-Wno-inconsistent-missing-override', '-Wno-invalid-offset-of',
             '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-omit-frame-pointer']
    for name in ['native/include', 'include', 'libs/JSystem/include', 'libs/RVL_SDK/include', 'libs/nw4r/include']:
        flags += ['-I', str(ROOT/name)]
    results = []
    with tempfile.TemporaryDirectory(prefix='smg2-pukupuku-parts-') as directory:
        work = Path(directory)
        for variant, column in [('production', 0), ('old-raw-index', 1)]:
            text = template.replace('// __APPEAR_WRITE__', sites[0][column]).replace('// __DEAD_WRITE__', sites[1][column])
            fixture = work/'test.cpp'; fixture.write_text(text); binary = work/'test'
            command = flags+[str(fixture), '-o', str(binary)]
            subprocess.run(command, check=True)
            run = subprocess.run([str(binary)], capture_output=True, text=True,
                                 env=dict(os.environ, UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=0'))
            if variant == 'production':
                assert run.returncode == 0, run.stdout+run.stderr
                print(run.stdout.strip())
                results.append({'variant': variant, 'exit': 0, 'stdout': run.stdout.strip()})
            else:
                assert run.returncode and 'FAIL: old index changes unrelated bind flag' in run.stderr, run.stdout+run.stderr
                print('old-raw-index: rejected (unrelated bind flag changed)')
                results.append({'variant': variant, 'rejected': True, 'reason': 'unrelated bind flag changed'})
    if args.output:
        args.output.write_text(json.dumps({'results': results}, indent=2)+'\n')


if __name__ == '__main__':
    main()
