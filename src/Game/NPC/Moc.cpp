#include "Game/NPC/Moc.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace {
    inline bool isAllCometComplete() {
    ScenarioDataIter scenario=MR::makeBeginScenarioDataIter();
    while(!scenario.isEnd()) {
        GalaxyStatusAccessor accessor=scenario.makeAccessor();
        if(accessor.getPowerStarNum()!=0 && accessor.hasCometScenario() && !accessor.hasPowerStar(accessor.getCometScenarioId()) && MR::getGalaxyStarTotalNum(accessor.getName())==0) return false;
        scenario.goNext();
    }
    return true;
    }
}
namespace NrvMoc {
    class MocNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static MocNrvWait sInstance;
    };
    MocNrvWait MocNrvWait::sInstance;
}
Moc::Moc(const char* name) : NPCActor(name),mTakeOutStar(nullptr),mAllCometsComplete(false),mFaceIndex(-1) {}
Moc::~Moc() {}
// NONMATCHING: callback evaluation and iterator-local lifetimes differ.
void Moc::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("Moc");
    caps.setDefault();
    initialize(iter,caps,"Moc",nullptr,nullptr);
    mParam._18="Turn"; mParam._14="Wait"; mParam._1C="Talk"; mParam._20="Turn";
    mSpinAction = cNPCSpinDefault; mTrampledAction = cNPCTrampledDefault; mPointingAction = cNPCPointingDefault; mReactionAction = cNPCReactionDefault;
    mSpinAttackRange=450.0f;
    MR::registerBranchFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&Moc::branchFunc));
    MR::registerEventFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&Moc::eventFunc));
    mAllCometsComplete=isAllCometComplete();
    mFaceIndex=0;
    MR::getJMapInfoArg0NoInit(iter,&mFaceIndex);
    char face[64]="FaceA";
    face[4]+=static_cast<s8>(mFaceIndex);
    MR::startBva(this,face);
    MR::invalidateHitSensors(this);
    MR::validateHitSensor(this,"Body");
    MR::validateHitSensor(this,face);
    bool giveStar=false;
    MR::getJMapInfoArg1NoInit(iter,&giveStar);
    if(giveStar) {
        mTakeOutStar=new TakeOutStar(this,"TakeOutStar","TakeOutStarMoc",&NrvMoc::MocNrvWait::sInstance,nullptr);
        MR::declarePowerStar(this);
    }
    makeActorAppeared();
}
bool Moc::branchFunc(u32 id) {
    if(id==0) return mAllCometsComplete;
    if(id==1) return MR::getPurpleCoinNum()>=100;
    return false;
}
bool Moc::eventFunc(u32 id) { if(id==0) return mTakeOutStar->takeOut(); return true; }
