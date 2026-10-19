#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBellyDragonBodyHill {
    NEW_NERVE(BellyDragonBodyHillNrvAppear, BellyDragonBodyHill, Appear);
    NEW_NERVE(BellyDragonBodyHillNrvDisappear, BellyDragonBodyHill, Disappear);
};  // namespace NrvBellyDragonBodyHill

BellyDragonBodyHill::BellyDragonBodyHill(const char* pName, MtxPtr pParentMtx, bool pIsDiveIn, bool pIsAutoDisappear)
    : LiveActor(pName), mParentMtx(pParentMtx), mIsDiveIn(pIsDiveIn), mIsCheckAppearEnd(false), mIsAutoDisappear(pIsAutoDisappear), mIsLongDisappear(false), mIsDiveSound(false) {
}

void BellyDragonBodyHill::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "BellyDragonBodyHill", false);
    initNerve(&NrvBellyDragonBodyHill::BellyDragonBodyHillNrvAppear::sInstance, 0);
    makeActorDead();
}

void BellyDragonBodyHill::appear() {
    LiveActor::appear();
    mIsCheckAppearEnd = false;
    setNerve(&NrvBellyDragonBodyHill::BellyDragonBodyHillNrvAppear::sInstance);
}

void BellyDragonBodyHill::calcAndSetBaseMtx() {
    mPosition.set<f32>(mParentMtx[0][3], mParentMtx[1][3], mParentMtx[2][3]);
    MR::setBaseTRMtx(this, mParentMtx);
}

void BellyDragonBodyHill::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
        if (mIsDiveSound) {
            if (mIsDiveIn) {
                MR::startActionSound(this, "AppearDiveIn", -1, -1, -1);
            } else {
                MR::startActionSound(this, "AppearDiveOut", -1, -1, -1);
            }
        }
    }
    if (mIsCheckAppearEnd && MR::isActionEnd(this)) {
        setNerve(&NrvBellyDragonBodyHill::BellyDragonBodyHillNrvDisappear::sInstance);
        return;
    }
    if (MR::isGreaterStep(this, 0xB4)) {
        setNerve(&NrvBellyDragonBodyHill::BellyDragonBodyHillNrvDisappear::sInstance);
        return;
    }
    if (mIsAutoDisappear && MR::isActionEnd(this)) {
        setNerve(&NrvBellyDragonBodyHill::BellyDragonBodyHillNrvDisappear::sInstance);
    }
}

void BellyDragonBodyHill::exeDisappear() {
    if (MR::isFirstStep(this)) {
        if (mIsLongDisappear) {
            MR::startAction(this, "DisappearLong");
        } else {
            MR::startAction(this, "Disappear");
        }
    }
    if (mIsDiveIn) {
        MR::startActionSound(this, "DisappearDiveIn", -1, -1, -1);
    } else {
        MR::startActionSound(this, "DisappearDiveOut", -1, -1, -1);
    }
    if (MR::isActionEnd(this)) {
        mIsCheckAppearEnd = false;
        mIsLongDisappear = false;
        kill();
    }
}

BellyDragonBodyHill::~BellyDragonBodyHill() {
}
