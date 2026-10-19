#include "Game/System/GrandGalaxyMapStates.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/Util.hpp"

namespace NrvGrandGalaxyMapStateNews {
    NEW_NERVE(GrandGalaxyMapStateNewsWait, GrandGalaxyMapStateNews, Wait);
    NEW_NERVE(GrandGalaxyMapStateNewsMeisterAppear, GrandGalaxyMapStateNews, MeisterAppear);
    NEW_NERVE(GrandGalaxyMapStateNewsTalk, GrandGalaxyMapStateNews, Talk);
    NEW_NERVE(GrandGalaxyMapStateNewsTalkEnd, GrandGalaxyMapStateNews, TalkEnd);
};  // namespace NrvGrandGalaxyMapStateNews

using namespace NrvGrandGalaxyMapStateNews;

GrandGalaxyMapStateNews::GrandGalaxyMapStateNews(GrandGalaxyMap* pHost)
    : ActorStateBase< GrandGalaxyMap >("State[グランドギャラクシーニュース]", pHost), mNewsIndex(-1), mTalkCtrls(nullptr) {}

void GrandGalaxyMapStateNews::init() {
    static const char* sMessageNames[] = {
        "GrandGalaxyMapNewsGrandGalaxyExplain", "WorldMapNewsAnotherGalaxy", "WorldMapNewsAnotherGalaxy",
        "WorldMapNewsAnotherGalaxy",            "WorldMapNewsAnotherGalaxy", "WorldMapNewsAnotherGalaxy",
    };

    initNerve(&GrandGalaxyMapStateNewsMeisterAppear::sInstance, 0);
    mTalkCtrls = new TalkMessageCtrl*[6];
    for (s32 i = 0; i < 6; i++) {
        TVec3f offset;
        offset.x = 0.0f;
        offset.y = 0.0f;
        offset.z = 0.0f;
        mTalkCtrls[i] = MR::createTalkCtrlDirect(mHost->getMeister(), sMessageNames[i], offset, nullptr);
    }

    mIsDead = true;
}

void GrandGalaxyMapStateNews::appear() {
    mIsDead = false;
    setNerve(&GrandGalaxyMapStateNewsWait::sInstance);
    mHost->startDemo();
}

void GrandGalaxyMapStateNews::kill() {
    mHost->endDemo();
    MR::deactivateDefaultGameLayout();
    mIsDead = true;
}

bool GrandGalaxyMapStateNews::checkNews() {
    s32 worldNo = GameDataFunction::calcLastEnteredWorldNo();
    if (!GameDataFunction::isOnGameEventFlag("グランドギャラクシーマップ初プレイ")) {
        mNewsIndex = 0;
        return true;
    }

    if (GameSequenceFunction::isWorldMapAfterGameOver() && GameDataFunction::getPlayerGameOverNum() % 3 == 0) {
        if (worldNo == 2 && !GameDataFunction::isOnGameEventFlag("ワールド2ゲームオーバーによる誘導")) {
            mNewsIndex = 1;
            return true;
        }
        if (worldNo == 3 && !GameDataFunction::isOnGameEventFlag("ワールド3ゲームオーバーによる誘導")) {
            mNewsIndex = 2;
            return true;
        }
        if (worldNo == 4 && !GameDataFunction::isOnGameEventFlag("ワールド4ゲームオーバーによる誘導")) {
            mNewsIndex = 3;
            return true;
        }
        if (worldNo == 5 && !GameDataFunction::isOnGameEventFlag("ワールド5ゲームオーバーによる誘導")) {
            mNewsIndex = 4;
            return true;
        }
        if (worldNo == 6 && !GameDataFunction::isOnGameEventFlag("ワールド6ゲームオーバーによる誘導")) {
            mNewsIndex = 5;
            return true;
        }
    }

    return false;
}

void GrandGalaxyMapStateNews::exeMeisterAppear() {
    if (MR::isFirstStep(this)) {
        mHost->appearMeister();
    }

    if (!mHost->isMeisterTalkStarting()) {
        setNerve(&GrandGalaxyMapStateNewsTalk::sInstance);
    }
}

void GrandGalaxyMapStateNews::exeTalk() {
    if (MR::isFirstStep(this)) {
        mHost->setMeisterTalk();
    }

    if (MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrls[mNewsIndex])) {
        nextNews();
    }
}

void GrandGalaxyMapStateNews::exeTalkEnd() {
    if (MR::isFirstStep(this)) {
        mHost->setMeisterTalkEndWait();
    }

    if (!mHost->isMeisterTalkEnding()) {
        kill();
    }
}

void GrandGalaxyMapStateNews::startNews() {
    if (mNewsIndex == 0) { setNerve(&GrandGalaxyMapStateNewsTalk::sInstance); } else if (1 <= mNewsIndex && mNewsIndex <= 5) { setNerve(&GrandGalaxyMapStateNewsTalk::sInstance); } else { kill(); }
}

void GrandGalaxyMapStateNews::onNewsFlag() {
    GameSequenceFunction::clearWorldMapAfterGameOver();
    if (mNewsIndex == 0) {
        GameDataFunction::onGameEventFlag("グランドギャラクシーマップ初プレイ");
    }
    if (mNewsIndex == 1) {
        GameDataFunction::onGameEventFlag("ワールド2ゲームオーバーによる誘導");
    }
    if (mNewsIndex == 2) {
        GameDataFunction::onGameEventFlag("ワールド3ゲームオーバーによる誘導");
    }
    if (mNewsIndex == 3) {
        GameDataFunction::onGameEventFlag("ワールド4ゲームオーバーによる誘導");
    }
    if (mNewsIndex == 4) {
        GameDataFunction::onGameEventFlag("ワールド5ゲームオーバーによる誘導");
    }
    if (mNewsIndex == 5) {
        GameDataFunction::onGameEventFlag("ワールド6ゲームオーバーによる誘導");
    }
}

void GrandGalaxyMapStateNews::nextNews() {
    onNewsFlag();
    if (checkNews()) {
        startNews();
    } else {
        setNerve(&GrandGalaxyMapStateNewsTalkEnd::sInstance);
    }
}

GrandGalaxyMapStateNews::~GrandGalaxyMapStateNews() {}

inline void GrandGalaxyMapStateNews::exeWait() {
    if (MR::isGreaterEqualStep(this, 60)) {
        setNerve(&GrandGalaxyMapStateNewsMeisterAppear::sInstance);
    }
}
