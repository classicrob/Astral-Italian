#include "Game/Screen/NrvBatteryInfo.hpp"
#include "Game/Screen/BatteryInfo.hpp"
#include "Game/Util.hpp"

NrvBatteryInfo::BatteryInfoEnought::BatteryInfoEnought() {}
NrvBatteryInfo::BatteryInfoLowAppear::BatteryInfoLowAppear() {}
NrvBatteryInfo::BatteryInfoLow::BatteryInfoLow() {}
NrvBatteryInfo::BatteryInfoLowDisappear::BatteryInfoLowDisappear() {}
NrvBatteryInfo::BatteryInfoLowHide::BatteryInfoLowHide() {}
NrvBatteryInfo::BatteryInfoDisappear::BatteryInfoDisappear() {}
NrvBatteryInfo::BatteryInfoCriticalAppear::BatteryInfoCriticalAppear() {}
NrvBatteryInfo::BatteryInfoCritical::BatteryInfoCritical() {}
NrvBatteryInfo::BatteryInfoNoneAppear::BatteryInfoNoneAppear() {}
NrvBatteryInfo::BatteryInfoNone::BatteryInfoNone() {}
NrvBatteryInfo::BatteryInfoNoneDisappear::BatteryInfoNoneDisappear() {}
NrvBatteryInfo::BatteryInfoNoneHide::BatteryInfoNoneHide() {}

void NrvBatteryInfo::BatteryInfoNoneHide::execute(Spine* spine) const {
    BatteryInfo* actor = static_cast<BatteryInfo*>(spine->mExecutor);
    actor->exeNoneHide();
}

void NrvBatteryInfo::BatteryInfoNoneDisappear::execute(Spine* spine) const {
    BatteryInfo* actor = static_cast<BatteryInfo*>(spine->mExecutor);
    actor->exeStartAnimAndSetNerveIfStopped("EndNone", &BatteryInfoNoneHide::sInstance);
}

void NrvBatteryInfo::BatteryInfoNone::execute(Spine* spine) const {
    BatteryInfo* actor = static_cast<BatteryInfo*>(spine->mExecutor);
    actor->exeNone();
}

void NrvBatteryInfo::BatteryInfoNoneAppear::execute(Spine* spine) const {
    BatteryInfo* actor = static_cast<BatteryInfo*>(spine->mExecutor);
    actor->exeNoneAppear();
}

void NrvBatteryInfo::BatteryInfoCritical::execute(Spine* spine) const {
    BatteryInfo* actor = static_cast<BatteryInfo*>(spine->mExecutor);
    actor->exeCritical();
}

void NrvBatteryInfo::BatteryInfoCriticalAppear::execute(Spine* spine) const {
    BatteryInfo* actor = static_cast<BatteryInfo*>(spine->mExecutor);
    actor->exeCriticalAppear();
}

void NrvBatteryInfo::BatteryInfoDisappear::execute(Spine* spine) const {
    BatteryInfo* actor = static_cast<BatteryInfo*>(spine->mExecutor);
    actor->exeStartAnimAndSetNerveIfStopped("EndCritical", &BatteryInfoEnought::sInstance);
}

void NrvBatteryInfo::BatteryInfoLowHide::execute(Spine* spine) const {
    BatteryInfo* actor = static_cast<BatteryInfo*>(spine->mExecutor);
    actor->exeLowHide();
}

void NrvBatteryInfo::BatteryInfoLowDisappear::execute(Spine* spine) const {
    BatteryInfo* actor = static_cast<BatteryInfo*>(spine->mExecutor);
    actor->exeStartAnimAndSetNerveIfStopped("EndLow", &BatteryInfoLowHide::sInstance);
}

void NrvBatteryInfo::BatteryInfoLow::execute(Spine* spine) const {
    BatteryInfo* actor = static_cast<BatteryInfo*>(spine->mExecutor);
    actor->exeLow();
}

void NrvBatteryInfo::BatteryInfoLowAppear::execute(Spine* spine) const {
    BatteryInfo* actor = static_cast<BatteryInfo*>(spine->mExecutor);
    actor->exeLowAppear();
}

void NrvBatteryInfo::BatteryInfoEnought::execute(Spine* spine) const {
    BatteryInfo* actor = static_cast<BatteryInfo*>(spine->mExecutor);
    actor->exeEnought();
}

