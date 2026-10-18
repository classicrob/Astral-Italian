#include "Game/NPC/MeisterDemoSimple.hpp"
#include "Game/NPC/Meister.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvMeisterDemoSimple {
    class MeisterDemoSimpleNrvWait : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            MeisterDemoSimple* actor = static_cast<MeisterDemoSimple*>(spine->mExecutor);
            if (MR::isDemoLastStep()) actor->mHost->endDemoState();
        }
        static MeisterDemoSimpleNrvWait sInstance;
    };
    MeisterDemoSimpleNrvWait MeisterDemoSimpleNrvWait::sInstance ATTRIBUTE_WEAK;
}

MeisterDemoSimple::MeisterDemoSimple(Meister* host, const JMapInfoIter& iter) : NerveExecutor("マイスターデモ[簡易]"), mHost(host) {
    registerDemo("ハラペコスターピースチコ紹介", "Meister_TicoFatIntroduce", iter);
    registerDemo("ノーマルエンディング後デモ", "Meister_AfterNormalEnding", iter);
    registerDemo("スター120個エンディング後デモ", "Meister_AfterStar120Ending", iter);
    registerDemo("ゲームオーバー後のデモ", "Meister_PriorityTalkPreviousScenarioMiss10_Flow000", iter);
    initNerve(&NrvMeisterDemoSimple::MeisterDemoSimpleNrvWait::sInstance, 0);
}
void MeisterDemoSimple::registerDemo(const char* demo, const char* message, const JMapInfoIter& iter) {
    if (MR::isDemoExist(demo)) {
        TVec3f offset = mHost->getTalkOffset();
        TalkMessageCtrl* talk = MR::createTalkCtrlDirectOnRootNodeAutomatic(mHost, iter, message, offset, nullptr);
        MR::onRootNodeAutomatic(talk);
        DemoFunction::registerDemoTalkMessageCtrlDirect(mHost, talk, demo);
        MR::registerDemoActionFunctorDirect(mHost, MR::Functor_Inline(this, &MeisterDemoSimple::start), demo, nullptr);
    }
}
void MeisterDemoSimple::start() { mHost->startDemoState(this); }
MeisterDemoSimple::~MeisterDemoSimple() {}
