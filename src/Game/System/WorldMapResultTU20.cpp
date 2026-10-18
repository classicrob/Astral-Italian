#pragma auto_inline off
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Util/TalkUtil.hpp"

/// Actor pair of the result map's home part (actor at 0x50 and 0x54).
struct WorldMapResultHomeActors {
    u8 _0[0x50];
    LiveActor* mActor50;
    LiveActor* mActor54;
};

extern "C" {
    void fn_804E87F0(WorldMapResultCommand*);
    void fn_804F75C0(WorldMapResultCommand*, const char*);
    void fn_804F82C0(WorldMapResultCommand*, s32, s32);
    void fn_804F7690(WorldMapResultCommand*, const TVec3f&);
    void fn_804F76D0(WorldMapResultCommand*, const TVec3f&);
    void fn_804F77A0(WorldMapResultCommand*, f32);
    void fn_804F7F50(WorldMapResultCommand*);
    void fn_804F7F00(WorldMapResultCommand*);
    void fn_804F83E0(WorldMapResultCommand*, const char*, s32);
    void fn_804F8380(WorldMapResultCommand*, const char*);
    void fn_804F33F0(TVec3f*, const TVec3f*, const TVec3f*, s32, s32);
    void fn_804E8850(WorldMapResultCommand*);
    LiveActor* fn_804E8470(WorldMapResultCommand*);
    TVec3f* fn_804E8510(WorldMapResultCommand*);
    WorldMapResultHomeActors* fn_804E8300(WorldMapResultCommand*);
    WorldMapResultWorld* fn_804F60C0(WorldMapResultCommand*);
    bool fn_80059330();
    s32 fn_804E8CA0(WorldMapResultCommand*);
}

extern TVec3f sCameraTable[4];

/// World news camera name ("ワールドニュースカメラ(ワールド1初プレイ)").
static const char* sWorldNewsCameraName = "ワールドニュースカメラ(ワールド1初プレイ)";

/// Result of the home leave: 0 = none, 1..6 = the variant used by the leave states.
s32 getHomeLeaveType(WorldMapResultCommand* pHost) {
    if (GameSequenceFunction::isWorldMapEnterType0() != 0) {
        return 0;
    }
    if (fn_804E8CA0(pHost) != 0) {
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

/// Message shown at the end of the leave ("離脱終了Ｓ（ホーム）" on world 7, "離脱終了（ホーム）" otherwise).
const char* getHomeLeaveEndName() {
    const char* name = "離脱終了（ホーム）";
    if (GameDataFunction::getCurrentWorldId() == 7) {
        name = "離脱終了Ｓ（ホーム）";
    }
    return name;
}

/// Home leave state ("State[開始]"): camera work and the leave messages of returning to the home.
WMR_STATE_DECL(WorldMapResultStateLeaveHome, void exeStart(); void exeMove(); void exeZoomIn(); void exeZoomOut(); void exeZoomOutSub();
               void exeMessage(); void exeFovOut(); void exeMessageEnd(); void moveCamera(s32); void calcCenter(); void playEnterSound();
               /* 0x10 */ f32 mFovy; /* 0x14 */ TVec3f mPos; /* 0x20 */ TVec3f mCenter;);

namespace NrvWorldMapResultStateLeaveHome {
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeStart);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeMove);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeZoomIn);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeZoomOut);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeZoomOutSub);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeMessage);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeFovOut);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeMessageEnd);
};  // namespace NrvWorldMapResultStateLeaveHome

WorldMapResultStateLeaveHome::WorldMapResultStateLeaveHome(WorldMapResultCommand* pHost) : ActorStateBase< WorldMapResultCommand >("State[開始]", pHost) {
    mFovy = 0.0f;
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
}

void WorldMapResultStateLeaveHome::init() {
    initNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeStart), 0);
    fn_804F8380(mHost, "離脱開始（ホーム）");
    fn_804F8380(mHost, "離脱終了（ホーム）");
    fn_804F8380(mHost, "離脱終了Ｓ（ホーム）");
    fn_804F8380(mHost, sWorldNewsCameraName);
    mIsDead = true;
}

void WorldMapResultStateLeaveHome::appear() {
    fn_804E87F0(mHost);
    mIsDead = false;
    fn_804F75C0(mHost, "Wait");
    fn_804F82C0(mHost, -1, 0);
    MR::deactivateDefaultGameLayout();
    MR::hidePlayer();
    s32 type = getHomeLeaveType(mHost);
    switch (type) {
    case 0:
        MR::forceCloseSystemWipeWhiteFade();
        break;
    case 1:
        fn_804F7F00(mHost);
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
    fn_804E8850(mHost);
    mIsDead = true;
}

void WorldMapResultStateLeaveHome::exeStart() {
    if (MR::isFirstStep(this)) {
        mPos = *fn_804E8510(mHost);
    }
    if (fn_80059330()) {
        LiveActor* pActor = fn_804E8470(mHost);
        MR::tryForceDeleteEffectAll(pActor);
        s32 type = getHomeLeaveType(mHost);
        switch (type) {
        case 0:
            MR::openSystemWipeWhiteFade(-1);
            fn_804F83E0(mHost, sWorldNewsCameraName, 0);
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
            fn_804F83E0(mHost, "離脱開始（ホーム）", 0);
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
            fn_804F83E0(mHost, "離脱開始（ホーム）", 0);
            setNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeMessageEnd));
            break;
        }
    }
}

void WorldMapResultStateLeaveHome::calcCenter() {
    WorldMapResultBox box;
    box.extendByWorld(fn_804F60C0(mHost));
    mCenter.set(box.mMax.x, mPos.y, mPos.z);
    mCenter.add(sCameraTable[3]);
}

void WorldMapResultStateLeaveHome::moveCamera(s32 step) {
    TVec3f pos;
    fn_804F33F0(&pos, &mCenter, &mPos, step, 0x12C);
    MR::setPosition(fn_804E8300(mHost)->mActor54, pos);
}

void WorldMapResultStateLeaveHome::playEnterSound() {
    MR::startActionSound(fn_804E8300(mHost)->mActor50, "EnterWorld", -1, -1, -1);
}

void WorldMapResultStateLeaveHome::exeMove() {
    fn_804E8300(mHost);
    if (MR::isFirstStep(this)) {
        MR::resetCameraMan();
    }
    moveCamera(getNerveStep());
    if (MR::isGreaterEqualStep(this, 0x14A)) {
        fn_804F7690(mHost, TVec3f(1.0f, 0.0f, 0.0f));
        fn_804F7F50(mHost);
        setNerve(GET_NERVE(WorldMapResultStateLeaveHome, WorldMapResultStateLeaveHomeZoomOut));
    }
}

void WorldMapResultStateLeaveHome::exeZoomIn() {
    if (MR::isFirstStep(this)) {
        playEnterSound();
        fn_804F7690(mHost, TVec3f(1.0f, 0.0f, 0.0f));
        MR::tryForceDeleteEffectAll(fn_804E8470(mHost));
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 0x5A);
    TVec3f sp8(mPos);
    sp8.add(sCameraTable[0]);
    TVec3f sp14 = sp8 * (1.0f - rate);
    TVec3f sp20 = mPos * rate;
    TVec3f sp2C(sp20);
    sp2C.add(sp14);
    fn_804F76D0(mHost, sp2C);
    if (MR::isGreaterEqualStep(this, 0x5A)) {
        kill();
    }
}

void WorldMapResultStateLeaveHome::exeZoomOut() {
    if (MR::isFirstStep(this)) {
        playEnterSound();
        fn_804F7690(mHost, TVec3f(1.0f, 0.0f, 0.0f));
        MR::tryForceDeleteEffectAll(fn_804E8470(mHost));
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 0x1E);
    TVec3f sp8(mPos);
    sp8.add(sCameraTable[1]);
    TVec3f sp14 = sp8 * (1.0f - rate);
    TVec3f sp20 = mPos * rate;
    TVec3f sp2C(sp20);
    sp2C.add(sp14);
    fn_804F76D0(mHost, sp2C);
    if (MR::isGreaterEqualStep(this, 0x1E)) {
        kill();
    }
}

void WorldMapResultStateLeaveHome::exeZoomOutSub() {
    if (MR::isFirstStep(this)) {
        playEnterSound();
        fn_804F7690(mHost, TVec3f(-1.0f, 0.0f, 0.0f));
        MR::tryForceDeleteEffectAll(fn_804E8470(mHost));
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 0x1E);
    TVec3f sp8(mPos);
    sp8.add(sCameraTable[2]);
    TVec3f sp14 = sp8 * (1.0f - rate);
    TVec3f sp20 = mPos * rate;
    TVec3f sp2C(sp20);
    sp2C.add(sp14);
    fn_804F76D0(mHost, sp2C);
    if (MR::isGreaterEqualStep(this, 0x1E)) {
        kill();
    }
}

void WorldMapResultStateLeaveHome::exeMessage() {
    if (MR::isFirstStep(this)) {
        fn_804F83E0(mHost, getHomeLeaveEndName(), 0x78);
    }
    if (MR::isLessStep(this, 0x32)) {
        fn_804F7690(mHost, TVec3f(0.0f, 0.0f, -1.0f));
    } else {
        fn_804F77A0(mHost, 0.08f);
    }
    if (MR::isStep(this, 0x3C)) {
        fn_804F82C0(mHost, -1, -1);
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
        fn_804F83E0(mHost, getHomeLeaveEndName(), 0x78);
    }
    if (MR::isStep(this, 0x3C)) {
        fn_804F82C0(mHost, -1, -1);
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
};  // namespace NrvWorldMapResultStateLeaveHome

TVec3f sCameraTable[4] = {
    TVec3f(-16000.0f, 0.0f, 0.0f),
    TVec3f(-9000.0f, 0.0f, 0.0f),
    TVec3f(9000.0f, 0.0f, 0.0f),
    TVec3f(-35000.0f, 0.0f, 0.0f),
};
