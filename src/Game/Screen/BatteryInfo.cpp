#include "Game/Screen/NrvBatteryInfo.hpp"
#include "Game/Screen/BatteryInfo.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/WPad.hpp"
#include "Game/System/WPadHolder.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

#define WPAD_BATTERY_LEVEL_EMPTY -1

namespace {
    static const s32 sDisplayFrameBatteryLow = 120;
    static const s32 sLowIntervalFrame = 18000;
    static const s32 sDisplayFrameBatteryNone = 450;
};  // namespace

namespace NrvBatteryInfo {
    BatteryInfoEnought BatteryInfoEnought::sInstance;
    BatteryInfoLowAppear BatteryInfoLowAppear::sInstance;
    BatteryInfoLow BatteryInfoLow::sInstance;
    BatteryInfoLowDisappear BatteryInfoLowDisappear::sInstance;
    BatteryInfoLowHide BatteryInfoLowHide::sInstance;
    BatteryInfoDisappear BatteryInfoDisappear::sInstance;
    BatteryInfoCriticalAppear BatteryInfoCriticalAppear::sInstance;
    BatteryInfoCritical BatteryInfoCritical::sInstance;
    BatteryInfoNoneAppear BatteryInfoNoneAppear::sInstance;
    BatteryInfoNone BatteryInfoNone::sInstance;
    BatteryInfoNoneDisappear BatteryInfoNoneDisappear::sInstance;
    BatteryInfoNoneHide BatteryInfoNoneHide::sInstance;
}


BatteryInfo::BatteryInfo(LayoutActor* pHost, const char* pPaneName, int channel)
    : NerveExecutor("BatteryInfo"), mHost(pHost), mPaneName(pPaneName), mChannel(channel) {
    initNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoNoneHide));
    MR::createAndAddPaneCtrl(mHost, mPaneName, 1);
    MR::hidePane(mHost, mPaneName);
}

void BatteryInfo::exeEnought() {
    if (MR::isFirstStep(this)) {
        MR::hidePane(mHost, mPaneName);
    }

    if (MR::getWPad(mChannel)->getBattery() == WPAD_BATTERY_LEVEL_EMPTY) {
        setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoNoneHide));
    } else if (MR::getWPad(mChannel)->getBattery() == WPAD_BATTERY_LEVEL_CRITICAL) {
        setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoCriticalAppear));
    } else if (MR::getWPad(mChannel)->getBattery() == WPAD_BATTERY_LEVEL_LOW) {
        setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoLowAppear));
    }
}

void BatteryInfo::exeLowAppear() {
    if (MR::isFirstStep(this)) {
        MR::showPane(mHost, mPaneName);
        MR::startSystemSE("SE_SY_BATTERY_LOW");
    }

    exeStartAnimAndSetNerveIfStopped("AppearLow", GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoLow));
}

void BatteryInfo::exeLow() {
    if (MR::isFirstStep(this)) {
        MR::startPaneAnim(mHost, mPaneName, "WaitLow", 0);
    }

    tryChangeNerveWithBatteryLeftAlreadyAppear();

    if (MR::isGreaterStep(this, ::sDisplayFrameBatteryLow)) {
        setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoLowDisappear));
    }
}

void BatteryInfo::exeLowDisappear() {
    exeStartAnimAndSetNerveIfStopped("EndLow", GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoLowHide));
}

void BatteryInfo::exeLowHide() {
    if (MR::isFirstStep(this)) {
        MR::hidePane(mHost, mPaneName);
    }

    if (MR::getWPad(mChannel)->getBattery() == WPAD_BATTERY_LEVEL_EMPTY) {
        if (mChannel == WPAD_CHAN1) {
            setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoNoneAppear));
        } else {
            setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoNoneHide));
        }
    } else if (MR::getWPad(mChannel)->getBattery() == WPAD_BATTERY_LEVEL_CRITICAL) {
        setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoCriticalAppear));
    } else if (MR::getWPad(mChannel)->getBattery() > WPAD_BATTERY_LEVEL_LOW) {
        setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoEnought));
    } else if (MR::isGreaterStep(this, ::sLowIntervalFrame)) {
        setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoLowAppear));
    }
}

void BatteryInfo::exeCriticalAppear() {
    if (MR::isFirstStep(this)) {
        MR::showPane(mHost, mPaneName);
        MR::startSystemSE("SE_SY_BATTERY_CRITICAL");
    }

    exeStartAnimAndSetNerveIfStopped("AppearCritical", GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoCritical));
}

void BatteryInfo::exeCritical() {
    if (MR::isFirstStep(this)) {
        MR::startPaneAnim(mHost, mPaneName, "WaitCritical", 0);
    }

    tryChangeNerveWithBatteryLeftAlreadyAppear();
}

void BatteryInfo::exeDisappear() {
    exeStartAnimAndSetNerveIfStopped("EndCritical", GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoEnought));
}

void BatteryInfo::exeNoneAppear() {
    if (MR::isFirstStep(this)) {
        MR::showPane(mHost, mPaneName);
        MR::startSystemSE("SE_SY_BATTERY_CRITICAL");
    }

    exeStartAnimAndSetNerveIfStopped("AppearNone", GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoNone));
}

void BatteryInfo::exeNone() {
    if (MR::isFirstStep(this)) {
        MR::startPaneAnim(mHost, mPaneName, "WaitNone", 0);
    }

    if (MR::isGreaterStep(this, ::sDisplayFrameBatteryNone)) {
        setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoNoneDisappear));
    }
}

void BatteryInfo::exeNoneDisappear() {
    exeStartAnimAndSetNerveIfStopped("EndNone", GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoNoneHide));
}

void BatteryInfo::exeNoneHide() {
    if (MR::isFirstStep(this)) {
        MR::hidePane(mHost, mPaneName);
    }

    if (MR::getWPad(mChannel)->getBattery() != -1) {
        setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoEnought));
    }
}

bool BatteryInfo::exeStartAnimAndSetNerveIfStopped(const char* pAnimName, const Nerve* pNerve) {
    if (MR::isFirstStep(this)) {
        MR::startPaneAnim(mHost, mPaneName, pAnimName, 0);
    }

    if (MR::isPaneAnimStopped(mHost, mPaneName, 0)) {
        setNerve(pNerve);

        return true;
    }

    return false;
}

bool BatteryInfo::tryChangeNerveWithBatteryLeftAlreadyAppear() {
    const Nerve* pNerve;

    switch (MR::getWPad(mChannel)->getBattery()) {
    case WPAD_BATTERY_LEVEL_EMPTY:
        if (mChannel == WPAD_CHAN1) {
            pNerve = GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoNoneAppear);
        } else {
            pNerve = GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoDisappear);
        }
        break;
    case WPAD_BATTERY_LEVEL_CRITICAL:
        pNerve = GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoCritical);
        break;
    case WPAD_BATTERY_LEVEL_LOW:
        pNerve = GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoLow);
        break;
    default:
        pNerve = GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoDisappear);
        break;
    }

    if (!isNerve(pNerve)) {
        setNerve(pNerve);

        return true;
    }

    return false;
}

BatteryLayout::BatteryLayout() : LayoutActor("BatteryLayout", true), mInfo1P(nullptr), mInfo2P(nullptr) {
}

void BatteryLayout::init(const JMapInfoIter& rIter) {
    initLayoutManager("BatteryInfo", 1);

    mInfo1P = new BatteryInfo(this, "WinBattery1", 0);
    mInfo2P = new BatteryInfo(this, "WinBattery2", 1);

    appear();
}

void BatteryLayout::appear() {
    LayoutActor::appear();
    mInfo1P->setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoNoneHide));
    mInfo2P->setNerve(GET_NERVE_DIRECT(NrvBatteryInfo, BatteryInfoNoneHide));
}

void BatteryLayout::control() {
    mInfo1P->updateNerve();
    mInfo2P->updateNerve();
}
