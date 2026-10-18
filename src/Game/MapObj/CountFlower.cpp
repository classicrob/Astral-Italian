#include "Game/MapObj/CountFlower.hpp"
#include "Game/MapObj/PlantGroup.hpp"
#include "Game/MapObj/PlantObjFactory.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace {
    // PLACEHOLDER(m1): purpose of the original initialized vector is unknown; resolve when its users are identified.
    TVec2f sUnidentifiedParams(375.0f, 50.0f);
}
namespace NrvCountFlower {
    NEW_NERVE(CountFlowerNrvCount, CountFlower, Count);
    class CountFlowerNrvDone : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            CountFlower* actor = static_cast<CountFlower*>(spine->mExecutor);
            if (MR::isOnSwitchB(actor)) {
                MR::requestAppearPowerStar(actor);
                actor->kill();
            }
        }
        static CountFlowerNrvDone sInstance;
    };
    CountFlowerNrvDone CountFlowerNrvDone::sInstance ATTRIBUTE_WEAK;
}
CountFlower::CountFlower(const char* name) : LiveActor(name), mPlants(nullptr), mPlantCount(0), mAppearPowerStar(false) {}
CountFlower::~CountFlower() {}
void CountFlower::init(const JMapInfoIter& iter) {
    MR::connectToSceneMapObjMovement(this);
    MR::initDefaultPos(this, iter);
    mPlantCount = MR::getChildObjNum(iter);
    mPlants = new PlantGroup*[mPlantCount];
    for (s32 i=0; i<mPlantCount; i++) {
        const char* name = nullptr;
        MR::getChildObjName(&name, iter, i);
        if (MR::isEqualString(name,"PlantA") || MR::isEqualString(name,"PlantB") || MR::isEqualString(name,"PlantC") || MR::isEqualString(name,"PlantD"))
            mPlants[i] = static_cast<PlantGroup*>(PlantObjFactory::initChildObj(iter,i));
        else mPlants[i] = nullptr;
    }
    initNerve(&NrvCountFlower::CountFlowerNrvCount::sInstance);
    makeActorAppeared();
    MR::initUseStageSwitchWriteA(this,iter);
    MR::initUseStageSwitchReadB(this,iter);
    MR::registerDemoSimpleCastAll(this);
    if (MR::isObjectName(iter,"CountFlowerPowerStar")) {
        MR::declarePowerStar(this);
        mAppearPowerStar = true;
    }
    MR::invalidateClipping(this);
}
void CountFlower::exeCount() {
    s32 count = 0;
    for (s32 i=0; i<mPlantCount; i++) {
        if (mPlants[i]->isBloomEnd()) count++;
    }
    if (count == mPlantCount) activateSwitch();
}
void CountFlower::activateSwitch() {
    if (MR::isValidSwitchA(this)) MR::onSwitchA(this);
    if (mAppearPowerStar) {
        setNerve(&NrvCountFlower::CountFlowerNrvDone::sInstance);
        return;
    }
    kill();
}
