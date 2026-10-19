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
    caps.mYoshiInteraction=false;
    initialize(iter,caps,nullptr,nullptr,nullptr);
    MR::offTalkCinemaFrame(mMsgCtrl);
    TVec3f up;
    up.set<f32>(2.0f*(mOrientationQuat.x*mOrientationQuat.y)-2.0f*(mOrientationQuat.w*mOrientationQuat.z),(1.0f-2.0f*(mOrientationQuat.x*mOrientationQuat.x))-2.0f*(mOrientationQuat.z*mOrientationQuat.z),2.0f*(mOrientationQuat.y*mOrientationQuat.z)+2.0f*(mOrientationQuat.w*mOrientationQuat.x));
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
    if(mTrampledStart) MR::startActionSound(this,"Trampled",-1,-1,-1);
}
void StarListBoard::exeOpen() {
    if(MR::isFirstStep(this)) { MR::startActionSound(this,"Open",-1,-1,-1); mStarList->appear(); }
}
