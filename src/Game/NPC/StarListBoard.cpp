// NONMATCHING: literal addressing remains.
#include "Game/NPC/StarListBoard.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/Screen/AllStarList.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvStarListBoard {
    NEW_NERVE(StarListBoardNrvWait,StarListBoard,Wait);
    NEW_NERVE(StarListBoardNrvOpen,StarListBoard,Open);
}
StarListBoard::StarListBoard(const char* name) : NPCActor(name),mStarList(nullptr) {}
StarListBoard::~StarListBoard() {}
void StarListBoard::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("StarListBoard");
    caps.setDefault();
    caps.mWaitNerve=&NrvStarListBoard::StarListBoardNrvWait::sInstance;
    caps.mShadowSize=30.0f;
    caps.mSensorSize=100.0f;
    caps.mSensorOffset.set(0.0f,130.0f,0.0f);
    caps.mLightCtrl=false;
    caps._95=false;
    initialize(iter,caps,nullptr,nullptr,nullptr);
    MR::offTalkCinemaFrame(mMsgCtrl);
    TVec3f up;
    up.set<f32>(2.0f*(_A0.x*_A0.y)-2.0f*(_A0.w*_A0.z),(1.0f-2.0f*(_A0.x*_A0.x))-2.0f*(_A0.z*_A0.z),2.0f*(_A0.y*_A0.z)+2.0f*(_A0.w*_A0.x));
    MR::setShadowProjection(this,nullptr,mPosition,up,true);
    MR::offCalcShadow(this,nullptr);
    MR::registerEventFunc(mMsgCtrl,TalkMessageFunc_Inline(this,&StarListBoard::eventFunc));
    mStarList=new AllStarList("オールスターリスト",false);
    mStarList->initWithoutIter();
    makeActorAppeared();
}
bool StarListBoard::eventFunc(u32 id) {
    if(id==0) {
        if(!isNerve(&NrvStarListBoard::StarListBoardNrvOpen::sInstance)) {
            mStarList->requestMovementOn();
            pushNerve(&NrvStarListBoard::StarListBoardNrvOpen::sInstance);
            return false;
        }
        if(MR::isDead(mStarList)) { popNerve(); return true; }
        return false;
    }
    return true;
}
void StarListBoard::exeWait() {
    MR::tryTalkNearPlayer(mMsgCtrl);
    if(_D8) MR::startActionSound(this,"Trampled",-1,-1,-1);
}
void StarListBoard::exeOpen() {
    if(MR::isFirstStep(this)) { MR::startActionSound(this,"Open",-1,-1,-1); mStarList->appear(); }
}
