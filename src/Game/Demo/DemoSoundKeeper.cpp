#include "Game/Demo/DemoSoundKeeper.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/GameAudio/ActionSoundKeeper.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Util/SystemUtil.hpp"

DemoSoundInfo::DemoSoundInfo() : mPartName(), mBgm(""), mSystemSe(""), mActionSe(""), mReturnBgm(), mBgmWipeoutFrame(-1), mAllSoundStopFrame(-1) {
}

#pragma auto_inline off
DemoSoundKeeper::DemoSoundKeeper(DemoExecutor* pExecutor) : DemoSheetKeeperBase(pExecutor), mActionSoundKeeper() {
    JMapInfo* pMap = nullptr;
    const DemoExecutor* pSheetExecutor = mExecutor;
    const s32& count = DemoFunction::createSheetParser(pSheetExecutor, getTypeString(), &pMap);
    mInfo.init(count);

    for (s32 i = 0; i < count; i++) {
        DemoSoundInfo info;
        pMap->getValue(i, "PartName", &info.mPartName);
        pMap->getValue(i, "Bgm", &info.mBgm);
        pMap->getValue(i, "SystemSe", &info.mSystemSe);
        pMap->getValue(i, "ActionSe", &info.mActionSe);
        pMap->getValue(i, "ReturnBgm", reinterpret_cast< s32* >(&info.mReturnBgm));
        pMap->getValue(i, "BgmWipeoutFrame", &info.mBgmWipeoutFrame);
        if (MR::hasCsvItem(pMap, "AllSoundStopFrame")) {
            pMap->getValue(i, "AllSoundStopFrame", &info.mAllSoundStopFrame);
        }
        mInfo.push_back(info);
        if (!MR::isNullOrEmptyString(info.mActionSe)) {
            initActionSoundKeeper();
        }
    }
}

#pragma auto_inline on
void DemoSoundKeeper::update() {
    DemoSheetKeeperInfoHolder< DemoSoundInfo >::update();

    if (mActionSoundKeeper != nullptr) {
        mActionSoundKeeper->update();
    }
}

void DemoSoundKeeper::initActionSoundKeeper() {
    if (mActionSoundKeeper == nullptr) {
        mActionSoundKeeper = new ActionSoundKeeper(4, mExecutor->mName, nullptr);
    }
}

void DemoSoundKeeper::executeType(const DemoSoundInfo* pInfo) {
    if (MR::isDemoPartFirstStep(pInfo->mPartName)) {
        if (pInfo->mAllSoundStopFrame >= 0 && isPermitBgmChange()) {
            MR::stopAllSound(pInfo->mAllSoundStopFrame);
        }

        if (pInfo->mBgmWipeoutFrame >= 0 && isPermitBgmChange()) {
            MR::stopStageBGM(pInfo->mBgmWipeoutFrame);
        }

        if (!MR::isNullOrEmptyString(pInfo->mBgm) && isPermitBgmChange()) {
            MR::startStageBGM(pInfo->mBgm, false);
        }

        if (!MR::isNullOrEmptyString(pInfo->mSystemSe)) {
            MR::startSystemSE(pInfo->mSystemSe);
        }

        if (!MR::isNullOrEmptyString(pInfo->mActionSe)) {
            mActionSoundKeeper->startSound(pInfo->mActionSe, -1, -1);
        }
    } else if (MR::isDemoPartLastStep(pInfo->mPartName) && pInfo->mReturnBgm && isPermitBgmChange()) {
        MR::startLastStageBGM();
    }
}

bool DemoSoundKeeper::isPermitBgmChange() {
    return !MR::isGalaxyCometEvent();
}

inline const char* DemoSoundKeeper::getTypeString() const {
    return "Sound";
}

inline const char* DemoSoundKeeper::getName() const {
    return "サウンド";
}

#include "Game/Demo/DemoSheetKeeperInfoHolder.hpp"
