#!/usr/bin/env python3
"""Exercise production boss nerve wrappers with native Spine and recorder executors.

These fixtures isolate dispatch from scene/resources. They do not host complete
boss scenes or qualify rendering, physics, or exe-body behavior.
"""
import argparse
import importlib.util
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def robot():
    source = (ROOT/'src/Game/Boss/KoopaJrRobot.cpp').read_text()
    nerves = source[source.index('namespace NrvKoopaJrRobot {'):source.index('KoopaJrRobot::KoopaJrRobot(')]
    code = r'''
#include "Game/LiveActor/Nerve.hpp"
#include <cstdio>
#include <cstdlib>
struct LiveActor { int killed = 0; void kill() { ++killed; } };
static const Nerve* next;
static bool finished;
namespace MR {
void updateActorStateAndNextNerve(LiveActor*, const Nerve* nerve) { next = nerve; }
bool updateActorState(LiveActor*) { return finished; }
}
void check(bool ok) { if (!ok) std::abort(); }
''' + nerves + r'''
int main() {
    using namespace NrvKoopaJrRobot;
    const Nerve* sequence[] = {&KoopaJrRobotNrvIntro::sInstance, &KoopaJrRobotNrvPhase1::sInstance,
        &KoopaJrRobotNrvPhase2::sInstance, &KoopaJrRobotNrvDemo::sInstance,
        &KoopaJrRobotNrvPhase3::sInstance, &KoopaJrRobotNrvDead::sInstance};
    LiveActor actor;
    for (int i = 0; i < 5; ++i) {
        next = nullptr;
        Spine spine(&actor, sequence[i]); spine.update();
        check(next == sequence[i + 1]); check(!actor.killed);
    }
    Spine spine(&actor, sequence[5]);
    finished = false; spine.update(); check(!actor.killed);
    finished = true; spine.update(); check(actor.killed == 1);
    std::puts("KoopaJrRobot: production phase sequence and terminal dispatch passed");
}
'''
    wrong = nerves.replace('&KoopaJrRobotNrvPhase2::sInstance);', '&KoopaJrRobotNrvDemo::sInstance);')
    assert wrong != nerves
    return code, code.replace(nerves, wrong)


FIXTURES = {'KoopaJrRobot': robot}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('families', nargs='*')
    ap.add_argument('--compiler', default='clang++')
    a = ap.parse_args()
    if any(name not in FIXTURES for name in a.families): ap.error('families: ' + ', '.join(FIXTURES))
    spec = importlib.util.spec_from_file_location('encoding', ROOT/'native/tools/encode_legacy_sources.py')
    encoding = importlib.util.module_from_spec(spec); spec.loader.exec_module(encoding)
    with tempfile.TemporaryDirectory(prefix='/tmp/wmap-boss-native-') as directory:
        work = Path(directory); encoded = work/'sjis'; encoding.prepare(ROOT, encoded)
        flags = [a.compiler, '-std=c++17', '-O1', '-DGALAXY_NATIVE=1', '-DGALAXY_HEADLESS=1',
                 '-fno-rtti', '-ffunction-sections', '-fdata-sections', '-Wno-register',
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
        for family in a.families or FIXTURES:
            code, mutant = FIXTURES[family]()
            for label, text in [('production', code), ('crossed-target', mutant)]:
                cpp = work/'test.cpp'; cpp.write_text(text); binary = work/'test'
                subprocess.run(flags + [str(cpp)] + objects + link + ['-o', str(binary)], check=True)
                result = subprocess.run([str(binary)], capture_output=True, text=True,
                                        env=dict(os.environ, UBSAN_OPTIONS='halt_on_error=1'), timeout=15)
                if label == 'production':
                    if result.returncode: raise RuntimeError(result.stdout + result.stderr)
                    print(result.stdout.strip())
                else:
                    assert result.returncode, 'crossed target escaped regression'
                    print(f'{family}: crossed-target negative control rejected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
