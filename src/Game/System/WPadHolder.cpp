#include "Game/System/WPadHolder.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/WPad.hpp"
#include "Game/System/WPadPointer.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/PadRecorder.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <revolution/wpad.h>

#define KPAD_STATUS_ARRAY_SIZE 120

namespace {
    bool isEnablePadUpdate() {
        if (GameSystemFunction::isHomeButtonMenuActive()) {
            return false;
        }

        return !GameSystemFunction::isOccurredSystemWarning();
    }
};  // namespace

WPadReadDataInfo::WPadReadDataInfo() : mStatusArray(), mValidStatusCount() {
    mStatusArray = new KPADStatus[KPAD_STATUS_ARRAY_SIZE];

    MR::zeroMemory(mStatusArray, sizeof(KPADStatus) * KPAD_STATUS_ARRAY_SIZE);
}

KPADStatus* WPadReadDataInfo::getKPadStatus(u32 index) const {
    if (index >= mValidStatusCount) {
        return nullptr;
    }

    return &mStatusArray[index];
}

KPADStatus* WPadReadDataInfo::getReadBuffer() {
    return mStatusArray;
}

u32 WPadReadDataInfo::getValidStatusCount() const {
    return mValidStatusCount;
}

void WPadReadDataInfo::copy(const WPadReadDataInfo* pOther) {
    mValidStatusCount = pOther->mValidStatusCount;

    for (s32 i = 0; i < static_cast< s32 >(mValidStatusCount); i++) {
        mStatusArray[i] = pOther->mStatusArray[i];
    }
}

WPadHolder::WPadHolder() : mReadDataInfoArray(), mUnifiedStatusBuffer(), mMode(MODE_GAME) {
    WPADRegisterAllocator(MR::allocFromWPadHeap, MR::freeFromWPadHeap);
    mUnifiedStatusBuffer = new KPADUnifiedWpadStatus[WPAD_MAX_CONTROLLERS * KPAD_STATUS_ARRAY_SIZE];
    KPADInitEx(mUnifiedStatusBuffer, WPAD_MAX_CONTROLLERS * KPAD_STATUS_ARRAY_SIZE);

    mRawReadDataInfoArray = new WPadReadDataInfo[WPAD_MAX_CONTROLLERS];
    mReadDataInfoArray = new WPadReadDataInfo[WPAD_MAX_CONTROLLERS];

    for (u32 i = 0; i < ARRAY_SIZE(mPad); i++) {
        mPad[i] = new WPad(i);
        mPad[i]->setReadInfo(&mReadDataInfoArray[i]);
    }

    setConnectCallback();
    initSensorBarPosition();
}

void WPadHolder::updateReadDataOnly() {
    WPadReadDataInfo* pReadData;

    for (s32 chan = WPAD_CHAN0; chan < WPAD_MAX_CONTROLLERS; chan++) {
        pReadData = &mRawReadDataInfoArray[chan];
        s32 error = 0;
        pReadData->mValidStatusCount = KPADReadEx(chan, pReadData->mStatusArray, KPAD_STATUS_ARRAY_SIZE, &error);
    }

    for (s32 chan = WPAD_CHAN0; chan < WPAD_MAX_CONTROLLERS; chan++) {
        mReadDataInfoArray[chan].copy(&mRawReadDataInfoArray[chan]);
    }

    readPadData();
}

void WPadHolder::updateProjectPadData() {
    for (u32 i = 0; i < ARRAY_SIZE(mPad); i++) {
        mPad[i]->update();
    }
}

void WPadHolder::updateRumbleAndInfoChecker() {
    for (u32 i = 0; i < ARRAY_SIZE(mPad); i++) {
        mPad[i]->updateRumbleAndInfoChecker();
    }
}

void WPadHolder::updateInGame() {
    for (s32 chan = WPAD_CHAN2; chan < WPAD_MAX_CONTROLLERS; chan++) {
        KPADStatus* pStatus = mReadDataInfoArray[chan].getKPadStatus(0);

        if (pStatus == nullptr) {
            continue;
        }

        if (pStatus->wpad_err != WPAD_ERR_NONE) {
            continue;
        }

        WPADDisconnect(chan);
    }
}

void WPadHolder::update() {
    updateReadDataOnly();

    bool isReplaying = GameSequenceFunction::isPadMode();
    if (isReplaying) {
        isReplaying = MR::isPadRecorderReading();
    }
    if (isReplaying) {
        isReplaying = !isEnablePadUpdate();
    }
    if (isReplaying) {
        updateRumbleAndInfoChecker();
    } else {
        updateProjectPadData();
    }

    if (mMode == MODE_GAME) {
        updateInGame();
    }
}

void WPadHolder::initSensorBarPosition() {
    u8 sensorBarPosition = WPADGetSensorBarPosition();
    f32 level = 0.0f;

    switch (sensorBarPosition) {
    case WPAD_SENSOR_BAR_POS_TOP:
        level = 0.35;
        break;
    case WPAD_SENSOR_BAR_POS_BOTTOM:
        level = -0.15;
        break;
    }

    for (u32 i = 0; i < ARRAY_SIZE(mPad); i++) {
        mPad[i]->mPointer->setSensorBarLevel(level);
    }
}

WPad* WPadHolder::getWPad(s32 channel) {
    u32 channelMax = ARRAY_SIZE(mPad);

    if (channel < channelMax) {
        return mPad[channel];
    }

    return nullptr;
}

void WPadHolder::setConnectCallback() {
    for (s32 chan = WPAD_CHAN0; chan < WPAD_MAX_CONTROLLERS; chan++) {
        KPADSetConnectCallback(chan, WPad::connectCallback);
        WPADSetExtensionCallback(chan, WPad::extensionCallback);
    }
}

void WPadHolder::readPadData() {
    if (isEnablePadUpdate()) {
        for (s32 chan = WPAD_CHAN0; chan < WPAD_MAX_CONTROLLERS; chan++) {
            MR::readPadData(chan, &mReadDataInfoArray[chan]);
        }
    }
}

// PLACEHOLDER(core): Petari-only; resolve when GameSystemSceneController is ported from the target
void WPadHolder::resetPad() {
    KPADReset();

    for (u32 i = 0; i < ARRAY_SIZE(mPad); i++) {
        mPad[i]->resetPad();
    }
}

namespace {
    WPadHolder* getWPadHolder() NO_INLINE {
        return SingletonHolder< GameSystem >::get()->mObjHolder->mWPadHolder;
    }
};  // namespace

namespace MR {
    WPad* getWPad(s32 channel) {
        return ::getWPadHolder()->getWPad(channel);
    }

    void resetWPad() {
        WPadHolder* pWPadHolder;

        pWPadHolder = ::getWPadHolder();
        pWPadHolder->resetPad();
    }

    void setWPadHolderModeHomeButton() {
        WPadHolder* pWPadHolder;

        pWPadHolder = ::getWPadHolder();
        pWPadHolder->mMode = WPadHolder::MODE_HOME_BUTTON;
    }

    void setWPadHolderModeGame() {
        WPadHolder* pWPadHolder;

        pWPadHolder = ::getWPadHolder();
        pWPadHolder->mMode = WPadHolder::MODE_GAME;
    }

    u32 getHBMKPadData(HBMKPadData* pPadData, s32 chan) {
        u32 type;
        s32 err = WPADProbe(chan, &type);
        KPADStatus* pStatus = ::getWPadHolder()->mRawReadDataInfoArray[chan].mStatusArray;

        switch (err) {
        case WPAD_ERR_NONE:
        case WPAD_ERR_BUSY:
        case WPAD_ERR_TRANSFER:
            pPadData->kpad = pStatus;
            pPadData->pos = pStatus->pos;
            pPadData->use_devtype = pStatus->dev_type;
            break;
        default:
            pPadData->kpad = nullptr;
            break;
        }
        return type;
    }

    void setAutoSleepTimeWiiRemote(bool isLongAutoSleep) {
        u8 minute = isLongAutoSleep ? 15 : 5;

        WPADSetAutoSleepTime(minute);
    }
};  // namespace MR

namespace MR {
    bool testHBMKPadTrigger(s32 channel, u32 buttons) {
        return (::getWPadHolder()->mRawReadDataInfoArray[channel].mStatusArray->trig & buttons) != 0;
    }
}
