#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

// Wait nerve: the comet is removed once its action has finished.
class MiniCometNrvWait : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        MiniComet* pComet = reinterpret_cast< MiniComet* >(pSpine->mExecutor);
        if (MR::isActionEnd(pComet)) {
            pComet->kill();
        }
    }

    static MiniCometNrvWait sInstance;
};

MiniComet::MiniComet(const char* pName) : LiveActor("コメット") {
    mName = pName;
}

void MiniComet::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "MiniComet", false);
    initNerve(&MiniCometNrvWait::sInstance, 0);
    MR::invalidateClipping(this);
    MR::registerDemoSimpleCastAll(this);
    makeActorAppeared();
}

void MiniComet::appear() {
    LiveActor::appear();
    MR::startAction(this, "Appear");
}

MiniComet::~MiniComet() {
}

MiniCometNrvWait MiniCometNrvWait::sInstance ATTRIBUTE_WEAK;
