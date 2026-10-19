#include "Game/NWC24/ReceiverTagMail.hpp"
#include "Game/Util.hpp"
#include "Game/Map/FileSelectFunc.hpp"
#include "Game/NWC24/NWC24Messenger.hpp"
#include "Game/Screen/ReplaceTagProcessor.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/StringUtil.hpp"

ReceiverTagMail::ReceiverTagMail(const char* pTaskName, const char* pSenderID) : mMessage(nullptr), mIsError(false), mSendComplete(1) {
    mTaskName = pTaskName;
    mSenderID = pSenderID;
    mMessage = new wchar_t[0x200];

    MR::zeroMemory(mMessage, 0x400);
}
void ReceiverTagMail::send(const char* pMessageId, const u8* pImage, u32 imageSize, bool backgroundEnabled) {
    wchar_t timeStr[28];
    MR::makeHoursMinuteSecondString(timeStr, GameDataFunction::getCumulativePlayTime());

    const wchar_t* pUserName;
    if (GameDataFunction::hasCurrentUserMiiId()) {
        pUserName = GameDataFunction::getUserName();
    } else {
        pUserName = MR::getGameMessageDirect("WiiMessage_UserNameDefault");
    }

    ReplaceTagFunction::ReplaceArgs(mMessage, 0x200, MR::getGameMessageDirect(pMessageId), pUserName, timeStr);

    MR::SendMailObj mail(mTaskName);
    mail.setMessageDirect(mMessage);
    mail.setSenderID(mSenderID);
    mail.setImage(pImage, imageSize);
    mail.setSilent();
    if (!backgroundEnabled) {
        mail.setLedOff();
    }
    mail.send();
    mSendComplete = 0;
}

void ReceiverTagMail::update() {
    if (mSendComplete != 0) {
        return;
    }

    if (!MR::isMailSent(mTaskName)) {
        return;
    }

    mIsError = MR::isMailErrorHappened(mTaskName);

    MR::termMail(mTaskName);

    mSendComplete = 1;
}

bool ReceiverTagMail::isDone() const {
    return mSendComplete - 1 == 0;
}

bool ReceiverTagMail::isError() const {
    return mIsError;
}
