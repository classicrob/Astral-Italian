// NONMATCHING: callback temporary allocation and literal addressing remain.
#include "Game/NPC/TogepinAttackMan.hpp"
#include "Game/NPC/PlayAttackMan.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/MapObj/Togepin.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvTogepinAttackMan {
    class TogepinAttackManNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static TogepinAttackManNrvWait sInstance;
    };
    TogepinAttackManNrvWait TogepinAttackManNrvWait::sInstance;
}
TogepinAttackMan::TogepinAttackMan(const char* name) : NPCActor(name),mScorer(nullptr),mTakeOutStar(nullptr),mFoulTalkCtrl(nullptr),mFoulTalkStarted(false) {}
TogepinAttackMan::~TogepinAttackMan() {}
void TogepinAttackMan::init(const JMapInfoIter& iter) {
    PlayAttackMan::initialize(this,iter,true);
    s32 count=MR::getChildObjNum(iter);
    for(s32 i=0;i<count;i++) {
        const char* name=nullptr;
        MR::getChildObjName(&name,iter,i);
        if(!mScorer && MR::isEqualString(name,"TogepinScorer")) {
            mScorer=new TogepinScorer("トゲピンスコアラー");
            MR::initChildObj(mScorer,iter,i);
        }
    }
    mFoulTalkCtrl=MR::createTalkCtrlDirect(this,iter,"ScoreAttackMan_Foul",TVec3f(0.0f,0.0f,0.0f),nullptr);
    MR::onRootNodeAutomatic(mFoulTalkCtrl);
    MR::declarePowerStar(this);
    mTakeOutStar=new TakeOutStar(this,"TakeOutStar","TakeOutStarScoreAttackMan",&NrvTogepinAttackMan::TogepinAttackManNrvWait::sInstance,nullptr);
    MR::registerBranchFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&TogepinAttackMan::branchFunc));
    MR::registerEventFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&TogepinAttackMan::eventFunc));
    MR::registerAnimeFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&TogepinAttackMan::animeFunc));
    MR::invalidateClipping(this);
}
void TogepinAttackMan::control() {
    NPCActor::control();
    if(mScorer->isPlaying()) {
        if(mScorer->mCounter>=3) {
            if(!mFoulTalkStarted) { MR::tryTalkForce(mFoulTalkCtrl); mFoulTalkStarted=true; }
        } else mFoulTalkStarted=false;
    } else mFoulTalkStarted=false;
}
bool TogepinAttackMan::branchFunc(u32 id) {
    switch(id) {
    case 0: return mScorer->isPlaying();
    case 1: return mScorer->isClear();
    case 2:
        if(MR::hasPowerStarInCurrentStageWithDeclarer(mName,-1)) return mScorer->isNewRecord();
        return false;
    default: return false;
    }
}
bool TogepinAttackMan::eventFunc(u32 id) {
    switch(id) {
    case 0: mScorer->start(); MR::invalidateClipping(this); return true;
    case 2: { TalkMessageCtrl* ctrl=mMsgCtrl; MR::setMessageArg(ctrl,mScorer->getScore()); return true; }
    case 3: { TalkMessageCtrl* ctrl=mMsgCtrl; MR::setMessageArg(ctrl,mScorer->getTargetScore()); return true; }
    case 4: { TalkMessageCtrl* ctrl=mMsgCtrl; MR::setMessageArg(ctrl,mScorer->getBestScore()); return true; }
    case 5: return mTakeOutStar->takeOut();
    default: return true;
    }
}
bool TogepinAttackMan::animeFunc(u32 id) {
    switch(id) {
    case 0: MR::startAction(this,"Talk"); return true;
    case 1: MR::startAction(this,"TalkOk"); return true;
    case 2: MR::startAction(this,"Jump"); return true;
    default: return true;
    }
}
