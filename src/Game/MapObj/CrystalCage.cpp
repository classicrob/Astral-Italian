#include "Game/MapObj/CrystalCage.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/DummyDisplayModel.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "math_types.hpp"
#include <JSystem/JGeometry/TUtil.hpp>
#include <JSystem/JMath.hpp>

namespace NrvCrystalCage {
    NEW_NERVE(CrystalCageNrvFall, CrystalCage, Fall);
    NEW_NERVE(CrystalCageNrvWait, CrystalCage, Wait);
    NEW_NERVE(CrystalCageNrvBreak, CrystalCage, Break);
    NEW_NERVE(CrystalCageNrvBreakAfter, CrystalCage, BreakAfter);
};  // namespace NrvCrystalCage
CrystalCage::CrystalCage(const char* pName)
    : LiveActor(pName), mCrystalCageType(0), mBreakObj(nullptr), _C4(1), _C8(0), mRumbleCalc(nullptr), _D0(0.0f, 0.0f, 1.0f), _DC(gZeroVec),
      _E8(gZeroVec), mDisplayModel(nullptr), _F8(gZeroVec), _104(0), _108(-1), mIsBreakObjVisible(false),
      mPowerStarState(0), mHasBinding(false), mIsInPlaneModeCube(false), mIsFallType(false), _110(gZeroVec) {
    _94.identity();
}

namespace {
    const Vec sCageOffsetA = {0.0f, 80.0f, -40.0f};
    const Vec sCageOffsetB = {0.0f, -37.0f, 0.0f};
}  // namespace

void CrystalCage::init(const JMapInfoIter& rIter) {
    if (MR::isEqualObjectName(rIter, "CrystalCageS")) {
        mCrystalCageType = 0;
    } else if (MR::isEqualObjectName(rIter, "CrystalCageM")) {
        mCrystalCageType = 1;
    } else {
        mCrystalCageType = 2;
    }

    if (mCrystalCageType == 0) {
        TVec3f trans;
        MR::getJMapInfoTrans(rIter, &trans);
        mIsInPlaneModeCube = MR::isInPlaneModeCube(trans);
    }

    MR::getJMapInfoArg4NoInit(rIter, &mIsFallType);

    const char* obj_name;
    MR::getObjectName(&obj_name, rIter);
    if (mIsInPlaneModeCube) {
        obj_name = "CrystalCage2D";
    }

    if (mIsFallType) {
        MR::initActor(this, rIter, obj_name, "FallType", false);
    } else {
        MR::initActor(this, rIter, obj_name, false);
    }

    initMapToolInfo(rIter);
    initModel();
    _E8.set< f32 >(mPosition);
    MR::setSensorFollowPos(this, "Body", &_E8);

    if (mIsInPlaneModeCube || mCrystalCageType == 2) {
        MR::invalidateHitSensors(this);
    }

    if (mHasBinding) {
        initBinder(50.0f, 0.0f, 0);
        MR::setBinderOffsetVec(this, &_110, false);
        MR::setBinderExceptActor(this, this);
    }

    if (mCrystalCageType == 2) {
        f32 mult = mScale.x * 250.0f;
        _C4 = 2;
        mDisplayModel = MR::createDummyDisplayModelCrystalItem(this, 7, TVec3f(0.0f, mult, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    } else {
        TVec3f size;
        TVec3f offset(0.0f, 0.0f, 0.0f);
        if (mCrystalCageType == 0) {
            size.set< f32 >(-30.0f, 100.0f, -30.0f);
        } else {
            size.set< f32 >(0.0f, 200.0f, 0.0f);
        }

        if (mIsInPlaneModeCube) {
            if (MR::getDummyDisplayModelId(rIter, -1) == 4) {
                size.set(sCageOffsetA);
            }

            offset.set(sCageOffsetB);
        }

        mDisplayModel = MR::createDummyDisplayModelCrystalItem(this, rIter, size, offset);

        if (mDisplayModel) {
            switch (MR::getDummyDisplayModelId(rIter, -1)) {
            case 0:
                MR::declareCoin(this, 1);
                break;
            case 4:
                MR::startBva(mDisplayModel, "Freeze");
                MR::declareStarPiece(this, 5);
                break;
            }
        }
    }

    if (isDisplayModelId(7)) {
        if (_108 == -1) {
            MR::declarePowerStar(this);
        } else {
            MR::declarePowerStar(this, _108);
        }
    }

    MR::startBva(this, obj_name);
    MR::setBvaFrameAndStop(this, 0.0f);

    if (_C4 > 1) {
        mRumbleCalc = new RumbleCalculatorCosMultLinear(4.0f, HALF_PI, 50.0f, 0x14);
    }

    MR::joinToGroupArray(this, rIter, mName, 0x20);

    if (mIsFallType) {
        TVec3f up(0.0f, 1.0f, 0.0f);
        MR::calcUpVec(&up, this);
        JMAVECScaleAdd(&up, &mPosition, &mPosition, 4000.0f);
        initNerve(GET_NERVE(CrystalCage, CrystalCageNrvFall), 0);
    } else {
        initNerve(GET_NERVE(CrystalCage, CrystalCageNrvWait), 0);
    }

    if (MR::tryRegisterDemoCast(this, rIter)) {
        if (mDisplayModel) {
            MR::tryRegisterDemoCast(mDisplayModel, rIter);
        }
    }

    if (mCrystalCageType == 2) {
        makeActorAppeared();
    } else {
        MR::useStageSwitchSyncAppear(this, rIter);
    }
}

void CrystalCage::initAfterPlacement() {
    if (!mIsBreakObjVisible || mHasBinding) {
        f32 val = mCrystalCageType == 2 ? 1000.0f : 300.0f;
        TVec3f up_vec;
        TVec3f stack_2C;
        TVec3f stack_20;
        MR::calcUpVec(&up_vec, this);
        // TODO: fix this during .scale pass
        stack_20.add(_DC, up_vec * (val * mScale.x));
        stack_2C.scale((-(2.0f * val) * mScale.x), up_vec);

        if (!MR::getFirstPolyOnLineToMapExceptActor(&_F8, 0, stack_20, stack_2C, this)) {
            _F8.set(_DC);
        }

        if (mHasBinding) {
            _110.sub(_F8, _DC);
            _110.add(up_vec * 50.0f);
            mVelocity.scale(-2.0f, up_vec);
        }
    }
}

void CrystalCage::kill() {
    LiveActor::kill();
    mBreakObj->kill();

    if (mDisplayModel) {
        mDisplayModel->kill();
    }
}

void CrystalCage::forceBreak() {
    if (isNerve(GET_NERVE(CrystalCage, CrystalCageNrvWait))) {
        if (!_104 || !MR::isClipped(this)) {
            MR::tryRumblePadStrong(this, WPAD_CHAN0);
            MR::invalidateClipping(this);
            MR::invalidateCollisionParts(this);
            MR::invalidateHitSensors(this);
            setNerve(GET_NERVE(CrystalCage, CrystalCageNrvBreak));
        } else {
            kill();
        }
    }
}

void CrystalCage::appear() {
    if (mIsFallType) {
        MR::invalidateClipping(this);
    }
    LiveActor::appear();
}

void CrystalCage::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!mCrystalCageType) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool CrystalCage::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(CrystalCage, CrystalCageNrvWait))) {
        return false;
    }

    if (MR::isMsgPlayerUpperPunch(msg)) {
        return false;
    }

    if (mCrystalCageType == 2) {
        if (!MR::isMsgPlayerHitAll(msg) || MR::isMsgSupportTicoSpin(msg)) {
            return false;
        }
    } else if (!MR::isMsgPlayerHitAll(msg)) {
        return false;
    }

    if (_C4 > 0 && !_C8) {
        if (_C4 == 1) {
            MR::invalidateClipping(this);
            MR::invalidateCollisionParts(this);
            MR::invalidateHitSensors(this);
            setNerve(GET_NERVE(CrystalCage, CrystalCageNrvBreak));
            return true;
        }

        if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && !MR::sendMsgReflect(pSender, pReceiver)) {
            return MR::isMsgPlayerSpinAttackOrSupportTico(msg);
        }

        {
            _C8 = 20;
            _C4 = _C4 - 1;

            MR::setBvaFrameAndStop(this, 1.0f);
            if (mCrystalCageType == 2) {
                MR::startActionSound(this, "OjCrystalCageLDamage", -1, -1, -1);
            } else {
                MR::startActionSound(this, "OjCrystalCageSDamage", -1, -1, -1);
            }

            mRumbleCalc->start(0);
            const TVec3f* pReceiverPos = MR::getSensorPos(pReceiver);
            _D0.sub(*MR::getSensorPos(pSender), *pReceiverPos);
            MR::normalize(&_D0);

            return true;
        }
    }

    return false;
}

bool CrystalCage::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mCrystalCageType != 2 && isNerve(GET_NERVE(CrystalCage, CrystalCageNrvWait))) {
        MR::invalidateClipping(this);
        MR::invalidateCollisionParts(this);
        MR::invalidateHitSensors(this);
        setNerve(GET_NERVE(CrystalCage, CrystalCageNrvBreak));
        return true;
    }

    return false;
}

void CrystalCage::initMapToolInfo(const JMapInfoIter& rIter) {
    MR::calcGravity(this);
    _DC.set(mPosition);

    if (MR::isEqualObjectName(rIter, "CrystalCageS") || MR::isEqualObjectName(rIter, "CrystalCageForOpening")) {
        mCrystalCageType = 0;
    } else if (MR::isEqualObjectName(rIter, "CrystalCageM")) {
        mCrystalCageType = 1;
    } else {
        mCrystalCageType = 2;
    }

    if (mCrystalCageType == 2) {
        MR::getJMapInfoArg0NoInit(rIter, &_108);
    } else {
        if (MR::isValidSwitchA(this)) {
            MR::listenStageSwitchOnA(this, MR::Functor_Inline(this, &CrystalCage::forceBreak));
        }

        MR::getJMapInfoArg0NoInit(rIter, &mIsBreakObjVisible);
        MR::getJMapInfoArg1NoInit(rIter, &mPowerStarState);
        MR::getJMapInfoArg2NoInit(rIter, &mHasBinding);
        MR::getJMapInfoArg3NoInit(rIter, &_104);
    }
}

void CrystalCage::initModel() {
    MR::makeMtxUpNoSupportPos(&_94, -mGravity, _DC);

    if (mCrystalCageType == 2) {
        mBreakObj = MR::createModelObjMapObjStrongLight("クリスタルケージ[大]壊れモデル", "CrystalCageLBreak", (MtxPtr)&_94);
    } else {
        mBreakObj = MR::createModelObjMapObjStrongLight("クリスタルケージ[小]壊れモデル", "CrystalCageSBreak", (MtxPtr)&_94);
    }

    MR::invalidateClipping(mBreakObj);
    MR::setScale(mBreakObj, mScale);
    MR::registerDemoSimpleCastAll(mBreakObj);
    mBreakObj->makeActorDead();
}

void CrystalCage::tryOnSwitchDead() {
    if (!MR::isValidSwitchDead(this)) {
        return;
    }

    s32 step = 0;

    if (mCrystalCageType == 2) {
        step = 0;
    }

    if (isDisplayModelId(3)) {
        step = 10;
    }

    if (MR::isStep(this, step)) {
        MR::onSwitchDead(this);
    }
}

// NONMATCHING: 68%; target saves LR/r31 after the null check (shrink-wrapped prologue), ours saves them up front (prologue placement only)
bool CrystalCage::isDisplayModelId(s32 id) {
    if (!mDisplayModel) {
        return false;
    }
    bool isSame = MR::getDummyDisplayModelId(mDisplayModel) == id;
    return isSame;
}

bool CrystalCage::hasPowerStarId() {
    switch (mPowerStarState) {
    case -1:
        return false;
    case 0:
        return true;
    default:
        return false;
    }
}

void CrystalCage::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "FallStart", -1, -1, -1);
        MR::emitEffect(this, "Blur");
    }

    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::calcUpVec(&up, this);
    TVec3f target(0.0f, 0.0f, 0.0f);
    JMAVECScaleAdd(&up, &_DC, &target, 4000.0f);
    f32 rate = MR::calcNerveEaseInRate(this, 30);
    MR::vecBlend(&mPosition, target, _DC, rate);
    MR::startActionSound(this, "Falling", -1, -1, -1);

    if (MR::isStep(this, 30)) {
        MR::deleteEffect(this, "Blur");
        MR::tryHitReaction(this, "\222\205\222n");
        MR::validateClipping(this);
        setNerve(GET_NERVE(CrystalCage, CrystalCageNrvWait));
    }
}

void CrystalCage::exeWait() {
    if (_C8 > 0) {
        _C8--;
    }

    if (mRumbleCalc) {
        if (mRumbleCalc->_4 < mRumbleCalc->_8) {
            mRumbleCalc->calc();
            TVec3f v9;
            v9.scale(mRumbleCalc->_C.y, _D0);
            mPosition.add(_DC, v9);
        } else {
            mPosition.set(_DC);
        }
    }

    if (mCrystalCageType == 0) {
        TVec3f up_vec;
        MR::calcUpVec(&up_vec, this);
        TVec3f v6;
        v6.scale(-40.0f, up_vec);
        v6.add(mPosition);
        TVec3f v5;
        v5.scale(140.0f, up_vec);
        v5.add(mPosition);
        MR::calcPerpendicFootToLineInside(&_E8, *MR::getPlayerPos(), v6, v5);
    }

    if (mCrystalCageType != 2) {
        MR::attachSupportTicoToTarget(this);
    }
}

void CrystalCage::exeBreak() {
    if (MR::isFirstStep(this)) {
        if (mIsBreakObjVisible) {
            MR::hideModel(this);
        } else {
            MR::setBvaFrameAndStop(this, mCrystalCageType == 2 ? 2.0f : 1.0f);
            mPosition.set(_F8);
        }

        if (mHasBinding) {
            MR::offBind(this);
            mVelocity.zero();
        }

        mBreakObj->appear();
        MR::startAction(mBreakObj, "Break");

        if (mCrystalCageType == 2) {
            MR::startActionSound(this, "OjCrystalCageLBreak", -1, -1, -1);
        } else {
            MR::emitEffect(this, "Break");
            MR::startActionSound(this, "OjCrystalCageSBreak", -1, -1, -1);

            if (hasPowerStarId()) {
                MR::startActionSound(this, "SyReadRiddleS", -1, -1, -1);
            }
        }

        if (isDisplayModelId(7)) {
            if (_108 == -1) {
                MR::requestAppearPowerStar(this, mDisplayModel->mPosition);
            } else {
                MR::requestAppearPowerStar(this, _108, mDisplayModel->mPosition);
            }
        }

        if (isDisplayModelId(0)) {
            MR::appearCoinPop(this, mDisplayModel->mPosition, 1);
        }

        if (isDisplayModelId(4)) {
            MR::appearStarPiece(this, mDisplayModel->mPosition, 5, 10.0f, 40.0f, false);
        }

        if (isDisplayModelId(5)) {
            MR::startActionSound(this, "OjCryCageMvTicoApr", -1, -1, -1);
        }

        if (isDisplayModelId(1)) {
            MR::startActionSound(this, "SvKinopioPuha", -1, -1, -1);
        }

        if (mDisplayModel) {
            mDisplayModel->kill();
        }
    }

    if (mCrystalCageType != 2 && MR::isStep(this, 2)) {
        MR::stopScene(3);
    }

    tryOnSwitchDead();

    if (MR::isActionEnd(mBreakObj)) {
        if (mIsBreakObjVisible) {
            kill();
        } else {
            setNerve(GET_NERVE(CrystalCage, CrystalCageNrvBreakAfter));
        }
    }
}

void CrystalCage::exeBreakAfter() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        mBreakObj->kill();
    }
}
