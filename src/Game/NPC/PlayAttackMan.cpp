// NONMATCHING: literal addresses and equivalent register allocation in initialize.
#include "Game/NPC/PlayAttackMan.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvPlayAttackMan {
class PlayAttackManNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static PlayAttackManNrvWait sInstance;
};
PlayAttackManNrvWait PlayAttackManNrvWait::sInstance;
}
PlayAttackMan::PlayAttackMan(const char* name) : NPCActor(name),mTakeOutStar(nullptr) {}
PlayAttackMan::~PlayAttackMan() {}
void PlayAttackMan::init(const JMapInfoIter& iter) {
    initialize(this,iter,true);
    mTakeOutStar=new TakeOutStar(this,"TakeOutStar","TakeOutStarScoreAttackMan",&NrvPlayAttackMan::PlayAttackManNrvWait::sInstance,nullptr);
    MR::declarePowerStar(this);
    MR::registerEventFunc(mMsgCtrl,TalkMessageFunc_Inline(this,&PlayAttackMan::eventFunc));
    makeActorAppeared();
}
bool PlayAttackMan::eventFunc(u32 id) {
    if(id==0) return mTakeOutStar->takeOut();
    return true;
}
void PlayAttackMan::initialize(NPCActor* actor,const JMapInfoIter& iter,bool message) {
    NPCActorCaps caps("ScoreAttackMan");
    caps.setDefault();
    caps.mMessage=message;
    actor->initialize(iter,caps,"ScoreAttackMan",nullptr,nullptr);
    const char* turnAction = "Turn";
    const char* waitAction = "Wait";
    const char* talkAction = "Talk";
    actor->mParam._18=turnAction;
    actor->mParam._14=waitAction;
    actor->mParam._1C=talkAction;
    actor->mParam._20=turnAction;
    actor->setDefaults();
    actor->mSpinAttackRange=450.0f;
}
