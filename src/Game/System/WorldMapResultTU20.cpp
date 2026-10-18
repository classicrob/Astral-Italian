#pragma auto_inline off
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Util/TalkUtil.hpp"
#include "Game/System/WorldMapResultStateLeaveHome.hpp"

extern "C" {
    void fn_804F33F0(TVec3f*, const TVec3f*, const TVec3f*, s32, s32);
    bool fn_80059330();
}

extern TVec3f sCameraTable[4];

static const char* sWorldNewsCameraName = "ワールドニュースカメラ(ワールド1初プレイ)";

s32 getHomeLeaveType(WorldMapOwner* pHost) {
    if (GameSequenceFunction::isWorldMapEnterType0() != 0) {
        return 0;
    }
    if (pHost->isWorldEventFlagOff() != 0) {
        return 1;
    }
    if (GameSequenceFunction::isWorldMapEnterType1() != 0) {
        return 2;
    }
    if (GameSequenceFunction::isWorldMapEnterType2() != 0) {
        return 3;
    }
    if (GameSequenceFunction::isWorldMapEnterType3() != 0) {
        return 4;
    }
    s32 type = 6;
    if (GameSequenceFunction::isWorldMapEnterType4() != 0) {
        type = 5;
    }
    return type;
}

const char* getHomeLeaveEndName() {
    const char* name = "離脱終了（ホーム）";
    if (GameDataFunction::getCurrentWorldId() == 7) {
        name = "離脱終了Ｓ（ホーム）";
    }
    return name;
}

WorldMapResultStateLeaveHome::WorldMapResultStateLeaveHome(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[開始]", pHost) {
    mFovy = 0.0f;
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
}

void WorldMapResultStateLeaveHome::init() {
    initNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeStart), 0);
    mHost->declareCamera("離脱開始（ホーム）");
    mHost->declareCamera("離脱終了（ホーム）");
    mHost->declareCamera("離脱終了Ｓ（ホーム）");
    mHost->declareCamera(sWorldNewsCameraName);
    mIsDead = true;
}

void WorldMapResultStateLeaveHome::appear() {
    mHost->startDemoNoCinemaCheck();
    mIsDead = false;
    mHost->changeRocketActionBck("Wait");
    mHost->startNodeCamera(-1, 0);
    MR::deactivateDefaultGameLayout();
    MR::hidePlayer();
    s32 type = getHomeLeaveType(mHost);
    switch (type) {
    case 0:
        MR::forceCloseSystemWipeWhiteFade();
        break;
    case 1:
        mHost->hideRocket();
        MR::forceCloseSystemWipeWhiteFade();
        break;
    case 2:
        MR::forceCloseSystemWipeWhiteFade();
        break;
    case 3:
        MR::forceCloseSystemWipeWhiteFade();
        break;
    case 4:
        MR::forceCloseSystemWipeWhiteFade();
        break;
    case 5:
        MR::forceOpenSystemWipeWhiteFade(type);
        MR::forceCloseSystemWipeFade();
        break;
    case 6:
        MR::forceCloseSystemWipeFade();
        break;
    }
    setNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeStart));
}

void WorldMapResultStateLeaveHome::kill() {
    GameSequenceFunction::setWorldMapEnterType5();
    MR::showPlayer();
    mHost->endDemo();
    mIsDead = true;
}

void WorldMapResultStateLeaveHome::exeStart() {
    if (MR::isFirstStep(this)) {
        mPos = *mHost->getRocketPosition();
    }
    if (fn_80059330()) {
        LiveActor* pActor = mHost->getRocket();
        MR::tryForceDeleteEffectAll(pActor);
        s32 type = getHomeLeaveType(mHost);
        switch (type) {
        case 0:
            MR::openSystemWipeWhiteFade(-1);
            mHost->startCamera(sWorldNewsCameraName, 0);
            setNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeZoomIn));
            return;
        case 1:
            MR::openSystemWipeWhiteFade(0xB4);
            calcCenter();
            moveCamera(0);
            setNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeMove));
            return;
        case 2:
            MR::openSystemWipeWhiteFade(-1);
            mHost->startCamera("離脱開始（ホーム）", 0);
            setNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeMessage));
            return;
        case 3:
            MR::openSystemWipeWhiteFade(-1);
            setNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeZoomOut));
            return;
        case 4:
            MR::openSystemWipeWhiteFade(-1);
            setNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeZoomOutSub));
            return;
        case 5:
            MR::openSystemWipeFade(-1);
            setNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeFovOut));
            return;
        case 6:
            MR::openSystemWipeFade(-1);
            mHost->startCamera("離脱開始（ホーム）", 0);
            setNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeMessageEnd));
            break;
        }
    }
}

void WorldMapResultStateLeaveHome::calcCenter() {
    WorldMapResultBox box;
    box.extendByWorld(mHost->getWorld());
    mCenter.set(box.mMax.x, mPos.y, mPos.z);
    mCenter.add(sCameraTable[3]);
}

void WorldMapResultStateLeaveHome::moveCamera(s32 step) {
    TVec3f pos;
    fn_804F33F0(&pos, &mCenter, &mPos, step, 0x12C);
    MR::setPosition(mHost->getCamera()->mCameraTarget, pos);
}

void WorldMapResultStateLeaveHome::playEnterSound() {
    MR::startActionSound(mHost->getCamera()->mRocket, "EnterWorld", -1, -1, -1);
}

void WorldMapResultStateLeaveHome::exeMove() {
    mHost->getCamera();
    if (MR::isFirstStep(this)) {
        MR::resetCameraMan();
    }
    moveCamera(getNerveStep());
    if (MR::isGreaterEqualStep(this, 0x14A)) {
        TVec3f dir(1.0f, 0.0f, 0.0f);
        mHost->faceRocketInstant(&dir);
        mHost->showRocket();
        setNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeZoomOut));
    }
}

void WorldMapResultStateLeaveHome::exeZoomIn() {
    if (MR::isFirstStep(this)) {
        playEnterSound();
        TVec3f dir(1.0f, 0.0f, 0.0f);
        mHost->faceRocketInstant(&dir);
        MR::tryForceDeleteEffectAll(mHost->getRocket());
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 0x5A);
    TVec3f sp8(mPos);
    sp8.add(sCameraTable[0]);
    TVec3f sp14 = sp8 * (1.0f - rate);
    TVec3f sp20 = mPos * rate;
    TVec3f sp2C(sp20);
    sp2C.add(sp14);
    mHost->setRocketPosFromPrev(sp2C);
    if (MR::isGreaterEqualStep(this, 0x5A)) {
        kill();
    }
}

void WorldMapResultStateLeaveHome::exeZoomOut() {
    if (MR::isFirstStep(this)) {
        playEnterSound();
        TVec3f dir(1.0f, 0.0f, 0.0f);
        mHost->faceRocketInstant(&dir);
        MR::tryForceDeleteEffectAll(mHost->getRocket());
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 0x1E);
    TVec3f sp8(mPos);
    sp8.add(sCameraTable[1]);
    TVec3f sp14 = sp8 * (1.0f - rate);
    TVec3f sp20 = mPos * rate;
    TVec3f sp2C(sp20);
    sp2C.add(sp14);
    mHost->setRocketPosFromPrev(sp2C);
    if (MR::isGreaterEqualStep(this, 0x1E)) {
        kill();
    }
}

void WorldMapResultStateLeaveHome::exeZoomOutSub() {
    if (MR::isFirstStep(this)) {
        playEnterSound();
        TVec3f dir(-1.0f, 0.0f, 0.0f);
        mHost->faceRocketInstant(&dir);
        MR::tryForceDeleteEffectAll(mHost->getRocket());
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 0x1E);
    TVec3f sp8(mPos);
    sp8.add(sCameraTable[2]);
    TVec3f sp14 = sp8 * (1.0f - rate);
    TVec3f sp20 = mPos * rate;
    TVec3f sp2C(sp20);
    sp2C.add(sp14);
    mHost->setRocketPosFromPrev(sp2C);
    if (MR::isGreaterEqualStep(this, 0x1E)) {
        kill();
    }
}

void WorldMapResultStateLeaveHome::exeMessage() {
    if (MR::isFirstStep(this)) {
        mHost->startCamera(getHomeLeaveEndName(), 0x78);
    }
    if (MR::isLessStep(this, 0x32)) {
        TVec3f dir(0.0f, 0.0f, -1.0f);
        mHost->faceRocketInstant(&dir);
    } else {
        mHost->setRocketSpeed(0.08f);
    }
    if (MR::isStep(this, 0x3C)) {
        mHost->startNodeCamera(-1, -1);
    }
    if (MR::isGreaterEqualStep(this, 0x50)) {
        kill();
    }
}

void WorldMapResultStateLeaveHome::exeFovOut() {
    if (MR::isFirstStep(this)) {
        mFovy = MR::getFovy();
    }
    f32 fovy = MR::getEaseOutValue((f32)getNerveStep() / (f32)0x1E, 179.0f, mFovy, 1.0f);
    MR::setFovy(fovy);
    if (MR::isGreaterEqualStep(this, 0x1E)) {
        kill();
    }
}

void WorldMapResultStateLeaveHome::exeMessageEnd() {
    if (MR::isFirstStep(this)) {
        mHost->startCamera(getHomeLeaveEndName(), 0x78);
    }
    if (MR::isStep(this, 0x3C)) {
        mHost->startNodeCamera(-1, -1);
    }
    if (MR::isGreaterEqualStep(this, 0x50)) {
        kill();
    }
}

WorldMapResultStateLeaveHome::~WorldMapResultStateLeaveHome() {
}

namespace NrvWorldMapResultStateLeaveHome {
    WMR_NERVE_DEF(WorldMapResultStateLeaveHomeMessageEnd, WorldMapResultStateLeaveHome, MessageEnd);
    WMR_NERVE_DEF(WorldMapResultStateLeaveHomeFovOut, WorldMapResultStateLeaveHome, FovOut);
    WMR_NERVE_DEF(WorldMapResultStateLeaveHomeMessage, WorldMapResultStateLeaveHome, Message);
    WMR_NERVE_DEF(WorldMapResultStateLeaveHomeZoomOutSub, WorldMapResultStateLeaveHome, ZoomOutSub);
    WMR_NERVE_DEF(WorldMapResultStateLeaveHomeZoomOut, WorldMapResultStateLeaveHome, ZoomOut);
    WMR_NERVE_DEF(WorldMapResultStateLeaveHomeZoomIn, WorldMapResultStateLeaveHome, ZoomIn);
    WMR_NERVE_DEF(WorldMapResultStateLeaveHomeMove, WorldMapResultStateLeaveHome, Move);
    WMR_NERVE_DEF(WorldMapResultStateLeaveHomeStart, WorldMapResultStateLeaveHome, Start);
};  
TVec3f sCameraTable[4] = {
    TVec3f(-16000.0f, 0.0f, 0.0f),
    TVec3f(-9000.0f, 0.0f, 0.0f),
    TVec3f(9000.0f, 0.0f, 0.0f),
    TVec3f(-35000.0f, 0.0f, 0.0f),
};
