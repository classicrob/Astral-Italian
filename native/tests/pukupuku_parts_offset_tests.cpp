// Typed-storage test only. Lifecycle/virtual methods below are inert test fixtures;
// the class declarations and both tested write statements come from production.
#include "Game/LiveActor/PartsModel.hpp"
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

JMapLinkInfo::JMapLinkInfo() : mFollowID(-1), mZoneID(-1), mContext(-1) {}
LiveActorFlag::LiveActorFlag() : mIsDead(false), mIsNotReleasedAnimFrame(false), mIsOnCalcAnim(false),
    mIsNoCalcView(false), mIsNoEntryDrawBuffer(false), mIsOnBind(false), mIsCalcGravity(false),
    mIsClipped(false), mIsClippingInvalid(false) {}
NameObj::NameObj(const char* name) : mName(name), mFlags(0), mExecutorIdx(-1), mLinkInfo() {}
NameObj::~NameObj() {}
#define EMPTY0(cls, name) void cls::name() {}
void NameObj::init(const JMapInfoIter&) {}
EMPTY0(NameObj, initAfterPlacement)
EMPTY0(NameObj, movement)
void NameObj::draw() const {}
EMPTY0(NameObj, calcAnim)
EMPTY0(NameObj, calcViewAndEntry)
EMPTY0(NameObj, startMovement)
EMPTY0(NameObj, endMovement)
LiveActor::LiveActor(const char* name) : NameObj(name), mPosition(0.0f), mRotation(0.0f), mScale(1.0f),
    mVelocity(0.0f), mGravity(0.0f), mActionKeeper(), mModelManager(), mSpine(), mSensorKeeper(), mBinder(),
    mRailRider(), mEffectKeeper(), mSoundHolder(), mFlags(), mShadowControllerList(), mCollisionParts(),
    mStageSwitchCtrl(), mStarPointerTarget(), mActorLightCtrl() {}
void LiveActor::init(const JMapInfoIter&) {}
EMPTY0(LiveActor, movement)
EMPTY0(LiveActor, calcAnim)
EMPTY0(LiveActor, calcViewAndEntry)
EMPTY0(LiveActor, appear)
EMPTY0(LiveActor, makeActorAppeared)
EMPTY0(LiveActor, kill)
EMPTY0(LiveActor, makeActorDead)
bool LiveActor::receiveMessage(u32, HitSensor*, HitSensor*) { return false; }
MtxPtr LiveActor::getBaseMtx() const { return nullptr; }
MtxPtr LiveActor::getTakingMtx() const { return nullptr; }
EMPTY0(LiveActor, startClipped)
EMPTY0(LiveActor, endClipped)
EMPTY0(LiveActor, calcAndSetBaseMtx)
void LiveActor::updateHitSensor(HitSensor*) {}
void LiveActor::attackSensor(HitSensor*, HitSensor*) {}
bool LiveActor::receiveMsgPush(HitSensor*, HitSensor*) { return false; }
bool LiveActor::receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*) { return false; }
PartsModel::PartsModel(LiveActor* host, const char* name, const char*, MtxPtr matrix, int, bool)
    : LiveActor(name), mHost(host), mFixedPos(), mMtx(matrix), mCalcOwnMatrix(true), mCheckHiddenHost(false), mIsDead(false) {}
void PartsModel::init(const JMapInfoIter&) {}
EMPTY0(PartsModel, movement)
EMPTY0(PartsModel, calcAnim)
EMPTY0(PartsModel, calcViewAndEntry)
EMPTY0(PartsModel, makeActorAppeared)
EMPTY0(PartsModel, makeActorDead)
EMPTY0(PartsModel, calcAndSetBaseMtx)

static unsigned checks;
static void require(bool pass, const char* reason) {
    ++checks;
    if (!pass) { std::fprintf(stderr, "FAIL: %s\n", reason); std::exit(1); }
}
static void appearWrite(PartsModel* mPartsModel) {
    // __APPEAR_WRITE__
}
static void deadWrite(PartsModel* mPartsModel) {
    // __DEAD_WRITE__
}
int main() {
    static_assert(sizeof(void*) == 8, "This regression exercises the 64-bit native layout");
    static_assert(offsetof(PartsModel, mCheckHiddenHost) == 233, "Measured canonical layout changed; rebind the test");
    static_assert(offsetof(LiveActor, mFlags) + offsetof(LiveActorFlag, mIsOnBind) == 157,
                  "Old byte index must demonstrably address the binder flag");
    Mtx matrix = {};
    for (bool hidden : {false, true}) for (bool bind : {false, true}) for (bool value : {true, false}) {
        struct Guarded {
            std::array<unsigned char, 16> front;
            PartsModel model;
            std::array<unsigned char, 16> back;
            Guarded(MtxPtr m) : front{}, model(nullptr, "test", nullptr, m, 0, false), back{} {}
        } storage(matrix);
        PartsModel& model = storage.model;
        storage.front.fill(0xA5); storage.back.fill(0x5A);
        model.mCheckHiddenHost = hidden; model.mFlags.mIsOnBind = bind;
        std::array<unsigned char, sizeof(PartsModel)> before;
        std::memcpy(before.data(), &model, before.size());
        if (value) appearWrite(&model); else deadWrite(&model);
        require(model.mFlags.mIsOnBind == bind, "old index changes unrelated bind flag");
        require(model.mCheckHiddenHost == value, "hidden-host flag has requested normal bool value");
        const auto* after = reinterpret_cast<const unsigned char*>(&model);
        for (std::size_t i = 0; i < before.size(); ++i) {
            if (i != offsetof(PartsModel, mCheckHiddenHost))
                require(before[i] == after[i], "write changed another object-representation byte");
        }
        require(model.mMtx == matrix, "matrix pointer retained");
        for (auto c : storage.front) require(c == 0xA5, "front guard retained");
        for (auto c : storage.back) require(c == 0x5A, "back guard retained");
        if (value) appearWrite(&model); else deadWrite(&model);
        require(model.mCheckHiddenHost == value && model.mFlags.mIsOnBind == bind, "repeated write is idempotent");
    }
    std::printf("PASS: %u canonical typed-field checks\n", checks);
}
