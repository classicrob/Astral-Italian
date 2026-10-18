#include "Game/System/GameSequenceTitle.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/GameSequence.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"

extern "C" {
    s32 fn_804D8620();
    void fn_804D85D0();
    void fn_80060740();
};

namespace {
    class GameSequenceTitleNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceTitleNrvWait sInstance;
    };
    class GameSequenceTitleNrvOpen : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static GameSequenceTitleNrvOpen sInstance;
    };
    class GameSequenceTitleNrvEnd : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }
        static GameSequenceTitleNrvEnd sInstance;
    };

    GameSequenceTitleNrvWait GameSequenceTitleNrvWait::sInstance ATTRIBUTE_WEAK;
    GameSequenceTitleNrvOpen GameSequenceTitleNrvOpen::sInstance ATTRIBUTE_WEAK;
    GameSequenceTitleNrvEnd GameSequenceTitleNrvEnd::sInstance ATTRIBUTE_WEAK;
};  // namespace

namespace {
    void GameSequenceTitleNrvWait::execute(Spine* pSpine) const {
        GameSequenceTitle* pTitle = reinterpret_cast< GameSequenceTitle* >(pSpine->mExecutor);
        if (fn_804D8620() != 0) {
            pTitle->setNerve(&GameSequenceTitleNrvOpen::sInstance);
        }
    }
    void GameSequenceTitleNrvOpen::execute(Spine* pSpine) const {
        GameSequenceTitle* pTitle = reinterpret_cast< GameSequenceTitle* >(pSpine->mExecutor);
        if (MR::isFirstStep(pTitle)) {
            fn_804D85D0();
            MR::openWipeFade(0x1E);
        }
    }
};  // namespace

GameSequenceTitle::GameSequenceTitle(GameSequence* pHost) : ActorStateBase< GameSequence >("GameSequenceTitle", pHost) {
}

GameSequenceTitle::~GameSequenceTitle() {
}

void GameSequenceTitle::init() {
    initNerve(&GameSequenceTitleNrvWait::sInstance, 0);
}

void GameSequenceTitle::appear() {
    mIsDead = false;
    setNerve(&GameSequenceTitleNrvWait::sInstance);
    fn_80060740();
}
