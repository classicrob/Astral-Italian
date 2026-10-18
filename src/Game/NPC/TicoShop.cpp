// NONMATCHING: independent init stores and literal aliases.
#include "Game/NPC/TicoShop.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/MapObj/BenefitItemLifeUp.hpp"
#include "Game/MapObj/BenefitItemOneUp.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const s32 sStarPieceNeed = 30;
    static const f32 sMouthSize = 30.0f;
    static const f32 sSensorSize = 100.0f;
    static const f32 sSensorOffset = 100.0f;
    static const f32 sShadowLength = 500.0f;
};  // namespace

namespace NrvTicoShop {
    NEW_NERVE(TicoShopNrvDemo, TicoShop, Demo);
};  // namespace NrvTicoShop

TicoShop::TicoShop(const char* pName) : TicoEat(pName), mOneUp(), mLifeUp(), mChoseOneUp(), mChoseLifeUp() {
}

void TicoShop::kill() {
    if (mOneUp != nullptr && mChoseOneUp) {
        MR::appearKinokoOneUpPop(mOneUp, getBaseMtx());
    }

    if (mLifeUp != nullptr && mChoseLifeUp) {
        MR::appearKinokoSuperPop(mLifeUp, getBaseMtx());
    }

    NPCActor::kill();
}

void TicoShop::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("TicoShop");
    NPCActorItem item("TicoShop");
    caps.setDefault();
    caps.mMessage=false;
    initialize(rIter, caps, "TicoShop", nullptr, nullptr);
    MR::getNPCItemData(&item, 0);
    equipment(item, false);

    if (_94 != nullptr) {
        MR::startAction(_94, "Shake");
    }

    if (_98 != nullptr) {
        MR::startAction(_98, "Shake");
    }

    s32 starPieceNeed = ::sStarPieceNeed;
    MR::getJMapInfoArg0NoInit(rIter, &starPieceNeed);
    initStarPiece(starPieceNeed);
    initMessage("Common_TicoShop000");
    MR::setMessageArg(mMsgCtrl, _194);
    MR::registerBranchFunc(getMsgCtrl(), TalkMessageFunc(this, &TicoShop::branchFunc));
    MR::registerEventFunc(getMsgCtrl(), TalkMessageFunc(this, &TicoShop::eventFunc));
    MR::setDistanceToTalk(mMsgCtrl, 350.0f);
    _180 = GET_NERVE(TicoShop, TicoShopNrvDemo);
    mOneUp = MR::createKinokoOneUp();
    mLifeUp = MR::createKinokoSuper();
    MR::setShadowDropLength(mOneUp, nullptr, ::sShadowLength);
    if(MR::isExistShadow(mLifeUp,nullptr)) MR::setShadowDropLength(mLifeUp, nullptr, ::sShadowLength);
    MR::startBva(this, "Small0");
    MR::startBrk(this, "Metamorphosis");
    MR::setBrkFrameAndStop(this, 0.0f);
    MR::setClippingFar200m(this);
    bool useTalkCamera=false;
    MR::getJMapInfoArg7NoInit(rIter,&useTalkCamera);
    if(useTalkCamera) MR::initTalkCamera(mMsgCtrl,rIter);
    TicoEat::init(rIter);
    _17C.set(0, 0xE6, 0xFF, 0xFF);
}

bool TicoShop::branchFunc(u32) {
    return hasEnoughStarPiece();
}

bool TicoShop::eventFunc(u32 var) {
    if (var < 3) {
        return TicoEat::eventFunc(var);
    }

    if (var == 3) {
        mChoseLifeUp = true;
    }

    if (var == 4) {
        mChoseOneUp = true;
    }

    return true;
}

void TicoShop::startReactionSound() {
    if (_D8) {
        MR::startActionSound(this, "SmNpcTrampled");
        MR::startActionSound(this, "SvTicoshopTrampled");
    }

    if (isPointingSe()) {
        MR::startDPDHitSound();
        MR::startActionSound(this, "SvTicoshopPoint");
    }

    if (_D9) {
        MR::startActionSound(this, "SmTicoshopSpin");
    }

    if (_DB) {
        MR::startActionSound(this, "SvTicoshopStarPieceHit");
    }
}

void TicoShop::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Demo");
        MR::startActionSound(this, "SmTicofatMetaItem");
        MR::startActionSound(this, "SmTicofatMeta");
    }

    if (MR::isActionEnd(this)) {
        MR::startActionSound(this, "SmMetamorphoseSmoke");
        kill();
    }
}
