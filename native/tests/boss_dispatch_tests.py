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


def block(text, anchor):
    start = text.index(anchor); opening = text.index('{', start)
    depth, end = 1, opening + 1
    while depth:
        if text[end] == '{': depth += 1
        elif text[end] == '}': depth -= 1
        end += 1
    return text[start:end]


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


def sandy(phase):
    cls = 'SandyStatePhase' + str(phase)
    source = (ROOT/('src/Game/Boss/' + cls + '.cpp')).read_text()
    nerves = source[source.index('namespace Nrv' + cls + ' {'):source.index('};  // namespace Nrv' + cls) + 2]
    states = ['Start', 'Close', 'Throw', 'Open', 'OpenWait', 'Damage', 'DamageClose']
    if phase == 3: states.append('ThrowBomb')
    code = '#include "Game/LiveActor/Nerve.hpp"\n#include <cstdio>\n#include <cstdlib>\n#include <cstring>\n#include <initializer_list>\n'
    code += 'struct ' + cls + ' { const char* called = ""; bool ready = false; const Nerve* next = nullptr;\n'
    code += 'void setNerve(const Nerve* n) { next = n; }\n'
    for name in ['Start', 'Damage'] + (['Throw', 'ThrowBomb', 'Open', 'OpenWait'] if phase == 3 else []):
        code += 'void exe' + name + '() { called = "' + name + '"; }\n'
    code += '};\n'
    code += 'bool sandyStateClose(' + cls + '* p, bool damage) { p->called = damage ? "DamageClose" : "Close"; return p->ready; }\n'
    code += 'bool sandyStateThrow(' + cls + '* p, int count, int delay, bool alt) { p->called = "Throw"; if (count != ' + str(3 if phase == 1 else 4) + ' || delay != ' + str(20 if phase == 1 else 90) + ' || alt != ' + ('false' if phase == 1 else 'true') + ') std::abort(); return p->ready; }\n'
    code += 'bool sandyStateOpenStart(' + cls + '* p, int delay) { p->called = "Open"; if (delay != 60) std::abort(); return p->ready; }\n'
    code += 'bool sandyStateOpenWait(' + cls + '* p, int delay) { p->called = "OpenWait"; if (delay != ' + str(240 if phase == 1 else 120) + ') std::abort(); return p->ready; }\n'
    code += nerves + '\nvoid check(bool ok) { if (!ok) std::abort(); }\nint main() {\n'
    transitions = {'Close': 'Throw', 'DamageClose': 'Throw'}
    if phase != 3: transitions.update(Throw='Open', Open='OpenWait', OpenWait='Close')
    for state in states:
        code += 'for (bool ready : {false, true}) { ' + cls + ' actor; actor.ready = ready; Spine spine(&actor, GET_NERVE(' + cls + ', ' + cls + 'Nrv' + state + ')); spine.update(); check(std::strcmp(actor.called, "' + state + '") == 0);\n'
        if state in transitions:
            code += 'check(actor.next == (ready ? GET_NERVE(' + cls + ', ' + cls + 'Nrv' + transitions[state] + ') : nullptr));\n'
        else: code += 'check(actor.next == nullptr);\n'
        code += '}\n'
    code += 'std::puts("' + cls + ': production wrappers, helper arguments and branch targets passed"); }\n'
    wrong = nerves.replace('setNerve(&' + cls + 'NrvThrow::sInstance);', 'setNerve(&' + cls + 'NrvDamage::sInstance);')
    assert wrong != nerves
    return code, code.replace(nerves, wrong)


def castle():
    source = (ROOT/'src/Game/Boss/KoopaJrCastleHead.cpp').read_text()
    header = (ROOT/'include/Game/Boss/KoopaJrCastleNerves.hpp').read_text()
    nerves = block(header, 'namespace NrvKoopaJrCastleHead {')
    instances = source[source.index('namespace NrvKoopaJrCastleHead {'):source.index('KoopaJrCastleHead::KoopaJrCastleHead(')]
    methods = [block(source, 'void KoopaJrCastleHead::setNerve' + name + '()') for name in 'ABCD']
    methods += [block(source, 'bool KoopaJrCastleHead::canStartAction()'),
                block(source, 'bool KoopaJrCastleHead::isPanicOrSpin() const'),
                block(source, 'void KoopaJrCastleHead::resetWaitUnlessDamaged()')]
    code = r'''
#include "Game/LiveActor/Nerve.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
struct KoopaJrCastleHead {
    const char* called = "";
    Spine spine{this, nullptr};
    void setNerve(const Nerve* n) { spine.setNerve(n); }
    bool isNerve(const Nerve* n) const { return spine.getCurrentNerve() == n; }
    void setNerveA(); void setNerveB(); void setNerveC(); void setNerveD();
    bool canStartAction(); bool isPanicOrSpin() const; void resetWaitUnlessDamaged();
    void exeWait() { called = "Wait"; }
    void exeFirstDamage() { called = "FirstDamage"; }
    void exeSecondDamage() { called = "SecondDamage"; }
    void exePanic() { called = "Panic"; }
    void exeSpinSign() { called = "SpinSign"; }
    void exeSpin() { called = "Spin"; }
};
void check(bool ok) { if (!ok) std::abort(); }
''' + nerves + instances + '\n'.join(methods) + r'''
int main() {
    using namespace NrvKoopaJrCastleHead;
    const Nerve* states[] = {&KoopaJrCastleHeadNrvA::sInstance, &KoopaJrCastleHeadNrvB::sInstance,
        &KoopaJrCastleHeadNrvC::sInstance, &KoopaJrCastleHeadNrvD::sInstance,
        &KoopaJrCastleHeadNrvE::sInstance, &KoopaJrCastleHeadNrvF::sInstance,
        &KoopaJrCastleHeadNrvG::sInstance, &KoopaJrCastleHeadNrvH::sInstance};
    KoopaJrCastleHead head;
    head.setNerveA(); head.spine.update(); check(std::strcmp(head.called, "FirstDamage") == 0);
    head.setNerveB(); head.spine.update(); check(std::strcmp(head.called, "SecondDamage") == 0);
    head.setNerveC(); check(head.spine.getCurrentNerve() == states[3]);
    head.setNerveD(); head.spine.update(); check(std::strcmp(head.called, "Wait") == 0);
    for (int i = 0; i < 8; ++i) {
        head.setNerve(states[i]); check(head.canStartAction() == (i == 0));
        head.resetWaitUnlessDamaged();
        check(head.spine.getCurrentNerve() == ((i == 1 || i == 2) ? states[i] : states[0]));
    }
    std::puts("KoopaJrCastleHead: production selectors dispatch damage/wait and gate actions correctly");
}
'''
    wrong = methods[0].replace('KoopaJrCastleHeadNrvB', 'KoopaJrCastleHeadNrvA')
    assert wrong != methods[0]
    return code, code.replace(methods[0], wrong)


def last_damage():
    header = (ROOT/'include/Game/Boss/KoopaStates.hpp').read_text()
    declaration = block(header, 'class KoopaStateLastDamage :') + ';'
    source = (ROOT/'src/Game/Boss/KoopaStateBaseTU13.cpp').read_text()
    control = block(source, 'void KoopaStateLastDamage::control()')
    code = r'''
#include "Game/LiveActor/ActorStateBase.hpp"
#include <JSystem/JGeometry.hpp>
#include <cstdio>
#include <cstdlib>
class HitSensor;
struct KoopaBattle { TVec3f mPosition; };
static int blendCalls;
namespace MR {
float getEaseInOutValue(float, float, float, float, float) { return 0.5f; }
void vecBlend(TVec3f* result, const TVec3f& from, const TVec3f& to, float rate) {
    ++blendCalls;
    result->set(from.x + (to.x - from.x) * rate,
                from.y + (to.y - from.y) * rate, from.z + (to.z - from.z) * rate);
}
}
''' + declaration + control + r'''
KoopaStateLastDamage::KoopaStateLastDamage(KoopaBattle* host) : ActorStateBase<KoopaBattle>("fixture", host) {
    mStep = 4; mDuration = 120;
    mStartPos.set(1.0f, 2.0f, 3.0f); mTargetPos.set(9.0f, 12.0f, 15.0f);
}
KoopaStateLastDamage::~KoopaStateLastDamage() {}
void KoopaStateLastDamage::appear() {}
int main() {
    KoopaBattle host;
    KoopaStateLastDamage state(&host);
    ActorStateBaseInterface* interface = &state;
    interface->control();
    if (blendCalls != 1 || state.mStep != 5 || host.mPosition.x != 5.0f ||
        host.mPosition.y != 7.0f || host.mPosition.z != 9.0f) std::abort();
    std::puts("KoopaStateLastDamage: base-interface control dispatch runs production position update");
}
'''
    mutant = code.replace('virtual void control();', 'void moveToTarget();').replace('KoopaStateLastDamage::control()', 'KoopaStateLastDamage::moveToTarget()')
    assert mutant != code
    return code, mutant


def ota():
    source = (ROOT/'src/Game/Boss/OtaKingMagma.cpp').read_text()
    nerves = block(source, 'namespace NrvOtaKingMagma {')
    states = ['Wait', 'AppearDemo', 'Attack', 'FireAttack', 'HitBack', 'Damage', 'Down', 'EndWait']
    code = '#include "Game/LiveActor/Nerve.hpp"\n#include <cstdio>\n#include <cstring>\n#include <cstdlib>\nstruct OtaKingMagma { const char* called = "";\n'
    for name in states: code += 'void exe' + name + '() { called = "' + name + '"; }\n'
    code += '};\n' + nerves + '\nint main() {\n'
    for name in states:
        code += '{ OtaKingMagma actor; Spine spine(&actor, GET_NERVE(OtaKingMagma, OtaKingMagmaNrv' + name + ')); spine.update(); if (std::strcmp(actor.called, "' + name + '") != 0) std::abort(); }\n'
    code += 'std::puts("OtaKingMagma: all production nerves dispatch to their exe function"); }\n'
    old = 'NEW_NERVE(OtaKingMagmaNrvDamage, OtaKingMagma, Damage)'
    assert nerves.count(old) == 1
    return code, code.replace(old, old.replace(', Damage)', ', Down)'))


FIXTURES = {'KoopaJrRobot': robot, 'KoopaJrCastleHead': castle, 'KoopaStateLastDamage': last_damage, 'OtaKingMagma': ota, 'SandyStatePhase1': lambda: sandy(1), 'SandyStatePhase2': lambda: sandy(2), 'SandyStatePhase3': lambda: sandy(3)}


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
        for path in ['src/Game/LiveActor/Nerve.cpp', 'src/Game/LiveActor/Spine.cpp',
                     'src/Game/LiveActor/ActorStateKeeper.cpp', 'src/Game/System/NerveExecutor.cpp']:
            obj = work/(Path(path).stem + '.o')
            subprocess.run(flags + ['-c', str(encoded/path), '-o', str(obj)], check=True)
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
