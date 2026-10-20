// U023: generated production control/init/getSensor definitions are inserted at
// the marker below. Sensor/keeper/info types and implementations are canonical.
// The actor fixture supplies joint placement, facing/chance and rumbler seams;
// it does not claim to instantiate a complete initialized game actor.
#include "Game/Enemy/HammerHeadPackun.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/HitSensorInfo.hpp"
#include "Game/LiveActor/HitSensorKeeper.hpp"
#include "Game/Util/NativeActorObserve.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

namespace {
int checks, published;
bool observing;
TVec3f capturedPosition, capturedDirection;
u32 capturedFlags;
int capturedState;
void check(bool value, const char* message) {
    ++checks;
    if (!value) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
struct RumblerProbe { int updates = 0; void update() { ++updates; } };
}
extern "C" int petari_ui_observing() { return observing; }
extern "C" void petari_actor(const char* kind, float x, float y, float z,
                             float dx, float dy, float dz, int state, unsigned flags) {
    check(std::strcmp(kind, "HammerHead") == 0, "actor kind");
    ++published; capturedPosition.set(x, y, z); capturedDirection.set(dx, dy, dz);
    capturedState = state; capturedFlags = flags;
}
// Only collision-group registration is external to this lookup-focused test.
namespace MR { void initHitSensorGroup(HitSensor*) {} }

class HammerHeadFixture {
public:
    HitSensorKeeper keeper{6};
    HitSensorKeeper* mSensorKeeper = &keeper;
    RumblerProbe rumbler;
    RumblerProbe* mJointRumbler = &rumbler;
    TVec3f front{0.25f, -0.5f, 0.75f};
    bool ready = false;
    std::vector<std::unique_ptr<HitSensor>> sensors;
    std::vector<std::unique_ptr<HitSensorInfo>> infos;
    std::vector<const char*> joints;
    void initSensor();
    void control();
    HitSensor* getSensor(const char*) const;
    bool isChance() const { return ready; }
    void initHitSensor(int count) { check(count == 6, "six sensors, not the one-sensor shortcut"); }
    ~HammerHeadFixture() { delete[] keeper.mSensorInfo; }
};
namespace MR {
void calcFrontVec(TVec3f* result, const HammerHeadFixture* actor) { *result = actor->front; }
HitSensor* addHitSensorAtJointMapObj(HammerHeadFixture* actor, const char* name,
                                    const char* joint, u16 receivers, f32 radius, const TVec3f& offset) {
    // Real constructors, hash and multi-entry lookup; the registrar only wires
    // fixture-owned entries in place of scene collision/joint registration.
    auto sensor = std::make_unique<HitSensor>(0x40, 0, radius, nullptr);
    auto info = std::make_unique<HitSensorInfo>(name, sensor.get(), nullptr, nullptr, offset, false);
    actor->joints.push_back(joint);
    actor->keeper.mSensorInfo[actor->keeper.mSensorCount++] = info.get();
    HitSensor* result = sensor.get();
    actor->sensors.push_back(std::move(sensor)); actor->infos.push_back(std::move(info));
    return result;
}
}

// __GAME_FUNCTIONS__

int main() {
    static_assert(sizeof(void*) == 8, "this regression checks native64 canonical layout");
    static_assert(sizeof(u32) == 4 && sizeof(f32) == 4, "game scalar widths");
    static_assert(offsetof(HitSensor, mPosition) == 4, "canonical position field");
    static_assert(offsetof(HitSensorInfo, mNameHash) == 8 && offsetof(HitSensorInfo, mSensor) == 16,
                  "native pointers move sensor-info fields; no Wii offset casts");
    std::printf("Native64 layout: LiveActor keeper=%zu HammerHead rumbler=%zu HitSensorInfo sensor=%zu\n",
                offsetof(LiveActor, mSensorKeeper), offsetof(HammerHeadPackun, mJointRumbler),
                offsetof(HitSensorInfo, mSensor));
    HammerHeadFixture actor;
    actor.initSensor();
    check(actor.keeper.mSensorCount == 6, "complete production registration");
    check(actor.getSensor("body") == nullptr, "lowercase lookup genuinely returns null");
    check(actor.getSensor("Body") == actor.sensors[0].get(), "canonical Body lookup");
    check(actor.getSensor("blow") == actor.sensors[1].get(), "other sensor distinct");
    check(std::strcmp(actor.joints[0], "Head") == 0, "Body attached to Head joint");
    check(actor.infos[0]->_C.x == 120.0f && actor.infos[0]->_C.y == 0 && actor.infos[0]->_C.z == 0,
          "production head offset retained");
    actor.sensors[0]->mPosition.set(123.0f, -456.0f, 789.0f);
    // Previously the null access occurred even when publishActor would return
    // early because observation was off. Run that case first for the control.
    observing = false; actor.control();
    check(published == 0 && actor.rumbler.updates == 1, "observation off still updates vanilla rumbler");
    observing = true; actor.control();
    check(published == 1 && actor.rumbler.updates == 2, "one publication and one rumbler update");
    check(capturedPosition.x == 123 && capturedPosition.y == -456 && capturedPosition.z == 789,
          "publishes actual animated sensor position, not zero or actor origin");
    check(capturedDirection.x == actor.front.x && capturedDirection.y == actor.front.y &&
          capturedDirection.z == actor.front.z, "front direction retained");
    check(capturedState == 0 && capturedFlags == PETARI_ACTOR_HOSTILE, "not-ready flags retained");
    actor.ready = true; actor.sensors[0]->mPosition.set(-19, 28, 37); actor.control();
    check(published == 2 && actor.rumbler.updates == 3, "next frame follows moved sensor once");
    check(capturedPosition.x == -19 && capturedPosition.y == 28 && capturedPosition.z == 37,
          "changed head position retained");
    check(capturedFlags == (PETARI_ACTOR_READY | PETARI_ACTOR_HOSTILE), "chance flags retained");
    std::printf("%d HammerHead observer checks passed\n", checks);
}
