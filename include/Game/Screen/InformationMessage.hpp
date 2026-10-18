#pragma once

#include "Game/Screen/LayoutActor.hpp"

class IconAButton;

class InformationMessage : public LayoutActor {
public:
    InformationMessage(bool isCenter);

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();

    void appearWithButtonLayout();
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

private:
    /* 0x2C */ IconAButton* mAButtonIcon;
    /* 0x30 */ s32 mWindowType;
    /* 0x34 */ bool mIsCenter;
    /* 0x35 */ bool _35;
};
