#!/usr/bin/env python3
"""Compile and run the real portable JMath source, without test SDK stubs."""
import argparse
import os
from pathlib import Path
import platform
import shlex
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cxx', default=os.environ.get('CXX', 'clang++'))
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[3]
    common = shlex.split(args.cxx) + [
        '-std=c++11', '-ffp-contract=off', '-ffunction-sections', '-fdata-sections',
        '-I' + str(root / 'libs/JSystem/include'),
        '-I' + str(root / 'libs/RVL_SDK/include'),
        str(root / 'tools/team/tests/jmath_native.cpp'),
        str(root / 'src/JSystem/JMath/JMath.cpp'),
    ]
    # Unused Euler/matrix functions reference SDK functions outside this test.
    # Remove those unused functions rather than supplying test implementations.
    common += ['-Wl,-dead_strip' if platform.system() == 'Darwin' else '-Wl,--gc-sections']
    with tempfile.TemporaryDirectory(prefix='jmath-native-') as directory:
        for name, flags in [
            ('optimized', ['-O2']),
            ('sanitized', ['-O1', '-g', '-fsanitize=address,undefined', '-fno-sanitize-recover=all']),
        ]:
            binary = str(Path(directory) / name)
            subprocess.run(common + flags + ['-o', binary], check=True)
            print(name + ':', flush=True)
            subprocess.run([binary], check=True)


if __name__ == '__main__':
    main()
