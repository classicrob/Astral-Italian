#include "Game/MapObj/BenefitItemObj.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/FlashingCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

void BenefitItemObj_FORCE_MATCH_SDATA2() {
    (void)1.0f;
    (void)0.0f;
    (void)0.5f;
    (void)5.0f;
    (void)10.0f;
    (void)140.0f;
    (void)2000.0f;
    (void)50.0f;
    (void)150.0f;
    (void)6.2831854820251465f;
    (void)7.0f;
    (void)0.019999999552965164f;
    (void)1.100000023841858f;
    (void)0.009999999776482582f;
    (void)0.949999988079071f;
    (void)20.0f;
    (void)15.0f;
    (void)0.30000001192092896f;
    (void)0.0010000000474974513f;
    (void)0.20000000298023224f;
    (void)0.10000000149011612f;
    (void)250.0f;
    (void)60.0f;
}

extern "C" void fn_800345E0(LiveActor*, const char*);

class ScoreAttackFunction {
public:
    static void score1000Points(const LiveActor*);
};

namespace NrvBenefitItemObj {
    NEW_NERVE(HostTypeNrvWait, BenefitItemObj, Wait);
    NEW_NERVE(HostTypeNrvShoot, BenefitItemObj, Shoot);
    NEW_NERVE(HostTypeNrvCatch, BenefitItemObj, CatchNerve);
    NEW_NERVE(HostTypeNrvAppearGround, BenefitItemObj, AppearGround);
    class HostTypeNrvPreEscape : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static HostTypeNrvPreEscape sInstance;
    };
    HostTypeNrvPreEscape HostTypeNrvPreEscape::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(HostTypeNrvEscape, BenefitItemObj, Escape);
    NEW_NERVE(HostTypeNrvTicoHold, BenefitItemObj, TicoHold);
    NEW_NERVE(HostTypeNrvTicoPull, BenefitItemObj, TicoPull);
};  // namespace NrvBenefitItemObj

namespace {
    static f32 cSpdRotateY = JGeometry::TUtil< f32 >::PI() / 60.0f;
};  // namespace

BenefitItemObj::BenefitItemObj(const char* pName, const char* pModelName)
    : LiveActor(pName), mCatchActor(nullptr), _A0(0.0f, 0.0f, 0.0f), _AC(0.0f, 0.0f, 0.0f), _B8(0), _BA(0), _BC(0), _BE(0), _C0(0),
      _C4(0.0f, 0.0f, 0.0f), _D0(0.0f, 0.0f, 0.0f), _DC(0), _DD(0), _DE(0), _DF(0), _E0(0), _E4(0), _E8(0.0f, 0.0f, 0.0f), _F4(0), _F5(1), _F6(0),
      _F7(0), _F8(0), _F9(0), _FA(0), _FB(0), _FC(0.0f), _100(0.0f), mModelName(pModelName), mFlashCtrl(nullptr), mCameraInfo(nullptr), _140(0),
      _144(0), _148(0.0f, 0.0f, 0.0f) {
    PSMTXIdentity(_104);
}

void BenefitItemObj::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, mModelName, false);
    MR::offCalcGravity(this);

    MR::calcUpVec(&mGravity, this);
    mGravity = -mGravity;

    mFlashCtrl = new FlashingCtrl(this, true);

    MR::offBind(this);

    if (MR::isValidInfo(rIter)) {
        if (MR::isConnectedWithRail(rIter)) {
            initRailRider(rIter);
        }

        _FC = 5.0f;

        if (MR::isExistRail(this)) {
            s32 railArg;

            if (MR::getRailArg0NoInit(this, &railArg)) {
                _FC = railArg / 10.0f;
            }
        }
    }

    MR::setClippingFar100m(this);
    MR::setClippingTypeSphere(this, 140.0f);

    initNerve(GET_NERVE(BenefitItemObj, HostTypeNrvWait));

    s32 arg0 = -1;
    s32 arg1 = -1;
    s32 arg2 = -1;
    s32 arg3 = -1;
    s32 arg4 = -1;
    s32 arg5 = -1;
    s32 arg6 = -1;
    s32 arg7 = -1;

    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg0NoInit(rIter, &arg0);
        MR::getJMapInfoArg1NoInit(rIter, &arg1);
        MR::getJMapInfoArg2NoInit(rIter, &arg2);
        MR::getJMapInfoArg3NoInit(rIter, &arg3);
        MR::getJMapInfoArg4NoInit(rIter, &arg4);
        MR::getJMapInfoArg5NoInit(rIter, &arg5);
        MR::getJMapInfoArg6NoInit(rIter, &arg6);
        MR::getJMapInfoArg7NoInit(rIter, &arg7);
    } else {
        _F6 = 1;
        _F5 = 0;
        _DE = 0;
        _DD = 0;
    }

    switch (arg0) {
    case -1:
    case 0:
        _F5 = 0;
        break;
    }

    switch (arg1 & 0xF) {
    case 1:
        _DD = 1;
        break;

    case 2:
        _DD = 2;
        break;

    case 3:
        _DD = 3;
        break;

    case 4:
        _DD = 4;
        break;
    }

    if (arg1 != -1 && (arg1 & 0x80) != 0) {
        _F4 = 1;
    }

    switch (arg2) {
    case 1:
        _DE = 1;
        break;

    case 2:
        _DE = 2;
        break;

    case 3:
        _DE = 3;
        break;
    }

    bool setClipType = arg3 == 1;

    _E0 = 0;

    switch (arg4) {
    case -1:
        _E0 = 1;
        break;

    case 1:
        _E0 = 2;
        break;

    case 2:
        _E0 = 3;
        break;

    case 3:
        _E0 = 4;
        break;
    }

    if (arg5 == 1 && MR::isValidInfo(rIter)) {
        MR::initActorCamera(this, rIter, &mCameraInfo);
        _F9 = 1;
    }

    if (arg6 == -1) {
        arg6 = 0;
    }

    _DF = arg6;
    f32 shadowDist = 2000.0f;

    if (arg7 != -1) {
        shadowDist = arg7;
    }

    if (!_DC && MR::isValidInfo(rIter)) {
        MR::initUseStageSwitchWriteDead(this, rIter);

        if (MR::initUseStageSwitchReadAppear(this, rIter)) {
            MR::listenStageSwitchOnAppear(this, MR::FunctorV0M< BenefitItemObj*, void (BenefitItemObj::*)() >(this, &BenefitItemObj::appear));
            _F6 = 1;
        } else if (MR::initUseStageSwitchReadA(this, rIter)) {
            _F7 = 1;
        }
    }

    if (_DC) {
        _F5 = 0;

        makeActorDead();

        return;
    }

    if (_F5) {
        makeActorAppeared();

        if (_F6) {
            MR::invalidateHitSensors(this);
        } else {
            MR::emitEffect(this, "Glow");
        }
    } else if (_F6) {
        makeActorDead();
    } else {
        makeActorAppeared();
        MR::emitEffect(this, "Glow");
    }

    if (!MR::isDead(this) && MR::isValidInfo(rIter)) {
        MR::useStageSwitchAwake(this, rIter);
    }

    MR::getRotatedAxisY(&_AC, mRotation);

    if (_E0) {
        MR::setShadowDropLength(this, nullptr, shadowDist);

        if (arg7 == -1) {
            MR::onShadowVolumeCutDropLength(this, nullptr);
            MR::setShadowVolumeEndDropOffset(this, nullptr, 50.0f);
        }

        if ((_E0 & 0x1) != 0) {
            if ((_E0 & 0x2) != 0) {
                MR::onCalcShadowDropPrivateGravity(this, nullptr);
                MR::onCalcShadow(this, nullptr);
            } else {
                MR::onCalcShadowDropPrivateGravityOneTime(this, nullptr);
                MR::onCalcShadowOneTime(this, nullptr);
            }
        } else if ((_E0 & 0x2) != 0) {
            MR::onCalcShadow(this, nullptr);
        } else {
            MR::onCalcShadowOneTime(this, nullptr);
        }

        if (setClipType) {
            MR::setClippingTypeSphere(this, shadowDist);
        }
    }

    MR::setBinderIgnoreMovingCollision(this);
}

void BenefitItemObj::appear() {
    LiveActor::appear();

    _F6 = 0;
    _F5 = 1;
    mCatchActor = nullptr;

    switch (_DD) {
    case 0:
        if (_DE) {
            setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvPreEscape));
        }

        MR::validateHitSensors(this);
        setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvWait));
        break;

    case 1:
        appearGround(60);
        break;

    case 2:
        appearThrowUp();
        break;

    case 3:
        appearThrowUpQuestionBox();
        break;

    case 4:
        appearGround(0);
        break;
    }

    LiveActor::calcAndSetBaseMtx();

    PSMTXCopy(getBaseMtx(), _104);

    if (_F9) {
        _FB = 1;
        MR::invalidateClipping(this);
    }

    if (_DE) {
        MR::invalidateClipping(this);
    }

    if (_F4) {
        MR::emitEffect(this, "AppearPicture");
    }
}

void BenefitItemObj::appearPop() {
    _DD = 2;
    _E4 = 0;
    appear();
}

void BenefitItemObj::appearHop() {
    _DD = 2;
    _E4 = 1;
    appear();
}

void BenefitItemObj::appearThrow(const TVec3f& rVel) {
    _DD = 2;
    _E4 = 2;
    _E8 = rVel;
    appear();
}

void BenefitItemObj::initAfterPlacement() {
    MR::setClippingRangeIncludeShadow(this, &_148, 150.0f);

    if (MR::isExistRail(this)) {
        MR::moveCoordAndTransToNearestRailPos(this);
    }
}

void BenefitItemObj::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::emitEffect(this, "Glow");
}

void BenefitItemObj::control() {
    tryStartDemo();
    MR::calcGravityOrZero(this);
    tryEndDemo();

    if (!_F8) {
        tryVanish();
    }
}

void BenefitItemObj::doRotateY() {
    if (isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvAppearGround))) {
        _100 = 0.0f;
    } else {
        _100 = MR::repeat(_100 + ::cSpdRotateY, 0.0f, TWO_PI);
    }
}

void BenefitItemObj::initEscape() {
    _B8 = 600;
    _BA = 0;
    _BC = 0;
    _BE = 0;
    _C0 = 0;

    TVec3f rotZ;
    MR::getRotatedAxisZ(&rotZ, mRotation);
    _C4 = rotZ * 7.0f;

    MR::startActionSound(this, "OjKinoko1upRunStart");
}

void BenefitItemObj::doEscape() {
    MR::startActionSound(this, "OjLvKinoko1upRun");

    bool val = false;

    if (_DE == 2) {
        val = true;
    }

    if (MR::isBindedGround(this)) {
        f32 length = _C4.length();
        MR::vecKillElement(_C4, mGravity, &_C4);
        _C4.setLength(length);

        if (val) {
            TVec3f vec38(mPosition - *MR::getPlayerPos());
            MR::vecKillElement(vec38, mGravity, &vec38);
            MR::normalizeOrZero(&vec38);

            if (_C4.dot(vec38) < 0.0f) {
                _C4 += vec38 * 0.02f;
                _C4.x *= 1.1f;
                _C4.y *= 1.1f;
                _C4.z *= 1.1f;
            } else {
                _C4 += vec38 * 0.01f;
                _C4.x *= 1.1f;
                _C4.y *= 1.1f;
                _C4.z *= 1.1f;
            }
        }

        if (_C4.length() > 7.0f) {
            _C4.setLength(7.0f);
        }

        if (_FA) {
            _FA = 0;
            MR::startAction(this, "Land");
        }

        _D0 = mGravity * 5.0f;

        MR::emitEffect(this, "Move");
    } else {
        _C4.x *= 0.95f;
        _C4.y *= 0.95f;
        _C4.z *= 0.95f;
        _FA = 1;

        _D0 += mGravity * 1.0f;

        if (_D0.length() > 20.0f) {
            _D0.setLength(20.0f);
        }

        MR::deleteEffect(this, "Move");
    }

    if (!MR::isBindedWall(this)) {
        return;
    }

    if (_FA) {
        return;
    }

    if (!(MR::getWallNormal(this).dot(mVelocity) < 0.0f)) {
        return;
    }

    _C4 += -MR::getWallNormal(this) * MR::vecKillElement(_C4, MR::getWallNormal(this), &_C4);
    _D0 += -mGravity * 20.0f;

    _FA = 1;
}

void BenefitItemObj::appearGround(s32 delay) {
    _144 = delay;

    MR::startActionSound(this, "SyItemAppear");

    setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvAppearGround));

    MR::hideModel(this);

    MR::invalidateClipping(this);
}

void BenefitItemObj::appearThrowUp() {
    MR::showModel(this);

    MR::startActionSound(this, "SyItemAppear");

    MR::startAction(this, "Appear");

    TVec3f rotY;
    MR::getRotatedAxisY(&rotY, mRotation);
    _AC.set(rotY);

    setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvShoot));

    switch (_E4) {
    case 0:
        mVelocity.set(rotY * 10.0f);
        break;

    case 1:
        mVelocity.set(rotY * 15.0f);
        break;

    case 2:
        mVelocity.set(_E8);
        break;

    default:
        mVelocity.set(rotY * 15.0f);
        break;
    }

    MR::invalidateClipping(this);
}

void BenefitItemObj::appearThrowUpQuestionBox() {
    appearThrowUp();
}

void BenefitItemObj::exeCatch() {
    MR::addPlayerLeft(1);
    MR::stopSound(this, "SE_OJ_KINOKO_1UP_RUN_START", 0);
    MR::explainOneUpIfAtFirst();
    ScoreAttackFunction::score1000Points(this);
}

bool BenefitItemObj::tryStartTicoHold() {
    if (!isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvTicoHold)) && !isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvTicoPull)) &&
        !isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvCatch))) {
        _F8 = 1;
        setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvTicoHold));
        MR::invalidateClipping(this);
        MR::calcGravity(this);
        return true;
    }

    return false;
}

bool BenefitItemObj::tryEndTicoHold() {
    if (isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvTicoHold))) {
        setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvTicoPull));
        return true;
    }

    return false;
}

void BenefitItemObj::attachSupportTico() {
    MR::attachSupportTicoToTarget(this);
}

void BenefitItemObj::setNerveShoot() {
    setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvShoot));
}

void BenefitItemObj::exeCatchNerve() {
    if (MR::isFirstStep(this)) {
        fn_800345E0(this, "ゲット");
        MR::hideModel(this);
        exeCatch();
        kill();
    }

    MR::zeroVelocity(this);
}

void BenefitItemObj::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateHitSensors(this);
    }

    if (_F5 && !_F6) {
        if (_DF) {
            doRotateY();
        } else if (!_F6) {
            if (_DF & 0x2) {
                doRotateY();
            } else if (_DF & 0x1) {
                doRotateY();
            }
        }
    }

    if (!_F7 || MR::isOnSwitchA(this)) {
        attachSupportTico();
    }
}

void BenefitItemObj::exeShoot() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
    }

    TVec3f newVel(mVelocity);
    f32 verticalSpeed = MR::vecKillElement(newVel, mGravity, &newVel);

    if (MR::isBindedGround(this)) {
        MR::startAction(this, "Land");

        MR::validateHitSensors(this);

        MR::zeroVelocity(this);

        if (_DE == 0) {
            if (MR::isGreaterStep(this, 10)) {
                setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvWait));
            }
        } else {
            setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvEscape));
        }

        return;
    }

    verticalSpeed += 0.5f;

    if (getNerveStep() == 60) {
        MR::validateHitSensors(this);
    }

    if (MR::isGreaterStep(this, 600)) {
        kill();
    }

    if (MR::isBindedWall(this)) {
        f32 speed = MR::vecKillElement(newVel, MR::getWallNormal(this), &newVel);
        newVel -= MR::getWallNormal(this) * speed * 0.3f;
    }

    newVel += mGravity * verticalSpeed;
    mVelocity.set(newVel);

    attachSupportTico();
}

void BenefitItemObj::exeAppearGround() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
    }

    if (MR::isLessStep(this, _144)) {
        return;
    }

    if (MR::isStep(this, _144)) {
        MR::showModel(this);

        MR::startAction(this, "Appear");

        MR::startActionSound(this, "OjKinoko1upGrow");
    }

    if (MR::isActionEnd(this)) {
        if (!_DE) {
            setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvWait));
        } else if (_144 > 0) {
            setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvEscape));
        } else {
            setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvPreEscape));
        }

        MR::validateHitSensors(this);
    }

    attachSupportTico();
}

bool BenefitItemObj::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    return MR::isMsgStarPieceReflect(msg);
}

bool BenefitItemObj::canGet() const {
    if (_F7 && !MR::isOnSwitchA(this)) {
        return false;
    }

    if (mCatchActor != nullptr) {
        return false;
    }

    if (isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvAppearGround))) {
        return !MR::isLessStep(this, _144);
    }

    bool ret = false;

    if (isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvWait)) || isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvShoot)) ||
        isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvPreEscape)) || isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvEscape))) {
        ret = true;
    }

    return ret;
}

void BenefitItemObj::calcBaseMtxWait() {
    LiveActor::calcAndSetBaseMtx();

    PSMTXCopy(getBaseMtx(), _104);

    MtxPtr rot = MR::tmpMtxRotYRad(_100);
    PSMTXConcat(_104, rot, getBaseMtx());
}

void BenefitItemObj::calcBaseMtxTico() {
    TPos3f pos;
    MR::makeMtxUpFrontPos(&pos, -mGravity, -MR::getCamZdir(), mPosition);

    MtxPtr rot = MR::tmpMtxRotYRad(_100);
    PSMTXConcat(pos, rot, getBaseMtx());

    MR::setMtxTrans(getBaseMtx(), mPosition);
}

void MR::setMtxTrans(MtxPtr pMtx, const TVec3f& rVec) {
    setMtxTrans(pMtx, rVec.x, rVec.y, rVec.z);
}

void BenefitItemObj::updateUpVec() {
    if (MR::isBindedGround(this)) {
        _AC = MR::getGroundNormal(this);
    } else if (MR::isBindedWall(this)) {
        _AC = MR::getWallNormal(this);
    }
}

void BenefitItemObj::calcBaseMtxMove(const TVec3f& rFront) {
    TVec3f front(rFront);
    MR::normalizeOrZero(&front);

    if (MR::isNearZero(front, 0.001f)) {
        return;
    }

    if (MR::isSameDirection(_AC, front, 0.01f)) {
        MR::setMtxTrans(getBaseMtx(), mPosition);
        return;
    }

    TPos3f pos;
    MR::makeMtxUpFrontPos(&pos, _AC, front, mPosition);
    MR::blendMtx(_104, pos, 0.2f, _104);
    MtxPtr rot = MR::tmpMtxRotYRad(_100);
    PSMTXConcat(_104, rot, getBaseMtx());
    MR::setMtxTrans(getBaseMtx(), mPosition);
}

void BenefitItemObj::blendFrontDir() {
    TVec3f dir(mVelocity);
    MR::normalizeOrZero(&dir);
    MR::vecBlendSphere(_A0, dir, &dir, 0.1f);
    _A0 = dir;
}

bool BenefitItemObj::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isDead(this)) {
        return false;
    }

    if (MR::isMsgItemEndMoveOrDrop(msg)) {
        if (isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvTicoHold))) {
            if (MR::isMsgItemSupportTicoDrop(msg)) {
                return tryEndTicoHold();
            }

            if (MR::isMsgItemEndMove(msg)) {
                setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvWait));
                _F8 = 0;
            }

            return true;
        }

        return true;
    }

    if (MR::isMsgItemSupportTicoDemoShow(msg)) {
        MR::emitEffect(this, "Glow");
    }

    if (MR::isMsgItemSupportTicoDemoHide(msg)) {
        MR::forceDeleteEffect(this, "Glow");
    }

    if (_F7 && !MR::isOnSwitchA(this)) {
        return false;
    }

    if (MR::receiveItemShowMsg(msg, pSender, pReceiver)) {
        return true;
    }

    if (MR::receiveItemHideMsg(msg, pSender, pReceiver)) {
        return true;
    }

    if (MR::isMsgItemStartMove(msg)) {
        return tryStartTicoHold();
    }

    if (MR::isMsgItemGet(msg)) {
        if (canGet()) {
            mCatchActor = MR::getSensorHost(pSender);
            setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvCatch));
            return true;
        }

        return false;
    }

    if (_F8) {
        return false;
    }

    if (MR::isMsgInhaleBlackHole(msg)) {
        MR::emitEffect(this, "Vanish");
        kill();
    }

    return false;
}

void BenefitItemObj::exeEscape() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);

        MR::onBind(this);

        initEscape();
    }

    doEscape();

    if (_B8) {
        MR::showModel(this);

        _B8--;

        if (_C0 != 0) {
            _C0--;
            MR::hideModelAndOnCalcAnim(this);
        }

        if (_BE != 0) {
            _BE--;

            if (_BE == 0) {
                _BE = _BA;
                _C0 = _BC;
            }
        }

        if (_B8 == 0x3C) {
            _BA = 4;
            _BC = 2;
        } else if (_B8 == 0x78) {
            _BA = 8;
            _BC = 3;
        } else if (_B8 == 0xB4) {
            _BA = 12;
            _BC = 4;
            _BE = 12;
            _C0 = 4;
        }

        if (_B8 == 0) {
            MR::startActionSound(this, "Vanish");
            MR::emitEffect(this, "Vanish");
            makeActorDead();
        }
    }

    if (MR::isExistRail(this)) {
        MR::moveCoord(this, _FC);
        _C4 = MR::getRailPos(this) - mPosition;
    }

    mVelocity = _C4;
    mVelocity += _D0;

    if ((_DF & 0x4) != 0) {
        doRotateY();
    }

    if (_E0 && !(_E0 & 0x2)) {
        if ((_E0 & 0x1) != 0) {
            MR::onCalcShadowOneTime(this, nullptr);
        } else {
            MR::onCalcShadowDropPrivateGravityOneTime(this, nullptr);
        }
    }

    attachSupportTico();
}

void BenefitItemObj::exeTicoHold() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
        MR::onCalcGravity(this);
    }

    MR::zeroVelocity(this);
    MR::onCalcShadowDropPrivateGravityOneTime(this, nullptr);
}

void BenefitItemObj::exeTicoPull() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::offCalcGravity(this);
        mGravity = *MR::getPlayerGravity();
        _94 = mPosition;
    }

    TVec3f playerPos(*MR::getPlayerCenterPos());
    MR::vecBlend(&mPosition, _94, playerPos, MR::calcNerveEaseOutRate(this, 20));

    f32 parabola = 250.0f * MR::calcNerveParabolaValue(this, 30);
    mPosition += -mGravity * parabola;

    if (MR::isGreaterStep(this, 30)) {
        mCatchActor = this;
        setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvCatch));
    }
}

void BenefitItemObj::calcAndSetBaseMtx() {
    if (isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvWait)) || isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvShoot))) {
        calcBaseMtxWait();
    } else if (isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvTicoHold)) || isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvTicoPull))) {
        calcBaseMtxTico();
    } else if (isNerve(GET_NERVE(BenefitItemObj, HostTypeNrvPreEscape))) {
        updateUpVec();
        blendFrontDir();
        calcBaseMtxMove(_A0);
    } else {
        updateUpVec();
        calcBaseMtxMove(mVelocity);
    }
}

bool BenefitItemObj::tryStartDemo() {
    if (!_FB) {
        return false;
    }

    if (!MR::tryStartDemoWithoutCinemaFrame(this, "出現")) {
        return false;
    }

    _FB = 0;

    MR::startActorCameraTargetSelf(this, mCameraInfo, -1);

    MR::requestMovementOn(this);

    MR::pauseOffCameraDirector();
    _140 = MR::getActorCameraFrames(this, mCameraInfo);

    if (_140 == 0) {
        _140 = 120;
    }

    return true;
}

bool BenefitItemObj::tryEndDemo() {
    if (!_F9) {
        return false;
    }

    if (_140 == 0) {
        return false;
    }

    _140--;

    if (_140 == 0) {
        _F9 = 0;

        MR::endActorCamera(this, mCameraInfo, false, -1);
        MR::endDemo(this, "出現");

        MR::validateClipping(this);

        return true;
    }

    return false;
}

bool BenefitItemObj::tryVanish() {
    if (MR::isBindedGroundDamageFire(this)) {
        MR::emitEffect(this, "Vanish");
        kill();
        return true;
    }

    if (!MR::isNearZero(mVelocity, 0.001f) && MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        MR::emitEffect(this, "Vanish");
        kill();
        return true;
    }

    return false;
}

inline void BenefitItemObj::exePreEscape() {
    if (MR::isGreaterEqualStep(this, 30)) {
        setNerve(GET_NERVE(BenefitItemObj, HostTypeNrvEscape));
    }

    attachSupportTico();
}

inline void NrvBenefitItemObj::HostTypeNrvPreEscape::execute(Spine* pSpine) const {
    BenefitItemObj* actor = reinterpret_cast< BenefitItemObj* >(pSpine->mExecutor);
    actor->exePreEscape();
}
