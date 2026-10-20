#!/usr/bin/env python3
"""U023: production observer/registration bodies and actual typed sensor lookup."""
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

    def function(path, signature):
        # Includes are removed for preprocessing only. The compiled test uses
        # canonical headers. These three bounded bodies have no brace literals.
        source = re.sub(r'^\s*#include[^\n]*', '', (ROOT/path).read_text(), flags=re.MULTILINE)
        text = subprocess.run([args.compiler, '-E', '-P', '-x', 'c++', '-DGALAXY_NATIVE=1', '-'],
                              input=source, text=True, capture_output=True, check=True).stdout
        assert text.count(signature) == 1, signature
        start = text.index(signature); opening = text.index('{', start); end = opening + 1; level = 1
        while level:
            if text[end] == '{': level += 1
            elif text[end] == '}': level -= 1
            end += 1
        return text[start:end]

    source = 'src/Game/Enemy/HammerHeadPackun.cpp'
    control = function(source, 'void HammerHeadPackun::control()')
    init = function(source, 'void HammerHeadPackun::initSensor()')
    getter = function('src/Game/LiveActor/LiveActor.cpp', 'HitSensor* LiveActor::getSensor(const char* pSensorName) const')
    assert control.count('getSensor("Body")') == 1
    definitions = '\n'.join([init.replace('HammerHeadPackun::', 'HammerHeadFixture::'),
                             getter.replace('LiveActor::', 'HammerHeadFixture::'),
                             control.replace('HammerHeadPackun::', 'HammerHeadFixture::')])
    template = (ROOT/'native/tests/hammerhead_observer_tests.cpp').read_text()
    code = template.replace('// __GAME_FUNCTIONS__', definitions)
    spec = importlib.util.spec_from_file_location('encode', ROOT/'native/tools/encode_legacy_sources.py')
    encoding = importlib.util.module_from_spec(spec); spec.loader.exec_module(encoding)
    with tempfile.TemporaryDirectory(prefix='smg2-hammerhead-') as directory:
        work = Path(directory); encoded = work/'sjis'; encoding.prepare(ROOT, encoded)
        flags = [args.compiler, '-std=c++17', '-O1', '-DGALAXY_NATIVE=1', '-fno-rtti',
                 '-fno-strict-aliasing', '-ffp-contract=off', '-ffunction-sections', '-fdata-sections',
                 '-Wno-register', '-Wno-inconsistent-missing-override', '-Wno-invalid-offsetof',
                 '-Werror=return-type', '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                 '-fno-omit-frame-pointer']
        for name in ['native/include', 'native/home_menu/include', 'include', 'libs/JSystem/include',
                     'libs/RVL_SDK/include', 'libs/nw4r/include', 'libs/RVLFaceLib/include', 'libs/libms/include']:
            if not name.startswith('native/'): flags += ['-I', str(encoded/name)]
            flags += ['-I', str(ROOT/name)]
        objects = []
        for path in ['src/Game/LiveActor/HitSensor.cpp', 'src/Game/LiveActor/HitSensorInfo.cpp',
                     'src/Game/LiveActor/HitSensorKeeper.cpp', 'src/Game/Util/HashUtil.cpp']:
            obj = work/(Path(path).stem+'.o')
            subprocess.run(flags+['-c', str(encoded/path), '-o', str(obj)], check=True); objects.append(obj)
        link = [args.compiler, '-fsanitize=address,undefined']
        link += ['-Wl,-dead_strip'] if sys.platform == 'darwin' else ['-Wl,--gc-sections']
        binary = work/'test'; harness = work/'harness.cpp'; obj = work/'harness.o'
        environment = dict(os.environ, UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=0')
        results = []
        for name, text in [('production', code), ('old-lowercase-key', code.replace('getSensor("Body")->mPosition',
                                                                                'getSensor("body")->mPosition'))]:
            if name != 'production': assert text != code
            harness.write_text(text)
            subprocess.run(flags+['-c', str(harness), '-o', str(obj)], check=True)
            subprocess.run(link+[str(obj)]+[str(p) for p in objects]+['-o', str(binary)], check=True)
            result = subprocess.run([str(binary)], capture_output=True, text=True, env=environment)
            if name == 'production':
                if result.returncode:
                    sys.stderr.write(result.stdout+result.stderr); raise RuntimeError('observer test failed')
                print(result.stdout.strip()); results.append({'variant': name, 'exit_code': 0, 'stdout': result.stdout.strip()})
            else:
                assert result.returncode and 'null pointer' in result.stderr and 'HitSensor' in result.stderr, result.stderr
                print('old-lowercase-key: rejected (actual null HitSensor access)')
                results.append({'variant': name, 'rejected': True, 'diagnostic_class': 'null HitSensor member access'})
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps({'results': results}, indent=2)+'\n')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
