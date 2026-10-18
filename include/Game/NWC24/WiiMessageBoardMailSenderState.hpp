#pragma once

#include "Game/NameObj/NameObj.hpp"
#include "Game/System/NerveExecutor.hpp"

class LayoutActor;
class ReceiverTagMail;
class ResourceHolder;

// Sends the Wii Message Board mail and shows the result (SMG2-only). Runs as a NerveExecutor.
class WiiMessageBoardMailSenderState : public NerveExecutor {
public:
    WiiMessageBoardMailSenderState();
    virtual ~WiiMessageBoardMailSenderState();

    void setup(s32, const char*, const char*);
    void exeSend();
    void exeShow();
    void exeEnd();

    /* 0x08 */ ReceiverTagMail* mReceiverTagMail;
    /* 0x0C */ ResourceHolder* mResourceHolder;
    /* 0x10 */ LayoutActor* mSysInfoWindow;
    /* 0x14 */ s32 mMailType;
    /* 0x18 */ const char* mResName;
    /* 0x1C */ const char* mGameEventFlag;
};

class WiiMessageBoardMailSender : public NameObj {
public:
    WiiMessageBoardMailSender();
    virtual ~WiiMessageBoardMailSender();

    /* 0x14 */ WiiMessageBoardMailSenderState* mState;
};
