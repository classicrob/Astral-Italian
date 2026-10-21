#include "Game/System/WorldMapResultLocal.hpp"

WorldMapResultStateCreateTico::WorldMapResultStateCreateTico(WorldMapResultCommand* pHost)
    : ActorStateBase< WorldMapResultCommand >("State[チコによるルート作成]", pHost) {
}

void WorldMapResultStateCreateTico::init() {
    initNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoClose), 0);
    mIsDead = true;
}

void WorldMapResultStateCreateTico::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoClose));
}

void WorldMapResultStateCreateTico::kill() {
    mIsDead = true;
    mHost->endResult();
}

void WorldMapResultStateCreateTico::exeClose() {
    if (MR::isFirstStep(this)) {
        MR::closeSystemWipeMario(-1);
    }

    if (MR::isSystemWipeBlank()) {
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoOpen));
    }
}

void WorldMapResultStateCreateTico::exeOpen() {
    if (!MR::isLessStep(this, getResultEndStep())) {
        if (MR::isStep(this, getResultEndStep())) {
            mHost->startResult(-1);
            mHost->setupTicoRouteCreate();
            MR::openSystemWipeMario(-1);
            return;
        }

        if (!MR::isSystemWipeActive()) {
            setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoExtend));
        }
    }
}

void WorldMapResultStateCreateTico::exeExtend() {
    WorldMapResultCommand* pHost = mHost;
    f32 rate = MR::calcNerveRate(this, 90);
    updateTicoRoad(pHost, rate);
    MR::startActionSound(mHost->mRocket, "RoadExtend", -1, -1, -1);

    if (MR::isStep(this, 90)) {
        mHost->refreshTicoStartNode();
    }

    if (MR::isGreaterEqualStep(this, 135)) {
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoWhiteClose));
    }
}

void WorldMapResultStateCreateTico::exeWhiteClose() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeWhiteFade(60);
    }

    if (!MR::isWipeActive()) {
        mHost->updateCurrentWorldUnits();
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoWhiteWait));
    }
}

void WorldMapResultStateCreateTico::exeWhiteOpen() {
    if (MR::isFirstStep(this)) {
        MR::openWipeWhiteFade(60);
    }

    if (!MR::isWipeActive()) {
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoMessage));
    }
}

void WorldMapResultStateCreateTico::exeMessage() {
    if (MR::isFirstStep(this)) {
        MR::appearInformationMessageDown("InformationRouteCreateTico", true);
        MR::startSubBGM("BGM_JG_NEW_GALAXY", false);
    }

    if (MR::isDeadInformationMessage()) {
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoCloseEnd));
    }
}

void WorldMapResultStateCreateTico::exeCloseEnd() {
    if (MR::isFirstStep(this)) {
        MR::closeSystemWipeMario(-1);
    }

    if (MR::isSystemWipeBlank()) {
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoEnd));
    }
}

void WorldMapResultStateCreateTico::exeEnd() {
    if (!MR::isLessStep(this, getResultEndStep())) {
        if (MR::isStep(this, getResultEndStep())) {
            MR::openSystemWipeMario(-1);
            kill();
        }
    }
}

WorldMapResultStateCreateTico::~WorldMapResultStateCreateTico() {
}

namespace NrvWorldMapResultStateCreateTico {
    WorldMapResultStateCreateTicoClose WorldMapResultStateCreateTicoClose::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateCreateTicoOpen WorldMapResultStateCreateTicoOpen::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateCreateTicoExtend WorldMapResultStateCreateTicoExtend::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateCreateTicoWhiteClose WorldMapResultStateCreateTicoWhiteClose::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateCreateTicoWhiteWait WorldMapResultStateCreateTicoWhiteWait::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateCreateTicoWhiteOpen WorldMapResultStateCreateTicoWhiteOpen::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateCreateTicoMessage WorldMapResultStateCreateTicoMessage::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateCreateTicoCloseEnd WorldMapResultStateCreateTicoCloseEnd::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateCreateTicoEnd WorldMapResultStateCreateTicoEnd::sInstance ATTRIBUTE_WEAK;

    void WorldMapResultStateCreateTicoEnd::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateCreateTico*>(pSpine->mExecutor)->exeEnd();
    }

    void WorldMapResultStateCreateTicoCloseEnd::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateCreateTico*>(pSpine->mExecutor)->exeCloseEnd();
    }

    void WorldMapResultStateCreateTicoMessage::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateCreateTico*>(pSpine->mExecutor)->exeMessage();
    }

    void WorldMapResultStateCreateTicoWhiteOpen::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateCreateTico*>(pSpine->mExecutor)->exeWhiteOpen();
    }

    void WorldMapResultStateCreateTicoWhiteWait::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateCreateTico*>(pSpine->mExecutor)->exeWhiteWait();
    }

    void WorldMapResultStateCreateTicoWhiteClose::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateCreateTico*>(pSpine->mExecutor)->exeWhiteClose();
    }

    void WorldMapResultStateCreateTicoExtend::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateCreateTico*>(pSpine->mExecutor)->exeExtend();
    }

    void WorldMapResultStateCreateTicoOpen::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateCreateTico*>(pSpine->mExecutor)->exeOpen();
    }

    void WorldMapResultStateCreateTicoClose::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateCreateTico*>(pSpine->mExecutor)->exeClose();
    }
};  // namespace NrvWorldMapResultStateCreateTico
