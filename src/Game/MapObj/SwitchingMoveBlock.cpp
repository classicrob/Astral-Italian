#include "Game/Util/Functor.hpp"
#include "Game/MapObj/SwitchingMoveBlock.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/MapObj/MoveBlockFunction.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace NrvSwitchingMoveBlock {
    class SwitchingMoveBlockNrvWait : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            LiveActor* actor = static_cast<LiveActor*>(spine->mExecutor);
            if (MR::isFirstStep(actor)) MR::validateClipping(actor);
        }

        static SwitchingMoveBlockNrvWait sInstance;
    };
    SwitchingMoveBlockNrvWait SwitchingMoveBlockNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(SwitchingMoveBlockNrvMove, SwitchingMoveBlock, Move);
};  // namespace NrvSwitchingMoveBlock

SwitchingMoveBlock::SwitchingMoveBlock(const char* pName) : LiveActor(pName) {
    mMoveFunction = nullptr;
}

void SwitchingMoveBlock::init(const JMapInfoIter& rIter) {
    char objectName[256];
    MR::getMapPartsObjectNameIfExistShapeID(objectName, sizeof(objectName), rIter);
    bool hasInitCsv = MR::isValidInitActorCsvParser(objectName, nullptr);
    if (!hasInitCsv) MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, objectName, false);
    if (!hasInitCsv) initSound(4, objectName, nullptr, TVec3f(0.0f));
    MR::connectToSceneMapObj(this);
    MR::initMapPartsClipping(this, rIter, nullptr, false);
    mMoveFunction = new MoveBlockFunction;
    mMoveFunction->init(rIter);
    if (MR::isObjectName(rIter, "RedBlueTurnBlock")) {
        mMoveFunction->setMoveVec(TVec3f(0.0f, 0.0f, 0.0f));
        mMoveFunction->setRotateVec(TVec3f(0.0f, 0.0f, 180.0f));
        mMoveFunction->setMoveStep(40);
        MR::createPartsModelMapObjStrongLight(this, "土台", "RedBlueTurnBlockBase", nullptr);
    } else {
        TVec3f move(0.0f, 0.0f, 0.0f);
        MR::getJMapInfoArg0NoInit(rIter, &move.x);
        MR::getJMapInfoArg1NoInit(rIter, &move.y);
        MR::getJMapInfoArg2NoInit(rIter, &move.z);
        TVec3f rotate(0.0f, 0.0f, 0.0f);
        MR::getJMapInfoArg3NoInit(rIter, &rotate.x);
        MR::getJMapInfoArg4NoInit(rIter, &rotate.y);
        MR::getJMapInfoArg5NoInit(rIter, &rotate.z);
        mMoveFunction->setMoveVec(move);
        mMoveFunction->setRotateVec(rotate);
        mMoveFunction->setMoveStep(40);
    }
    initHitSensor(1);
    MR::addBodyMessageSensorMapObj(this);
    MR::initCollisionParts(this, objectName, getSensor(nullptr), nullptr);
    initNerve(&NrvSwitchingMoveBlock::SwitchingMoveBlockNrvWait::sInstance, 0);
    MR::initUseStageSwitchReadA(this, rIter);
    MR::listenStageSwitchOnOffA(this, MR::Functor(this, &SwitchingMoveBlock::listenSwitchOnA), MR::Functor(this, &SwitchingMoveBlock::listenSwitchOffA));
    MR::useStageSwitchAwake(this, rIter);
    makeActorAppeared();
}

void SwitchingMoveBlock::control() {
    mPosition.x = mMoveFunction->mPosition.x;
    mPosition.y = mMoveFunction->mPosition.y;
    mPosition.z = mMoveFunction->mPosition.z;
}

void SwitchingMoveBlock::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mMoveFunction->mRotation);
}

void SwitchingMoveBlock::listenSwitchOnA() {
    if (MR::isDead(this)) {
        return;
    }
    MR::invalidateClipping(this);
    mMoveFunction->setForward(true);
    setNerve(&NrvSwitchingMoveBlock::SwitchingMoveBlockNrvMove::sInstance);
    MR::startActionSoundAuto(this, "MoveStart", -1, -1, -1);
}

void SwitchingMoveBlock::listenSwitchOffA() {
    if (MR::isDead(this)) {
        return;
    }
    MR::invalidateClipping(this);
    mMoveFunction->setForward(false);
    setNerve(&NrvSwitchingMoveBlock::SwitchingMoveBlockNrvMove::sInstance);
    MR::startActionSoundAuto(this, "MoveStartReverse", -1, -1, -1);
}

void SwitchingMoveBlock::exeMove() {
    if (mMoveFunction->update()) {
        setNerve(&NrvSwitchingMoveBlock::SwitchingMoveBlockNrvWait::sInstance);
        MR::startActionSoundAuto(this, "MoveEnd", -1, -1, -1);
    } else {
        MR::startActionSoundAuto(this, "MoveLv", -1, -1, -1);
    }
}

SwitchingMoveBlock::~SwitchingMoveBlock() {
}
