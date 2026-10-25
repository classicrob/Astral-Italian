#include "Game/Util/Functor.hpp"
// NONMATCHING: callback temporary allocation and literal addressing remain.
#include "Game/NPC/MameMuimuiAttackMan.hpp"
#include "Game/NPC/PlayAttackMan.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/Enemy/MameMuimuiScorer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvMameMuimuiAttackMan {
    class MameMuimuiAttackManNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static MameMuimuiAttackManNrvWait sInstance;
    };
    MameMuimuiAttackManNrvWait MameMuimuiAttackManNrvWait::sInstance;
}
MameMuimuiAttackMan::MameMuimuiAttackMan(const char* name) : NPCActor(name),mScorer(nullptr),mTakeOutStar(nullptr) {}
MameMuimuiAttackMan::~MameMuimuiAttackMan() {}
void MameMuimuiAttackMan::init(const JMapInfoIter& iter) {
    PlayAttackMan::initialize(this,iter,true);
    s32 count=MR::getChildObjNum(iter);
    for(s32 i=0;i<count;i++) {
        const char* name=nullptr;
        MR::getChildObjName(&name,iter,i);
        if(!mScorer) {
            if(MR::isEqualString(name,"MameMuimuiScorer")) {
                mScorer=new MameMuimuiScorer("マメムイムイスコアラー");
                MR::initChildObj(mScorer,iter,i);
            } else if(MR::isEqualString(name,"MameMuimuiScorerLv2")) {
                mScorer=new MameMuimuiScorer("マメムイムイスコアラーLv2");
                MR::initChildObj(mScorer,iter,i);
            }
        }
    }
    if(mScorer) mScorer->registerEndCallback(MR::Functor_Inline(this,&MameMuimuiAttackMan::facePlayerAfterGame));
    MR::declarePowerStar(this);
    mTakeOutStar=new TakeOutStar(this,"TakeOutStar","TakeOutStarScoreAttackMan",&NrvMameMuimuiAttackMan::MameMuimuiAttackManNrvWait::sInstance,nullptr);
    MR::registerBranchFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&MameMuimuiAttackMan::branchFunc));
    MR::registerEventFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&MameMuimuiAttackMan::eventFunc));
    MR::registerAnimeFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&MameMuimuiAttackMan::animeFunc));
}
void MameMuimuiAttackMan::facePlayerAfterGame() { tryPullNullNerve(); MR::turnPlayerToActor(this,180.0f); }
bool MameMuimuiAttackMan::branchFunc(u32 id) {
    switch(id) {
    case 0: return mScorer->isPlaying();
    case 1: return mScorer->isClear();
    case 2:
        if(MR::hasPowerStarInCurrentStageWithDeclarer(mName,-1)) return mScorer->isNewRecord();
        return false;
    default: return false;
    }
}
bool MameMuimuiAttackMan::eventFunc(u32 id) {
    switch(id) {
    case 0: mScorer->start(); return true;
    case 2: { TalkMessageCtrl* ctrl=mMsgCtrl; MR::setMessageArg(ctrl,mScorer->getScore()); return true; }
    case 3: { TalkMessageCtrl* ctrl=mMsgCtrl; MR::setMessageArg(ctrl,mScorer->getTargetScore()); return true; }
    case 4: { TalkMessageCtrl* ctrl=mMsgCtrl; MR::setMessageArg(ctrl,mScorer->getBestScore()); return true; }
    case 5: return mTakeOutStar->takeOut();
    default: return true;
    }
}
bool MameMuimuiAttackMan::animeFunc(u32 id) {
    switch(id) {
    case 0: MR::startAction(this,"Talk"); return true;
    case 1: MR::startAction(this,"TalkOk"); return true;
    case 2: MR::startAction(this,"Jump"); return true;
    default: return true;
    }
}
