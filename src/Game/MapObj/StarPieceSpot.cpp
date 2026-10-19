#include "Game/MapObj/StarPieceSpot.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const f32 sLaunchRandomSpeed = 15.0f;
    static const f32 sLaunchVerticalSpeed = 40.0f;
};  // namespace

StarPieceSpot::StarPieceSpot(const char* pName) : LiveActor(pName) {
}

// Director host accounting prevents a repeated trigger from exceeding the declared supply.
void StarPieceSpot::emit() {
    MR::appearStarPiece(this, mPosition, MR::getDeclareRemnantStarPieceCount(this), ::sLaunchRandomSpeed, ::sLaunchVerticalSpeed, false);
}

void StarPieceSpot::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);

    s32 pieceCount = 5;
    MR::getJMapInfoArg0NoInit(rIter, &pieceCount);
    MR::declareStarPiece(this, pieceCount);

    initSound(1, "StarPieceSpot", nullptr, TVec3f(0.0f));

    if (MR::initUseStageSwitchReadAppear(this, rIter)) {
        MR::listenStageSwitchOnAppear(this, MR::FunctorV0M< StarPieceSpot*, void (StarPieceSpot::*)() >(this, &StarPieceSpot::emit));
        kill();
    } else {
        initHitSensor(1);
        MR::addHitSensorMapObjSimple(this, "body", 8, 150.0f, TVec3f(0.0f, 0.0f, 0.0f));
        appear();
    }
}

bool StarPieceSpot::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        emit();
        kill();
        return false;
    } else {
        return false;
    }
}
