#define JGEOMETRY_VEC3_WMR_OUTLINE
#pragma auto_inline off
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Util/TalkUtil.hpp"
#include "Game/System/WorldMapResultStateLeaveHome.hpp"

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
    bool type4 = GameSequenceFunction::isWorldMapEnterType4();
    s32 type = 6;
    if (type4 != 0) {
        type = 5;
    }
    return type;
}

const char* getHomeLeaveEndName() {
    s32 world = GameDataFunction::getCurrentWorldId();
    const char* name = "離脱終了（ホーム）";
    if (world == 7) {
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

// NONMATCHING: regalloc (stack slots, address calculation, or load scheduling).
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
        MR::forceOpenSystemWipeWhiteFade();
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
        const TVec3f* pos = mHost->getRocketPosition();
        mPos.x = pos->x;
        mPos.y = pos->y;
        mPos.z = pos->z;
    }
    if (reinterpret_cast<bool (*)()>(MR::tryEndModelWipe)()) {
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

// NONMATCHING: regalloc (stack slots, address calculation, or load scheduling).
void WorldMapResultStateLeaveHome::calcCenter() {
    WorldMapResultBox box;
    box.extendByWorld(mHost->getWorld());
    mCenter.set<f32>(box.mMax.x, mPos.y, mPos.z);
    mCenter.add(sCameraTable[3]);
}

void WorldMapResultStateLeaveHome::moveCamera(s32 step) {
    TVec3f pos;
    calcRoutePos(&pos, mCenter, mPos, step, 0x12C);
    MR::setPosition(mHost->getCamera()->mCameraTarget, pos);
}

#pragma dont_inline on
void WorldMapResultStateLeaveHome::playEnterSound() {
    MR::startActionSound(mHost->getCamera()->mRocket, "EnterWorld", -1, -1, -1);
}

#pragma dont_inline reset

void WorldMapResultStateLeaveHome::exeMove() {
    mHost->getCamera();
    if (MR::isFirstStep(this)) {
        MR::resetCameraMan();
    }
    moveCamera(getNerveStep());
    if (MR::isGreaterEqualStep(this, 0x14A)) {
        WorldMapOwner* host = mHost;
        host->faceRocketInstant(&TVec3f(1.0f, 0.0f, 0.0f));
        mHost->showRocket();
        setNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeZoomOut));
    }
}

// NONMATCHING: regalloc (stack slots, address calculation, or load scheduling).
void WorldMapResultStateLeaveHome::exeZoomIn() {
    if (MR::isFirstStep(this)) {
        playEnterSound();
        WorldMapOwner* host = mHost;
        host->faceRocketInstant(&TVec3f(1.0f, 0.0f, 0.0f));
        MR::tryForceDeleteEffectAll(mHost->getRocket());
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 0x5A);
    WorldMapOwner* host = mHost;
    TVec3f sp8(mPos);
    sp8.add(sCameraTable[0]);
    TVec3f sp14 = sp8 * (1.0f - rate);
    TVec3f sp20 = mPos * rate;
    TVec3f sp2C(sp20);
    sp2C.add(sp14);
    host->setRocketPosFromPrev(sp2C);
    if (MR::isGreaterEqualStep(this, 0x5A)) {
        kill();
    }
}

// NONMATCHING: regalloc (stack slots, address calculation, or load scheduling).
void WorldMapResultStateLeaveHome::exeZoomOut() {
    if (MR::isFirstStep(this)) {
        playEnterSound();
        WorldMapOwner* host = mHost;
        host->faceRocketInstant(&TVec3f(1.0f, 0.0f, 0.0f));
        MR::tryForceDeleteEffectAll(mHost->getRocket());
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 0x1E);
    WorldMapOwner* host = mHost;
    TVec3f sp8(mPos);
    sp8.add(sCameraTable[1]);
    TVec3f sp14 = sp8 * (1.0f - rate);
    TVec3f sp20 = mPos * rate;
    TVec3f sp2C(sp20);
    sp2C.add(sp14);
    host->setRocketPosFromPrev(sp2C);
    if (MR::isGreaterEqualStep(this, 0x1E)) {
        kill();
    }
}

// NONMATCHING: regalloc (stack slots, address calculation, or load scheduling).
void WorldMapResultStateLeaveHome::exeZoomOutSub() {
    if (MR::isFirstStep(this)) {
        playEnterSound();
        WorldMapOwner* host = mHost;
        host->faceRocketInstant(&TVec3f(-1.0f, 0.0f, 0.0f));
        MR::tryForceDeleteEffectAll(mHost->getRocket());
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 0x1E);
    WorldMapOwner* host = mHost;
    TVec3f sp8(mPos);
    sp8.add(sCameraTable[2]);
    TVec3f sp14 = sp8 * (1.0f - rate);
    TVec3f sp20 = mPos * rate;
    TVec3f sp2C(sp20);
    sp2C.add(sp14);
    host->setRocketPosFromPrev(sp2C);
    if (MR::isGreaterEqualStep(this, 0x1E)) {
        kill();
    }
}

void WorldMapResultStateLeaveHome::exeMessage() {
    if (MR::isFirstStep(this)) {
        WorldMapOwner* host = mHost;
        host->startCamera(getHomeLeaveEndName(), 0x78);
    }
    if (MR::isLessStep(this, 0x32)) {
        WorldMapOwner* host = mHost;
        host->faceRocketInstant(&TVec3f(0.0f, 0.0f, -1.0f));
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
    s32 duration = 30;
    f32 fovy = MR::getEaseOutValue((f32)getNerveStep() / (f32)duration, 179.0f, mFovy, 1.0f);
    MR::setFovy(fovy);
    if (MR::isGreaterEqualStep(this, 0x1E)) {
        kill();
    }
}

void WorldMapResultStateLeaveHome::exeMessageEnd() {
    if (MR::isFirstStep(this)) {
        WorldMapOwner* host = mHost;
        host->startCamera(getHomeLeaveEndName(), 0x78);
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
    void WorldMapResultStateLeaveHomeMessageEnd::execute(Spine* spine) const {
        reinterpret_cast<WorldMapResultStateLeaveHome*>(spine->mExecutor)->exeMessageEnd();
    }
    void WorldMapResultStateLeaveHomeFovOut::execute(Spine* spine) const {
        reinterpret_cast<WorldMapResultStateLeaveHome*>(spine->mExecutor)->exeFovOut();
    }
    void WorldMapResultStateLeaveHomeMessage::execute(Spine* spine) const {
        reinterpret_cast<WorldMapResultStateLeaveHome*>(spine->mExecutor)->exeMessage();
    }
    void WorldMapResultStateLeaveHomeZoomOutSub::execute(Spine* spine) const {
        reinterpret_cast<WorldMapResultStateLeaveHome*>(spine->mExecutor)->exeZoomOutSub();
    }
    void WorldMapResultStateLeaveHomeZoomOut::execute(Spine* spine) const {
        reinterpret_cast<WorldMapResultStateLeaveHome*>(spine->mExecutor)->exeZoomOut();
    }
    void WorldMapResultStateLeaveHomeZoomIn::execute(Spine* spine) const {
        reinterpret_cast<WorldMapResultStateLeaveHome*>(spine->mExecutor)->exeZoomIn();
    }
    void WorldMapResultStateLeaveHomeMove::execute(Spine* spine) const {
        reinterpret_cast<WorldMapResultStateLeaveHome*>(spine->mExecutor)->exeMove();
    }
    void WorldMapResultStateLeaveHomeStart::execute(Spine* spine) const {
        reinterpret_cast<WorldMapResultStateLeaveHome*>(spine->mExecutor)->exeStart();
    }
    WorldMapResultStateLeaveHomeStart WorldMapResultStateLeaveHomeStart::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateLeaveHomeMove WorldMapResultStateLeaveHomeMove::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateLeaveHomeZoomIn WorldMapResultStateLeaveHomeZoomIn::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateLeaveHomeZoomOut WorldMapResultStateLeaveHomeZoomOut::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateLeaveHomeZoomOutSub WorldMapResultStateLeaveHomeZoomOutSub::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateLeaveHomeMessage WorldMapResultStateLeaveHomeMessage::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateLeaveHomeFovOut WorldMapResultStateLeaveHomeFovOut::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateLeaveHomeMessageEnd WorldMapResultStateLeaveHomeMessageEnd::sInstance ATTRIBUTE_WEAK;
};  
// NONMATCHING: regalloc (static initializer base-load scheduling).
TVec3f sCameraTable[4] = {
    TVec3f(-16000.0f, 0.0f, 0.0f),
    TVec3f(-9000.0f, 0.0f, 0.0f),
    TVec3f(9000.0f, 0.0f, 0.0f),
    TVec3f(-35000.0f, 0.0f, 0.0f),
};
