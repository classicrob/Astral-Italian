#pragma once

#include <revolution.h>

class ActionBgmCtrl;
class ActionFlagCtrl;
class ActionFootPrintCtrl;
class ActionScreenBlurCtrl;
class ActorAnimKeeper;
class ActorHitReactionCtrl;
class ActorItemGenerator;
class ActorPadAndCameraCtrl;
class ActorParam;
class LiveActor;

class ActorActionKeeper {
public:
    ActorActionKeeper(LiveActor*, ActorAnimKeeper*, ActionFlagCtrl*, ActorPadAndCameraCtrl*, ActorHitReactionCtrl*, ActorItemGenerator*,
                      ActorParam*, ActionScreenBlurCtrl*, ActionFootPrintCtrl*, ActionBgmCtrl*);

    static ActorActionKeeper* tryCreate(LiveActor*, const char*);
    bool startAction(const char*);
    void initFlagCtrl();
    void updateAnimKeeper();
    void update();

    /* 0x00 */ LiveActor* mActor;
    /* 0x04 */ ActorAnimKeeper* mAnimKeeper;
    /* 0x08 */ ActionFlagCtrl* mFlagCtrl;
    /* 0x0C */ ActorPadAndCameraCtrl* mPadAndCameraCtrl;
    /* 0x10 */ ActorHitReactionCtrl* mHitReactionCtrl;
    /* 0x14 */ ActorItemGenerator* mItemGenerator;
    /* 0x18 */ ActorParam* mActorParam;
    /* 0x1C */ ActionScreenBlurCtrl* mScreenBlurCtrl;
    /* 0x20 */ ActionFootPrintCtrl* mFootPrintCtrl;
    /* 0x24 */ ActionBgmCtrl* mBgmCtrl;
};
