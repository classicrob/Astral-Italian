#include "Game/NPC/TicoComet.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/AstroDemoFunction.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/Screen/GalaxyMapController.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/JMath/JMath.hpp>

namespace NrvTicoEat {
    NEW_NERVE(TicoEatNrvEatPre, TicoEat, EatPre);
    NEW_NERVE(TicoEatNrvEatNow, TicoEat, EatNow);
    NEW_NERVE(TicoEatNrvEatPst, TicoEat, EatPst);
    NEW_NERVE(TicoEatNrvEatEnd, TicoEat, EatEnd);
    NEW_NERVE(TicoEatNrvReaction, TicoEat, Reaction);
};  // namespace NrvTicoEat

namespace {
    class TicoEatParam : public AnimScaleParam {
    public:
        TicoEatParam();
    };

    TicoEatParam::TicoEatParam() : AnimScaleParam() {
        _20 = 0.12f;
        _24 = 0.12f;
        _28 = 0.9f;
    }

    static TicoEatParam sParam;
};  // namespace

TicoEat::TicoEat(const char* pName) : Tico(pName) {
    _190 = 0;
    _194 = 0;
    _198 = 0;
}

void TicoEat::init(const JMapInfoIter& rIter) {
    mScaleController = new AnimScaleController(&::sParam);
    mReactionNerve = GET_NERVE(TicoEat, TicoEatNrvReaction);
    mBodyJointMtx = MR::getJointMtx(this, "Center");
    mParam.setMoveAction("Wait", "Wait");
    mParam.setTalkAction("Talk", "Talk");
    mParam._4 = 3000.0f;
    setDefaults();
    _12C = 1000.0f;
}

void TicoEat::control() {
    if (MR::isBckPlaying(this, "Joy2")) {
        MR::startActionSound(this, "SmLvTicofatGlad", 0);
    }

    Tico::control();
}

bool TicoEat::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceGift(msg)) {
        if (_198 == _194) {
            MR::startAction(this, "Eat0Normal");
            MR::startBva(this, "Big0");
        }

        f32 val = (0.66f * _194);
        if (_198 == (s32)val) {
            f32 frame = MR::getBckFrame(this);
            MR::startAction(this, "Eat1Normal");
            MR::startBva(this, "Big1");
            MR::setBckFrame(this, frame);
            mScaleController->startHitReaction();
        }

        val = (0.33f * _194);
        if (_198 == (s32)val) {
            f32 frame = MR::getBckFrame(this);
            MR::startAction(this, "Eat2Normal");
            MR::startBva(this, "Big2");
            MR::setBckFrame(this, frame);
            mScaleController->startHitReaction();
        }

        _198 -= MR::getStarPieceGiftCount(msg);
        return true;
    }

    return NPCActor::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

void TicoEat::initStarPiece(s32 num) {
    _194 = num;
    _198 = num;
    MR::declareStarPieceReceiver(this, num);
}

void TicoEat::resetStarPiece() {
    MR::clearGotCountStarPieceReceiver(this);
    _198 = _194;
}

bool TicoEat::tryEat() {
    if (isEmptyNerve()) {
        pushNerve(GET_NERVE(TicoEat, TicoEatNrvEatPre));
    }

    if (isNerve(GET_NERVE(TicoEat, TicoEatNrvEatEnd))) {
        popNerve();
        return true;
    }

    return false;
}

bool TicoEat::hasEnoughStarPiece() const {
    return MR::getStarPieceNum() >= _194;
}

bool TicoEat::eventFunc(u32 event) {
    if (event == 0) {
        return tryEat();
    }

    if (event == 1) {
        MR::forceAppearStarPieceCounterForTicoFat();
    }

    if (event == 2) {
        MR::forceSyncStarPieceCounter();
        MR::disappearStarPieceCounterForTicoFat();
    }

    return true;
}

void TicoEat::exeReaction() {
    if (MR::isFirstStep(this) && (_D8 || _DB)) {
        mScaleController->startHitReaction();
    }

    startReactionSound();

    if (MR::tryStartReactionAndPopNerve(this)) {
        return;
    }
}

void TicoEat::exeEatPre() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Excite");
        MR::startBva(this, "Big0");
    }

    if (MR::faceToVector(&_A0, -MR::getCamZdir(), mParam._8)) {
        popAndPushNerve(GET_NERVE(TicoEat, TicoEatNrvEatNow));
    }
}

void TicoEat::exeEatNow() {
    if (MR::isFirstStep(this)) {
    }

    if (_198 != 0 && MR::isIntervalStep(this, 3)) {
        MR::giftStarPieceToTarget(getSensor("Mouth"), _194 / 100 + 1);
    }

    MR::startActionSound(this, "SmLvTicofatAbsorb");
    MR::startActionSound(this, "SmLvTicofatEating");
    MR::startActionSound(this, "SmLvTicofatEating", 0);

    if (_198 == 0) {
        popAndPushNerve(GET_NERVE(TicoEat, TicoEatNrvEatPst));
    }
}

void TicoEat::exeEatPst() {
    if (MR::isGreaterEqualStep(this, 4)) {
        mParam._1C = "Joy2";
        mParam._20 = "Joy2";
        MR::startAction(this, "Joy2");
        popAndPushNerve(GET_NERVE(TicoEat, TicoEatNrvEatEnd));
    }
}

void TicoEat::exeEatEnd() {
}


void TicoEat::startReactionSound() {}
