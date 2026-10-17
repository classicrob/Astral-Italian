#include "Game/Boss/OtaKingMagma.hpp"
#include "Game/AudioLib/AudAnmSoundObject.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"

namespace NrvOtaKingMagma {
    NEW_NERVE(OtaKingMagmaNrvWait, OtaKingMagma, Wait);
    NEW_NERVE(OtaKingMagmaNrvAppearDemo, OtaKingMagma, AppearDemo);
    NEW_NERVE(OtaKingMagmaNrvAttack, OtaKingMagma, Attack);
    NEW_NERVE(OtaKingMagmaNrvFireAttack, OtaKingMagma, FireAttack);
    NEW_NERVE(OtaKingMagmaNrvHitBack, OtaKingMagma, HitBack);
    NEW_NERVE(OtaKingMagmaNrvDamage, OtaKingMagma, Damage);
    NEW_NERVE(OtaKingMagmaNrvDown, OtaKingMagma, Down);
    NEW_NERVE(OtaKingMagmaNrvEndWait, OtaKingMagma, EndWait);
};  // namespace NrvOtaKingMagma

namespace {
    const Vec cBloomModelOffset = {0.0, 10.0f, 0.0f};
};  // namespace

OtaKingMagma::OtaKingMagma(LiveActor* pHost, s32 drawBufferType)
    : PartsModel(pHost, "オタキングマグマ", "OtaKingMagma", nullptr, drawBufferType, true), mBloomModel(), _A0(), _A4() {
    _A0 = MR::initDLMakerProjmapEffectMtxSetter(this);
    MR::newDifferedDLBuffer(this);
}

void OtaKingMagma::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::startBtk(this, "OtaKingMagma");
    initHitSensor(1);
    MR::initCollisionParts(this, "OtaKingMagma", MR::addBodyMessageSensorEnemy(this), nullptr);
    initNerve(GET_NERVE(OtaKingMagma, OtaKingMagmaNrvWait));
    PartsModel::init(rIter);

    mBloomModel = new PartsModel(mHost, "オタキングマグマブルーム", "OtaKingMagmaBloom", nullptr, 31, true);
    _A4 = MR::initDLMakerProjmapEffectMtxSetter(mBloomModel);
    MR::newDifferedDLBuffer(mBloomModel);
    mBloomModel->mPosition.set(mPosition);
    TVec3f& rBloomPosition = mBloomModel->mPosition;
    rBloomPosition.add(::cBloomModelOffset);
    mBloomModel->mRotation.set(mRotation);
    mBloomModel->mScale.set(mScale);
    mBloomModel->initWithoutIter();
    MR::startBtk(mBloomModel, "OtaKingMagmaBloom");
    _A0->updateMtxUseBaseMtx();
    _A4->updateMtxUseBaseMtx();
    MR::registerDemoSimpleCastAll(this);
}

void OtaKingMagma::appearDemo() {
    setNerve(GET_NERVE(OtaKingMagma, OtaKingMagmaNrvAppearDemo));
}

void OtaKingMagma::attack() {
    setNerve(GET_NERVE(OtaKingMagma, OtaKingMagmaNrvAttack));
}

void OtaKingMagma::fireAttack() {
    setNerve(GET_NERVE(OtaKingMagma, OtaKingMagmaNrvFireAttack));
}

void OtaKingMagma::hitBack() {
    setNerve(GET_NERVE(OtaKingMagma, OtaKingMagmaNrvHitBack));
}

void OtaKingMagma::damage() {
    setNerve(GET_NERVE(OtaKingMagma, OtaKingMagmaNrvDamage));
}

void OtaKingMagma::down() {
    setNerve(GET_NERVE(OtaKingMagma, OtaKingMagmaNrvDown));
}

void OtaKingMagma::wait() {
    MR::startBckNoInterpole(this, "Wait");
    MR::startBckNoInterpole(mBloomModel, "Wait");
    setNerve(GET_NERVE(OtaKingMagma, OtaKingMagmaNrvWait));
}

void OtaKingMagma::endWait() {
    MR::startBckNoInterpole(this, "EndWait");
    MR::startBckNoInterpole(mBloomModel, "EndWait");
    setNerve(GET_NERVE(OtaKingMagma, OtaKingMagmaNrvEndWait));
}

void OtaKingMagma::control() {
    mBloomModel->mPosition.set(mPosition);
    TVec3f& rBloomPosition = mBloomModel->mPosition;
    rBloomPosition.add(::cBloomModelOffset);
}

void OtaKingMagma::startBckThisAndBloomModel(const char* pBckName) {
    MR::startAction(this, pBckName);
    MR::startAction(mBloomModel, pBckName);
}

void OtaKingMagma::exeWait() {
    if (MR::isFirstStep(this)) {
        startBckThisAndBloomModel("Wait");
        MR::emitEffect(this, "Steam");
        MR::emitEffect(this, "BloomMagma");
    }

    MR::startActionSound(this, "AtLvMagmaWind", -1, -1, -1);
}

void OtaKingMagma::exeAppearDemo() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "BmOtakingMagmaDamage");
        startBckThisAndBloomModel("Down");
    }

    MR::startActionSound(this, "AtLvMagmaWind", -1, -1, -1);
    MR::setNerveAtBckStopped(this, GET_NERVE(OtaKingMagma, OtaKingMagmaNrvWait));
}

void OtaKingMagma::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "BmOtakingMagmaAttack");
        startBckThisAndBloomModel("Attack");
    }

    MR::startActionSound(this, "AtLvMagmaWind", -1, -1, -1);
    MR::setNerveAtBckStopped(this, GET_NERVE(OtaKingMagma, OtaKingMagmaNrvWait));
}

void OtaKingMagma::exeFireAttack() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "BmOtakingMagmaAttack");
        startBckThisAndBloomModel("FireAttack");
    }

    MR::startActionSound(this, "AtLvMagmaWind", -1, -1, -1);
    MR::setNerveAtBckStopped(this, GET_NERVE(OtaKingMagma, OtaKingMagmaNrvWait));
}

void OtaKingMagma::exeHitBack() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "BmOtakingMagmaHitback");
        startBckThisAndBloomModel("HitBack");
    }

    if (mHost != nullptr) {
        MR::startActionSound(this, "AtLvMagmaWind", -1, -1, -1);
    }

    MR::setNerveAtBckStopped(this, GET_NERVE(OtaKingMagma, OtaKingMagmaNrvWait));
}

void OtaKingMagma::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "BmOtakingMagmaDamage");
        startBckThisAndBloomModel("Damage");
    }

    MR::startActionSound(this, "AtLvMagmaWind", -1, -1, -1);
    MR::setNerveAtBckStopped(this, GET_NERVE(OtaKingMagma, OtaKingMagmaNrvWait));
}

void OtaKingMagma::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "BmOtakingMagmaDamage");
        startBckThisAndBloomModel("Down");
    }

    MR::startActionSound(this, "AtLvMagmaWind", -1, -1, -1);
    MR::setNerveAtBckStopped(this, GET_NERVE(OtaKingMagma, OtaKingMagmaNrvEndWait));
}

void OtaKingMagma::exeEndWait() {
    if (MR::isFirstStep(this)) {
        startBckThisAndBloomModel("EndWait");
    }

    MR::startActionSound(this, "AtLvMagmaWind", -1, -1, -1);
}
