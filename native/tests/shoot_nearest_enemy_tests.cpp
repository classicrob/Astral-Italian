#include <petari/shoot_nearest_enemy.hpp>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>

namespace P = PetariNative::ShootNearestEnemy;
static int checks;
static void check(bool ok, const char* what) {
    ++checks;
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", what); std::exit(1); }
}
static P::Candidate enemy(float x, float y = 0, float z = 0) {
    return {{true, true, true}, true, true, {x, y, z}, 25};
}
static void filters() {
    for (int field = 0; field < 5; ++field) {
        auto c = enemy(20);
        switch (field) {
        case 0: c.actor.alive = false; break;
        case 1: c.actor.unclipped = false; break;
        case 2: c.actor.shown = false; break;
        case 3: c.validSensor = false; break;
        case 4: c.enemy = false; break;
        }
        P::Selection select({0,0,0}); int callbacks = 0;
        auto called = [&] { ++callbacks; return true; };
        check(!select.consider(c, called, called) && callbacks == 0, "invalid actor/sensor skipped before queries");
    }
    for (float value : {std::numeric_limits<float>::quiet_NaN(),
                        std::numeric_limits<float>::infinity(),
                        -std::numeric_limits<float>::infinity()}) {
        for (int coordinate = 0; coordinate < 3; ++coordinate) {
            auto c = enemy(20);
            if (coordinate == 0) c.position.x = value;
            if (coordinate == 1) c.position.y = value;
            if (coordinate == 2) c.position.z = value;
            P::Selection select({0,0,0});
            check(!select.consider(c, [] {return true;}, [] {return true;}), "nonfinite coordinate rejected");
        }
        auto c = enemy(20); c.radius = value; P::Selection select({0,0,0});
        check(!select.consider(c, [] {return true;}, [] {return true;}), "nonfinite radius rejected");
        P::Selection badOrigin({value,0,0});
        check(!badOrigin.consider(enemy(20), [] {return true;}, [] {return true;}), "nonfinite player rejected");
    }
    auto c = enemy(20); c.radius = -1; P::Selection select({0,0,0});
    check(!select.consider(c, [] {return true;}, [] {return true;}), "negative radius rejected");
}
static void nearest() {
    P::Selection select({0,0,0}); int id = -1, queries = 0;
    const auto offer = [&](int index, P::Candidate c, bool visible, bool accepted) {
        if (select.consider(c, [&] {++queries;return visible;}, [&] {++queries;return accepted;})) id = index;
    };
    offer(0, enemy(3000.01f), true, true);
    check(id == -1 && queries == 0, "outside range never queried");
    offer(1, enemy(3000), true, true); check(id == 1, "inclusive range edge");
    offer(2, enemy(100), false, true); check(id == 1, "occluded candidate not selected");
    offer(3, enemy(50), true, false); check(id == 1, "rejected nearest not selected");
    offer(4, enemy(80), true, true); check(id == 4, "next accepting nearest selected");
    offer(5, enemy(-80), true, true); check(id == 4, "exact tie keeps first");
    offer(6, enemy(30,40), true, true); check(id == 6, "3D squared distance chooses closer");
    const int before = queries;
    offer(7, enemy(60), true, true); check(id == 6 && queries == before, "farther candidates do not negotiate");
    P::Selection offset({100,0,0});
    check(offset.consider(enemy(110), []{return true;}, []{return true;}), "distance measured from Mario");
    check(!offset.consider(enemy(0), []{return true;}, []{return true;}), "camera/world-origin distance not used");
    P::Selection huge({-std::numeric_limits<float>::max(),0,0});
    check(!huge.consider(enemy(std::numeric_limits<float>::max()), []{return true;}, []{return true;}), "finite huge coordinates cannot overflow into range");
}
static void sight() {
    const P::Vec3 from{0,0,0}, to{100,0,0};
    check(P::unobstructed(from,to,false,{}), "no map hit visible");
    check(!P::unobstructed(from,to,true,{99,0,0}), "polygon before target blocks");
    check(P::unobstructed(from,to,true,to), "polygon at endpoint follows normal strict test");
    check(P::unobstructed(from,to,true,{101,0,0}), "polygon beyond target does not block");
    const float nan = std::numeric_limits<float>::quiet_NaN();
    check(!P::unobstructed(from,to,true,{nan,0,0}), "nonfinite map result rejected");
    check(!P::safeSegment({nan,0,0},to), "nonfinite camera rejected");
    check(!P::safeSegment(from,{std::numeric_limits<float>::max(),0,0}), "segment safe for normal float calculations");
}
static void transaction() {
    check(!P::normalInputBusy(false,-1,false), "normal no-ammo reset still permits empty feedback");
    check(!P::normalInputBusy(false,0,false), "idle normal shooter permits V");
    check(P::normalInputBusy(false,1,false), "positive charge blocks V");
    check(P::normalInputBusy(false,0,true), "held B takes priority");
    check(P::normalInputBusy(true,0,false), "normal shot suppresses second launch");
    P::Request ready{true,true,0,true,true,false,5};
    int target = 1, find = 0, launch = 0, empty = 0, effects = 0, ammo = 5;
    bool found = true, succeeds = true;
    const auto run = [&](P::Request r) {
        return P::run(r,[&]() -> int* {++find;return found ? &target : nullptr;},
                      [&](int* t) {++launch;check(t == &target,"selected target passed to normal shoot");if (!succeeds) return false;--ammo;return true;},
                      [&] {++empty;},[&] {++effects;});
    };
    check(run({}) == P::Result::Ignored, "default off");
    for (int gate = 0; gate < 6; ++gate) {
        auto r = ready;
        if (gate == 0) r.enabled = false;
        if (gate == 1) r.pressed = false;
        if (gate == 2) r.port = 1;
        if (gate == 3) r.pointerUsable = false;
        if (gate == 4) r.canShoot = false;
        if (gate == 5) r.busy = true;  // normal B shot, held B or positive charge
        const auto result = run(r);
        check(result == (gate < 3 ? P::Result::Ignored : P::Result::Blocked), "normal gates/busy suppress action");
    }
    check(find == 0 && launch == 0 && empty == 0 && effects == 0 && ammo == 5, "off/blocked has no world or ammo/effects");
    auto noAmmo = ready; noAmmo.ammo = 0;
    check(run(noAmmo) == P::Result::NoAmmo && empty == 1 && find == 0 && launch == 0 && effects == 0 && ammo == 5, "no ammo only emits empty feedback");
    found = false;
    check(run(ready) == P::Result::NoTarget && launch == 0 && effects == 0 && ammo == 5, "no target spends/emits nothing");
    found = true; succeeds = false;
    check(run(ready) == P::Result::Failed && launch == 1 && effects == 0 && ammo == 5, "pool/launch failure no spend/effects and no retry");
    succeeds = true;
    check(run(ready) == P::Result::Fired && launch == 2 && effects == 1 && ammo == 4, "one success calls ordinary shoot once; no second deduction");
    bool pending = true; int takes = 0;
    const auto take = [&] {++takes;const bool result = pending;pending = false;return result;};
    check(!P::takePress(1,take) && pending && takes == 0, "2P cannot consume press");
    check(P::takePress(0,take) && takes == 1 && !pending, "1P consumes once");
    check(!P::takePress(0,take) && takes == 2, "no repeat on later control");
}
int main() {
    filters(); nearest(); sight(); transaction();
    std::printf("ShootEnemy policy: %d checks passed\n", checks);
}
