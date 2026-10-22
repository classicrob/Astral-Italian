#include "Game/MapObj/RestartObj.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/SearchBeamer.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "math_types.hpp"
#include <JSystem/JMath.hpp>

namespace GameSequenceFunction {
    void collectTicoCoinIfSaved();
};  // namespace GameSequenceFunction

namespace NrvRestartObj {
    NEW_NERVE(RestartObjNrvWait, RestartObj, Wait);
    NEW_NERVE(RestartObjNrvCollect, RestartObj, Collect);

    class RestartObjNrvCollectAfter : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            RestartObj* actor = reinterpret_cast< RestartObj* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                MR::tryStartActionNotPlaying(actor, "After");
            }
        };

        static RestartObjNrvCollectAfter sInstance;
    };
    RestartObjNrvCollectAfter RestartObjNrvCollectAfter::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(RestartObjNrvShake, RestartObj, Shake);
    NEW_NERVE(RestartObjNrvAfter, RestartObj, After);
};  // namespace NrvRestartObj

namespace {
    static TVec2f sRestartObjSize(12.0f, 10.0f);
}

RestartObj::RestartObj(const char* pName) : LiveActor(pName), mIdInfo(nullptr), mConnector(nullptr) {
}

void RestartObj::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "RestartObj", false);
    s32 no = -1;
    MR::getJMapInfoArg0NoInit(rIter, &no);
    mIdInfo = new JMapIdInfo(no, rIter);
    mConnector = new MapObjConnector(this);

    mLockOnTarget = MR::createYoshiLockOnTarget(this, MR::getJointMtx(this, "Pole3"), TVec3f(0.0f));

    changeTexture(*mIdInfo == *MR::getRestartMarioNo());
    if (*mIdInfo == *MR::getRestartMarioNo() && !GameSequenceFunction::isPadMode2()) {
        initNerve(GET_NERVE(RestartObj, RestartObjNrvCollectAfter), 0);
    } else {
        initNerve(GET_NERVE(RestartObj, RestartObjNrvWait), 0);
    }
    makeActorAppeared();
}

void RestartObj::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Before");
        MR::emitEffect(this, "Light");
    }
    if (!MR::isValidSwitchA(this) || MR::isOnSwitchA(this)) {
        MR::attachSupportTicoToTarget(this);
        YoshiFunction::tryYoshiEat(mLockOnTarget);
    }
}

void RestartObj::exeCollect() {
    if (MR::isFirstStep(this)) {
        MR::tryRumblePadMiddle(this, 0);
        MR::setRestartMarioNo(*mIdInfo);
        GameSequenceFunction::collectTicoCoinIfSaved();
        MR::startAction(this, "Get");
        MR::startActionSound(this, "Get", -1, -1, -1);
        changeTexture(true);
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(RestartObj, RestartObjNrvCollectAfter));
    }
}

void RestartObj::exeShake() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shake");
        MR::startActionSound(this, "Shake", -1, -1, -1);
        MR::tryRumblePadWeak(this, 0);
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(RestartObj, RestartObjNrvAfter));
    }
}

void RestartObj::exeAfter() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "After");
    }
    if (MR::isStep(this, 5)) {
        setNerve(GET_NERVE(RestartObj, RestartObjNrvCollectAfter));
    }
}

void RestartObj::initAfterPlacement() {
    mConnector->attachToUnder();
}

void RestartObj::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    mConnector->connect();
    MR::calcUpVec(&mGravity, this);
    mGravity.negate();
}

void RestartObj::updateHitSensor(HitSensor* pSensor) {
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::calcUpVec(&up, this);
    TVec3f dir(0.0f, 0.0f, 0.0f);
    JMAVECScaleAdd(&up, &mPosition, &dir, 400.0f);
    TVec3f* pPlayerPos = MR::getPlayerPos();
    MR::calcPerpendicFootToLineInside(MR::getSensorPos2(pSensor), *pPlayerPos, mPosition, dir);
}

void RestartObj::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isPlayerInRush() || MR::isPlayerElementModeHopper()) {
        if (MR::isSensorPlayerOrRide(pReceiver) && !MR::isSensorGhost(pReceiver) && MR::isEqualSensor(pSender, this, "Body") &&
            isNerve(GET_NERVE(RestartObj, RestartObjNrvWait)) && !isNerve(GET_NERVE(RestartObj, RestartObjNrvCollect))) {
            setNerve(GET_NERVE(RestartObj, RestartObjNrvCollect));
        }
    } else if (MR::isEqualSensor(pSender, this, "Shake")) {
        if (isNerve(GET_NERVE(RestartObj, RestartObjNrvAfter))) {
            setNerve(GET_NERVE(RestartObj, RestartObjNrvAfter));
            return;
        }
        if (isNerve(GET_NERVE(RestartObj, RestartObjNrvCollectAfter))) {
            setNerve(GET_NERVE(RestartObj, RestartObjNrvShake));
        }
    }
}

bool RestartObj::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(RestartObj, RestartObjNrvWait))) {
        if (MR::isValidSwitchA(this) && !MR::isOnSwitchA(this)) {
            return false;
        }
    }
    if (!MR::isEqualSensor(pReceiver, this, "Spin")) {
        return false;
    }
    if (isNerve(GET_NERVE(RestartObj, RestartObjNrvCollect)) || isNerve(GET_NERVE(RestartObj, RestartObjNrvShake))) {
        return false;
    }
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        if (isNerve(GET_NERVE(RestartObj, RestartObjNrvWait))) {
            setNerve(GET_NERVE(RestartObj, RestartObjNrvCollect));
        } else {
            setNerve(GET_NERVE(RestartObj, RestartObjNrvShake));
        }
        return true;
    }
    return false;
}

bool RestartObj::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorPlayerOrRide(pSender)) {
        return false;
    }
    if (!isNerve(GET_NERVE(RestartObj, RestartObjNrvWait))) {
        return false;
    }
    if (isNerve(GET_NERVE(RestartObj, RestartObjNrvWait))) {
        if (MR::isValidSwitchA(this) && !MR::isOnSwitchA(this)) {
            return false;
        }
    }
    if (MR::isEqualSensor(pReceiver, this, "Body")) {
        if (MR::isMsgPlayerKick(msg)) {
            setNerve(GET_NERVE(RestartObj, RestartObjNrvCollect));
            return true;
        }
    }
    if (MR::isMsgYoshiStartTongue(msg)) {
        setNerve(GET_NERVE(RestartObj, RestartObjNrvCollect));
        return false;
    }
    return false;
}

void RestartObj::changeTexture(bool isLuigi) {
    f32 frame = 0.0f;
    if (isLuigi) {
        if (MR::isPlayerLuigi()) {
            frame = 2.0f;
        } else {
            frame = 1.0f;
        }
    }
    MR::startBtpAndSetFrameAndStop(this, "Change", frame);
}

RestartObj::~RestartObj() {
}

