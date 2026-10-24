#include "Game/System/PlayResultInStageHolder.hpp"

#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"

PlayResultInStageHolder::PlayResultInStageHolder() : NameObj("プレイ結果保持[ステージ中]"),
    mIsStageResult(false), mHasPowerStar(false), mIsNotGreenStar(false),
     mClearedPowerStarNum(-1), mClearedStarPieceNum(0), mStarPieceAdded(0), mClearedCoinNum(0), mCoinAdded(0), mPurpleCoinNum(0),
     mIsPurpleCoinFull(false), mIsCollectTicoCoinSaved(false), mIsTicoCoinSaved(false), mPlayTime(0), mIsNewBestTime(false), mRestartMarioNo(nullptr),
     mMissCount(0), mHasRetryGalaxy(true), mIsPowerStarAppeared(false), mIsLuigiIntruding(false), mPlayerHealth(3),
     mCoinOneUpNum(0), mIsUnlockGhostLuigi(false), mPadMode(0), mIsSuperDreamer(false) {
    MR::zeroMemory(mStageName, sizeof(mStageName));
    JMapIdInfo* restart = new JMapIdInfo();
    if (restart != nullptr) {
        restart->_0 = -1;
        restart->mZoneID = -1;
    }
    mRestartMarioNo = restart;
    mNewStageList.mBase = new PlayResultStageEntry[0x40];
    mNewStageList.mCapacity = 0x40;
    mResultStageList.mBase = new PlayResultStageEntry[0x40];
    mResultStageList.mCapacity = 0x40;

}

// NONMATCHING: regalloc; target retains redundant null checks for embedded list members (FP order/grouping checked: no FP instructions).
PlayResultInStageHolder::~PlayResultInStageHolder() {
}

void PlayResultInStageHolder::reset() {
    MR::zeroMemory(mStageName, 0x30);
    mIsStageResult = false;
    mHasPowerStar = false;
    mIsNotGreenStar = false;
    mClearedPowerStarNum = -1;
    mClearedStarPieceNum = 0;
    mStarPieceAdded = 0;
    mClearedCoinNum = 0;
    mCoinAdded = 0;
    mPurpleCoinNum = 0;
    mIsPurpleCoinFull = false;
    mIsCollectTicoCoinSaved = false;
    mIsTicoCoinSaved = false;
    mPlayTime = 0;
    mIsNewBestTime = false;
    *mRestartMarioNo = MR::getInitializeStartIdInfo();
    mMissCount = 0;
    mHasRetryGalaxy = true;
    mIsPowerStarAppeared = false;
    mIsLuigiIntruding = GameDataFunction::isOnGameEventFlag("でしゃばりルイージ出現中");
    mPlayerHealth = 3;
    mCoinOneUpNum = 0;
    mIsUnlockGhostLuigi = false;
    mPadMode = 0;
    mIsSuperDreamer = false;
    mNewStageList.mCount = 0;
    mResultStageList.mCount = 0;

    ScenarioDataIter iter = MR::makeBeginScenarioDataIter();
    while (!iter.isEnd()) {
        GalaxyStatusAccessor accessor = iter.makeAccessor();
        const char* pName = accessor.getName();
        s32 galaxyStarTotalNum = MR::getGalaxyStarTotalNum(pName);
        PlayResultStageEntry entry;
        entry.mStageName = pName;
        entry.mGalaxyStarTotalNum = galaxyStarTotalNum;
        mNewStageList.add(entry);
        iter.goNext();
    }
}

void PlayResultInStageHolder::clearStageResult() {
    PlayResultInStageHolder* pHolder = this;
    pHolder->mIsStageResult = false;
    pHolder->mClearedPowerStarNum = -1;
    pHolder->mPurpleCoinNum = 0;
    pHolder->mIsPowerStarAppeared = false;
}

void PlayResultInStageHolder::setStageName(const char* pName) {
    MR::copyString(mStageName, pName, 0x30);
}

const char* PlayResultInStageHolder::getStageName() const {
    return mStageName;
}

void PlayResultInStageHolder::setupStageResult() {
    PlayResultInStageHolder* pHolder = this;
    const char* pName;
    s32 starNo = pHolder->mClearedPowerStarNum;
    pHolder->mIsStageResult = true;
    pName = pHolder->mStageName;
    pHolder->mHasPowerStar = !GameDataFunction::hasPowerStar(pName, starNo);
    if (pHolder->mHasPowerStar) {
        bool isNotGreen = false;
        if (pHolder->mIsSuperDreamer == false) {
            GalaxyStatusAccessor accessor = MR::makeGalaxyStatusAccessor(pName);
            if (!accessor.isGreenStar(starNo)) {
                isNotGreen = true;
            }
        }
        pHolder->mIsNotGreenStar = isNotGreen;
    } else {
        bool isBronze = false;
        if (pHolder->mIsSuperDreamer == false && MR::isPowerStarBronze(pName, starNo)) {
            isBronze = true;
        }
        pHolder->mIsNotGreenStar = isBronze;
    }
    if (pHolder->mPlayTime != 0 && static_cast<u32>(pHolder->mPlayTime) < 0x34BC0U && pHolder->mIsSuperDreamer == false) {
        s32 count = GameDataFunction::getScenarioClearTime(pName, starNo);
        bool newBest = false;
        if (count != 0 && static_cast<u32>(pHolder->mPlayTime) < static_cast<u32>(count)) {
            newBest = true;
        }
        pHolder->mIsNewBestTime = newBest;
    } else {
        pHolder->mPlayTime = 0;
    }
}

void PlayResultInStageHolder::setClearedPowerStarNum(s32 num) {
    mClearedPowerStarNum = num;
}

s32 PlayResultInStageHolder::getClearedPowerStarNum() const {
    return mClearedPowerStarNum;
}

bool PlayResultInStageHolder::addStarPieceNum(s32 num) {
    mClearedStarPieceNum = MR::clamp(mClearedStarPieceNum + num, 0, 999);
    bool added = false;
    if (mStarPieceAdded + 100 <= mClearedStarPieceNum) {
        added = true;
        mStarPieceAdded += 100;
    }
    return added;
}

s32 PlayResultInStageHolder::getClearedStarPieceNum() const {
    return MR::clamp(mClearedStarPieceNum, 0, 999);
}

bool PlayResultInStageHolder::addCoinNum(s32 num) {
    mClearedCoinNum = MR::clamp(mClearedCoinNum + num, 0, 999);
    return false;
}

s32 PlayResultInStageHolder::getClearedCoinNum() const {
    return MR::clamp(mClearedCoinNum, 0, 999);
}

bool PlayResultInStageHolder::incPurpleCoinNum() {
    mPurpleCoinNum++;
    return mPurpleCoinNum == 100;
}

s32 PlayResultInStageHolder::getPurpleCoinNum() const {
    return MR::clamp(mPurpleCoinNum, 0, 999);
}

void PlayResultInStageHolder::setCollectTicoCoinSaved() {
    mIsCollectTicoCoinSaved = true;
    mIsTicoCoinSaved = !GameDataFunction::isOnGalaxyFlagTicoCoin(mStageName);
}

void PlayResultInStageHolder::updateNewlyOpenedGalaxies() {
    PlayResultInStageHolder* pHolder = this;
    for (s32 i = 0; i < pHolder->mNewStageList.mCount; i++) {
        PlayResultStageEntry* pEntry = &pHolder->mNewStageList.mBase[i];
        if (pEntry->mGalaxyStarTotalNum) {
            continue;
        }
        const char* pName = pEntry->mStageName;
        if (MR::getGalaxyStarTotalNum(pName) != 0) {
            PlayResultStageEntry added;
            added.mStageName = pName;
            added.mGalaxyStarTotalNum = 1;
            pHolder->mResultStageList.add(added);
        }
    }
}

bool PlayResultInStageHolder::isNewlyOpenedGalaxy(const char* pName) const {
    const PlayResultInStageHolder* pHolder = this;
    for (s32 i = 0; i < pHolder->mResultStageList.mCount; i++) {
        if (MR::isEqualString(pHolder->mResultStageList.mBase[i].mStageName, pName)) {
            return true;
        }
    }
    return false;
}

void PlayResultInStageHolder::setRestartMarioNo(const JMapIdInfo& rInfo) {
    *mRestartMarioNo = rInfo;
}

void PlayResultInStageHolder::clearAfterMiss() {
    mMissCount++;
    clearForRetry();
}

void PlayResultInStageHolder::clearForRetry() {
    PlayResultInStageHolder* pHolder = this;
    s32 padMode = pHolder->mPadMode;
    pHolder->mClearedCoinNum = 0;
    pHolder->mCoinAdded = 0;
    pHolder->mHasRetryGalaxy = false;
    pHolder->mPlayerHealth = 3;
    pHolder->mIsSuperDreamer = false;
    pHolder->mIsPurpleCoinFull = false;
    if (padMode != 0) {
        pHolder->mPadMode = 0;
    }
}

s32 PlayResultInStageHolder::getPlayerHealth() const {
    if (MR::isStageMarioFaceShipOrWorldMap()) {
        return 3;
    }
    return mPlayerHealth;
}

void PlayResultInStageHolder::setCoinOneUpNum(s32 value) {
    PlayResultInStageHolder* pHolder = this;
    pHolder->mCoinOneUpNum = value;
}

void PlayResultInStageHolder::setPadModeDreamer() {
    mPadMode = 1;
    mHasRetryGalaxy = false;
    mPlayerHealth = MR::getPlayerLife();
}

void PlayResultInStageHolder::setPadModeSuperDreamer() {
    mPadMode = 2;
    mIsSuperDreamer = true;
    mHasRetryGalaxy = false;
    mPlayerHealth = MR::getPlayerLife() > 3 ? 6 : 3;
}

void PlayResultInStageHolder::clearPadMode() {
    mPadMode = 0;
}

void PlayResultStageList::add(const PlayResultStageEntry& rEntry) {
    s32 index = mCount++;
    mBase[index] = rEntry;
}

PlayResultStageList::~PlayResultStageList() {
    if (mBase != nullptr) {
        delete[] mBase;
    }
}

PlayResultStageEntry::PlayResultStageEntry() {
    mStageName = 0;
    mGalaxyStarTotalNum = 0;
}

PlayResultStageList::PlayResultStageList() {
    mBase = nullptr;
    mCapacity = 0;
    mCount = 0;
}
