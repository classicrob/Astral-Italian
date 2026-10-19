#include "Game/MapObj/StarPieceGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/StarPiece.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>

namespace NrvStarPieceGroup {
    NEW_NERVE(HostTypeNrvGroup, StarPieceGroup, Group);
    NEW_NERVE(HostTypeNrvFlow, StarPieceGroup, Flow);
};  // namespace NrvStarPieceGroup

StarPieceGroup::StarPieceGroup(const char* pName)
    : LiveActor(pName), mPieces(), mRailCoords(), mHasAppeared(), mEmitAppearEffects(), mNumPieces(), mCircleRadius(400.0f), mIsRail(), mPlaceAtPathPoints(), mLifeTimer(-1),
      mRailSpeed(10.0f), mRailClippingCenter(0.0f, 0.0f, 0.0f) {
}

bool StarPieceGroup::isExistAnyStarPiece() {
    for (int i = 0; i < mNumPieces; i++) {
        if (!MR::isDead(mPieces[i])) {
            return true;
        }
    }

    return false;
}

void StarPieceGroup::forceReplaceStarPieceAll() {
    placementAllPiece();
    for (int i = 0; i < mNumPieces; i++) {
        mPieces[i]->appearFromGroup();
        if (mEmitAppearEffects) {
            MR::emitEffect(mPieces[i], "Appear");
        }
    }

    if (mEmitAppearEffects) {
        MR::startActionSound(this, "OjStarPieceBurst");
    }
}

void StarPieceGroup::forceKillStarPieceAll(bool withEffect) {
    for (int i = 0; i < mNumPieces; i++) {
        if (MR::isDead(mPieces[i])) {
            continue;
        }

        if (!(mPieces[i]->isOnRailMove() || mPieces[i]->isFloat())) {
            continue;
        }

        mPieces[i]->makeActorDead();

        if (withEffect) {
            MR::emitEffect(mPieces[i], "Appear");
            MR::startActionSound(this, "OjStarPieceBreak");
        }
    }
}

void StarPieceGroup::init(const JMapInfoIter& rIter) {
    initSound(3, "StarPieceGroup", nullptr, TVec3f(0.0f));
    const char* objectName = nullptr;
    MR::getObjectName(&objectName, rIter);
    bool isFlow;

    if (MR::isEqualString(objectName, "StarPieceFlow")) {
        initNerve(GET_NERVE(StarPieceGroup, HostTypeNrvFlow));
        isFlow = true;
    } else {
        initNerve(GET_NERVE(StarPieceGroup, HostTypeNrvGroup));
        isFlow = false;
    }

    s32 arg2 = -1;
    MR::initDefaultPos(this, rIter);

    if (isFlow) {
        mRailSpeed = 10.0f;
        mNumPieces = 1;
        MR::getJMapInfoArg0NoInit(rIter, &mRailSpeed);
        MR::getJMapInfoArg1NoInit(rIter, &mNumPieces);
    } else {
        mNumPieces = 6;
        MR::getJMapInfoArg0NoInit(rIter, &mNumPieces);
        MR::getJMapInfoArg1NoInit(rIter, &mCircleRadius);
        MR::getJMapInfoArg2NoInit(rIter, &arg2);
        MR::getJMapInfoArg5NoInit(rIter, &mLifeTimer);
        MR::joinToGroupArray(this, rIter, mName, 32);
    }

    bool isRail = MR::isConnectedWithRail(rIter);
    mIsRail = false;

    if (isRail) {
        mIsRail = true;
        initRailRider(rIter);
        if (arg2 == 1) {
            mNumPieces = MR::getRailPointNum(this);
            mPlaceAtPathPoints = true;
        } else {
            mPlaceAtPathPoints = false;
        }
    }

    MR::useStageSwitchAwake(this, rIter);

    if (MR::initUseStageSwitchReadA(this, rIter)) {
        MR::listenStageSwitchOnOffA(this, MR::Functor(this, &StarPieceGroup::onSwitchA), MR::Functor(this, &StarPieceGroup::offSwitchA));
    }

    if (MR::initUseStageSwitchReadB(this, rIter)) {
        MR::listenStageSwitchOnB(this, MR::Functor(this, &StarPieceGroup::onSwitchB));
    }

    MR::initUseStageSwitchWriteDead(this, rIter);
    mPieces = new StarPiece*[mNumPieces];
    mRailCoords = new f32[mNumPieces];

    MR::declareStarPiece(this, mNumPieces);

    for (u32 i = 0; i < mNumPieces; i++) {
        mPieces[i] = new StarPiece("グループスターピース");
        if (isFlow) {
            mPieces[i]->initAndSetRailMoveFromGroup(rIter);
        } else {
            mPieces[i]->initAndSetFloatingFromGroup(rIter);
        }

        if (MR::isExistStageSwitchAppear(rIter)) {
            mPieces[i]->makeActorDead();
        }

        if (MR::isValidSwitchA(this)) {
            mPieces[i]->setCollectionEnabled(false);
        }
    }

    MR::connectToSceneMapObjMovement(this);

    if (isRail) {
        MR::initAndSetRailClipping(&mRailClippingCenter, this, 100.0f, 500.0f);
    }

    MR::validateClipping(this);
    MR::setGroupClipping(this, rIter, 32);
    for (u32 i = 0; i < mNumPieces; i++) {
        MR::invalidateClipping(mPieces[i]);
    }

    if (MR::tryRegisterDemoCast(this, rIter)) {
        for (u32 i = 0; i < mNumPieces; i++) {
            MR::tryRegisterDemoCast(mPieces[i], rIter);
        }
    }

    mEmitAppearEffects = false;
    MR::useStageSwitchSyncAppear(this, rIter);

    mEmitAppearEffects = true;
}

void StarPieceGroup::makeActorAppeared() {
    LiveActor::makeActorAppeared();

    if (mHasAppeared) {
        makeActorDead();
    } else {
        mHasAppeared = true;
        forceReplaceStarPieceAll();
    }
}

void StarPieceGroup::appear() {
    if (!mHasAppeared) {
        MR::startActionSound(this, "OjStarPieceBurst");
    }

    LiveActor::appear();
}

void StarPieceGroup::makeActorDead() {
    LiveActor::makeActorDead();
    if (MR::isStageMarioFaceShip()) {
        forceKillStarPieceAll(false);
    } else {
        forceKillStarPieceAll(true);
    }
}

void StarPieceGroup::control() {
    if (mLifeTimer != -1) {
        MR::invalidateClipping(this);
        mLifeTimer--;
        if (mLifeTimer <= 0) {
            kill();
        }
    }
}

void StarPieceGroup::placementAllPiece() {
    if (!mIsRail) {
        placementPieceOnCircle();
        return;
    }

    if (mPlaceAtPathPoints) {
        placementPieceOnRailPoint();
        return;
    }

    placementPieceOnRail();
}

#pragma push
#pragma opt_propagation off
// NONMATCHING: single-piece position argument register scheduling differs.
void StarPieceGroup::placementPieceOnCircle() {
    if (mNumPieces == 1) {
        MR::setPosition(mPieces[0], mPosition);
        return;
    }

    TPos3f mtxTRS;
    MR::makeMtxTRS(mtxTRS.toMtxPtr(), this);

    TVec3f zDir;
    TVec3f xDir;
    mtxTRS.getXDir(xDir);
    mtxTRS.getZDir(zDir);

    const TVec3f center(*getPosition());
    f32 currentAngle = 0.0f;
    f32 angleBetweenPieces = TWO_PI / mNumPieces;
    for (u32 i = 0; i < mNumPieces; i++) {
        f32 cos = mCircleRadius * MR::cos(currentAngle);
        f32 sin = mCircleRadius * MR::sin(currentAngle);
        mPieces[i]->mPosition = zDir * cos + xDir * sin + center;
        currentAngle += angleBetweenPieces;
    }

    MR::setClippingTypeSphere(this, mCircleRadius);
}
#pragma pop

void StarPieceGroup::placementPieceOnRail() {
    f32 railLength = MR::getRailTotalLength(this);
    f32 railCoord;
    f32 coordDistBetweenPieces;

    if (mNumPieces == 1) {
        coordDistBetweenPieces = 0.0f;
    } else if (MR::isLoopRail(this) == false) {
        coordDistBetweenPieces = railLength / (mNumPieces - 1.0f);
    } else {
        coordDistBetweenPieces = railLength / mNumPieces;
    }

    for (int i = 0; i < mNumPieces; i++) {
        if (i * coordDistBetweenPieces < 0.0f) {
            railCoord = 0.0f;
        } else if (i * coordDistBetweenPieces > railLength) {
            railCoord = railLength;
        } else {
            railCoord = i * coordDistBetweenPieces;
        }

        TVec3f railPos;
        MR::calcRailPosAtCoord(&railPos, this, railCoord);
        MR::resetPosition(mPieces[i], railPos);
        mRailCoords[i] = railCoord;
    }
}

void StarPieceGroup::placementPieceOnRailPoint() {
    s32 numRailPoints = MR::getRailPointNum(this);
    for (int i = 0; i < numRailPoints; i++) {
        TVec3f railPointPos;
        MR::calcRailPointPos(&railPointPos, this, i);
        MR::resetPosition(mPieces[i], railPointPos);
    }
}

void StarPieceGroup::onSwitchA() {
    for (u32 i = 0; i < mNumPieces; i++) {
        mPieces[i]->setCollectionEnabled(true);
    }
}

void StarPieceGroup::offSwitchA() {
    for (u32 i = 0; i < mNumPieces; i++) {
        mPieces[i]->setCollectionEnabled(false);
    }
}

void StarPieceGroup::onSwitchB() {
    for (u32 i = 0; i < mNumPieces; i++) {
        mPieces[i]->setFall();
    }
}

void StarPieceGroup::startClipped() {
    LiveActor::startClipped();
    for (u32 i = 0; i < mNumPieces; i++) {
        if (!(mPieces[i]->isOnRailMove() || mPieces[i]->isFloat())) {
            continue;
        }

        mPieces[i]->startClipped();
    }
}

void StarPieceGroup::endClipped() {
    LiveActor::endClipped();
    for (u32 i = 0; i < mNumPieces; i++) {
        if (!(mPieces[i]->isOnRailMove() || mPieces[i]->isFloat())) {
            continue;
        }

        mPieces[i]->endClipped();
    }
}

void StarPieceGroup::exeFlow() {
    f32 railLength = MR::getRailTotalLength(this);
    for (u32 i = 0; i < mNumPieces; i++) {
        mRailCoords[i] = MR::repeat(mRailCoords[i] + mRailSpeed, 0.0f, railLength);
        if (mPieces[i]->isOnRailMove()) {
            TVec3f railPos;
            MR::calcRailPosAtCoord(&railPos, this, mRailCoords[i]);
            MR::resetPosition(mPieces[i], railPos);
        }
    }
}

void StarPieceGroup::exeGroup() {
}
