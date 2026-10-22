#!/usr/bin/env python3
"""Run production world-map nerve wrappers through canonical native Spine/Nerve.

Executor methods record which exe was selected. This isolates dispatch from scene
resources; it does not qualify the exe bodies or an initialized world-map scene.
"""
import argparse
import importlib.util
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
FAMILIES = {
    'PointWarp': ('WorldMapResultTU19', ['Start', 'Wait', 'End']),
}


def block(text, anchor):
    start = text.index(anchor)
    opening = text.index('{', start)
    depth, end = 1, opening + 1
    while depth:
        if text[end] == '{': depth += 1
        elif text[end] == '}': depth -= 1
        end += 1
    return text[start:end]


def fixture(family):
    unit, states = FAMILIES[family]
    cls = 'WorldMapResultState' + family
    local = (ROOT/'include/Game/System/WorldMapResultLocal.hpp').read_text()
    source = (ROOT/('src/Game/System/' + unit + '.cpp')).read_text()
    header = local if family in ('CreateTico', 'RouteOpen') else (ROOT/('include/Game/System/' + cls + '.hpp')).read_text()
    declarations = block(header, 'namespace Nrv' + cls)
    definitions = block(source, 'namespace Nrv' + cls)
    macros = local[local.index('#define WMR_NERVE_DECL'):local.index('namespace NrvWorldMapResultStateEnd')]
    code = '#include "Game/LiveActor/Nerve.hpp"\n#include <cstdio>\n#include <cstring>\n#include <cstdlib>\n' + macros + declarations + ';\n'
    code += 'struct ' + cls + ' { const char* called = "";\n'
    for state in states:
        code += 'void exe' + state + '() { called = "' + state + '"; }\n'
    code += '};\n' + definitions + '\n'
    code += 'void check(bool ok) { if (!ok) std::abort(); }\nint main() {\n'
    for state in states:
        code += '{ ' + cls + ' executor; Spine spine(&executor, GET_NERVE(' + cls + ', ' + cls + state + ')); spine.update(); check(std::strcmp(executor.called, "' + state + '") == 0); check(spine.mStep == 1); }\n'
    code += 'std::puts("' + family + ': production nerve dispatch passed"); }\n'
    # Restore the original crossed wrapper as a negative control.
    wrong = {'PointWarp': ('exeStart();', 'exeEnd();')}[family]
    assert definitions.count(wrong[0]) == 1
    mutant = code.replace(wrong[0], wrong[1])
    assert mutant != code
    return code, mutant


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('families', nargs='*', choices=list(FAMILIES))
    ap.add_argument('--compiler', default='clang++')
    args = ap.parse_args()
    spec = importlib.util.spec_from_file_location('encode', ROOT/'native/tools/encode_legacy_sources.py')
    encoding = importlib.util.module_from_spec(spec); spec.loader.exec_module(encoding)
    with tempfile.TemporaryDirectory(prefix='wmap-native-') as directory:
        work = Path(directory); encoded = work/'sjis'; encoding.prepare(ROOT, encoded)
        flags = [args.compiler, '-std=c++17', '-O1', '-DGALAXY_NATIVE=1', '-fno-rtti',
                 '-ffunction-sections', '-fdata-sections', '-Wno-register',
                 '-Wno-inconsistent-missing-override', '-fsanitize=address,undefined',
                 '-fno-sanitize-recover=all', '-fno-omit-frame-pointer']
        for name in ['native/include', 'native/home_menu/include', 'include', 'libs/JSystem/include',
                     'libs/RVL_SDK/include', 'libs/nw4r/include', 'libs/RVLFaceLib/include', 'libs/libms/include']:
            if not name.startswith('native/'): flags += ['-I', str(encoded/name)]
            flags += ['-I', str(ROOT/name)]
        objects = []
        for unit in ['Nerve', 'Spine', 'ActorStateKeeper']:
            obj = work/(unit + '.o')
            subprocess.run(flags + ['-c', str(encoded/('src/Game/LiveActor/' + unit + '.cpp')), '-o', str(obj)], check=True)
            objects.append(str(obj))
        link = ['-Wl,-dead_strip'] if sys.platform == 'darwin' else ['-Wl,--gc-sections']
        for family in args.families or FAMILIES:
            code, mutant = fixture(family)
            for label, text in [('production', code), ('crossed-wrapper', mutant)]:
                cpp = work/'test.cpp'; cpp.write_text(text); binary = work/'test'
                subprocess.run(flags + [str(cpp)] + objects + link + ['-o', str(binary)], check=True)
                result = subprocess.run([str(binary)], capture_output=True, text=True,
                                        env=dict(os.environ, UBSAN_OPTIONS='halt_on_error=1'), timeout=15)
                if label == 'production':
                    if result.returncode: raise RuntimeError(result.stdout + result.stderr)
                    print(result.stdout.strip())
                else:
                    assert result.returncode, 'crossed wrapper escaped regression'
                    print(f'{family}: crossed-wrapper negative control rejected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
