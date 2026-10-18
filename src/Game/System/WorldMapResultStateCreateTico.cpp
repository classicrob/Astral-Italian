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

    if (fn_80059100()) {
        setNerve(GET_NERVE(WorldMapResultStateCreateTico, WorldMapResultStateCreateTicoOpen));
    }
}

void WorldMapResultStateCreateTico::exeOpen() {
    if (!MR::isLessStep(this, getResultEndStep())) {
        if (MR::isStep(this, getResultEndStep())) {
            mHost->startResult(-1);
            fn_804F0700(mHost);
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
    MR::calcNerveRate(this, 90);
    fn_804F0810(pHost);
    MR::startActionSound(mHost->mRocket, "RoadExtend", -1, -1, -1);

    if (MR::isStep(this, 90)) {
        fn_804F08C0(mHost);
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
        fn_804F0920(mHost);
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

    if (fn_80059100()) {
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
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoClose, WorldMapResultStateCreateTico, Close);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoOpen, WorldMapResultStateCreateTico, Open);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoExtend, WorldMapResultStateCreateTico, Extend);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoWhiteClose, WorldMapResultStateCreateTico, WhiteClose);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoWhiteWait, WorldMapResultStateCreateTico, WhiteWait);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoWhiteOpen, WorldMapResultStateCreateTico, WhiteOpen);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoMessage, WorldMapResultStateCreateTico, Message);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoCloseEnd, WorldMapResultStateCreateTico, CloseEnd);
    WMR_NERVE_DEF(WorldMapResultStateCreateTicoEnd, WorldMapResultStateCreateTico, End);
};  // namespace NrvWorldMapResultStateCreateTico
