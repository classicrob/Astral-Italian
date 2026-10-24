#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStateCommon.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util.hpp"

namespace {
    // single pattern, four corners, four corners (mirrored order)
    KoopaStateLastChasePattern sPatternsSingle[2] = {
        KoopaStateLastChasePattern(-1.0f, 0.0f, 40, 150),
        KoopaStateLastChasePattern(1.0f, 0.0f, 40, 150),
    };
    KoopaStateLastChasePattern sPatternsCorner[4] = {
        KoopaStateLastChasePattern(1.0f, 1.0f, 40, 150),
        KoopaStateLastChasePattern(-1.0f, 1.0f, 40, 150),
        KoopaStateLastChasePattern(1.0f, -1.0f, 40, 150),
        KoopaStateLastChasePattern(-1.0f, -1.0f, 40, 150),
    };
    KoopaStateLastChasePattern sPatternsMirror[4] = {
        KoopaStateLastChasePattern(1.0f, 1.0f, 40, 150),
        KoopaStateLastChasePattern(-1.0f, 1.0f, 40, 150),
        KoopaStateLastChasePattern(1.0f, -1.0f, 40, 150),
        KoopaStateLastChasePattern(-1.0f, -1.0f, 40, 150),
    };
};  // namespace

namespace NrvKoopaStateLastChase {
    class KoopaStateLastChaseNrvDashWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLastChaseNrvDashWait sInstance;
    };
    KoopaStateLastChaseNrvDashWait KoopaStateLastChaseNrvDashWait::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLastChaseNrvRoll : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLastChaseNrvRoll sInstance;
    };
    KoopaStateLastChaseNrvRoll KoopaStateLastChaseNrvRoll::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLastChaseNrvDashEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLastChaseNrvDashEnd sInstance;
    };
    KoopaStateLastChaseNrvDashEnd KoopaStateLastChaseNrvDashEnd::sInstance ATTRIBUTE_WEAK;
    class KoopaStateLastChaseNrvMoveEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaStateLastChaseNrvMoveEnd sInstance;
    };
    KoopaStateLastChaseNrvMoveEnd KoopaStateLastChaseNrvMoveEnd::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKoopaStateLastChase

inline void KoopaStateLastChase::exeDashEnd() {
    MR::startSoundInCameraDistance(mHost, "LastChaseRoll", -1, -1, 31000.0f);
    moveToTarget();
    MR::updateActorStateAndNextNerve(this, &NrvKoopaStateLastChase::KoopaStateLastChaseNrvMoveEnd::sInstance);
}

KoopaStateLastChase::KoopaStateLastChase(KoopaBattle* pHost)
    : ActorStateBase< KoopaBattle >("クッパ最終追跡", pHost), mDashWait(nullptr), mDashEnd(nullptr) {
    mPosA.x = 0.0f;
    mPosA.y = 0.0f;
    mPosA.z = 0.0f;
    mPosB.x = 0.0f;
    mPosB.y = 0.0f;
    mPosB.z = 0.0f;
    mPosC.x = 0.0f;
    mPosC.y = 0.0f;
    mPosC.z = 0.0f;
    mQuat.x = 0.0f;
    mQuat.y = 0.0f;
    mQuat.z = 0.0f;
    mQuat.w = 1.0f;
    mPatterns = nullptr;
    mPatternIdx = 0;
    mPatternNum = 0;
    mRollStep = 0;
    mHoldStep = 0;
    mStep = 0;
    _64 = true;
    mIsLoopAnim = false;
    mIsLeft = false;
    _67 = false;
    pHost->appearDefenseForm();
    setPatternMode(1);
    initNerve(&NrvKoopaStateLastChase::KoopaStateLastChaseNrvDashWait::sInstance, 2);
    mDashWait = new KoopaStateDefense(mHost, true);
    MR::initActorState(this, mDashWait, &NrvKoopaStateLastChase::KoopaStateLastChaseNrvDashWait::sInstance, "DashWait");
    mDashEnd = new KoopaStateDefense(mHost, false);
    MR::initActorState(this, mDashEnd, &NrvKoopaStateLastChase::KoopaStateLastChaseNrvDashEnd::sInstance, "DashEnd");
}

void KoopaStateLastChase::setPatternMode(s32 mode) {
    switch (mode) {
    case 1:
        mPatterns = sPatternsSingle;
        mPatternNum = 2;
        break;
    case 2:
        mPatterns = sPatternsCorner;
        mPatternNum = 4;
        break;
    case 3:
        mPatterns = sPatternsMirror;
        mPatternNum = 4;
        break;
    }
    mPatternIdx = 0;
}

void KoopaStateLastChase::appear() {
    mIsDead = false;
    MR::startKoopaBattleFaceCtrl(mHost, 30);
    if (_64) {
        mPatternIdx = 0;
    } else {
        nextPattern();
    }
    setupTarget();
    MR::tryStartActionNotPlaying(mHost, "ShellRotate");
    setNerve(&NrvKoopaStateLastChase::KoopaStateLastChaseNrvRoll::sInstance);
}

void KoopaStateLastChase::control() {
}

void KoopaStateLastChase::setupTarget() {
    mPosA.x = mHost->mPosition.x;
    mPosA.y = mHost->mPosition.y;
    mPosA.z = mHost->mPosition.z;
    _67 = calcTargetPos(&mPosB, mPosA, mPatternIdx);
    calcTargetPos(&mPosC, mPosB, mPatternIdx + 1);
    mRollStep = mPatterns[mPatternIdx].mRollStep;
    mHoldStep = mPatterns[mPatternIdx].mHoldStep;
    mIsLeft = mPatterns[mPatternIdx].mX > 0.0f;
}

void KoopaStateLastChase::exeDashWait() {
    if (MR::isFirstStep(this)) {
        setupTarget();
        mDashWait->setStartActionName(mIsLeft ? "LastMoveStartLeft" : "LastMoveStartRight");
        mQuat.set< f32 >(mHost->mPoseQuat);
    }
    TVec3f dir;
    MR::normalizeOrZero(&dir, mPosB - mPosA);
    TQuat4f* pQuat = &mHost->mPoseQuat;
    MR::turnQuatZDirRate(pQuat, mQuat, dir, 0.5f * MR::calcNerveEaseInOutRate(this, 70));
    MR::updateActorStateAndNextNerve(this, &NrvKoopaStateLastChase::KoopaStateLastChaseNrvRoll::sInstance);
}

void KoopaStateLastChase::exeRoll() {
    if (MR::isFirstStep(this)) {
        mQuat.set< f32 >(mHost->mPoseQuat);
        mStep = 0;
    }
    MR::rotateQuatXAxisDegree(&mHost->mPoseQuat, mQuat, MR::calcNerveValue(this, mRollStep - 4, 0.0f, 720.0f));
    moveToTarget();
    MR::startSoundInCameraDistance(mHost, "LastChaseRoll", -1, -1, 31000.0f);
    if (MR::isGreaterStep(this, mRollStep - 5)) {
        setNerve(&NrvKoopaStateLastChase::KoopaStateLastChaseNrvDashEnd::sInstance);
    }
}

void KoopaStateLastChase::exeMoveEnd() {
    if (MR::isFirstStep(this)) {
        MR::startKoopaBattleFaceCtrl(mHost, 30);
        mQuat.set< f32 >(mHost->mPoseQuat);
        MR::startAction(mHost, mIsLeft ? "LastMoveEndLeft" : "LastMoveEndRight");
        mIsLoopAnim = false;
    }
    if (!mIsLoopAnim && MR::isActionEnd(mHost)) {
        MR::startAction(mHost, mIsLeft ? "LastMoveEndLeftLoop" : "LastMoveEndRightLoop");
        mIsLoopAnim = true;
    }
    bool isEnd = moveToTarget();
    f32 rate = MR::calcNerveEaseInOutRate(this, mHoldStep);
    TQuat4f* pQuat;
    KoopaBattle* pHost = mHost;
    pQuat = &pHost->mPoseQuat;
    pQuat->set< f32 >(mQuat);
    pQuat->slerp(pHost->mBattleQuat, rate);
    if (MR::isGreaterStep(this, mHoldStep)) {
        if (isEnd) {
            kill();
        } else {
            nextPattern();
            setNerve(&NrvKoopaStateLastChase::KoopaStateLastChaseNrvDashWait::sInstance);
        }
    }
}

void KoopaStateLastChase::nextPattern() {
    mPatternIdx++;
    if (mPatternNum > mPatternIdx) {
        return;
    }
    mPatternIdx = 0;
}

// NONMATCHING: local FPR allocation differs after preserving the constant-first addition.
bool KoopaStateLastChase::calcTargetPos(TVec3f* pDst, const TVec3f& rSrc, s32 idx) {
    s32 num = mPatternNum;
    f32 z = 3500.0f + MR::calcKoopaBattleFrontDot(rSrc);
    bool res = false;
    s32 i = (num + idx) % num;
    TVec3f local;
    local.x = 3500.0f * mPatterns[i].mX;
    local.y = 3500.0f * mPatterns[i].mY;
    local.z = z;
    if (z > -5000.0f) {
        local.z = -5000.0f;
        res = true;
    }
    MR::calcKoopaBattleWorldPos(pDst, local);
    return res;
}

bool KoopaStateLastChase::isChasing() const {
    bool res;
    if (mHost->mIsDefenseForm) {
        res = false;
    } else {
        res = false;
        if (isNerve(&NrvKoopaStateLastChase::KoopaStateLastChaseNrvDashWait::sInstance) ||
            isNerve(&NrvKoopaStateLastChase::KoopaStateLastChaseNrvMoveEnd::sInstance)) {
            res = true;
        }
    }
    return res;
}

bool KoopaStateLastChase::moveToTarget() {
    f32 rate;
    if (mStep <= mRollStep) {
        rate = 0.8f * MR::getEaseInValue(static_cast< f32 >(mStep) / static_cast< f32 >(mRollStep), 0.0f, 1.0f, 1.0f);
    } else {
        rate = 0.8f;
        f32 t = static_cast< f32 >(mStep - mRollStep) / static_cast< f32 >(mHoldStep);
        f32 clamped;
        if (t < 0.0f) {
            clamped = 0.0f;
        } else if (t > 1.0f) {
            clamped = 1.0f;
        } else {
            clamped = t;
        }
        rate += (1.0f - 0.8f) * MR::getEaseOutValue(clamped, 0.0f, 1.0f, 1.0f);
    }
    MR::vecBlend(&mHost->mPosition, mPosA, mPosB, rate);
    bool res = false;
    if (mStep >= mRollStep + mHoldStep && _67) {
        res = true;
    }
    mStep++;
    return res;
}

KoopaStateLastChase::~KoopaStateLastChase() {
}

void NrvKoopaStateLastChase::KoopaStateLastChaseNrvDashWait::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLastChase* >(pSpine->mExecutor)->exeDashWait();
}

void NrvKoopaStateLastChase::KoopaStateLastChaseNrvRoll::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLastChase* >(pSpine->mExecutor)->exeRoll();
}

void NrvKoopaStateLastChase::KoopaStateLastChaseNrvDashEnd::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLastChase* >(pSpine->mExecutor)->exeDashEnd();
}

void NrvKoopaStateLastChase::KoopaStateLastChaseNrvMoveEnd::execute(Spine* pSpine) const {
    reinterpret_cast< KoopaStateLastChase* >(pSpine->mExecutor)->exeMoveEnd();
}
