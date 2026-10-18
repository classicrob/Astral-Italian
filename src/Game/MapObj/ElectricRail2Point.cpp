#pragma auto_inline off
#include "Game/MapObj/ElectricRail2Point2.hpp"
#include "Game/MapObj/ElectricRailHolder.hpp"
#include "Game/LiveActor/RailObj.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvElectricRail2Point {
    class ElectricRail2PointNrvMove : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<ElectricRail2Point*>(spine->mExecutor)->exeMove(); }
        static ElectricRail2PointNrvMove sInstance;
    };
}
ElectricRail2Point::ElectricRail2Point(const char* name) : LiveActor(name), mFootPos(0.0f,0.0f,0.0f), mCurIndex(0), mDivCount(60), mSideA(0.0f,1.0f,0.0f), mSideB(1.0f,0.0f,0.0f), mElemCount(0) {}
// NONMATCHING: vector copy/assignment inlining and array loop scheduling.
void ElectricRail2Point::init(const JMapInfoIter& iter) {
    ElectricRailFunction::registerRail(this,ElectricRail_Red);
    MR::initDefaultPos(this,iter);
    initHitSensor(1);
    MR::addHitSensorPosEnemyAttack(this,"Attack",8,35.0f,&mFootPos,TVec3f(0.0f,0.0f,0.0f));
    initSound(4,"ElectricRail",&mFootPos,TVec3f(0.0f));
    MR::connectToSceneMapObjMovement(this);
    MR::getJMapInfoArg0NoInit(iter,&mDivCount);
    s32 count=MR::getChildObjNum(iter);
    mRailGroup=new DeriveActorGroup<LiveActor>("２点移動電撃レールガイドグループ",2);
    mRailGroup->initWithoutIter();
    for (s32 i=0;i<count;i++) {
        const char* name=nullptr;
        MR::getChildObjName(&name,iter,i);
        if (MR::isEqualString(name,"RailObj")) {
            LiveActor* actor=MR::createRailObj("２点移動電撃レールガイド",this,iter,i);
            actor->makeActorAppeared();
            mRailGroup->registerActor(actor);
        }
    }
    mElems=new ElectricRailMovingPoint[count];
    mElemCount=count;
    for (s32 i=0;i<count;i++) {
        LiveActor* rail = mRailGroup->getActor(i);
        ElectricRailMovingPoint* point = &mElems[i];
        TVec3f pos(MR::getRailPos(rail));
        MR::setPosition(point,pos);
        mElems[i].initWithoutIter();
    }
    initShadow(iter);
    MR::setClippingTypeSphere(this,1000.0f);
    MR::setGroupClipping(this,iter,16);
    MR::invalidateClipping(this);
    initNerve(&NrvElectricRail2Point::ElectricRail2PointNrvMove::sInstance);
    MR::calcGravityOrZero(this);
    TVec3f up(-mGravity);
    if (!MR::isNearZero(up,0.001f)) mSideA=up;
    MR::useStageSwitchSyncAppear(this,iter);
}
