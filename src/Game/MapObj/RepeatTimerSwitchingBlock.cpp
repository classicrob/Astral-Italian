#include "Game/RhythmLib/AudRhythmTiming.hpp"
#include "Game/MapObj/RepeatTimerSwitchingBlock.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"


// zero-valued .sbss2 constants (loaded via sda21, not folded)
const volatile f32 sZeroShift = 0.0f;
const volatile f32 sZeroShiftQuick = 0.0f;

namespace NrvRepeatTimerSwitchingBlock {
    class RepeatTimerSwitchingBlockNrvChange : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static RepeatTimerSwitchingBlockNrvChange sInstance;
    };

    class RepeatTimerSwitchingBlockNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static RepeatTimerSwitchingBlockNrvWait sInstance;
    };

    class RepeatTimerSwitchingBlockNrvSwitchOn : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static RepeatTimerSwitchingBlockNrvSwitchOn sInstance;
    };

    class RepeatTimerSwitchingBlockNrvSwitchOff : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static RepeatTimerSwitchingBlockNrvSwitchOff sInstance;
    };
};  // namespace NrvRepeatTimerSwitchingBlock

RepeatTimerSwitchingBlock::RepeatTimerSwitchingBlock(const char* pName) : LiveActor(pName) {
    mTimer = 100;
    mWaitStep = 8.0f;
    mWaitRandom = 4.0f;
    mShiftFrame = sZeroShift;
    mIsActing = false;
    mIsInverse = false;
    mIsQuick = false;
    mFrameCount = 0;
    mIsDelayWait = false;
}

void RepeatTimerSwitchingBlock::init(const JMapInfoIter& rIter) {
    char name[0x100];
    MR::getMapPartsObjectNameIfExistShapeID(name, 0x100, rIter);
    MR::initActor(this, rIter, name, false);
    MR::connectToSceneMapObj(this);
    MR::initDefaultPos(this, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mIsInverse);
    MR::getJMapInfoArg1NoInit(rIter, &mIsQuick);
    if (mIsQuick) {
        mShiftFrame = sZeroShiftQuick;
        mWaitStep = 2.0f;
        mWaitRandom = 1.8f;
    }
    initEffectKeeper(0, nullptr, false);
    initSound(4, nullptr, nullptr, TVec3f(0.0f));
    MR::initMapPartsClipping(this, rIter, nullptr, false);
    initHitSensor(1);
    MR::addBodyMessageSensorMapObj(this);
    MR::initCollisionParts(this, name, getSensor(nullptr), nullptr);
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(RepeatTimerSwitchingBlock, RepeatTimerSwitchingBlockNrvWait), 0);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::needStageSwitchReadB(this, rIter);
    makeActorAppeared();
}

void RepeatTimerSwitchingBlock::control() {
    if (mIsDelayWait && !MR::isStageStateScenarioOpeningCamera()) {
        setNerve(GET_NERVE(RepeatTimerSwitchingBlock, RepeatTimerSwitchingBlockNrvWait));
        mIsDelayWait = false;
    }
}

void RepeatTimerSwitchingBlock::exeChange() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "ChangeOn", -1, -1, -1);
    }
}

void RepeatTimerSwitchingBlock::exeWait() {
    if (MR::isFirstStep(this)) {
        if (mIsInverse) {
            MR::invalidateCollisionParts(this);
            MR::startAction(this, "SwitchOffSign");
            MR::setAllAnimFrameAtEnd(this, "SwitchOffSign");
            setNerve(GET_NERVE(RepeatTimerSwitchingBlock, RepeatTimerSwitchingBlockNrvSwitchOn));
            if (MR::isValidSwitchA(this)) {
                MR::onSwitchA(this);
            }
        } else {
            MR::validateCollisionParts(this);
            MR::startAction(this, "SwitchOnSign");
            MR::setAllAnimFrameAtEnd(this, "SwitchOnSign");
            setNerve(GET_NERVE(RepeatTimerSwitchingBlock, RepeatTimerSwitchingBlockNrvSwitchOff));
            if (MR::isValidSwitchA(this)) {
                MR::offSwitchA(this);
            }
        }
        if (MR::isStageStateScenarioOpeningCamera()) {
            mIsDelayWait = true;
        }
        mTimer = (s32)(mShiftFrame - (f32)(s32)MR::getBrkFrameMax(this));
    }
}

// NONMATCHING: string-pool offsets (target pool starts with "ChangeOn" at lbl+0; ours orders literals differently, addi rX,rX,off differs)
void RepeatTimerSwitchingBlock::exeSwitchOn() {
    if (MR::isFirstStep(this)) {
        mIsActing = false;
        mFrameCount = 0;
    }
    if (isSwitchTimingReady()) {
        mIsActing = true;
        if (mIsQuick) {
            MR::startAction(this, "SwitchOnSignQuick");
        } else {
            MR::startAction(this, "SwitchOnSign");
        }
        mFrameCount = 1;
    }
    if (mFrameCount > 0) {
        mFrameCount = mFrameCount + 1;
    }
    if (mIsQuick) {
        if (mFrameCount == 6) {
            MR::startActionSound(this, "SignOn", -1, -1, -1);
        }
    } else {
        if (mFrameCount == 0x20) {
            MR::startActionSound(this, "SignOn", -1, -1, -1);
        }
        if (mFrameCount == 0x3E) {
            MR::startActionSound(this, "SignOn", -1, -1, -1);
        }
        if (mFrameCount == 0x5C) {
            MR::startActionSound(this, "SignOn", -1, -1, -1);
        }
        if (mFrameCount == 0x75) {
            MR::startActionSound(this, "ChangeOn", -1, -1, -1);
        }
    }
    bool isDone = false;
    if (mIsActing) {
        if (MR::isActionEnd(this)) {
            MR::validateCollisionParts(this);
            if (mIsQuick) {
                MR::startAction(this, "SwitchOnSignQuick");
            } else {
                MR::startAction(this, "SwitchOnSign");
            }
            MR::setAllAnimFrameAtEnd(this, "SwitchOnSign");
            setNerve(GET_NERVE(RepeatTimerSwitchingBlock, RepeatTimerSwitchingBlockNrvSwitchOff));
            if (MR::isValidSwitchA(this)) {
                MR::offSwitchA(this);
            }
            isDone = true;
        }
    }
    if (isSwitchBOn()) {
        if (mIsActing) {
            if (MR::isGreaterStep(this, mTimer)) {
                if (mIsQuick) {
                    MR::setAllAnimFrameAndStop(this, "SwitchOnSignQuick", 0.0f);
                } else {
                    MR::setAllAnimFrameAndStop(this, "SwitchOnSign", 0.0f);
                }
            }
        }
        if (isDone) {
            MR::invalidateCollisionParts(this);
        }
        setNerve(GET_NERVE(RepeatTimerSwitchingBlock, RepeatTimerSwitchingBlockNrvChange));
    }
}

// NONMATCHING: string-pool offsets (target pool starts with "ChangeOn" at lbl+0; ours orders literals differently, addi rX,rX,off differs)
void RepeatTimerSwitchingBlock::exeSwitchOff() {
    if (MR::isFirstStep(this)) {
        mIsActing = false;
        mFrameCount = 0;
    }
    if (isSwitchTimingReady()) {
        mIsActing = true;
        if (mIsQuick) {
            MR::startAction(this, "SwitchOffSignQuick");
        } else {
            MR::startAction(this, "SwitchOffSign");
        }
        mFrameCount = 1;
    }
    if (mFrameCount > 0) {
        mFrameCount = mFrameCount + 1;
    }
    if (mIsQuick) {
        if (mFrameCount == 6) {
            MR::startActionSound(this, "SignOff", -1, -1, -1);
        }
    } else {
        if (mFrameCount == 0x20) {
            MR::startActionSound(this, "SignOff", -1, -1, -1);
        }
        if (mFrameCount == 0x3E) {
            MR::startActionSound(this, "SignOff", -1, -1, -1);
        }
        if (mFrameCount == 0x5C) {
            MR::startActionSound(this, "SignOff", -1, -1, -1);
        }
    }
    bool isDone = false;
    if (mIsActing) {
        if (MR::isActionEnd(this)) {
            MR::invalidateCollisionParts(this);
            MR::startAction(this, "SwitchOffSign");
            if (mIsQuick) {
                MR::setAllAnimFrameAtEnd(this, "SwitchOffSignQuick");
            } else {
                MR::setAllAnimFrameAtEnd(this, "SwitchOffSign");
            }
            setNerve(GET_NERVE(RepeatTimerSwitchingBlock, RepeatTimerSwitchingBlockNrvSwitchOn));
            if (MR::isValidSwitchA(this)) {
                MR::onSwitchA(this);
            }
            isDone = true;
        }
    }
    if (isSwitchBOn()) {
        if (mIsActing) {
            if (MR::isGreaterStep(this, mTimer)) {
                if (mIsQuick) {
                    MR::setAllAnimFrameAndStop(this, "SwitchOffSignQuick", 0.0f);
                } else {
                    MR::setAllAnimFrameAndStop(this, "SwitchOffSign", 0.0f);
                }
            }
        }
        if (isDone) {
            MR::validateCollisionParts(this);
        }
        setNerve(GET_NERVE(RepeatTimerSwitchingBlock, RepeatTimerSwitchingBlockNrvChange));
    }
}

bool RepeatTimerSwitchingBlock::isSwitchBOn() {
    if (MR::isValidSwitchB(this)) {
        return MR::isOnSwitchB(this);
    }
    return false;
}

s32 RepeatTimerSwitchingBlock::isSwitchTimingReady() {
    if (MR::isGreaterEqualStep(this, mTimer)) {
        return AudRhythmWrap::isBeatTiming((s32)mWaitStep, mWaitRandom);
    }
    return 0;
}

RepeatTimerSwitchingBlock::~RepeatTimerSwitchingBlock() {
}

namespace NrvRepeatTimerSwitchingBlock {
    RepeatTimerSwitchingBlockNrvChange RepeatTimerSwitchingBlockNrvChange::sInstance ATTRIBUTE_WEAK;
    RepeatTimerSwitchingBlockNrvWait RepeatTimerSwitchingBlockNrvWait::sInstance ATTRIBUTE_WEAK;
    RepeatTimerSwitchingBlockNrvSwitchOn RepeatTimerSwitchingBlockNrvSwitchOn::sInstance ATTRIBUTE_WEAK;
    RepeatTimerSwitchingBlockNrvSwitchOff RepeatTimerSwitchingBlockNrvSwitchOff::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvRepeatTimerSwitchingBlock

inline void NrvRepeatTimerSwitchingBlock::RepeatTimerSwitchingBlockNrvSwitchOff::execute(Spine* pSpine) const {
    RepeatTimerSwitchingBlock* actor = reinterpret_cast< RepeatTimerSwitchingBlock* >(pSpine->mExecutor);
    actor->exeSwitchOff();
}

inline void NrvRepeatTimerSwitchingBlock::RepeatTimerSwitchingBlockNrvSwitchOn::execute(Spine* pSpine) const {
    RepeatTimerSwitchingBlock* actor = reinterpret_cast< RepeatTimerSwitchingBlock* >(pSpine->mExecutor);
    actor->exeSwitchOn();
}

inline void NrvRepeatTimerSwitchingBlock::RepeatTimerSwitchingBlockNrvWait::execute(Spine* pSpine) const {
    RepeatTimerSwitchingBlock* actor = reinterpret_cast< RepeatTimerSwitchingBlock* >(pSpine->mExecutor);
    actor->exeWait();
}



inline void NrvRepeatTimerSwitchingBlock::RepeatTimerSwitchingBlockNrvChange::execute(Spine* pSpine) const {
    RepeatTimerSwitchingBlock* actor = reinterpret_cast< RepeatTimerSwitchingBlock* >(pSpine->mExecutor);
    actor->exeChange();
}
