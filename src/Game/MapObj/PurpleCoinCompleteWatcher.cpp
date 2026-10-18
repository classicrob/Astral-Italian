#include "Game/MapObj/PurpleCoinCompleteWatcher.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvPurpleCoinCompleteWatcher {
NEW_NERVE(PurpleCoinCompleteWatcherNrvWait, PurpleCoinCompleteWatcher, Wait);
class PurpleCoinCompleteWatcherNrvComplete : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static PurpleCoinCompleteWatcherNrvComplete sInstance;
};
PurpleCoinCompleteWatcherNrvComplete PurpleCoinCompleteWatcherNrvComplete::sInstance;
}
PurpleCoinCompleteWatcher::PurpleCoinCompleteWatcher(const char* name) : LiveActor(name) {}
void PurpleCoinCompleteWatcher::init(const JMapInfoIter& iter) {
    MR::connectToSceneMapObjMovement(this);
    initNerve(&NrvPurpleCoinCompleteWatcher::PurpleCoinCompleteWatcherNrvWait::sInstance);
    MR::invalidateClipping(this);
    MR::initUseStageSwitchWriteA(this, iter);
    makeActorAppeared();
}
void PurpleCoinCompleteWatcher::exeWait() {
    if (MR::isPurpleCoinNum100()) {
        if (MR::isValidSwitchA(this)) MR::onSwitchA(this);
        setNerve(&NrvPurpleCoinCompleteWatcher::PurpleCoinCompleteWatcherNrvComplete::sInstance);
    }
}
PurpleCoinCompleteWatcher::~PurpleCoinCompleteWatcher() {}
