#pragma auto_inline off
#include "Game/Boss/BossJugem.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBossJugemStateThrowAll {
    NEW_NERVE(BossJugemStateThrowAllNrvMove, BossJugemStateThrowAll, Move);
    class BossJugemStateThrowAllNrvReady : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            BossJugemStateThrowAll* pActor = reinterpret_cast< BossJugemStateThrowAll* >(pSpine->mExecutor);
            if (MR::isActionEnd(pActor->mHost)) {
                pActor->selectThrow();
            }
        }
        static BossJugemStateThrowAllNrvReady sInstance;
    };
    BossJugemStateThrowAllNrvReady BossJugemStateThrowAllNrvReady::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(BossJugemStateThrowAllNrvThrowOne, BossJugemStateThrowAll, ThrowOne);
    NEW_NERVE(BossJugemStateThrowAllNrvThrowTwo, BossJugemStateThrowAll, ThrowTwo);
    NEW_NERVE(BossJugemStateThrowAllNrvChance, BossJugemStateThrowAll, Chance);
    NEW_NERVE(BossJugemStateThrowAllNrvMoveBack, BossJugemStateThrowAll, MoveBack);
};  // namespace NrvBossJugemStateThrowAll

BossJugemStateThrowAll::BossJugemStateThrowAll(BossJugem* pHost)
    : ActorStateBase< BossJugem >("\203\173\203\130\203\127\203\205\203\121\203\200\223\212\202\260\201\151\210\352\220\304\201\152", pHost), mThrowOne(nullptr), mThrowTwo(nullptr), mTargetPos(0.0f, 0.0f, 0.0f), mOriginPos(0.0f, 0.0f, 0.0f),
      mMoveFrames(0), _38(0.0f), mSpeedBonus(0.0f) {
}

void BossJugemStateThrowAll::init() {
    initNerve(&NrvBossJugemStateThrowAll::BossJugemStateThrowAllNrvMove::sInstance, 2);
    mThrowOne = new BossJugemStateThrowOne(mHost);
    mThrowTwo = new BossJugemStateThrowTwo(mHost);
    MR::initActorState(this, mThrowOne, &NrvBossJugemStateThrowAll::BossJugemStateThrowAllNrvThrowOne::sInstance, "ThrowOne");
    MR::initActorState(this, mThrowTwo, &NrvBossJugemStateThrowAll::BossJugemStateThrowAllNrvThrowTwo::sInstance, "ThrowTwo");
}

void BossJugemStateThrowAll::appear() {
    mIsDead = false;
    setNerve(&NrvBossJugemStateThrowAll::BossJugemStateThrowAllNrvMove::sInstance);
    mRemain = mHost->getRemainTogezoNum();
    if (mRemain < 2) {
        mRemain = 2;
    }
}

void BossJugemStateThrowAll::exeMove() {
    BossJugem* host;
    if (MR::isFirstStep(this)) {
        mOriginPos = mHost->mPosition;
        TVec3f railPos(0.0f);
        host = mHost;
        MR::calcRailPosAtCoord(&railPos, host, 0.5f * MR::getRailTotalLength(host));
        TVec3f railDir(0.0f);
        host = mHost;
        MR::calcRailDirectionAtCoord(&railDir, host, 0.5f * MR::getRailTotalLength(host));
        TPos3f mtx;
        PSMTXIdentity(mtx.toMtxPtr());
        MR::makeMtxSideUpPos(&mtx, railDir, -mHost->mGravity, railPos);
        MR::addTransMtxLocalZ(mtx.toMtxPtr(), 3000.0f);
        MR::extractMtxTrans(mtx.toMtxPtr(), &mTargetPos);
        mMoveFrames = (mTargetPos - mOriginPos).length() / 50.0f;
        if (mMoveFrames < 90) {
            mMoveFrames = 90;
        }
        if (mMoveFrames > 120) {
            mMoveFrames = 120;
        }
    }
    mHost->turnToPlayer(0.06f);
    host = mHost;
    host->updateMoveBackAnim(getNerveStep(), mMoveFrames + 60);
    TVec3f* position = &mHost->mPosition;
    MR::vecBlend(position, mOriginPos, mTargetPos, MR::calcNerveEaseInOutRate(this, mMoveFrames));
    if (MR::isGreaterStep(this, mMoveFrames)) {
        setNerve(&NrvBossJugemStateThrowAll::BossJugemStateThrowAllNrvReady::sInstance);
    }
}

void BossJugemStateThrowAll::exeThrowOne() {
    if (MR::isFirstStep(this)) {
        BossJugemStateThrowOne* pThrow = mThrowOne;
        pThrow->mSpeed = 64.0f + mSpeedBonus;
        pThrow->mAngle = 20.0f;
    }
    if (MR::updateActorState(this)) {
        setNerve(&NrvBossJugemStateThrowAll::BossJugemStateThrowAllNrvChance::sInstance);
    }
}

void BossJugemStateThrowAll::exeThrowTwo() {
    if (MR::isFirstStep(this)) {
        BossJugemStateThrowTwo* pThrow = mThrowTwo;
        pThrow->mSpeed = 64.0f + mSpeedBonus;
        pThrow->mAngle = 20.0f;
    }
    if (MR::updateActorState(this)) {
        if (mHost->getRemainTogezoNum() > 0) {
            selectThrow();
        } else {
            setNerve(&NrvBossJugemStateThrowAll::BossJugemStateThrowAllNrvChance::sInstance);
        }
    }
}

void BossJugemStateThrowAll::exeChance() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("WaitNormal");
    }
    if (MR::isStep(this, 20)) {
        mHost->startActionAll("ChanceTime");
    }
    if (MR::isGreaterStep(this, 120)) {
        setNerve(&NrvBossJugemStateThrowAll::BossJugemStateThrowAllNrvMoveBack::sInstance);
    }
}

void BossJugemStateThrowAll::exeMoveBack() {
    BossJugem* host;
    if (MR::isFirstStep(this)) {
        mOriginPos = mHost->mPosition;
        host = mHost;
        MR::calcRailPosAtCoord(&mTargetPos, host, MR::getRailCoord(host));
        mMoveFrames = (mTargetPos - mOriginPos).length() / 50.0f;
        if (mMoveFrames < 90) {
            mMoveFrames = 90;
        }
        if (mMoveFrames > 120) {
            mMoveFrames = 120;
        }
    }
    mHost->turnToPlayer(0.06f);
    host = mHost;
    host->updateMoveAnim(getNerveStep(), mMoveFrames);
    TVec3f* position = &mHost->mPosition;
    MR::vecBlend(position, mOriginPos, mTargetPos, MR::calcNerveEaseInOutRate(this, mMoveFrames));
    if (MR::isGreaterStep(this, mMoveFrames)) {
        kill();
    }
}

void BossJugemStateThrowAll::selectThrow() {
    s32 remain = mRemain;
    if (remain > 0) {
        mSpeedBonus = 10.0f * (1.0f - 0.5f * (remain % 3));
        if (remain == 1) {
            mRemain = remain - 1;
            setNerve(&NrvBossJugemStateThrowAll::BossJugemStateThrowAllNrvThrowOne::sInstance);
        } else {
            mRemain = remain - 2;
            setNerve(&NrvBossJugemStateThrowAll::BossJugemStateThrowAllNrvThrowTwo::sInstance);
        }
    } else {
        setNerve(&NrvBossJugemStateThrowAll::BossJugemStateThrowAllNrvChance::sInstance);
    }
}
