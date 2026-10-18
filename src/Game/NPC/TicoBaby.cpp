// NONMATCHING: literal addresses and pending canonical helper target names.
#include "Game/NPC/TicoBaby.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
extern const char* lbl_807CFF40;
extern s32 lbl_807E3108;
namespace NrvTicoBaby {
class TicoBabyNrvWait : public Nerve {
public:
    virtual void execute(Spine* spine) const { static_cast<TicoBaby*>(spine->mExecutor)->exeWait(); }
    static TicoBabyNrvWait sInstance;
};
class TicoBabyNrvDemo : public Nerve {
public:
    virtual void execute(Spine* spine) const { static_cast<TicoBaby*>(spine->mExecutor)->exeDemo(); }
    static TicoBabyNrvDemo sInstance;
};
TicoBabyNrvWait TicoBabyNrvWait::sInstance;
TicoBabyNrvDemo TicoBabyNrvDemo::sInstance;
}
TicoBaby::TicoBaby(const char* name) : LiveActor(name) {}
TicoBaby::~TicoBaby() {}
void TicoBaby::init(const JMapInfoIter& iter) {
    MR::initActor(this,iter,"TicoBaby","TicoBabyAnim",nullptr,false);
    MR::tryInitDemoSheetAnim(this,iter,lbl_807CFF40,"DemoMeetTicoBaby");
    initNerve(&NrvTicoBaby::TicoBabyNrvWait::sInstance);
    makeActorAppeared();
    MR::startAction(this,"LieDown");
}
void TicoBaby::exeWait() {
    if(MR::isFirstStep(this)) {
        MR::moveVolumeStageBGM(0.0f,150);
        MR::startAction(this,"LieDown");
    }
    MR::startSoundInPlayerDistance(this,"Shining",-1,-1,1000.0f);
    if(MR::isDemoActive(lbl_807CFF40)) setNerve(&NrvTicoBaby::TicoBabyNrvDemo::sInstance);
}
void TicoBaby::exeDemo() {
    if(MR::isDemoActive(lbl_807CFF40) && !MR::isStoryBookDemoActive()) {
        if(MR::isDemoPartActive("マリオ驚き") && MR::getDemoPartStep("マリオ驚き")==lbl_807E3108) MR::startSubBGM("BGM_SMG2_EV_PROLO_03",false);
        if(MR::isDemoPartActive("チコ帽子の中へ") && MR::getDemoPartStep("チコ帽子の中へ")==114) MR::startSubBGM("BGM_SMG2_JG_MEETCHIKO",false);
    }
}
void TicoBaby::kill() {
    MR::stopSubBGM(60);
    MR::moveVolumeStageBGM(1.0f,60);
    MR::setPlayerSwingPermission(true);
    MR::startPlayerNormalBva();
    LiveActor::kill();
}
