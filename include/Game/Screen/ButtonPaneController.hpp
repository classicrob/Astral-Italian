#pragma once

#include "Game/System/NerveExecutor.hpp"

class LayoutActor;
class Nerve;
class J3DFrameCtrl;

class ButtonPaneController : public NerveExecutor {
public:
    /// @brief Creates a new `ButtonPaneController`.
    ButtonPaneController(LayoutActor* pHost, const char* pPaneName, const char* pBoundingPaneName, u32 animIndex, bool);

    void setPointingAnimIndex(u32);
    bool trySelectOnPointing();
    bool trySelectWithPad(bool (*)());
    bool isSelectedRepeat() const;
    void update();
    void appear();
    void disappear();
    bool trySelect();
    bool trySelectImmediately();
    bool isHidden() const;
    bool isPointing() const;
    bool isPointingTrigger() const;
    bool isTimingForSelectedSe() const;
    void invalidateDecide();
    void invalidateAppearance();
    bool isAppearing() const;
    bool isDisappearing() const;
    void forceToWait();
    void forceToHide();
    bool isFirstStepWait() const;
    bool isWait() const;
    bool isDecidedWait() const;
    bool onPointing();
    void decide();
    bool isPointingPaneDirect() const;
    bool startAnimAtFirstStep(const char*);
    bool startPointingAnimAtFirstStep(const char*);
    void setNerveAtAnimStopped(const Nerve*);
    bool isPointingPane() const;
    bool tryPointing(bool);
    bool tryNotPointing(bool);
    f32 calcPointingAnimStartFrame() const;
    void exeHidden();
    void exeAppear();
    void exeWait();
    void exePointing();
    void exeNotPointing();
    void exeDecided();
    void exeDecidedWait();
    void exeDecidedRepeat();
    void exeDecidedRepeatWithPad();
    void exeDecidedToDisappear();
    void exeDisappear();

    /* 0x08 */ LayoutActor* mHost;
    /* 0x0C */ const char* mPaneName;
    /* 0x10 */ const char* mBoundingPaneName;
    /* 0x14 */ u32 mAnimIndex;
    /* 0x18 */ u32 mOffAnimIndex;
    /* 0x1C */ bool mDecidedToDisappear;
    /* 0x20 */ f32 mPointingAnimStartFrame;
    /* 0x24 */ bool mIsSelected;
    /* 0x25 */ bool mIsPointing;
    /* 0x26 */ bool mDisappearOnDecide;
    /* 0x27 */ bool mIsPointingExternal;
    /* 0x28 */ bool mIsActive;
    /* 0x29 */ bool mIsDisabled;
    /* 0x2C */ bool (*mRepeatPadTest)();
    /* 0x30 */ const char* mAppearAnimName;
    /* 0x34 */ const char* mWaitAnimName;
    /* 0x38 */ const char* mDecideAnimName;
    /* 0x3C */ const char* mPointingAnimName;
    /* 0x40 */ const char* mNotPointingAnimName;
    /* 0x44 */ const char* mDisappearAnimName;
    /* 0x48 */ const char* mOffAnimName;
    /* 0x4C */ J3DFrameCtrl* mFrameCtrl;
};
