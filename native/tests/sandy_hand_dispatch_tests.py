#!/usr/bin/env python3
"""Exercise production SandyHand attack handlers through the native virtual interface.

The declarations, two handler bodies and message predicates come from production
files. Owner/scale services are isolated fixtures; this does not host a boss scene.
The former binding is compiled too and must fail the same behavior check.
"""
import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def block(source, signature):
    start = source.index(signature)
    end = source.index('{', start) + 1
    depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


def fixture():
    source = (ROOT / 'src/Game/Boss/SandyHand.cpp').read_text()
    base = (ROOT / 'include/Game/LiveActor/LiveActor.hpp').read_text()
    actor = block((ROOT / 'include/Game/Boss/Sandy.hpp').read_text(), 'class SandyHand :')
    sensor = (ROOT / 'src/Game/Util/ActorSensorUtil.cpp').read_text()
    methods = ('receiveMsgPlayerAttack', 'receiveMsgEnemyAttack')
    base_decls, actor_decls = [], []
    for method in methods:
        line = next(line.strip() for line in base.splitlines() if 'virtual bool ' + method + '(' in line)
        if line.endswith(';'):
            line = line[:-1] + ' { return false; }'
        base_decls.append(line)
        actor_decls.append(next(line.strip() for line in actor.splitlines() if 'virtual bool ' + method + '(' in line))
    fields = [next(line.strip() for line in actor.splitlines() if re.search(r'\* ' + name + r';', line))
              for name in ('mSandy', 'mScaleController')]
    predicates = '\n'.join(block(sensor, 'bool ' + name + '(u32 msg)')
                           for name in ('isMsgExplosionAttack', 'isMsgStarPieceReflect', 'isMsgFireBallAttack'))
    bodies = '\n'.join(block(source, 'bool SandyHand::' + method + '(') for method in methods)
    prefix = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
using u32 = std::uint32_t;
struct HitSensor {};
struct Sandy {
    int blurCalls = 0;
    bool acceptBlur = true;
    bool tryStartBlur() { ++blurCalls; return acceptBlur; }
};
struct AnimScaleController {
    int hits = 0;
    void startHitReaction() { ++hits; }
};
void require(bool ok, const char* message) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
'''
    prefix += '\nnamespace MR {\n' + predicates + '\n}\n'
    prefix += 'struct LiveActor { virtual ~LiveActor() = default;\n' + '\n'.join(base_decls) + '\n};\n'
    prefix += 'struct SandyHand : LiveActor {\n' + '\n'.join(actor_decls + fields)
    prefix += '\nSandyHand(Sandy& owner, AnimScaleController& scale) : mSandy(&owner), mScaleController(&scale) {}\n};\n'
    test = r'''
__attribute__((noinline)) bool player(LiveActor* actor, u32 msg) {
    return actor->receiveMsgPlayerAttack(msg, nullptr, nullptr);
}
__attribute__((noinline)) bool enemy(LiveActor* actor, u32 msg) {
    return actor->receiveMsgEnemyAttack(msg, nullptr, nullptr);
}
int main() {
    Sandy owner; AnimScaleController scale; SandyHand hand(owner, scale);
    LiveActor* actor = &hand;
    require(player(actor, 0x14), "player star reflection selects hit reaction");
    require(player(actor, 0xB), "player fireball selects hit reaction");
    require(scale.hits == 2 && owner.blurCalls == 0, "player attacks only affect scale");
    require(!player(actor, 0x64), "player channel rejects enemy explosion");
    require(!player(actor, 0xFFFFFFFF), "player channel rejects unrelated message");
    require(scale.hits == 2 && owner.blurCalls == 0, "rejected player messages have no effects");
    require(!enemy(actor, 0x14) && !enemy(actor, 0xB), "enemy channel rejects player attacks");
    require(!enemy(actor, 0xFFFFFFFF), "enemy channel rejects unrelated message");
    require(scale.hits == 2 && owner.blurCalls == 0, "rejected enemy messages have no effects");
    for (bool accept : {false, true}) {
        owner.acceptBlur = accept;
        int before = owner.blurCalls;
        require(enemy(actor, 0x64) == accept, "enemy explosion forwards owner result");
        require(owner.blurCalls == before + 1 && scale.hits == 2, "enemy explosion only invokes owner blur");
    }
    std::puts("SandyHand: production player/enemy virtual dispatch passed");
}
'''
    prefix = '#include <initializer_list>\n' + prefix
    player_name = 'SandyHand::receiveMsgPlayerAttack('
    enemy_name = 'SandyHand::receiveMsgEnemyAttack('
    assert bodies.count(player_name) == bodies.count(enemy_name) == 1
    former = bodies.replace(player_name, 'SandyHand::swapDispatch(').replace(enemy_name, player_name).replace('SandyHand::swapDispatch(', enemy_name)
    return prefix + bodies + test, prefix + former + test


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='clang++')
    args = parser.parse_args()
    production, former = fixture()
    flags = [args.compiler, '-std=c++17', '-O1', '-g', '-fsanitize=address,undefined',
             '-fno-sanitize-recover=all', '-fno-omit-frame-pointer']
    with tempfile.TemporaryDirectory(prefix='sandy-hand-dispatch-') as directory:
        work = Path(directory)
        for name, code in [('production', production), ('former-binding', former)]:
            cpp, binary = work / (name + '.cpp'), work / name
            cpp.write_text(code)
            subprocess.run(flags + [str(cpp), '-o', str(binary)], check=True)
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15,
                                    env=dict(os.environ, UBSAN_OPTIONS='halt_on_error=1'))
            if name == 'production':
                if result.returncode:
                    raise RuntimeError(result.stdout + result.stderr)
                print(result.stdout.strip())
            else:
                assert result.returncode == 1 and 'FAIL: player star reflection selects hit reaction' in result.stderr, result.stderr
                print('SandyHand: former-binding control rejected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
