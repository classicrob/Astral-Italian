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

PlayResultInStageHolder::PlayResultInStageHolder() : NameObj("\x83v\x83\x8c\x83""C\x8c\x8b\x89\xca\x95\xdb\x8e\x9d[\x83X\x83""e\x81[\x83W\x92\x86]") {
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
    mRestartMarioNo = new JMapIdInfo();
    mMissCount = 0;
    mHasRetryGalaxy = true;
    mIsPowerStarAppeared = false;
    mIsLuigiIntruding = false;
    mPlayerHealth = 3;
    mCoinOneUpNum = 0;
    mIsUnlockGhostLuigi = false;
    mPadMode = 0;
    mIsSuperDreamer = false;
    mNewStageList.mBase = nullptr;
    mNewStageList.mCapacity = 0;
    mNewStageList.mCount = 0;
    mResultStageList.mBase = nullptr;
    mResultStageList.mCapacity = 0;
    mResultStageList.mCount = 0;

    mNewStageList.mBase = new PlayResultStageEntry[0x40];
    mNewStageList.mCapacity = 0x40;
    mResultStageList.mBase = new PlayResultStageEntry[0x40];
    mResultStageList.mCapacity = 0x40;

    reset();
}

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
    mIsLuigiIntruding = GameDataFunction::isOnGameEventFlag("\x82\xc5\x82\xb5\x82\xe1\x82\xce\x82\xe8\x83\x8b\x83""C\x81[\x83W\x8fo\x8c\xbb\x92\x86");
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
        PlayResultStageEntry entry;
        entry.mValue = reinterpret_cast< u32 >(pName);
        entry.mFlag = MR::getGalaxyStarTotalNum(pName) != 0;
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
    s32 starNo = pHolder->mClearedPowerStarNum;
    pHolder->mIsStageResult = true;
    const char* pName = pHolder->mStageName;
    pHolder->mHasPowerStar = !GameDataFunction::hasPowerStar(pName, starNo);
    if (!pHolder->mHasPowerStar) {
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
    if (pHolder->mPlayTime != 0 && pHolder->mPlayTime < 0x34BC0 && pHolder->mIsSuperDreamer == false) {
        s32 count = GameDataFunction::getScenarioClearTime(pName, starNo);
        if (count != 0 && pHolder->mPlayTime < count) {
            pHolder->mIsNewBestTime = true;
        } else {
            pHolder->mIsNewBestTime = false;
        }
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

void PlayResultInStageHolder::addStarPieceNum(s32 num) {
    mClearedStarPieceNum = MR::clamp(mClearedStarPieceNum + num, 0, 999);
    if (mStarPieceAdded + 100 > mClearedStarPieceNum) {
        mIsPowerStarAppeared = true;
    } else {
        mStarPieceAdded += 100;
    }
}

s32 PlayResultInStageHolder::getClearedStarPieceNum() const {
    return MR::clamp(mClearedStarPieceNum, 0, 999);
}

void PlayResultInStageHolder::addCoinNum(s32 num) {
    mClearedCoinNum = MR::clamp(mClearedCoinNum + num, 0, 999);
}

s32 PlayResultInStageHolder::getClearedCoinNum() const {
    return MR::clamp(mClearedCoinNum, 0, 999);
}

void PlayResultInStageHolder::incPurpleCoinNum() {
    mPurpleCoinNum++;
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
        if (pEntry->mFlag) {
            continue;
        }
        const char* pName = reinterpret_cast< const char* >(pEntry->mValue);
        if (MR::getGalaxyStarTotalNum(pName) != 0) {
            PlayResultStageEntry added;
            added.mValue = pEntry->mValue;
            added.mFlag = true;
            pHolder->mResultStageList.add(added);
        }
    }
}

bool PlayResultInStageHolder::isNewlyOpenedGalaxy(const char* pName) const {
    const PlayResultInStageHolder* pHolder = this;
    for (s32 i = 0; i < pHolder->mResultStageList.mCount; i++) {
        if (MR::isEqualString(reinterpret_cast< const char* >(pHolder->mResultStageList.mBase[i].mValue), pName)) {
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
    pHolder->mStarPieceAdded = 0;
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
    PlayResultStageEntry* pDst = &mBase[mCount];
    mCount++;
    pDst->mValue = rEntry.mValue;
    pDst->mFlag = rEntry.mFlag;
}

PlayResultStageList::~PlayResultStageList() {
    if (mBase != nullptr) {
        delete[] mBase;
    }
}

PlayResultStageEntry::PlayResultStageEntry() {
    mValue = 0;
    mFlag = false;
}

PlayResultStageList::PlayResultStageList() {
    mBase = nullptr;
    mCapacity = 0;
    mCount = 0;
}
