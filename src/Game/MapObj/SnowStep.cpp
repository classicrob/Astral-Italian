#include "Game/MapObj/SnowStep.hpp"
#include <revolution/mtx.h>
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

extern "C" void fn_80034910(LiveActor*);

namespace NrvSnowStep {
    NEW_NERVE(SnowStepNrvExtend, SnowStep, Extend);
    NEW_NERVE(SnowStepNrvWait, SnowStep, Wait);
    NEW_NERVE(SnowStepNrvShrink, SnowStep, Shrink);
};  // namespace NrvSnowStep

SnowStep::SnowStep(const char* pName) : LiveActor(pName) {
    mQuat.x = 0.0f;
    mQuat.y = 0.0f;
    mQuat.z = 0.0f;
    mQuat.w = 1.0f;
    mExtendStep = 10;
    mWaitStep = 60;
    mShrinkStep = 60;
    mSize = 100.0f;
    mBaseScale = 1.0f;
    mBaseMtx.identity();
}

void SnowStep::init(const JMapInfoIter& rIter) {
    MR::initActor(this, "SnowStep", false);
    MR::initCollisionParts(this, "SnowStep", getSensor(nullptr), mBaseMtx);
    initNerve(&NrvSnowStep::SnowStepNrvWait::sInstance, 0);
    MR::invalidateClipping(this);
    makeActorDead();
}

void SnowStep::appearSnowStep(const TVec3f& rPos, const TVec3f& rUp, s32 extendStep, s32 waitStep, s32 shrinkStep, f32 size) {
    mSize = size;
    mExtendStep = extendStep;
    mWaitStep = waitStep;
    mShrinkStep = shrinkStep;
    setNerve(&NrvSnowStep::SnowStepNrvExtend::sInstance);
    TVec3f offset(rUp);
    offset *= 20.0f;
    TVec3f pos(rPos);
    pos += offset;
    MR::setPosition(this, pos);
    MR::setScale(this, 0.01f);
    MR::makeQuatUpNoSupport(&mQuat, rUp);
    appear();
}

void SnowStep::control() {
}

void SnowStep::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
    mBaseMtx = *reinterpret_cast< const TPos3f* >(getBaseMtx());
    MR::preScaleMtx(mBaseMtx, mBaseScale);
}

void SnowStep::exeExtend() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Extend");
        MR::startActionSound(this, "Extend", -1, -1, -1);
        mBaseScale = mSize / 100.0f;
    }
    f32 scale = MR::calcNerveEaseOutValue(this, mExtendStep, 1.0f, mSize);
    MR::setScale(this, scale / 100.0f);
    if (MR::isGreaterStep(this, mExtendStep)) {
        setNerve(&NrvSnowStep::SnowStepNrvWait::sInstance);
    }
}

void SnowStep::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::setScale(this, mSize / 100.0f);
        mBaseScale = mSize / 100.0f;
    }
    if (MR::isGreaterStep(this, mWaitStep)) {
        setNerve(&NrvSnowStep::SnowStepNrvShrink::sInstance);
    }
}

void SnowStep::exeShrink() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shrink");
    }
    f32 scale = MR::calcNerveEaseOutValue(this, mShrinkStep, mSize, 1.0f);
    MR::setScale(this, scale / 100.0f);
    mBaseScale = scale / 100.0f;
    if (MR::isGreaterStep(this, mShrinkStep)) {
        fn_80034910(this);
        MR::startActionSound(this, "Shrink", -1, -1, -1);
        kill();
    }
}

extern "C" s32 fn_80304510() {
    return 0x52;
}

SnowStep::~SnowStep() {
}

SnowStepHolder::SnowStepHolder() : LiveActorGroup("\x90\xE1\x91\xAB\x8F\xEA\x8A\xC7\x97\x9D", 0x100) {
    for (s32 i = 0; i < 0x100; i++) {
        SnowStep* pActor = new SnowStep("\x90\xE1\x91\xAB\x8F\xEA");
        pActor->initWithoutIter();
        registerActor(pActor);
    }
}

SnowStepHolder::~SnowStepHolder() {
}

namespace MR {
    void createSnowStepHolder() {
        createSceneObj(SceneObj_SnowStepHolder);
    }

    bool tryAppearSnowStep(const TVec3f& rPos, const TVec3f& rUp, s32 extendStep, s32 waitStep, s32 shrinkStep, f32 size) {
        LiveActorGroup* pGroup = static_cast< LiveActorGroup* >(getSceneObjHolder()->getObj(SceneObj_SnowStepHolder));
        LiveActor* pActor = nullptr;
        if (pGroup->getDeadActor() != nullptr) {
            pActor = pGroup->getDeadActor();
        }
        if (pActor == nullptr) {
            return false;
        }
        static_cast< SnowStep* >(pActor)->appearSnowStep(rPos, rUp, extendStep, waitStep, shrinkStep, size);
        return true;
    }
};  // namespace MR
