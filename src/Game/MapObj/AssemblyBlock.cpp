#include "Game/MapObj/AssemblyBlock.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    static const s32 sDefaultTimer = 300;
    static const s32 sStepForAssemble = 10;
    static const s32 sStepForReturn = 10;
    static const s32 sFlashTime = 100;
    static const f32 sReturnOffset = 50.0f;
    // static const f32 sFloatAccelMax = _;
    // static const f32 sFloatCycleTune = _;
    static const f32 sFloatRotSpeedNoSign = 0.1f;
    static const char* const sReturnPosName = "合体ブロック故郷点";
};  // namespace

namespace NrvAssemblyBlock {
    NEW_NERVE(AssemblyBlockNrvWait, AssemblyBlock, Wait);
    NEW_NERVE(AssemblyBlockNrvAssemble, AssemblyBlock, Assemble);
    NEW_NERVE(AssemblyBlockNrvAssembleWait, AssemblyBlock, AssembleWait);
    NEW_NERVE(AssemblyBlockNrvReturn, AssemblyBlock, Return);
    NEW_NERVE(AssemblyBlockNrvTimer, AssemblyBlock, Timer);
};  // namespace NrvAssemblyBlock

AssemblyBlock::AssemblyBlock(const char* pName)
    : LiveActor(pName), mObjArg7(-1), mPlayerSearchDistance(-1.0f), mClippingCenter(0.0f, 0.0f, 0.0f), mFloatRotateSpeed(), mTimer(::sDefaultTimer),
      mBloomModel(), mIsTimerModel() {
    mCurrentMtx.identity();
    mAssembledMtx.identity();
    mReturnMtx.identity();
}

void AssemblyBlock::init(const JMapInfoIter& rIter) {
    TVec3f stack_3C;
    TVec3f stack_30;
    TVec3f stack_24;
    TVec3f stack_18;
    char name[256];

    MR::initDefaultPos(this, rIter);
    MR::tryRegisterNamePosLinkObj(this, rIter, ::sReturnPosName);

    if (!MR::tryFindLinkNamePos(this, ::sReturnPosName, mReturnMtx.toMtxPtr())) {
        stack_18.x = 0.0f;
        stack_18.y = 1000.0f;
        stack_18.z = 1000.0f;
        stack_3C.add(mPosition, stack_18);
        mReturnMtx.setTrans(stack_3C);
    }

    MR::getMapPartsObjectNameIfExistShapeID(name, sizeof(name), rIter);
    MR::initActor(this, rIter, name, false);

    if (MR::isEqualString(name, "AssemblyBlockPartsTimerA")) {
        mIsTimerModel = true;
    } else {
        mIsTimerModel = false;
    }

    mAssembledMtx.set(getBaseMtx());
    mCurrentMtx.set(mReturnMtx);
    MR::setBaseTRMtx(this, mCurrentMtx);

    if (MR::isEqualSubString(name, "PartsIce")) {
        MR::connectToSceneIndirectMapObj(this);
    } else {
        MR::connectToSceneMapObj(this);
    }

    initHitSensor(1);
    MR::addBodyMessageSensorMapObj(this);
    MR::initCollisionPartsAutoEqualScaleOne(this, name, getSensor(nullptr), nullptr);

    mAssembledMtx.getTrans(stack_30);
    mReturnMtx.getTrans(stack_24);
    mClippingCenter.add(stack_30, stack_24);
    mClippingCenter.x *= 0.5f;
    mClippingCenter.y *= 0.5f;
    mClippingCenter.z *= 0.5f;
    f32 boundingRadius;
    MR::calcModelBoundingRadius(&boundingRadius, this);
    MR::setClippingTypeSphere(this, boundingRadius + mClippingCenter.distance(stack_24), &mClippingCenter);
    MR::useStageSwitchAwake(this, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mPlayerSearchDistance);

    if (mPlayerSearchDistance <= 0.0f) {
        mPlayerSearchDistance = boundingRadius * 2.0f;
    }

    MR::getJMapInfoArg1NoInit(rIter, &mTimer);
    MR::getJMapInfoArg7NoInit(rIter, &mObjArg7);

    if (MR::getRandom(0l, 2l)) {
        mFloatRotateSpeed = ::sFloatRotSpeedNoSign;
    } else {
        mFloatRotateSpeed = -::sFloatRotSpeedNoSign;
    }

    if (mObjArg7 == -1) {
        initEffectKeeper(0, "AssemblyBlock", false);
    } else {
        initEffectKeeper(0, nullptr, false);
    }

    initSound(4, "AssemblyBlock", nullptr, TVec3f(0.0f));

    if (MR::isEqualString(name, "AssemblyBlockPartsTimerA")) {
        mBloomModel = MR::createBloomModel2(this, getBaseMtx());
    }

    MR::tryStartAllAnim(this, "Wait");
    initNerve(GET_NERVE(AssemblyBlock, AssemblyBlockNrvWait));
    makeActorAppeared();
}

void AssemblyBlock::exeWait() {
    if (MR::isFirstStep(this)) {
        if (MR::isEffectValid(this, "Blur")) {
            MR::deleteEffect(this, "Blur");
        }

        MR::validateCollisionParts(this);
        MR::validateHitSensors(this);
    }

    MR::rotateMtxLocalXDegree(mCurrentMtx.toMtxPtr(), mFloatRotateSpeed);
    MR::rotateMtxLocalYDegree(mCurrentMtx.toMtxPtr(), mFloatRotateSpeed);
    MR::rotateMtxLocalZDegree(mCurrentMtx.toMtxPtr(), mFloatRotateSpeed);

    f32 scalar = MR::sinDegree(getNerveStep());

    TVec3f trans;
    mCurrentMtx.getTrans(trans);
    trans.scaleAdd(scalar, mGravity, trans);
    mCurrentMtx.setTrans(trans);

    tryStartAssemble();
}

void AssemblyBlock::exeAssemble() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Blur");

        if (mIsTimerModel) {
            MR::startActionSound(this, "OjAssembleBlockStF");
        } else {
            MR::startActionSound(this, "OjAssembleBlockStart");
        }

        MR::invalidateCollisionParts(this);
        MR::invalidateHitSensors(this);
    }

    MR::blendMtx(mReturnMtx.toMtxPtr(), mAssembledMtx.toMtxPtr(), MR::calcNerveRate(this, ::sStepForAssemble), mCurrentMtx.toMtxPtr());

    if (MR::isStep(this, ::sStepForAssemble)) {
        if (mIsTimerModel) {
            MR::startActionSound(this, "OjAssembleBlockEdF");
        } else {
            MR::startActionSound(this, "OjAssembleBlockEnd");
        }

        setNerve(GET_NERVE(AssemblyBlock, AssemblyBlockNrvAssembleWait));
    }
}

void AssemblyBlock::exeAssembleWait() {
    if (MR::isFirstStep(this)) {
        if (MR::isEffectValid(this, "Blur")) {
            MR::deleteEffect(this, "Blur");
        }

        MR::tryRumblePadWeak(this, WPAD_CHAN0);
        MR::validateCollisionParts(this);
        MR::validateHitSensors(this);
    }

    if (mObjArg7 == 0) {
        setNerve(GET_NERVE(AssemblyBlock, AssemblyBlockNrvTimer));
    } else {
        tryStartReturn();
    }
}

void AssemblyBlock::exeReturn() {
    if (MR::isFirstStep(this)) {
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
        MR::emitEffect(this, "Blur");

        if (mIsTimerModel) {
            MR::startActionSound(this, "OjAssembleBlockRetF");
        } else {
            MR::startActionSound(this, "OjAssembleBlockReturn");
        }

        MR::invalidateCollisionParts(this);
        MR::invalidateHitSensors(this);
    }

    MR::blendMtx(mAssembledMtx.toMtxPtr(), mReturnMtx.toMtxPtr(), MR::calcNerveRate(this, ::sStepForReturn), mCurrentMtx.toMtxPtr());

    if (MR::isStep(this, ::sStepForReturn)) {
        setNerve(GET_NERVE(AssemblyBlock, AssemblyBlockNrvWait));
    }
}

void AssemblyBlock::exeTimer() {
    if (MR::isStep(this, mTimer - ::sFlashTime)) {
        MR::tryStartAllAnim(this, "Disappear");
    }

    if (MR::isStep(this, mTimer)) {
        if (mIsTimerModel) {
            MR::startActionSound(this, "OjAssembleBlockVanF");
        } else {
            MR::startActionSound(this, "OjAssembleBlockVanish");
        }

        if (mBloomModel) {
            mBloomModel->kill();
        }

        kill();
    }
}

void AssemblyBlock::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mCurrentMtx);
}

bool AssemblyBlock::tryStartAssemble() {
    bool isNotNear = !MR::isNearPlayerAnyTime(this, mPlayerSearchDistance);

    if (isNotNear) {
        return false;
    }

    if (MR::isPlayerDead()) {
        return false;
    }

    setNerve(GET_NERVE(AssemblyBlock, AssemblyBlockNrvAssemble));

    return true;
}

bool AssemblyBlock::tryStartReturn() {
    if (MR::isNearPlayerAnyTime(this, mPlayerSearchDistance + ::sReturnOffset)) {
        return false;
    }

    if (MR::isPlayerDead()) {
        return false;
    }

    setNerve(GET_NERVE(AssemblyBlock, AssemblyBlockNrvReturn));

    return true;
}
