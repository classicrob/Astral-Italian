#include "Game/Effect/WaterFortressRain.hpp"
#include "Game/Screen/TimeStopScreenEffect.hpp"
#include "Game/Util.hpp"

// Object name passed to initEffectKeeper and initSound.
static const char* sObjectName = "WaterFortressRain";

WaterFortressRain::WaterFortressRain(const char* pName) : LiveActor(pName) {
    mEffectMtx.identity();
}

WaterFortressRain::~WaterFortressRain() {
}

void WaterFortressRain::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    initEffectKeeper(0, sObjectName, false);
    initSound(4, sObjectName, NULL, TVec3f(0.0f));
    initNerve(GET_NERVE(WaterFortressRain, WaterFortressRainNrvRain), 0);
    MR::invalidateClipping(this);
    MR::useStageSwitchSyncAppear(this, rIter);
    MR::makeMtxTR(mEffectMtx, mPosition, mRotation);
    MR::setEffectHostMtx(this, "Rain", mEffectMtx);
    MR::setEffectHostMtx(this, "RainSlow", mEffectMtx);
}

void WaterFortressRain::control() {
    TVec3f* pPlayerPos = MR::getPlayerPos();
    MR::setMtxTrans(mEffectMtx, pPlayerPos->x, pPlayerPos->y, pPlayerPos->z);
}

void WaterFortressRain::exeRain() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Rain");
        MR::forceDeleteEffect(this, "RainSlow");
    }

    if (MR::isOnTimeStopScreenEffect()) {
        setNerve(GET_NERVE(WaterFortressRain, WaterFortressRainNrvRainSlow));
    }
}

void WaterFortressRain::exeRainSlow() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "RainSlow");
        MR::deleteEffect(this, "Rain");
    }

    if (!MR::isOnTimeStopScreenEffect()) {
        setNerve(GET_NERVE(WaterFortressRain, WaterFortressRainNrvRain));
    }
}
