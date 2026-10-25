#include "Game/MapObj/Candlestand.hpp"
#include "Game/Effect/SpinPullParticleCallBack.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/Util.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

namespace {
    extern char sBurnSound[];

    inline void addItemHeight(TVec3f* pPosition, const TVec3f& up, f32 height) {
        *pPosition += up * height;
    }
};  // namespace

namespace NrvCandlestand {
    NEW_NERVE(HostTypeWaitFire, Candlestand, WaitFire);
    NEW_NERVE(HostTypeFire, Candlestand, Fire);
    NEW_NERVE(HostTypeBurn, Candlestand, Burn);
    class HostTypeEmit : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            Candlestand* actor = reinterpret_cast< Candlestand* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                actor->emitEffectFire();
                actor->setNerve(GET_NERVE(Candlestand, HostTypeBurn));
            }
        };
        static HostTypeEmit sInstance;
    };
    HostTypeEmit HostTypeEmit::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(HostTypeAttack, Candlestand, Attack);
    NEW_NERVE(HostTypeExtinguish, Candlestand, Extinguish);
    NEW_NERVE(HostTypeFlicker, Candlestand, Flicker);
};  // namespace NrvCandlestand

namespace {
    struct Param {
        /* 0x00 */ const char* mObjName;
        /* 0x04 */ f32 mClippingRadius;
        /* 0x08 */ f32 mSensorRange;
        /* 0x0C */ f32 mSensorHeight;
        /* 0x10 */ f32 mItemHeight;
        /* 0x14 */ f32 mCoinAngle;
        /* 0x18 */ bool mCanUseSwitch;
    };

    static const Param sParams[] = {
        {"PhantomCandlestand", 500.0f, 50.0f, 220.0f, 0.0f, 0.0f, false},
        {"TeresaMansionCandlestand", 800.0f, 70.0f, 300.0f, 220.0f, -5.0f, false},
        {"CandlestandIceVolcano", 500.0f, 150.0f, 220.0f, 0.0f, 0.0f, true},
    };

    const Param* getParam(const char* pObjName) NO_INLINE {
        for (u32 i = 0; i < ARRAY_SIZE(::sParams); i++) {
            if (MR::isEqualString(pObjName, ::sParams[i].mObjName)) {
                return &::sParams[i];
            }
        }

        return nullptr;
    }
};  // namespace

Candlestand::Candlestand(const char* pName) : MapObjActor(pName), mItem(-1), mHasItemAppear(), mSpinPtclCb(), mConnector() {
}

void Candlestand::init(const JMapInfoIter& rIter) {
    MapObjActor::init(rIter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    info.setupHitSensor();

    TVec3f sensorOffset(0.0f, ::getParam(mObjectName)->mSensorHeight, 0.0f);

    f32 sensorRange = ::getParam(mObjectName)->mSensorRange;
    info.setupHitSensorParam(8, sensorRange, sensorOffset);
    f32 clippingRadius = ::getParam(mObjectName)->mClippingRadius;
    info.setupClippingRadius(clippingRadius);
    info.setupNerve(GET_NERVE(Candlestand, HostTypeEmit));
    info.setupAffectedScale();
    info.setupSound(4, "CandleStand");
    initialize(rIter, info);

    const char* pObjName = mObjectName;
    ::getParam(pObjName);
    ::getParam(pObjName);
    MR::getJMapInfoArg0NoInit(rIter, &mItem);

    bool hasConnector = false;
    MR::getJMapInfoArg1NoInit(rIter, &hasConnector);
    if (hasConnector) {
        mConnector = new MapObjConnector(this);
    }

    if (mItem == -1) {
        MR::declareCoin(this, 1);
    }

    if (mItem == 0) {
        MR::declareStarPiece(this, 1);
    }

    MR::initStarPointerTarget(this, 150.0f, TVec3f(0.0f, 150.0f, 0.0f));

    if (::getParam(mObjectName)->mCanUseSwitch) {
        if (MR::isValidSwitchA(this)) {
            setNerve(GET_NERVE(Candlestand, HostTypeWaitFire));
        } else {
            setNerve(GET_NERVE(Candlestand, HostTypeEmit));
        }
    }

    MR::registerDemoSimpleCastAll(this);

    mSpinPtclCb = new SpinPullParticleCallBack();
}

void Candlestand::initAfterPlacement() {
    MapObjActor::initAfterPlacement();

    if (mConnector != nullptr) {
        mConnector->attachToUnder();
    }
}

void Candlestand::calcAndSetBaseMtx() {
    if (mConnector != nullptr) {
        mConnector->connect();
    }

    MapObjActor::calcAndSetBaseMtx();
}

void Candlestand::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    if (::getParam(mObjectName)) {
        return;
    }
}

void Candlestand::startClipped() {
    MapObjActor::startClipped();

    if (isNerve(GET_NERVE(Candlestand, HostTypeBurn)) || isNerve(GET_NERVE(Candlestand, HostTypeAttack))) {
        deleteEffectFire();
    }
}

void Candlestand::endClipped() {
    if (isNerve(GET_NERVE(Candlestand, HostTypeBurn)) || isNerve(GET_NERVE(Candlestand, HostTypeAttack))) {
        emitEffectFire();
    }

    MapObjActor::endClipped();
}

void Candlestand::control() {
    if (isNerve(GET_NERVE(Candlestand, HostTypeBurn))) {
        MR::attachSupportTicoToTarget(this);
    }
}

void Candlestand::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(Candlestand, HostTypeBurn))) {
        return;
    }

    f32 range = mScale.x * 50.0f;
    f32 radius = MR::getSensorRadius(pReceiver);

    if (!MR::isNear(pSender, pReceiver, range + radius)) {
        return;
    }

    if (!MR::sendMsgEnemyAttackFire(pReceiver, pSender)) {
        return;
    }

    setNerve(GET_NERVE(Candlestand, HostTypeAttack));
}

bool Candlestand::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgSupportTicoSpin(msg)) {
        return tryExtinguish();
    }

    if (!isNerve(GET_NERVE(Candlestand, HostTypeWaitFire))) {
        return false;
    }

    if (!MR::isMsgFireBallAttack(msg)) {
        return false;
    }

    f32 range = mScale.x * 150.0f;
    f32 radius = MR::getSensorRadius(pSender);

    if (!MR::isNear(pSender, pReceiver, range + radius)) {
        return false;
    }

    setNerve(GET_NERVE(Candlestand, HostTypeFire));
    return true;
}

bool Candlestand::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(Candlestand, HostTypeWaitFire))) {
        return false;
    }

    if (!MR::isMsgEnemyAttackFire(msg)) {
        return false;
    }

    f32 range = mScale.x * 150.0f;
    f32 radius = MR::getSensorRadius(pSender);

    if (!MR::isNear(pSender, pReceiver, range + radius)) {
        return false;
    }

    setNerve(GET_NERVE(Candlestand, HostTypeFire));
    return true;
}

bool Candlestand::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isInSpinStormRange(msg, pSender, pReceiver, (350.0f * mScale.x))) {
        return tryExtinguish();
    }

    return false;
}

bool Candlestand::tryExtinguish() {
    if (!isNerve(GET_NERVE(Candlestand, HostTypeBurn))) {
        return false;
    }

    if (::getParam(mObjectName)->mCanUseSwitch) {
        setNerve(GET_NERVE(Candlestand, HostTypeFlicker));
        return true;
    }

    setNerve(GET_NERVE(Candlestand, HostTypeExtinguish));
    return true;
}

void Candlestand::emitEffectFire() {
    if (isObjectName("TeresaMansionCandlestand")) {
        MR::emitEffect(this, "Fire01");
        MR::emitEffect(this, "Fire02");
    } else {
        MR::emitEffect(this, "Fire");
    }
}

void Candlestand::deleteEffectFire() {
    if (isObjectName("TeresaMansionCandlestand")) {
        MR::deleteEffect(this, "Fire01");
        MR::deleteEffect(this, "Fire02");
    } else {
        MR::deleteEffect(this, "Fire");
    }
}

void Candlestand::emitEffectExtinguishFire() {
    if (isObjectName("TeresaMansionCandlestand")) {
        MR::emitEffectWithParticleCallBack(this, "Extinguish01", mSpinPtclCb);
        MR::emitEffectWithParticleCallBack(this, "Extinguish02", mSpinPtclCb);
    } else {
        MR::emitEffectWithParticleCallBack(this, "Extinguish", mSpinPtclCb);
    }
}

void Candlestand::appearItem() {
    if (mHasItemAppear) {
        return;
    }

    f32 angle;
    f32 radians;

    TVec3f position(mPosition);
    f32 height = ::getParam(mObjectName)->mItemHeight;
    if (::getParam(mObjectName)->mCanUseSwitch) {
        height *= mScale.y;
    }

    TVec3f upVec;
    MR::calcUpVec(&upVec, this);

    addItemHeight(&position, upVec, height);

    if (mItem == -1) {
        angle = ::getParam(mObjectName)->mCoinAngle;
        if (MR::isNearZero(angle)) {
            MR::appearCoinPop(this, position, 1);
        } else {
            TVec3f front;
            MR::calcFrontVec(&front, this);

            TVec3f up;
            MR::calcUpVec(&up, this);

            TPos3f rotation;
            radians = MR::toRadian(angle);
            rotation.makeRotate(front, radians);
            rotation.mult(up, up);

            MR::appearCoinPopToDirection(this, position, up, 1);
        }
    } else if (mItem == 0) {
        MR::appearStarPiece(this, position, 1, 10.0f, 40.0f, false);
    }

    mHasItemAppear = true;
}

void Candlestand::exeWaitFire() {
}

void Candlestand::exeFire() {
    if (MR::isFirstStep(this)) {
        emitEffectFire();
        MR::startActionSound(this, "OjFireStandIgnit");
    }

    MR::startActionSound(this, ::sBurnSound);

    if (MR::isStep(this, 60)) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }

        setNerve(GET_NERVE(Candlestand, HostTypeBurn));
    }
}

namespace {
    char sBurnSound[] = "OjLvPhantomToachBurn";
};  // namespace

void Candlestand::exeExtinguish() {
    if (MR::isFirstStep(this)) {
        deleteEffectFire();
        emitEffectExtinguishFire();
        MR::startActionSound(this, "OjPhantomToachOff");
    }

    if (MR::isStep(this, 15)) {
        if (MR::isValidSwitchA(this)) {
            MR::offSwitchA(this);
        }

        appearItem();

        if (::getParam(mObjectName)->mCanUseSwitch) {
            setNerve(GET_NERVE(Candlestand, HostTypeWaitFire));
        }
    }
}

void Candlestand::exeFlicker() {
    if (MR::isFirstStep(this)) {
        deleteEffectFire();

        if (isObjectName("TeresaMansionCandlestand")) {
            MR::emitEffect(this, "Extinguish01");
            MR::emitEffect(this, "Extinguish02");
        } else {
            MR::emitEffect(this, "Extinguish");
        }

        MR::startActionSound(this, "OjPhantomToachOff");
    }

    if (MR::isStep(this, 30)) {
        emitEffectFire();
        setNerve(GET_NERVE(Candlestand, HostTypeBurn));
    }
}

void Candlestand::exeAttack() {
    if (MR::isStep(this, 30)) {
        setNerve(GET_NERVE(Candlestand, HostTypeBurn));
    } else {
        MR::startActionSound(this, ::sBurnSound);
    }
}

void Candlestand::exeBurn() {
    MR::startActionSound(this, ::sBurnSound);
}

Candlestand::~Candlestand() {
}
