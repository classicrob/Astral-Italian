#include "Game/System/GameEventFlagTable.hpp"
#include "Game/System/GalaxyInfoTable.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/StringUtil.hpp"
#include <algorithm>
#include <cstdio>

namespace {
    static const GameEventFlag sGameEventFlagStatic[] = {
        {"ハチマリオ初変身"},
        {"テレサマリオ初変身"},
        {"ホッパーマリオ初変身"},
        {"ファイアマリオ初変身"},
        {"アイスマリオ初変身"},
        {"無敵マリオ初変身"},
        {"ゴロ岩マリオ初変身"},
        {"雲マリオ初変身"},
        {"ドリル初ゲット"},
        {"ライフアップキノコ解説"},
        {"１ＵＰキノコ解説"},
        {"ヨッシー出会い"},
        {"コメットメダル解説"},
        {"２Ｐサポート解説"},
        {"タマコロチュートリアル"},
        {"グライバードチュートリアル"},
        {"ワールド1初プレイ"},
        {"ワールド2初プレイ"},
        {"ワールド3初プレイ"},
        {"ワールド4初プレイ"},
        {"ワールド5初プレイ"},
        {"ワールド6初プレイ"},
        {"ワールド7初プレイ"},
        {"ワールド2他のワールドへ誘導"},
        {"ワールド3他のワールドへ誘導"},
        {"ワールド4他のワールドへ誘導"},
        {"ワールド5他のワールドへ誘導"},
        {"ワールド6他のワールドへ誘導"},
        {"ワールド2ゲームオーバーによる誘導"},
        {"ワールド3ゲームオーバーによる誘導"},
        {"ワールド4ゲームオーバーによる誘導"},
        {"ワールド5ゲームオーバーによる誘導"},
        {"ワールド6ゲームオーバーによる誘導"},
        {"グランドギャラクシーマップ初プレイ"},
        {"グリーンスター出現デモ"},
        {"クッパ最終戦直前デモ"},
        {"IsOpenScenarioGoroRockGalaxy3"},
        {"IsOpenScenarioJungleGliderGalaxy2"},
        {"IsOpenScenarioThunderFleetGalaxy3"},
        {"IsOpenScenarioChallengeGliderGalaxy2"},
        {"IsOpenScenarioHoneyBeeVillageGalaxy2"},
        {"IsOpenScenarioUnderGroundDangeonGalaxy2"},
        {"IsOpenScenarioMokumokuValleyGalaxy2"},
        {"オープニング実行"},
        {"コメット解説"},
        {"銀行屋キノピオ初回"},
        {"でしゃばりルイージ出現開始"},
        {"ノーマルエンディング実行"},
        {"ノーマルエンディング後デモ"},
        {"スター120個エンディング実行"},
        {"スター120個エンディング後デモ"},
        {"最終ギャラクシー出現"},
        {"ノーマルエンディングメール送信"},
        {"コンプリートメール送信"},
        {"スターピースカウンターストップ"},
        {"コインカウンターストップ"},
        {"ルイージプレイ済"},
        {"でしゃばりルイージ出現中"},
        {"ゲームオーバーで終了"},
        {0},
    };

    struct GameEventFlagSortLt {
        bool operator()(GameEventFlagTableInstance::Key& key1, GameEventFlagTableInstance::Key& key2) {
            return (key1.mHashCode < key2.mHashCode) ? true : false;
        }
    };
};  // namespace

bool GameEventFlagIter::isEnd() const {
    if (GameEventFlagIter::isValid() == false) {
        return true;
    }

    return GameEventFlagTable::getFlag(mIter) == nullptr;
}

void GameEventFlagIter::goNext() {
    if (GameEventFlagIter::isValid()) {
        mIter++;
    }
}

const GameEventFlag* GameEventFlagIter::getFlag() const {
    return GameEventFlagTable::getFlag(mIter);
}

bool GameEventFlagIter::isValid() const {
    if (mIter >= 0) {
        return true;
    } else {
        return false;
    }
}

GameEventFlagTableInstance::GameEventFlagTableInstance() : mSortTable(nullptr), mLength(0) {
    initSortTable();
}

const GameEventFlag* GameEventFlagTableInstance::findFlag(const char* flagName) {
    int i;
    u16 hashCode = MR::getHashCode(flagName);
    i = mLength;
    i = (i * 8) / 8;
    GameEventFlagTableInstance::Key* key = mSortTable;

    while (i > 0) {
        int mid = i / 2;

        if (key[i / 2].mHashCode < hashCode) {
            i -= mid + 1;
            key = &key[mid] + 1;
        } else {
            i = mid;
        }
    }

    if (key->mHashCode == hashCode && MR::isEqualString(flagName, key->mFlag->mName)) {
        return key->mFlag;
    }

    return nullptr;
}

void GameEventFlagTableInstance::initSortTable() {
    mSortTable = new GameEventFlagTableInstance::Key[GameEventFlagTable::getTableSize()];
    mLength = GameEventFlagTable::getTableSize();

    for (int i = 0; i < mLength; i++) {
        const GameEventFlag* pFlag = GameEventFlagTable::getFlag(i);

        mSortTable[i].mFlag = pFlag;
        mSortTable[i].mHashCode = MR::getHashCode(pFlag->mName);
    }

    std::sort(mSortTable, mSortTable + mLength, GameEventFlagSortLt());
}

namespace GameEventFlagTable {
    GameEventFlagIter getBeginIter() {
        return GameEventFlagIter();
    }

    GameEventFlagAccessor makeAccessor(const char* pFlagName) {
        return findFlag(pFlagName);
    }

    s32 getTableSize() {
        return ARRAY_SIZE(::sGameEventFlagStatic) - 1;  // last entry is a null terminator
    }

    const GameEventFlag* getFlag(int index) {
        if (index < 0 || getTableSize() <= index) {
            return nullptr;
        }

        return &::sGameEventFlagStatic[index];
    }

    const GameEventFlag* findFlag(const char* pFlagName) {
        return SingletonHolder< GameEventFlagTableInstance >::get()->findFlag(pFlagName);
    }

    bool isExist(const char* pName) {
        return findFlag(pName) != nullptr;
    }

    int getIndex(const GameEventFlag* pFlag) {
        return pFlag - &::sGameEventFlagStatic[0];
    }

    int getIndexFromHashCode(u16 hashCode) {
        for (GameEventFlagIter iter = getBeginIter(); !iter.isEnd(); iter.goNext()) {
            GameEventFlagAccessor accessor = GameEventFlagAccessor(getFlag(iter.mIter));

            if (hashCode == (MR::getHashCode(accessor.getName()) & 0x7FFF)) {
                return getIndex(getFlag(iter.mIter));
            }
        }

        return -1;
    }
};  // namespace GameEventFlagTable
