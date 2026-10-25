#pragma auto_inline off
#include "Game/System/MiniPartsActors.hpp"
#include "Game/MapObj/StarPieceTracking.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"

MiniStarPieceMine::MiniStarPieceMine(s32 worldId, s32 pointIndex) : LiveActor("スターピース鉱山") {
    mWorldId = worldId;
    mPointIndex = pointIndex;
}

MiniStarPieceMine::~MiniStarPieceMine() {
}

void MiniStarPieceMine::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "MiniStarPieceMine", false);
    initNerve(&NrvMiniStarPieceMine::MiniStarPieceMineNrvWait::sInstance, 0);
    MR::addEffectHitNormal(this, "Hit");
    MR::setEffectBaseScale(this, "Hit", 10.0f);
    MR::declareStarPiece(this, 8);

    mPieces = new NameObj*[8];
    for (s32 idx = 0; idx < 8; idx++) {
        NameObj* pPiece = new StarPieceTrackingPiece("追尾スターピース");
        mPieces[idx] = pPiece;
        mPieces[idx]->initWithoutIter();
    }

    if (isGalaxyFlag()) {
        setNerve(&NrvMiniStarPieceMine::MiniStarPieceMineNrvBreak::sInstance);
        makeActorDead();
        return;
    }

    makeActorAppeared();
}

void MiniStarPieceMine::makeActorAppeared() {
    if (isNerve(&NrvMiniStarPieceMine::MiniStarPieceMineNrvWait::sInstance)) {
        LiveActor::makeActorAppeared();
    }
}

void MiniStarPieceMine::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        return;
    }

    if (!isNerve(&NrvMiniStarPieceMine::MiniStarPieceMineNrvWait::sInstance)) {
        return;
    }

    if (!MR::isSensorRide(pReceiver)) {
        return;
    }

    f32 one;
    f32 zero = 0.0f;
    one = 1.0f;
    for (s32 idx = 0; idx < 8; idx++) {
        TVec3f up(zero, one, zero);
        static_cast<StarPieceTrackingPiece*>(mPieces[idx])->appearScattered(mPosition, up, 10.0f, 100.0f);
    }

    setGalaxyFlag();
    setNerve(&NrvMiniStarPieceMine::MiniStarPieceMineNrvBreak::sInstance);
    MR::startActionSound(this, "StarPieceBreak", -1, -1, -1);
    MR::emitEffectHit(this, mPosition, "Hit");
    MR::emitEffect(this, "Break");
    MR::shakeCameraNormal();
    MR::tryRumblePadMiddle(this, 0);
    MR::stopSceneForDefaultHit(5);
    kill();
}

void MiniStarPieceMine::setEffectDrawOrder(s32 order) {
    MR::setAllEffectDrawOrder(this, order);
}

namespace NrvMiniStarPieceMine {
    inline void MiniStarPieceMineNrvWait::execute(Spine* pSpine) const {
    }

    inline void MiniStarPieceMineNrvBreak::execute(Spine* pSpine) const {
    }

    MiniStarPieceMineNrvWait MiniStarPieceMineNrvWait::sInstance ATTRIBUTE_WEAK;
    MiniStarPieceMineNrvBreak MiniStarPieceMineNrvBreak::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvMiniStarPieceMine
