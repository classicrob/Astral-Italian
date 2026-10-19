#include "Game/MapObj/NoteFairy.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/Note.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/NoteCounter.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"

namespace NrvNoteFairy {
    NEW_NERVE(NoteFairyNrvHide, NoteFairy, Hide);
    NEW_NERVE(NoteFairyNrvStartAppearDemo, NoteFairy, StartAppearDemo);
    NEW_NERVE(NoteFairyNrvRailMoveStart, NoteFairy, RailMoveStart);
    NEW_NERVE(NoteFairyNrvDemoAppear, NoteFairy, DemoAppear);
    NEW_NERVE(NoteFairyNrvAppearNoteBloom, NoteFairy, AppearNoteBloom);
    NEW_NERVE(NoteFairyNrvAppearNote, NoteFairy, AppearNote);
    NEW_NERVE(NoteFairyNrvEndWait, NoteFairy, EndWait);
    NEW_NERVE(NoteFairyNrvSuccessDemo, NoteFairy, SuccessDemo);
    NEW_NERVE(NoteFairyNrvSuccessDemoWait, NoteFairy, SuccessDemoWait);
    NEW_NERVE(NoteFairyNrvSuccess, NoteFairy, Success);
};  // namespace NrvNoteFairy

NoteFairy::NoteFairy(const char* pName) : LiveActor(pName) {
    mOriginPos.x = 0.0f;
    mOriginPos.y = 0.0f;
    mOriginPos.z = 0.0f;
    mDemoAppearPos.x = 0.0f;
    mDemoAppearPos.y = 0.0f;
    mDemoAppearPos.z = 0.0f;
    mAppearanceSpeed = 13.0f;
    mTimeLimit = 20;
    mMelodyNoteNum = 0;
    _B0 = 0;
    mSong = -1;
    mCompletionJingle = -1;
    mNoteCoord = 150.0f;
    mNoteArray = nullptr;
    mLastGotNote = nullptr;
    mAppearanceType = -1;
    _CC = false;
    mIsDirectBloom = false;
    mHasDemo = false;
    mIsStageBgmFade = false;
    mIsSilentRiddle = false;
    mIsRailPointPlacement = false;
    mPowerStarSpawnType = -1;
    mCameraInfo = nullptr;
}

void NoteFairy::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    s32 noteNum = -1;
    MR::getJMapInfoArg0WithInit(rIter, &noteNum);
    MR::getJMapInfoArg1NoInit(rIter, &mAppearanceSpeed);
    MR::getJMapInfoArg2NoInit(rIter, &mTimeLimit);
    mAppearanceType = -1;
    MR::getJMapInfoArg3WithInit(rIter, &mAppearanceType);

    switch (mAppearanceType) {
    case -1:
        break;
    case 1:
        break;
    case 3:
        _CC = 1;
        break;
    case 5:
        mIsDirectBloom = 1;
        break;
    case 6:
        mIsDirectBloom = 1;
        mIsSilentRiddle = 1;
        break;
    }

    MR::getJMapInfoArg4WithInit(rIter, &mSong);
    MR::getJMapInfoArg5WithInit(rIter, &mCompletionJingle);
    s32 arg6;
    MR::getJMapInfoArg6WithInit(rIter, &arg6);

    if (arg6 > 0) {
        mIsStageBgmFade = false;
    } else {
        mIsStageBgmFade = true;
    }

    s32 powerStarType = 0;
    MR::getJMapInfoArg7NoInit(rIter, &powerStarType);

    if (powerStarType != 0) {
        MR::declarePowerStar(this);

        if (powerStarType == 2) {
            mPowerStarSpawnType = 1;
        } else if (powerStarType == 3) {
            mPowerStarSpawnType = 2;
        } else {
            mPowerStarSpawnType = 0;
        }
    }

    mOriginPos.set(mPosition);
    mTimeLimit *= 60;
    mHasDemo = MR::tryRegisterDemoCast(this, rIter);
    initRailRider(rIter);
    MR::initActor(this, rIter, "Note", false);
    MR::startBrk(this, "Note");
    MR::connectToSceneNoSilhouettedMapObjStrongLight(this);
    initEffectKeeper(0, "NoteFairy", false);
    initSound(6, "NoteFairy", nullptr, TVec3f(0.0f));
    MR::initShadowVolumeSphere(this, 50.0f);
    s32 railArg = -1;
    MR::getRailArg0WithInit(this, &railArg);

    if (noteNum > 0) {
        if (mSong < 0) {
            mMelodyNoteNum = noteNum;
        } else {
            mMelodyNoteNum = MR::getRemixMelodyNoteNum(mSong);
        }

        mNoteCoord = MR::getRailTotalLength(this) / mMelodyNoteNum;
    } else {
        if (noteNum == -2) {
            s32 pointNum = MR::getRailPointNum(this);
            if (mSong < 0) {
                mMelodyNoteNum = pointNum;
            } else {
                mMelodyNoteNum = MR::getRemixMelodyNoteNum(mSong);
            }

            if (railArg < 0) {
                mNoteCoord = 200.0f;
            } else {
                mNoteCoord = railArg;
            }

            mIsRailPointPlacement = true;
        } else {
            if (mSong < 0) {
                mMelodyNoteNum = MR::getRailTotalLength(this) / mNoteCoord;
            } else {
                mMelodyNoteNum = MR::getRemixMelodyNoteNum(mSong);
                mNoteCoord = MR::getRailTotalLength(this) / mMelodyNoteNum;
            }
        }
    }

    mNoteArray = new Note*[mMelodyNoteNum];

    for (s32 i = 0; i < mMelodyNoteNum; i++) {
        TVec3f stack_24;

        if (mIsRailPointPlacement) {
            MR::calcRailPointPos(&stack_24, this, i);
        } else {
            stack_24.set(MR::getRailPos(this));
            MR::moveCoord(this, mNoteCoord);
        }

        mNoteArray[i] = new Note("音符", MR::getRailDirection(this), this);
        MR::setPosition(mNoteArray[i], stack_24);
        mNoteArray[i]->initWithoutIter();
        mNoteArray[i]->mCounter = mTimeLimit;
    }

    MR::moveCoordToStartPos(this);
    MR::setRailCoord(this, 0.0f);
    TVec3f stack_18(MR::getRailDirection(this));
    stack_18.y = 0.0f;
    MR::normalizeOrZero(&stack_18);
    stack_18.x *= -800.0f;
    stack_18.y *= -800.0f;
    stack_18.z *= -800.0f;
    mDemoAppearPos.x = stack_18.x + mPosition.x;
    mDemoAppearPos.y = (0.5f * mPosition.y) + (0.5f * MR::getRailPos(this).y);
    mDemoAppearPos.z = stack_18.z + mPosition.z;
    MR::needStageSwitchReadAppear(this, rIter);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initUseStageSwitchReadB(this, rIter);
    MR::initActorCamera(this, rIter, &mCameraInfo);
    initNerve(GET_NERVE(NoteFairy, NoteFairyNrvHide));
    makeActorAppeared();
}

void NoteFairy::control() {
    if (!isNerve(GET_NERVE(NoteFairy, NoteFairyNrvHide)) && MR::isValidSwitchB(this) && !MR::isOnSwitchB(this)) {
        kill();
    } else if (MR::isPowerStarGetDemoActive()) {
        kill();
    }
}

void NoteFairy::kill() {
    if (mIsStageBgmFade) {
        MR::moveVolumeStageBGMForNoteFairy(1.0f, 120);
    }

    killAllNotes();
    LiveActor::kill();
}

void NoteFairy::killAllNotes() {
    for (s32 i = 0; i < mMelodyNoteNum; i++) {
        mNoteArray[i]->kill();
    }
}

void NoteFairy::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        MR::invalidateClipping(this);
    }

    if (MR::isOnSwitchAppear(this)) {
        if (!mIsSilentRiddle) {
            MR::startActionSound(this, "SyReadRiddleS", -1, -1, -1);
        }

        MR::invalidateClipping(this);

        if (mIsStageBgmFade) {
            MR::moveVolumeStageBGMForNoteFairy(0.0f, 30);
        }

        switch (mAppearanceType) {
        case 1:
        case 2:
        case 3:
            setNerve(GET_NERVE(NoteFairy, NoteFairyNrvStartAppearDemo));
            return;
        case 4:
        case 5:
        case 6:
            setNerve(GET_NERVE(NoteFairy, NoteFairyNrvRailMoveStart));
            return;
        }

        setNerve(GET_NERVE(NoteFairy, NoteFairyNrvAppearNote));
    }
}

// NONMATCHING: vector subtraction register allocation and saved-register frame differ.
void NoteFairy::exeStartAppearDemo() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::emitEffect(this, "Blur");
    }

    if (MR::isStep(this, 30)) {
        TVec3f stack_8 = mOriginPos - *MR::getPlayerPos();
        MR::setPlayerFrontVec(stack_8, 1);
        enterDemoAppear(GET_NERVE(NoteFairy, NoteFairyNrvDemoAppear), true);
    }
}

void NoteFairy::exeRailMoveStart() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::emitEffect(this, "Blur");
        mPosition = MR::getRailPos(this);
    }

    if (mIsDirectBloom) {
        setNerve(GET_NERVE(NoteFairy, NoteFairyNrvAppearNoteBloom));
    } else if (MR::isStep(this, 0)) {
        enterDemoAppear(GET_NERVE(NoteFairy, NoteFairyNrvAppearNoteBloom), false);
    }
}

void NoteFairy::exeDemoAppear() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Blur");
    }

    if (MR::isLessEqualStep(this, 60)) {
        f32 ease = MR::getEaseOutValue(getNerveStep() / 60.0f, 0.0f, 1.0f, 1.0f);
        mPosition.x = (mOriginPos.x * (1.0f - ease)) + (mDemoAppearPos.x * ease);
        mPosition.y = (((mOriginPos.y * (1.0f - ease)) + (mDemoAppearPos.y * ease)) - (300.0f * MR::sinDegree(180.0f * ease)));
        mPosition.z = (mOriginPos.z * (1.0f - ease)) + (mDemoAppearPos.z * ease);
    }

    if (MR::isGreaterStep(this, 70)) {
        f32 ease = MR::getEaseInValue((getNerveStep() - 70) / 60.0f, 0.0f, 1.0f, 1.0f);
        mPosition.x = (mDemoAppearPos.x * (1.0f - ease)) + (ease * MR::getRailPos(this).x);
        mPosition.y = mDemoAppearPos.y - (MR::sinDegree(90.0f * ease) * (mDemoAppearPos.y - MR::getRailPos(this).y));
        mPosition.z = (mDemoAppearPos.z * (1.0f - ease)) + (ease * MR::getRailPos(this).z);
    }

    MR::startActionSound(this, "OjLvFlowerFairyMove", -1, -1, -1);
    mRotation.y = MR::getSceneObj< NoteGroup >(SceneObj_NoteGroup)->mRotation;

    if (MR::isStep(this, 130)) {
        setNerve(GET_NERVE(NoteFairy, NoteFairyNrvAppearNoteBloom));
    }
}

void NoteFairy::exeAppearNoteBloom() {
    if (MR::isFirstStep(this)) {
        mVelocity.zero();
        MR::declareNoteNumMaxAndMelodyToCounter(this, mMelodyNoteNum, mSong, mNoteCoord);
    }

    if (!_CC && !mIsDirectBloom && MR::isStep(this, 60) && mAppearanceType != -1) {
        MR::endDemo(this, "出現");
        MR::endActorCamera(this, mCameraInfo, false);
    }

    MR::startActionSound(this, "OjLvFlowerFairyMove", -1, -1, -1);
    MR::moveCoordAndFollowTrans(this, mAppearanceSpeed);
    mRotation.y = MR::getSceneObj< NoteGroup >(SceneObj_NoteGroup)->mRotation;

    while (_B0 < mMelodyNoteNum && MR::getRailCoord(this) >= getNoteCoord(_B0)) {
        mNoteArray[_B0]->appear();
        mNoteArray[_B0]->mBrkFrame = 5.0f * _B0;

        if (MR::isLessStep(this, 60) && mAppearanceType != -1) {
            MR::requestMovementOn(mNoteArray[_B0]);
        }

        _B0++;
    }

    if (_B0 >= mMelodyNoteNum) {
        if (_CC) {
            MR::endDemo(this, "出現");
            MR::endActorCamera(this, mCameraInfo, false);
        }

        MR::hideModel(this);
        MR::deleteEffect(this, "Blur");
        setNerve(GET_NERVE(NoteFairy, NoteFairyNrvEndWait));
    }
}

void NoteFairy::exeAppearNote() {
    if (MR::isFirstStep(this)) {
        mVelocity.zero();
        MR::declareNoteNumMaxAndMelodyToCounter(this, mMelodyNoteNum, mSong, mNoteCoord);

        for (s32 i = 0; i < mMelodyNoteNum; i++) {
            mNoteArray[i]->appear();
            mNoteArray[i]->mBrkFrame = 5.0f * i;
        }

        setNerve(GET_NERVE(NoteFairy, NoteFairyNrvEndWait));
    }
}

void NoteFairy::exeEndWait() {
    if (MR::isFirstStep(this)) {
        for (s32 i = 0; i < mMelodyNoteNum; i++) {
            mNoteArray[i]->startCountDown();
        }
    }

    MR::startLevelSound(this, "SE_SY_NO_SOUND_FANFARE", -1, -1, -1);
    s32 timeLeft = mTimeLimit - getNerveStep();

    if (timeLeft <= 170) {
        MR::startSystemLevelSE("SE_RS_LV_NOTE_TIMER_FAST", 0);
    } else if (timeLeft <= mTimeLimit / 2) {
        MR::startSystemLevelSE("SE_RS_LV_NOTE_TIMER_MIDDLE", 30);
    } else {
        MR::startSystemLevelSE("SE_RS_LV_NOTE_TIMER_SLOW", 85);
    }

    if (isDeadAllNotes()) {
        kill();
    }
}

void NoteFairy::exeSuccessDemo() {
    if (MR::tryStartDemoRegistered(this, nullptr)) {
        setNerve(GET_NERVE(NoteFairy, NoteFairyNrvSuccessDemoWait));
    }
}

void NoteFairy::exeSuccessDemoWait() {
    if (!MR::isTimeKeepDemoActive()) {
        setNerve(GET_NERVE(NoteFairy, NoteFairyNrvSuccess));
    }
}

void NoteFairy::exeSuccess() {
    if (MR::isFirstStep(this)) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }

        switch (mPowerStarSpawnType) {
        case 1:
            MR::requestAppearPowerStar(this, mLastGotNote->mPosition);
            break;
        case 2:
            MR::requestAppearPowerStar(this, mLastGotNote->mPosition);
            break;
        case 0:
            MR::requestAppearPowerStar(this, mOriginPos);
            break;
        }

        setNerve(GET_NERVE(NoteFairy, NoteFairyNrvEndWait));
    }
}

void NoteFairy::enterDemoAppear(const Nerve* pNerve, bool hasNoFrame) {
    MR::startActorCameraTargetSelf(this, mCameraInfo);

    if (hasNoFrame) {
        MR::requestStartDemo(this, "出現", pNerve, nullptr);
    } else {
        MR::requestStartDemoWithoutCinemaFrame(this, "出現", pNerve, nullptr);
    }

    MR::requestMovementOn(MR::getSceneObj< NoteGroup >(SceneObj_NoteGroup));
    MR::requestMovementOn(this);
}

bool NoteFairy::isDeadAllNotes() {
    for (s32 i = 0; i < mMelodyNoteNum; i++) {
        if (!MR::isDead(mNoteArray[i])) {
            return false;
        }
    }

    return true;
}

void NoteFairy::setLastGotNote(Note* pNote) {
    mLastGotNote = pNote;
}

f32 NoteFairy::getNoteCoord(s32 val) {
    if (mIsRailPointPlacement) {
        return MR::getRailPointCoord(this, val);
    }

    return val * mNoteCoord;
}

bool NoteFairy::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgSuccess(msg)) {
        killAllNotes();

        switch (mCompletionJingle) {
        case -1:
            break;
        case 0:
            MR::startActionSound(this, "SyTotalComplete");
            break;
        }

        if (mHasDemo) {
            setNerve(GET_NERVE(NoteFairy, NoteFairyNrvSuccessDemo));
        } else {
            setNerve(GET_NERVE(NoteFairy, NoteFairyNrvSuccess));
        }

        return true;
    }

    return false;
}

NoteFairy::~NoteFairy() {
}
