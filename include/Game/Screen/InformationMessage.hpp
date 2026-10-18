#pragma once

#include "Game/Screen/LayoutActor.hpp"

class IconAButton;

class InformationMessage : public LayoutActor {
public:
    InformationMessage(bool isCenter);

    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();
    virtual void calcAnim();
    virtual void draw() const;
    virtual void appear();
    virtual void kill();

    void setAButtonPos();
    void disappear();
    void setMessage(const char* pMessageId);
    void setMessage(const wchar_t* pMessage);
    void setReplaceString(const wchar_t* pString, s32 param2);
    const char* getInfConfirmName() const;
    void exeAppear();
    void exeWait();
    void exeDisappear();

    void setCenter(bool isCenter) {
        mIsCenter = isCenter;
    }

    // core4: accessors used by ScreenUtil (names guessed; PLACEHOLDER(core))
    void setWindowType(s32 windowType) {  // PLACEHOLDER(core): name guessed; writes mWindowType (0x30)
        mWindowType = windowType;
    }
    void setAButtonPosFlag(bool isOn) {  // PLACEHOLDER(core): name guessed; writes _35 (0x35)
        _35 = isOn;
    }

private:
    /* 0x2C */ IconAButton* mAButtonIcon;
    /* 0x30 */ s32 mWindowType;
    /* 0x34 */ bool mIsCenter;
    /* 0x35 */ bool _35;
};
