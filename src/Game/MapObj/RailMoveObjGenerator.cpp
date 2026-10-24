#include "Game/MapObj/RailMoveObjGenerator.hpp"
#include "Game/MapObj/RailMoveObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvRailMoveObjGenerator {
    NEW_NERVE(RailMoveObjGeneratorNrvWait,RailMoveObjGenerator,Wait);
    NEW_NERVE(RailMoveObjGeneratorNrvStart,RailMoveObjGenerator,Wait);
    NEW_NERVE(RailMoveObjGeneratorNrvRepeat,RailMoveObjGenerator,Repeat);
    class RailMoveObjGeneratorNrvGenerate : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            RailMoveObj* obj;
            RailMoveObjGenerator* actor=static_cast<RailMoveObjGenerator*>(spine->mExecutor);
            actor->beforeGenerate();
            obj=actor->getDeadRailMoveObj();
            obj->appear();
            obj->setNerveMove();
            actor->setNerve(&RailMoveObjGeneratorNrvRepeat::sInstance);
        }
        static RailMoveObjGeneratorNrvGenerate sInstance;
    };
    RailMoveObjGeneratorNrvGenerate RailMoveObjGeneratorNrvGenerate::sInstance ATTRIBUTE_WEAK;
}
RailMoveObjGenerator::RailMoveObjGenerator(const char* name) : LiveActor(name),mObjectCount(0),mGenerateInterval(180),mInitialDelay(0),mObjects(nullptr) {}
RailMoveObjGenerator::~RailMoveObjGenerator() {}
void RailMoveObjGenerator::init(const JMapInfoIter& iter) {
    MR::initDefaultPos(this,iter);
    MR::connectToSceneMapObjMovement(this);
    fn_802EB5B0();
    fn_802EB5C0();
    MR::getJMapInfoArg6NoInit(iter,&mInitialDelay);
    MR::getJMapInfoArg7NoInit(iter,&mGenerateInterval);
    initRailMoveObjs(iter);
    MR::initUseStageSwitchReadB(this,iter);
    MR::invalidateClipping(this);
    initNerve(&NrvRailMoveObjGenerator::RailMoveObjGeneratorNrvWait::sInstance);
    MR::useStageSwitchSyncAppear(this,iter);
}
void RailMoveObjGenerator::fn_802EB5B0() {}
void RailMoveObjGenerator::fn_802EB5C0() {}
void RailMoveObjGenerator::kill() {
    for (s32 i=0;i<mObjectCount;i++) {
        if (!MR::isDead(mObjects[i])) mObjects[i]->kill();
    }
    LiveActor::kill();
}
void RailMoveObjGenerator::exeWait() {
    if (!MR::isValidSwitchB(this) || MR::isOnSwitchB(this)) {
        if (isNerve(&NrvRailMoveObjGenerator::RailMoveObjGeneratorNrvWait::sInstance)) setNerve(&NrvRailMoveObjGenerator::RailMoveObjGeneratorNrvStart::sInstance);
        if (getDeadRailMoveObj() && MR::isGreaterEqualStep(this,mInitialDelay)) setNerve(&NrvRailMoveObjGenerator::RailMoveObjGeneratorNrvGenerate::sInstance);
    }
}
void RailMoveObjGenerator::exeRepeat() {
    if (getDeadRailMoveObj() && MR::isGreaterEqualStep(this,mGenerateInterval)) setNerve(&NrvRailMoveObjGenerator::RailMoveObjGeneratorNrvGenerate::sInstance);
}
void RailMoveObjGenerator::beforeGenerate() {}
RailMoveObj* RailMoveObjGenerator::getDeadRailMoveObj() {
    for (s32 i=0;i<mObjectCount;i++) if (MR::isDead(mObjects[i])) return mObjects[i];
    return nullptr;
}
KoopaJrLv2LiftGenerator::~KoopaJrLv2LiftGenerator() {}

#include "Game/MapObj/RailMoveObjGenerator.hpp"
#include "Game/MapObj/RailMoveObj.hpp"
#include "Game/Util.hpp"
void KoopaJrLv2LiftGenerator::initRailMoveObjs(const JMapInfoIter& iter) {
    mObjectCount=MR::getChildObjNum(iter);
    mObjects=new RailMoveObj*[mObjectCount];
    for (s32 i=0;i<mObjectCount;i++) {
        const char* name=nullptr;
        MR::getChildObjName(&name,iter,i);
        mObjects[i]=new RailMoveObj("クッパJrLv2リフト[ジェネレータ]","KoopaJrLv2Lift");
        MR::initChildObj(mObjects[i],iter,i);
        mObjects[i]->makeActorDead();
    }
}
