#!/usr/bin/env python3
"""Compile/run U066–072 production functions and three rejecting controls.

Before JMAFloatConversion lands, use --overlay /path/to/frozen/overlay.
Only temporary copies/objects are written; no full native or Wii build.
"""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('compiler', nargs='?', default='clang++')
    parser.add_argument('--overlay', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    spec = importlib.util.spec_from_file_location('encode', ROOT / 'native/tools/encode_legacy_sources.py')
    encoding = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(encoding)
    results = []
    with tempfile.TemporaryDirectory(prefix='smg2-game-math-') as directory:
        work = Path(directory)
        encoded = work / 'sjis'
        encoding.prepare(ROOT, encoded)
        command = [args.compiler, '-std=c++17', '-O1', '-DGALAXY_NATIVE=1', '-fno-rtti',
                   '-fno-strict-aliasing', '-ffp-contract=off', '-ffunction-sections', '-fdata-sections',
                   '-Wno-register', '-Wno-inconsistent-missing-override', '-Werror=return-type',
                   '-fsanitize=address,undefined,float-cast-overflow', '-fno-sanitize-recover=all',
                   '-fno-omit-frame-pointer']
        if args.overlay:
            command += ['-I', str(args.overlay.resolve())]
        for name in ['native/include', 'native/home_menu/include', 'include', 'libs/JSystem/include',
                     'libs/RVL_SDK/include', 'libs/nw4r/include', 'libs/RVLFaceLib/include', 'libs/libms/include']:
            if not name.startswith('native/'):
                command += ['-I', str(encoded / name)]
            command += ['-I', str(ROOT / name)]
        sources = ['src/Game/Util/MathUtil.cpp', 'src/Game/Util/MtxUtil.cpp',
                   'src/JSystem/JMath/JMATrigonometric.cpp']
        objects = []
        for source in sources + ['native/tests/game_math_conversion_tests.cpp']:
            path = encoded / source if source.startswith('src/') else ROOT / source
            obj = work / (Path(source).stem + '.o')
            subprocess.run(command + ['-c', str(path), '-o', str(obj)], check=True)
            objects.append(obj)
        link = [args.compiler, '-fsanitize=address,undefined,float-cast-overflow']
        link += ['-Wl,-dead_strip'] if sys.platform == 'darwin' else ['-Wl,--gc-sections']
        binary = work / 'game-math-tests'
        subprocess.run(link + [str(p) for p in objects] + ['-o', str(binary)], check=True)
        environment = dict(os.environ, UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=0')
        result = subprocess.run([str(binary)], capture_output=True, text=True, env=environment)
        if result.returncode:
            sys.stderr.write(result.stdout + result.stderr)
            raise RuntimeError('production conversion test failed')
        print(result.stdout.strip())
        results.append({'variant': 'production', 'exit_code': result.returncode, 'stdout': result.stdout.strip()})
        # Mutate ONLY scratch copies of the actual production file. Neither
        # compiler failure nor an unrelated failing binary qualifies a control.
        math = (encoded / sources[0]).read_bytes()
        mtx = (encoded / sources[1]).read_bytes()
        assert math.count(b'JMath::truncToS16(') == 3
        assert mtx.count(b'JMath::truncToS16(') == 4
        mutants = {
            'legacy-direct-cast': (0, math.replace(b'JMath::truncToS16(', b'static_cast<s16>('), 'runtime error:'),
            'legacy-rotation-cast': (1, mtx.replace(b'JMath::truncToS16(', b'static_cast<s16>('), 'runtime error:'),
            'wrong-five-bit-shift': (0, math.replace(b'const u32 shift = q & 0x3f;',
                                                    b'const u32 shift = q & 0x1f;'), 'FAIL: SLW q: x')
        }
        for name, (index, source, diagnostic) in mutants.items():
            assert source != (math if index == 0 else mtx), name
            mutant = work / (name + '.cpp')
            mutant.write_bytes(source)
            obj = work / (name + '.o')
            subprocess.run(command + ['-c', str(mutant), '-o', str(obj)], check=True)
            selected = list(objects)
            selected[index] = obj
            subprocess.run(link + [str(p) for p in selected] + ['-o', str(binary)], check=True)
            result = subprocess.run([str(binary)], capture_output=True, text=True, env=environment)
            assert result.returncode != 0 and diagnostic in result.stderr, (name, result.returncode, result.stderr)
            results.append({'variant': name, 'rejected': True, 'exit_code': result.returncode,
                            'diagnostic_class': 'float-cast-overflow' if name.startswith('legacy') else 'wrong SLW result'})
            print(name + ': rejected')
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps({'results': results}, indent=2) + '\n')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
