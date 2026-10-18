#include "Game/NPC/MeisterDemoGrandStarReturn.hpp"
#include "Game/NPC/Meister.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvMeisterDemoGrandStarReturn {
    class MeisterDemoGrandStarReturnNrvWait : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            MeisterDemoGrandStarReturn* actor = static_cast<MeisterDemoGrandStarReturn*>(spine->mExecutor);
            if (MR::isDemoLastStep()) actor->mHost->endDemoState();
        }
        static MeisterDemoGrandStarReturnNrvWait sInstance;
    };
    MeisterDemoGrandStarReturnNrvWait MeisterDemoGrandStarReturnNrvWait::sInstance ATTRIBUTE_WEAK;
}

MeisterDemoGrandStarReturn::MeisterDemoGrandStarReturn(Meister* host, const JMapInfoIter& iter) : NerveExecutor("マイスターデモ[グランドスター帰還後]"), mHost(host) {
    registerDemo("グランドスター帰還後（ワールド１）", "Meister_AfterGrandStarReturn1", iter);
    registerDemo("グランドスター帰還後（ワールド２）", "Meister_AfterGrandStarReturn2", iter);
    registerDemo("グランドスター帰還後（ワールド３）", "Meister_AfterGrandStarReturn3", iter);
    registerDemo("グランドスター帰還後（ワールド４）", "Meister_AfterGrandStarReturn4", iter);
    registerDemo("グランドスター帰還後（ワールド５）", "Meister_AfterGrandStarReturn5", iter);
    initNerve(&NrvMeisterDemoGrandStarReturn::MeisterDemoGrandStarReturnNrvWait::sInstance, 0);
}
void MeisterDemoGrandStarReturn::registerDemo(const char* demo, const char* message, const JMapInfoIter& iter) {
    if (MR::isDemoExist(demo)) {
        TVec3f offset = mHost->getTalkOffset();
        TalkMessageCtrl* talk = MR::createTalkCtrlDirectOnRootNodeAutomatic(mHost, iter, message, offset, nullptr);
        DemoFunction::registerDemoTalkMessageCtrlDirect(mHost, talk, demo);
        MR::registerDemoActionFunctorDirect(mHost, MR::Functor_Inline(this, &MeisterDemoGrandStarReturn::start), demo, nullptr);
    }
}
void MeisterDemoGrandStarReturn::start() { mHost->startDemoState(this); }
MeisterDemoGrandStarReturn::~MeisterDemoGrandStarReturn() {}
