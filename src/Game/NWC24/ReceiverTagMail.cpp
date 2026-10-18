#include "Game/NWC24/ReceiverTagMail.hpp"
#include "Game/Util.hpp"
#include "Game/Map/FileSelectFunc.hpp"
#include "Game/NWC24/NWC24Messenger.hpp"
#include "Game/Screen/ReplaceTagProcessor.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/StringUtil.hpp"

ReceiverTagMail::ReceiverTagMail(const char* pTaskName, const char* pMessageID, const char* pSenderID)
    : mMessage(nullptr), mMessageLength(0), mIsError(false), _18(1) {
    mTaskName = pTaskName;
    mMessageID = pMessageID;
    mSenderID = pSenderID;
    mMessageLength = MR::getStringLengthWithMessageTag(MR::getGameMessageDirect(mMessageID)) + FileSelectFunc::getMiiNameBufferSize();
    mMessage = new wchar_t[mMessageLength];

    MR::zeroMemory(mMessage, mMessageLength * sizeof(wchar_t));
}

#if (VERSION == RMGJ01 || VERSION == RMGE01 || VERSION == RMGP01 || VERSION == RMGP01)


#elif (VERSION == RMGK01)

#endif

void ReceiverTagMail::update() {
    if (_18 != 0) {
        return;
    }

    if (!MR::isMailSent(mTaskName)) {
        return;
    }

    mIsError = MR::isMailErrorHappened(mTaskName);

    MR::termMail(mTaskName);

    _18 = 1;
}

bool ReceiverTagMail::isDone() const {
    return _18 - 1 == 0;
}

bool ReceiverTagMail::isError() const {
    return mIsError;
}
