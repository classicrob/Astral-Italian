#include "Game/System/GameSequenceMarioFaceShipEvent.hpp"
#include "Game/System/GameSequenceMarioFaceShip.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/PostmanEvent.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/System/GameDataConst.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSequenceInGame.hpp"
#include "Game/System/MarioFacePlanetSwitch.hpp"
#include "Game/System/MarioFaceShipEventDataHolder.hpp"
#include "Game/System/PlayResultMarioFaceShipHolder.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"


namespace {
    class GameSequenceMarioFaceShipNrvStart : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceMarioFaceShipNrvStart sInstance;
    };
    class GameSequenceMarioFaceShipNrvEvent : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceMarioFaceShipNrvEvent sInstance;
    };
    class GameSequenceMarioFaceShipNrvEnd : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceMarioFaceShipNrvEnd sInstance;
    };

    GameSequenceMarioFaceShipNrvStart GameSequenceMarioFaceShipNrvStart::sInstance ATTRIBUTE_WEAK;
    GameSequenceMarioFaceShipNrvEvent GameSequenceMarioFaceShipNrvEvent::sInstance ATTRIBUTE_WEAK;
    GameSequenceMarioFaceShipNrvEnd GameSequenceMarioFaceShipNrvEnd::sInstance ATTRIBUTE_WEAK;
};  // namespace

GameSequenceMarioFaceShip::GameSequenceMarioFaceShip(GameSequenceInGame* pHost) : ActorStateBase< GameSequenceInGame >("[シーケンス]マリオ顔惑星", pHost) {
    mEvent = nullptr;
    mPlayResultHolder = nullptr;
}

void GameSequenceMarioFaceShip::init() {
    initNerve(&GameSequenceMarioFaceShipNrvStart::sInstance, 4);
    mEvent = new GameSequenceMarioFaceShipEvent(this);
    MR::initActorState(this, mEvent, &GameSequenceMarioFaceShipNrvEvent::sInstance, "Event");
    mPlayResultHolder = new PlayResultMarioFaceShipHolder();
}

void GameSequenceMarioFaceShip::appear() {
    mIsDead = false;
    setNerve(&GameSequenceMarioFaceShipNrvStart::sInstance);
}

PlayResultMarioFaceShipHolder* GameSequenceMarioFaceShip::getPlayResultHolder() const {
    return mPlayResultHolder;
}

void GameSequenceMarioFaceShip::exeStart() {
    if (GameSequenceFunction::isGameSequenceWipeStarted()) {
        bool isRetry = MR::getCurrentMarioStartIdInfo()._0 == 1;
        GameSceneFunction::startStagePlayRetry();
        if (isRetry) {
            MR::requestEndModelWipeScene();
        }
        if (mEvent->hasStartEvent()) {
            setNerve(&GameSequenceMarioFaceShipNrvEvent::sInstance);
        } else {
            if (isRetry) {
                MR::forceOpenWipeCircle();
                MR::forceOpenSystemWipeWhiteFade();
            } else {
                MR::openWipeCircle(-1);
            }
            setNerve(&GameSequenceMarioFaceShipNrvEnd::sInstance);
        }
        MarioFacePlanetSwitch::notifyApplySatisfied();
    }
}

GameSequenceMarioFaceShip::~GameSequenceMarioFaceShip() {
}

namespace {
    void GameSequenceMarioFaceShipNrvEnd::execute(Spine* pSpine) const {
        GameSequenceMarioFaceShip* pSeq = reinterpret_cast< GameSequenceMarioFaceShip* >(pSpine->mExecutor);
        if (MR::isFirstStep(pSeq)) {
            MR::startMarioFaceShipBGM(false);
            GameSequenceFunction::resetPlayResultInStage();
        }
    }
    void GameSequenceMarioFaceShipNrvEvent::execute(Spine* pSpine) const {
        GameSequenceMarioFaceShip* pSeq = reinterpret_cast< GameSequenceMarioFaceShip* >(pSpine->mExecutor);
        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&GameSequenceMarioFaceShipNrvEnd::sInstance);
        }
    }
    void GameSequenceMarioFaceShipNrvStart::execute(Spine* pSpine) const {
        GameSequenceMarioFaceShip* pSeq = reinterpret_cast< GameSequenceMarioFaceShip* >(pSpine->mExecutor);
        pSeq->exeStart();
    }
};  // namespace
