#!/usr/bin/env python3
"""Actual Popo control body + canonical YoshiLockOnTarget constructor/layout."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('compiler', nargs='?', default='clang++')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    source = re.sub(r'^\s*#include[^\n]*', '', (ROOT/'src/Game/Enemy/Popo.cpp').read_text(), flags=re.MULTILINE)
    source = subprocess.run([args.compiler, '-E', '-P', '-x', 'c++', '-DGALAXY_NATIVE=1', '-'],
                            input=source, text=True, capture_output=True, check=True).stdout
    signature = 'void Popo::control()'
    assert source.count(signature) == 1
    begin = source.index(signature); opening = source.index('{', begin); end = opening + 1; level = 1
    while level:
        if source[end] == '{': level += 1
        elif source[end] == '}': level -= 1
        end += 1
    control = source[begin:end].replace('Popo::control()', 'PopoFixture::control()')
    current = 'mYoshiLockOn->mOffset = mLockOnOffset;'
    assert control.count(current) == 1
    old = '*reinterpret_cast< TVec3f* >(reinterpret_cast< u8* >(mYoshiLockOn) + 8) = mLockOnOffset;'
    template = (ROOT/'native/tests/popo_lockon_offset_tests.cpp').read_text()
    spec = importlib.util.spec_from_file_location('encode', ROOT/'native/tools/encode_legacy_sources.py')
    encoding = importlib.util.module_from_spec(spec); spec.loader.exec_module(encoding)
    with tempfile.TemporaryDirectory(prefix='smg2-popo-lockon-') as directory:
        work = Path(directory); encoded = work/'sjis'; encoding.prepare(ROOT, encoded)
        flags = [args.compiler, '-std=c++17', '-O1', '-DGALAXY_NATIVE=1', '-fno-rtti',
                 '-fno-strict-aliasing', '-ffp-contract=off', '-ffunction-sections', '-fdata-sections',
                 '-Wno-register', '-Wno-inconsistent-missing-override', '-Werror=return-type',
                 '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-omit-frame-pointer']
        for name in ['native/include', 'native/home_menu/include', 'include', 'libs/JSystem/include',
                     'libs/RVL_SDK/include', 'libs/nw4r/include', 'libs/RVLFaceLib/include', 'libs/libms/include']:
            if not name.startswith('native/'): flags += ['-I', str(encoded/name)]
            flags += ['-I', str(ROOT/name)]
        objects = []
        for path in ['src/Game/Player/YoshiLockOnTarget.cpp']:
            obj = work/(Path(path).stem+'.o')
            src = encoded/path if path.startswith('src/') else ROOT/path
            subprocess.run(flags+['-c', str(src), '-o', str(obj)], check=True); objects.append(obj)
        link = [args.compiler, '-fsanitize=address,undefined']
        link += ['-Wl,-dead_strip'] if sys.platform == 'darwin' else ['-Wl,--gc-sections']
        harness = work/'harness.cpp'; obj = work/'harness.o'; binary = work/'test'
        environment = dict(os.environ, UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=0')
        results = []
        for name, body in [('production', control), ('old-raw-eight', control.replace(current, old))]:
            harness.write_text(template.replace('// __POPO_CONTROL__', body))
            subprocess.run(flags+['-c', str(harness), '-o', str(obj)], check=True)
            subprocess.run(link+[str(obj)]+[str(p) for p in objects]+['-o', str(binary)], check=True)
            result = subprocess.run([str(binary)], capture_output=True, text=True, env=environment)
            if name == 'production':
                if result.returncode:
                    sys.stderr.write(result.stdout+result.stderr); raise RuntimeError('Popo typed-field test failed')
                print(result.stdout.strip()); results.append({'variant': name, 'exit_code': 0, 'stdout': result.stdout.strip()})
            else:
                assert result.returncode and 'FAIL: matrix pointer bytes overwritten' in result.stderr, result.stderr
                print('old-raw-eight: rejected (matrix pointer bytes overwritten)')
                results.append({'variant': name, 'rejected': True, 'diagnostic_class': 'actual matrix pointer corruption'})
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps({'results': results}, indent=2)+'\n')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
