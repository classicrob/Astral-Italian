#!/usr/bin/env python3
"""Build actual legacy text TUs with typed char16_t and ASan/UBSan.

Run with nice -n 19. --baseline REV compiles that revision's implementation
against the current regression test; it must fail. No game data is required.
"""
import argparse
import importlib.util
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('encoding', ROOT / 'native/tools/encode_legacy_sources.py')
encoding = importlib.util.module_from_spec(spec)
spec.loader.exec_module(encoding)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--case', choices=['region', 'conversion'], default='region')
    parser.add_argument('--baseline', help='git revision used for implementation sources only')
    args = parser.parse_args()
    sources = ['src/Game/Util/StringUtil.cpp']
    if args.case == 'region':
        sources += ['src/Game/System/Language.cpp', 'src/Game/System/NerveExecutor.cpp',
                    'src/JSystem/JMath/random.cpp']
    with tempfile.TemporaryDirectory(prefix='petari-text-') as tmp:
        tmp = Path(tmp)
        legacy = tmp / 'legacy'
        encoding.prepare(ROOT, legacy)
        if args.baseline:
            for source in sources:
                original = subprocess.check_output(['git', 'show', args.baseline + ':' + source], cwd=ROOT).decode()
                (legacy / source).write_text('#include <petari/utf16.h>\n#include <petari/msl_stdio.h>\n' + encoding.transform(original))
        includes = ['include', 'libs/RVL_SDK/include', 'libs/JSystem/include',
                    'libs/nw4r/include', 'libs/libms/include', 'libs/RVLFaceLib/include']
        command = [os.environ.get('CXX', 'clang++'), '-std=c++17', '-DGALAXY_NATIVE=1',
                   '-O1', '-g', '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                   '-fno-omit-frame-pointer', '-fno-strict-aliasing', '-fno-rtti',
                   '-Wno-register', '-Wno-unknown-pragmas', '-Wno-inconsistent-missing-override',
                   '-Wno-format']
        for directory in includes:
            command += ['-I', str(legacy / directory)]
        command += ['-I', str(ROOT / 'native/include')]
        test_source = 'region_prefix_tests.cpp' if args.case == 'region' else 'utf16_conversion_tests.cpp'
        command += [str(ROOT / 'native/tests' / test_source)]
        command += [str(legacy / source) for source in sources]
        command += ['-Wl,-dead_strip', '-o', str(tmp / 'test')]
        subprocess.run(command, check=True)
        result = subprocess.run([str(tmp / 'test')], capture_output=True, text=True)
        if args.baseline:
            expected = ('AddressSanitizer: stack-buffer-overflow' if args.case == 'region'
                        else 'Incorrect conversion:')
            if result.returncode == 0 or expected not in result.stderr:
                raise SystemExit('baseline did not reproduce the expected failure')
            # Summarize diagnostics without copying lengthy sanitizer logs.
            diagnostic = next((line.strip() for line in (result.stderr + result.stdout).splitlines()
                               if 'SUMMARY:' in line or 'Incorrect' in line), 'test returned ' + str(result.returncode))
            print('Expected baseline failure:', diagnostic)
        else:
            if result.returncode:
                print(result.stderr)
                raise SystemExit(result.returncode)
            print(result.stdout, end='')


if __name__ == '__main__':
    main()
