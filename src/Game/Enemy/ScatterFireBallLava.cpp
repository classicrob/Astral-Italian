#define COLOR8_GX_CTOR_INLINE
#include "Game/Enemy/ScatterFireBallLava.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/ClipAreaDrop.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/LightUtil.hpp"

namespace {
    const char* sShapeName = "VolumeSphere";
    const s32 cBaseSize = 400;
    extern const f32 cMargin;
}

#define ADDRESS_NERVE(name, actorType, method) \
    class name : public Nerve { \
    public: \
        virtual void execute(Spine* spine) const { static_cast<actorType*>(spine->mExecutor)->method(); } \
        static name sInstance; \
    }; \
    name name::sInstance ATTRIBUTE_WEAK;

namespace NrvScatterFireBallLava {
    NEW_NERVE(ScatterFireBallLavaNrvWait, ScatterFireBallLava, Wait);
    ADDRESS_NERVE(ScatterFireBallLavaNrvAppear, ScatterFireBallLava, exeLavaAppear);
    NEW_NERVE(ScatterFireBallLavaNrvHold, ScatterFireBallLava, Hold);
    ADDRESS_NERVE(ScatterFireBallLavaNrvShrink, ScatterFireBallLava, exeLavaShrink);
}

ScatterFireBallLava::ScatterFireBallLava(LiveActor* host, const char* name) : LiveActor(name), mHost(host), mClipArea(nullptr), mRadius(0.0f) {}

void ScatterFireBallLava::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "FireBallSpreadLava", false);
    initNerve(GET_NERVE(ScatterFireBallLava, ScatterFireBallLavaNrvWait), 0);
    mClipArea = new ClipAreaDrop("クリップエリアのしずく", sShapeName);
    mClipArea->initWithoutIter();
    MR::invalidateClipping(this);
    makeActorDead();
    MR::hideModel(this);
}

void ScatterFireBallLava::control() {
    static const GXColor light = {255, 192, 0, 255};
    MR::requestPointLight(this, mPosition, light, 1.0f, -1, -1.0f);
}

void ScatterFireBallLava::kill() {
    if (MR::isEffectValid(this, "Land")) MR::deleteEffect(this, "Land");
    mClipArea->kill();
    LiveActor::kill();
}

// NONMATCHING: player-position and up-vector pointers use exchanged saved registers.
void ScatterFireBallLava::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (MR::isSensorPlayer(receiver) && MR::isOnGroundPlayer()) {
        MR::calcGravity(this);
        const TVec3f* playerPos;
        TVec3f up = -mGravity;
        f32 along;
        f32 across;
        playerPos = MR::getPlayerPos();
        MR::func_8003DEF0(&along, &across, MR::getSensorPos(sender), playerPos, &up);
        if (across < 10.0f && along < mRadius - cMargin) MR::sendMsgEnemyAttackFire(receiver, sender);
    }
}

void ScatterFireBallLava::landAt(const TVec3f& pos) {
    mPosition = pos;
    mRotation.x = 0.0f;
    mRotation.y = 0.0f;
    mRotation.z = 0.0f;
    setNerve(GET_NERVE(ScatterFireBallLava, ScatterFireBallLavaNrvAppear));
    makeActorAppeared();
    MR::tryHitReaction(this, "着弾");
    ClipAreaDrop* clip = mClipArea;
    MR::setPosition(clip, mPosition);
    s32 size = cBaseSize;
    clip->setBaseSize(size);
    clip->setExtendsEndTime(60);
    clip->setShrinkTime(180, 240);
    clip->appear();
}

void ScatterFireBallLava::exeLavaAppear() {
    s32 size = cBaseSize;
    mRadius = size * MR::calcNerveRate(this, 60);
    if (MR::isGreaterEqualStep(this, 60)) setNerve(GET_NERVE(ScatterFireBallLava, ScatterFireBallLavaNrvHold));
}

void ScatterFireBallLava::exeLavaShrink() {
    if (MR::isFirstStep(this)) MR::startActionSound(this, "Shrink", -1, -1, -1);
    s32 size = cBaseSize;
    mRadius = size * (1.0f - MR::calcNerveRate(this, 60));
    if (MR::isGreaterEqualStep(this, 60)) {
        setNerve(GET_NERVE(ScatterFireBallLava, ScatterFireBallLavaNrvWait));
        MR::tryHitReaction(this, "消滅");
        kill();
    }
}

ScatterFireBallLava::~ScatterFireBallLava() {}

void ScatterFireBallLava::exeHold() {
    if (MR::isGreaterEqualStep(this, 120)) setNerve(GET_NERVE(ScatterFireBallLava, ScatterFireBallLavaNrvShrink));
}

void ScatterFireBallLava::exeWait() {}

namespace { const f32 cMargin = 10.0f; }
