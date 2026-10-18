#include "Game/MapObj/PurpleCoinHolder.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/Coin.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"

namespace {
    static const s32 sMaxPurpleCoin = 256;
    // static const s32 sGetIncrementCount = _;
};  // namespace

PurpleCoinHolder::PurpleCoinHolder() : DeriveActorGroup< Coin >("パープルコイン管理", ::sMaxPurpleCoin) {
    MR::createPurpleCoinCounter();
    if (!MR::isStagePurpleCoinsMoreThan100(MR::getCurrentStageName())) {
        MR::declarePowerStarCoin100();
    }
}

void PurpleCoinHolder::initAfterPlacement() {
    MR::validatePurpleCoinCounter();
}

namespace {
    PurpleCoinHolder* getPurpleCoinHolder() {
        return MR::getSceneObj< PurpleCoinHolder >(SceneObj_PurpleCoinHolder);
    }
};  // namespace

namespace MR {
    void createPurpleCoinHolder() {
        MR::createSceneObj(SceneObj_PurpleCoinHolder);
    }

    void addToPurpleCoinHolder(const NameObj* pObj, Coin* pCoin) {
        ::getPurpleCoinHolder()->registerActor(pCoin);
    }
};  // namespace MR
