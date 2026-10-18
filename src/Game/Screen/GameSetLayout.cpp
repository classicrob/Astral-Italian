#include "Game/Screen/GameSetLayout.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvGameSetLayout {
    NEW_NERVE(Appear, GameSetLayout, Appear);
    class Wait : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            LayoutActor* actor = static_cast<LayoutActor*>(spine->mExecutor);
            if (MR::isFirstStep(actor)) MR::startAnim(actor, "Wait", 0);
        }
        static Wait sInstance;
    };
    Wait Wait::sInstance;
    NEW_NERVE(End, GameSetLayout, End);
}

GameSetLayout::GameSetLayout() : LayoutActor("ゲームセットレイアウト", true) {}
void GameSetLayout::init(const JMapInfoIter&) {
    initLayoutManager("GameSet", 1);
    initNerve(&NrvGameSetLayout::Appear::sInstance);
    MR::connectToSceneLayout(this);
    kill();
}
void GameSetLayout::appear() {
    LayoutActor::appear();
    setNerve(&NrvGameSetLayout::Appear::sInstance);
}
void GameSetLayout::disappear() {
    if (!isNerve(&NrvGameSetLayout::End::sInstance)) setNerve(&NrvGameSetLayout::End::sInstance);
}
void GameSetLayout::exeAppear() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "Appear", 0);
    if (MR::isAnimStopped(this, 0)) setNerve(&NrvGameSetLayout::Wait::sInstance);
}
void GameSetLayout::exeEnd() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "End", 0);
    if (MR::isAnimStopped(this, 0)) kill();
}
GameSetLayout::~GameSetLayout() {}
