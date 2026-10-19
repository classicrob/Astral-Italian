#pragma once

#include <revolution.h>

class TalkMessageInfo {
public:
    TalkMessageInfo();

    bool isNormalTalk() const;
    bool isShortTalk() const;
    bool isEventTalk() const;
    bool isGlobalEventTalk() const;
    bool isAutoEventTalk() const;
    bool isComposeTalk() const;
    bool isFlowTalk() const;
    bool isNullTalk() const;
    bool isBalloonCall() const;
    bool isBalloonFix() const;
    bool isBalloonSign() const;
    bool isBalloonInfo() const;
    bool isBalloonIcon() const;
    bool isCameraNormal() const;
    bool isCameraEvent() const;

    u8* mMessageText;
    u16 mCameraSetID;
    s8 mTalkSoundId;
    u8 mCameraType;
    u8 mTalkType;
    u8 mBalloonType;
    s8 mMessageAreaArg;
    s8 mAlreadyReadFlagId;
};
