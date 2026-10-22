#!/usr/bin/env python3
"""Production TwoLegs wrappers/use sites through native Spine, with scene fixtures.

Executor methods record calls. This isolates dispatch from boss scene resources;
the DOL oracle separately checks instance/constructor/vtable ordering and labels.
"""
from pathlib import Path
import importlib.util
import os
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def block(text, anchor):
    start = text.index(anchor)
    end = text.index('{', start) + 1
    depth = 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


def fixture():
    main = (ROOT/'src/Game/Boss/TwoLegs.cpp').read_text()
    shot = (ROOT/'src/Game/Boss/TwoLegsBulletTU04.cpp').read_text()
    damage = (ROOT/'src/Game/Boss/TwoLegsBulletTU06.cpp').read_text()
    macros = main[main.index('#define UPDATE_NERVE'):main.index('namespace NrvTwoLegs')]
    code = r'''
#include "Game/LiveActor/Nerve.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
void check(bool value) { if (!value) std::abort(); }
struct Camera { int starts = 0; void start() { ++starts; } };
struct TwoLegs {
    bool first = false, complete = false;
    int stateUpdates = 0, nextUpdates = 0, selections = 0, actions = 0, kills = 0;
    int mFlagC0 = 0;
    Camera camera; Camera* mCamera = &camera;
    bool updateStateAndNext() { ++nextUpdates; return complete; }
    void selectNextState() { ++selections; }
    void kill() { ++kills; }
    bool isPhase0() { return true; }
    bool isLastPhase() { return false; }
    bool tryKillStateIfPlayerBetweenFeet(void*) { return false; }
};
namespace MR {
bool isFirstStep(TwoLegs* host) { return host->first; }
bool updateActorState(TwoLegs* host) { ++host->stateUpdates; return host->complete; }
void startAction(TwoLegs* host, const char* action) { check(std::strcmp(action, "WaitFar") == 0); ++host->actions; }
}
'''
    code += macros + block(main, 'namespace NrvTwoLegs') + '\n'
    code += r'''
struct HitSensor {};
struct TwoLegsStateWalkDamage {
    static bool tryStrongStamp(void*, TwoLegs*, HitSensor*, HitSensor*) { return true; }
    static bool tryStamp(void*, TwoLegs*, HitSensor*, HitSensor*) { return true; }
};
struct TwoLegsStateShot {
    TwoLegs host; TwoLegs* mHost = &host;
    Spine* spine = nullptr;
    int called = 0, mShotCount = 0, mBulletNum = 0;
    bool isNerve(const Nerve* nerve) { return spine->isCurrentNerve(nerve); }
    void setNerve(const Nerve* nerve) { spine->setNerve(nerve); }
    void kill() {} void shootBullet() {} int countAliveBullets() { return 0; }
    bool tryAttackStrong(HitSensor*, HitSensor*); bool tryAttack(HitSensor*, HitSensor*);
    void exeAttack();
    void exeStandUp() { called = 1; }
    void exeReverseSign() { called = 3; }
    void exeSitDown() { called = 4; }
};
namespace MR {
bool isFirstStep(TwoLegsStateShot*) { return false; }
bool isStep(TwoLegsStateShot*, int) { return false; }
bool isActionEnd(TwoLegs*) { return true; }
}
struct TwoLegsStateDamage {
    Spine* spine = nullptr; int called = 0;
    bool isHide();
    bool isNerve(const Nerve* nerve) { return spine->isCurrentNerve(nerve); }
    void exeDamage() { called = 1; } void exeRunAwayJump() { called = 2; }
    void exeHide() { called = 3; } void exeRunAwayLand() { called = 4; }
};
'''
    code += block(shot, 'namespace NrvTwoLegsStateShot') + '\n'
    code += block(damage, 'namespace NrvTwoLegsStateDamage') + '\n'
    for signature in ['bool TwoLegsStateShot::tryAttackStrong(',
                      'bool TwoLegsStateShot::tryAttack(', 'void TwoLegsStateShot::exeAttack()']:
        code += block(shot, signature) + '\n'
    code += block(damage, 'bool TwoLegsStateDamage::isHide()') + '\nint main() {\n'
    states = ['Start', 'Wait', 'WalkNoDamage', 'WalkDamage', 'Jump', 'Shot',
              'Damage', 'ChangePos', 'Opening', 'Explosion']
    for state in states:
        code += '{ TwoLegs host; for (bool first : {false,true}) for (bool done : {false,true}) {\n'
        code += 'host = TwoLegs(); host.mCamera = &host.camera; host.first = first; host.complete = done;\n'
        code += 'Spine spine(&host, GET_NERVE(TwoLegs, TwoLegsNrv' + state + ')); spine.update();\n'
        code += 'check(spine.mStep == 1);\n'
        code += 'check(host.actions == ' + ('int(first)' if state == 'Start' else '0') + ');\n'
        code += 'check(host.stateUpdates == ' + ('1' if state in ['Wait', 'Explosion'] else '0') + ');\n'
        code += 'check(host.nextUpdates == ' + ('0' if state in ['Start', 'Wait', 'Explosion'] else '1') + ');\n'
        for field, expected in [('selections', state == 'Wait'), ('mFlagC0', state == 'Shot'),
                                ('camera.starts', state == 'Opening'), ('kills', state == 'Explosion')]:
            code += 'check(host.' + field + ' == ' + ('int(done)' if expected else '0') + ');\n'
        code += '} }\n'
    code += r'''
{
    TwoLegsStateShot actor;
    const Nerve* reverse = GET_NERVE(TwoLegsStateShot, TwoLegsStateShotNrvReverseSign);
    const Nerve* sit = GET_NERVE(TwoLegsStateShot, TwoLegsStateShotNrvSitDown);
    for (const Nerve* current : {reverse, sit}) {
        Spine spine(&actor, current); actor.spine = &spine;
        check(actor.tryAttackStrong(nullptr, nullptr) == (current == reverse));
        check(actor.tryAttack(nullptr, nullptr) == (current == reverse));
    }
    Spine spine(&actor, GET_NERVE(TwoLegsStateShot, TwoLegsStateShotNrvAttack)); actor.spine = &spine;
    actor.exeAttack(); spine.update(); check(actor.called == 3);
}
{
    TwoLegsStateDamage actor;
    const Nerve* hide = GET_NERVE(TwoLegsStateDamage, TwoLegsStateDamageNrvHide);
    const Nerve* land = GET_NERVE(TwoLegsStateDamage, TwoLegsStateDamageNrvRunAwayLand);
    for (const Nerve* current : {hide, land}) {
        Spine spine(&actor, current); actor.spine = &spine; spine.update();
        check(actor.isHide() == (actor.called == 4));
    }
}
std::puts("TwoLegs: ten main wrappers and four corrected use sites pass");
}
'''
    return code.replace('#include <cstring>', '#include <cstring>\n#include <initializer_list>')


def main():
    spec = importlib.util.spec_from_file_location('encode', ROOT/'native/tools/encode_legacy_sources.py')
    encoding = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(encoding)
    with tempfile.TemporaryDirectory(prefix='demo-twolegs-native-') as directory:
        work = Path(directory)
        legacy = work/'legacy'
        encoding.prepare(ROOT, legacy)
        flags = ['clang++', '-std=c++17', '-O1', '-DGALAXY_NATIVE=1', '-fno-rtti',
                 '-ffunction-sections', '-fdata-sections', '-Wno-register',
                 '-Wno-inconsistent-missing-override', '-fsanitize=address,undefined',
                 '-fno-sanitize-recover=all', '-fno-omit-frame-pointer']
        for name in ['native/include', 'native/home_menu/include', 'include', 'libs/JSystem/include',
                     'libs/RVL_SDK/include', 'libs/nw4r/include', 'libs/RVLFaceLib/include', 'libs/libms/include']:
            if not name.startswith('native/'):
                flags += ['-I', str(legacy/name)]
            flags += ['-I', str(ROOT/name)]
        objects = []
        for unit in ['Nerve', 'Spine', 'ActorStateKeeper']:
            obj = work/(unit + '.o')
            subprocess.run(flags + ['-c', str(legacy/('src/Game/LiveActor/' + unit + '.cpp')), '-o', str(obj)], check=True)
            objects.append(str(obj))
        code = fixture()
        variants = [('production', code),
                    ('wrong-main-wrapper', code.replace('pActor->selectNextState();', 'pActor->kill();')),
                    ('former-shot-binding', code.replace('isNerve(&NrvTwoLegsStateShot::TwoLegsStateShotNrvReverseSign', 'isNerve(&NrvTwoLegsStateShot::TwoLegsStateShotNrvSitDown')),
                    ('former-damage-binding', code.replace('return isNerve(&NrvTwoLegsStateDamage::TwoLegsStateDamageNrvRunAwayLand', 'return isNerve(&NrvTwoLegsStateDamage::TwoLegsStateDamageNrvHide'))]
        for label, text in variants:
            assert text != code or label == 'production'
            cpp = work/'test.cpp'; cpp.write_text(text); binary = work/'test'
            link = ['-Wl,-dead_strip'] if sys.platform == 'darwin' else ['-Wl,--gc-sections']
            subprocess.run(flags + [str(cpp)] + objects + link + ['-o', str(binary)], check=True)
            result = subprocess.run([str(binary)], capture_output=True, text=True,
                                    env=dict(os.environ, UBSAN_OPTIONS='halt_on_error=1'), timeout=15)
            if label == 'production':
                assert result.returncode == 0, result.stdout + result.stderr
                print(result.stdout.strip())
            else:
                assert result.returncode != 0, label + ' escaped regression'
                print(label + ': rejected')


if __name__ == '__main__':
    main()
