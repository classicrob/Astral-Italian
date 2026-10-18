#pragma once

#include "Game/NameObj/NameObj.hpp"

class CameraInfo;
class DestroyCounter;
class CounterLayoutController;
class InformationMessage;
class MarioMeter;
class MarioSubMeter;
class MissLayout;
class NoteCounter;
class OneUpBoard;
class PurpleCoinCounter;
class YesNoLayout;

class GameSceneLayoutHolder : public NameObj {
public:
    /// @brief Creates a new `GameSceneLayoutHolder`.
    GameSceneLayoutHolder();

    void createPurpleCoinCounter();
    void createDestroyCounter(s32);
    DestroyCounter* getDestroyCounter() const;
    void appearDestroyCounter();
    void killDestroyCounter();
    PurpleCoinCounter* getPurpleCoinCounter() const;
    void initLifeCount(s32);
    void setLifeCount(u32);
    void setOxygenRatio(f32);
    void setBeePowerRatio(f32);
    void changeLifeMeterModeGround();
    void changeLifeMeterModeWater();
    void changeLifeMeterModeBee();
    void requestOneUp(s32);
    void requestPowerUpHPMeter();
    void requestForceAppearHPMeter();
    void startMiss();
    bool isMissAnimEnd() const;
    void activateDefaultGameLayout();
    void deactivateDefaultGameLayout();

    /* 0x14 */ InformationMessage* mInformationMessage;
    /* 0x18 */ YesNoLayout* mYesNoLayout;
    /* 0x1C */ CameraInfo* mCameraInfo;
    /* 0x20 */ MarioMeter* mMarioMeter;
    /* 0x24 */ MarioSubMeter* mMarioSubMeter;
    /* 0x28 */ MissLayout* mMissLayout;
    /* 0x2C */ OneUpBoard* mOneUpBoard;
    /* 0x30 */ NoteCounter* mNoteCounter;
    /* 0x34 */ CounterLayoutController* mCounterLayoutCtrl;
    /* 0x38 */ PurpleCoinCounter* mPurpleCoinCounter;
    /* 0x3C */ DestroyCounter* mDestroyCounter;
    /* 0x40 */ bool mIsActiveDefaultGameLayout;
};

namespace MR {
    GameSceneLayoutHolder* getGameSceneLayoutHolder();
};  // namespace MR
