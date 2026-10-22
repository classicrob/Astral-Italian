#!/usr/bin/env python3
"""Run extracted dispatch sites and their production nerve wrappers on the host.

Scene services are deterministic fixtures; these checks exercise the selected
instance's virtual execute call, without loading the full native game.
Each former binding is also compiled and must fail the behavioral assertion.
"""
from pathlib import Path
import argparse
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def function(source, signature):
    begin = source.index(signature)
    end = source.index('{', begin) + 1
    depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[begin:end]


CASES = {
    'Fluff': {
        'path': 'Game/Ride/Fluff',
        'signature': 'void Fluff::onDemoAction()',
        'nerves': ['FluffNrvBeforeBloom', 'FluffNrvFreeBloom'],
        'old': ('FluffNrvFreeBloom)', 'FluffNrvBeforeBloom)'),
        'fixture': r'''
namespace MR { int movements = 0; void requestMovementOn(void*) { ++movements; } }
struct Fluff : Host {
    void* mModel = this;
    void onDemoAction();
    void exeFreeBloom() { called = 1; }
};
''',
        'test': r'''
Fluff actor;
actor.current = GET_NERVE(Fluff, FluffNrvBeforeBloom);
actor.onDemoAction();
require(MR::movements == 1, "model movement requested");
Spine spine{&actor}; actor.current->execute(&spine);
require(actor.called == 1, "demo action dispatches exeFreeBloom");
''',
    },
    'MarioLauncher': {
        'path': 'Game/MapObj/MarioLauncher',
        'signature': 'bool MarioLauncher::isFollowValid(s32) const',
        'nerves': ['MarioLauncherNrvWait', 'MarioLauncherNrvReady', 'MarioLauncherNrvAim', 'MarioLauncherNrvFlight'],
        'old': ('MarioLauncherNrvAim)) || isNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvFlight)',
                'MarioLauncherNrvWait)) || isNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvReady)'),
        'fixture': r'''
using s32 = int;
struct MarioLauncher : Host {
    bool isFollowValid(s32) const;
    void exeWait() { called = 1; } void exeReady() { called = 2; }
    void exeAim() { called = 3; } void exeFlight() { called = 4; }
};
''',
        'test': r'''
MarioLauncher actor;
const Nerve* wait = GET_NERVE(MarioLauncher, MarioLauncherNrvWait);
const Nerve* ready = GET_NERVE(MarioLauncher, MarioLauncherNrvReady);
const Nerve* aim = GET_NERVE(MarioLauncher, MarioLauncherNrvAim);
const Nerve* flight = GET_NERVE(MarioLauncher, MarioLauncherNrvFlight);
for (const Nerve* nerve : {wait, ready, aim, flight}) {
    actor.current = nerve;
    Spine spine{&actor}; actor.current->execute(&spine);
    for (s32 id : {-1, 0, 1})
        require(actor.isFollowValid(id) == (actor.called == 3 || actor.called == 4), "follow valid only while Aim or Flight executes");
}
''',
    },
    'HomeButtonLayout': {
        'path': 'Game/Screen/HomeButtonLayout',
        'signature': 'void HomeButtonLayout::forceToDeactive()',
        'nerves': ['HomeButtonLayoutPrepare', 'HomeButtonLayoutActive', 'HomeButtonLayoutDeactive'],
        'old': ('isNerve(GET_NERVE(HomeButtonLayout, HomeButtonLayoutPrepare))',
                'isNerve(GET_NERVE(HomeButtonLayout, HomeButtonLayoutActive))'),
        'fixture': r'''
namespace RSO { int blackouts = 0; void HBMStartBlackOut() { ++blackouts; } }
struct HomeButtonLayout : Host {
    void* mMenuContext = nullptr; bool mForcedDeactivation = false;
    void forceToDeactive();
    void exePrepare() { called = 1; }
    void exeActive() { called = 2; }
    void exeDeactive() { called = 3; }
};
''',
        'test': r'''
HomeButtonLayout actor;
const Nerve* prepare = GET_NERVE(HomeButtonLayout, HomeButtonLayoutPrepare);
const Nerve* active = GET_NERVE(HomeButtonLayout, HomeButtonLayoutActive);
const Nerve* deactive = GET_NERVE(HomeButtonLayout, HomeButtonLayoutDeactive);
actor.current = prepare; actor.forceToDeactive();
require(actor.current == prepare && !actor.mForcedDeactivation && RSO::blackouts == 0, "null menu is ignored");
actor.mMenuContext = &actor;
for (const Nerve* initial : {prepare, active, deactive}) {
    actor.current = initial; actor.called = 0; actor.mForcedDeactivation = false;
    actor.forceToDeactive();
    require(actor.mForcedDeactivation, "forced deactivation recorded");
    Spine spine{&actor}; actor.current->execute(&spine);
    require(actor.called == (initial == active ? 2 : 3), "Prepare dispatches Deactive; Active finishes blackout");
}
require(RSO::blackouts == 3, "blackout requested for each valid menu");
''',
    },
    'FileSelector': {
        'path': 'Game/Map/FileSelector',
        'signature': 'void FileSelector::exeTitleEnd()',
        'nerves': ['FileSelectorNrvFileSelectStart', 'FileSelectorNrvFileSelect'],
        'old': ('FileSelectorNrvFileSelect)', 'FileSelectorNrvFileSelectStart)'),
        'fixture': r'''
struct Camera { bool far = true; void goToFarPoint() {} bool isAtFarPoint() { return far; } };
struct MiiSelect { void collectValidMiiIndex() {} };
namespace MR {
bool loaded = true, first = false;
bool isFirstStep(void*) { return first; }
bool isValidSwitchA(void*) { return false; }
void onSwitchA(void*) {}
void startStarPointerModeFileSelect(void*) {}
bool isEndStationedArchiveLoader() { return loaded; }
}
void petari_milestone(const char*) {}
struct FileSelector : Host {
    Camera camera; MiiSelect mii;
    Camera* mCameraController = &camera; MiiSelect* mMiiSelect = &mii;
    void calcBasePos(float) {} void appearAllItems() {} void initAllItems() {}
    void invalidateSelectAll() {} void updateChildObjs() {}
    int rotations = 0;
    void validateRotateAllItems() { ++rotations; }
    void exeTitleEnd();
    void exeFileSelectStart() { called = 1; }
    void exeFileSelect() { called = 2; }
};
''',
        'test': r'''
FileSelector actor;
for (bool first : {false, true}) {
    MR::first = first;
    for (bool far : {false, true}) for (bool loaded : {false, true}) {
        actor.current = nullptr; actor.called = 0; actor.rotations = 0;
        actor.camera.far = far; MR::loaded = loaded;
        actor.exeTitleEnd();
        if (far && loaded) {
            require(actor.rotations == 1, "rotation enabled");
            require(actor.current != nullptr, "selected an instance");
            Spine spine{&actor}; actor.current->execute(&spine);
            require(actor.called == 2, "title completion dispatches exeFileSelect");
        } else require(actor.current == nullptr, "wait for camera and archives");
    }
}
''',
    },
}


def run(name, case, work):
    source = (ROOT / ('src/' + case['path'] + '.cpp')).read_text()
    body = function(source, case['signature'])
    wrappers = []
    for nerve in case['nerves']:
        lines = [line for line in source.splitlines() if 'NEW_NERVE(' + nerve + ',' in line]
        if not lines:
            wrappers.append(function(source, 'class ' + nerve) + ';\n'
                            + nerve + ' ' + nerve + '::sInstance;')
        else:
            assert len(lines) == 1, nerve
            wrappers.append(lines[0])
    header = (ROOT / 'include/Game/LiveActor/Nerve.hpp').read_text()
    macros = '#define NERVE_INSTANCE(name) inline name name::sInstance;\n'
    macros += header[header.index('#define NEW_NERVE('):]
    prefix = r'''
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
struct Spine { void* mExecutor; };
struct Nerve { virtual void execute(Spine*) const = 0; };
struct Host {
    const Nerve* current = nullptr;
    int called = 0;
    void setNerve(const Nerve* value) { current = value; }
    bool isNerve(const Nerve* value) const { return current == value; }
};
void require(bool condition, const char* message) {
    if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
'''
    old_from, old_to = case['old']
    assert body.count(old_from) == 1
    for label, site in [('production', body), ('former-binding', body.replace(old_from, old_to))]:
        cpp = work / (name + '.cpp')
        binary = work / name
        cpp.write_text(prefix + macros + case['fixture'] + '\nnamespace Nrv' + name + ' {\n'
                       + '\n'.join(wrappers) + '\n}\n' + site + '\nint main() {\n'
                       + case['test'] + '\n}\n')
        subprocess.run(['clang++', '-std=c++17', '-DGALAXY_NATIVE=1', '-O1',
                        '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                        str(cpp), '-o', str(binary)], check=True)
        result = subprocess.run([str(binary)], capture_output=True, text=True)
        if label == 'production':
            assert result.returncode == 0, result.stderr
        else:
            assert result.returncode != 0 and 'FAIL:' in result.stderr, result.stderr
        print(f'{name}: {label} ' + ('PASS' if label == 'production' else 'rejected'))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('families', nargs='*', choices=list(CASES))
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='misc-native-dispatch-') as directory:
        for name in args.families or CASES:
            run(name, CASES[name], Path(directory))
