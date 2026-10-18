#include "Game/MapObj/HipDropMoveBlock.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActionSoundUtil.hpp"


namespace NrvHipDropMoveBlock {
    class HipDropMoveBlockNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const;

        static HipDropMoveBlockNrvWait sInstance;
    };
    HipDropMoveBlockNrvWait HipDropMoveBlockNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(HipDropMoveBlockNrvWaitSwitch, HipDropMoveBlock, WaitSwitch);
    NEW_NERVE(HipDropMoveBlockNrvMove, HipDropMoveBlock, Move);
};  // namespace NrvHipDropMoveBlock

HipDropMoveBlock::HipDropMoveBlock(const char* pName) : LiveActor(pName) {
    mMoveFunction = nullptr;
    mFlag94 = true;
    mFlag95 = false;
    mFlag96 = false;
    mFlag97 = false;
    mSwitchCount = 0;
    mSwitches = nullptr;
    mActiveSwitch = nullptr;
}

void HipDropMoveBlock::init(const JMapInfoIter& rIter) {
    char sObjName[0x100];
    MR::getMapPartsObjectNameIfExistShapeID(sObjName, 0x100, rIter);
    if (MR::initActor(this, rIter, sObjName, false) == 0) {
        MR::initDefaultPos(this, rIter);
        MR::connectToSceneMapObj(this);
        initEffectKeeper(0, nullptr, false);
        initSound(4, nullptr, nullptr, TVec3f(0.0f));
        MR::initMapPartsClipping(this, rIter, nullptr, false);
    }
    initHitSensor(1);
    MR::addBodyMessageSensorMapObj(this);
    MR::initCollisionParts(this, sObjName, getSensor(nullptr), nullptr);

    mMoveFunction = new MoveBlockFunction();
    mMoveFunction->init(rIter);

    f32 dist = 100.0f;
    MR::getJMapInfoArg0NoInit(rIter, &dist);
    TVec3f dir(0.0f, dist, 0.0f);
    mMoveFunction->setMoveVec(dir);

    s32 val1 = 1;
    MR::getJMapInfoArg1NoInit(rIter, &val1);
    mMoveFunction->setReturnStep(val1);

    s32 val2 = 60;
    MR::getJMapInfoArg2NoInit(rIter, &val2);
    mMoveFunction->setMoveStep(val2);

    s32 val3 = -1;
    MR::getJMapInfoArg3NoInit(rIter, &val3);
    mFlag94 = val3 != 0;

    MR::getJMapInfoArg4NoInit(rIter, &val3);
    mFlag95 = (val3 + 1) != 0;

    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, nullptr);
    initNerve(GET_NERVE(HipDropMoveBlock, HipDropMoveBlockNrvWait), 0);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initUseStageSwitchReadB(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
    MR::tryRegisterDemoCast(this, rIter);
    mFlag97 = tryInitSwitch();
    makeActorAppeared();

    if (MR::isEqualString(sObjName, "HipDropMoveBlockHShape")) {
        mFlag96 = true;
    }
}

bool HipDropMoveBlock::tryInitSwitch() {
    const char* pNames = "InitHipDropMoveBlockSwitch";
    JMapInfo* pCsv = MR::tryCreateCsvParser(this, "\200j\"\200");
    if (pCsv != nullptr) {
        mSwitchCount = MR::getCsvDataElementNum(pCsv);
        mSwitches = new HipDropMoveBlockSwitch*[mSwitchCount];
        mSwitchParams = new HipDropMoveBlockSwitchParam*[mSwitchCount];

        f32 zero = 0.0f;
        for (s32 i = 0; i < mSwitchCount; i++) {
            HipDropMoveBlockSwitchParam* pParam = new HipDropMoveBlockSwitchParam;
            if (pParam != nullptr) {
                pParam->mOffset.set(0.0f, 0.0f, 0.0f);
                pParam->mRotate.x = zero;
                pParam->mRotate.y = zero;
                pParam->mRotate.z = zero;
            }
            mSwitchParams[i] = pParam;
            HipDropMoveBlockSwitchParam* pCur = mSwitchParams[i];
            MR::getCsvDataVec(&pCur->mOffset, pCsv, pNames + 0x34, i);
            MR::getCsvDataVec(&pCur->mRotate, pCsv, pNames + 0x3C, i);

            HipDropMoveBlockSwitch* pSwitch = new HipDropMoveBlockSwitch(pNames + 0x44, this, getBaseMtx(),
                                                                         pCur->mOffset, pCur->mRotate);
            mSwitches[i] = pSwitch;
            mSwitches[i]->initWithoutIter();
        }

        return true;
    }

    return false;
}

bool HipDropMoveBlock::isValidSensorHost(HitSensor* pSensor) {
    if (mSwitchCount == 0) {
        return true;
    }

    s32 offset = 0;
    s32 i = 0;
    while (i < mSwitchCount) {
        LiveActor* pHost = MR::getSensorHost(pSensor);
        if (*reinterpret_cast<LiveActor**>(reinterpret_cast<u8*>(mSwitches) + offset) == pHost) {
            return true;
        }
        i++;
        offset += 4;
    }

    return false;
}

void HipDropMoveBlock::control() {
    mPosition.set(mMoveFunction->mPos);
}

void HipDropMoveBlock::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, *reinterpret_cast<const TQuat4f*>(&mMoveFunction->mQuat));
}

bool HipDropMoveBlock::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg)) {
        if (!isValidSensorHost(pSender)) {
            return false;
        }

        if (mSwitchCount > 0) {
            mActiveSwitch = reinterpret_cast<HipDropMoveBlockSwitch*>(MR::getSensorHost(pSender));
        }

        if (!isNerve(GET_NERVE(HipDropMoveBlock, HipDropMoveBlockNrvWait))) {
            return false;
        }

        if (MR::isValidSwitchB(this) && !MR::isOnSwitchB(this)) {
            setNerve(GET_NERVE(HipDropMoveBlock, HipDropMoveBlockNrvWaitSwitch));
            return false;
        }

        TVec3f dir;
        mMoveFunction->calcMoveVec(&dir);
        MR::normalizeOrZero(&dir);

        TVec3f up;
        up.set(*MR::getPlayerGroundNormal());
        f32 dot = dir.dot(up);

        if (mFlag94 && dot > 0.9f) {
            if (!mMoveFunction->canMove(0)) {
                return false;
            }
            mMoveFunction->setForward(0);
        } else if (dot < -0.9f) {
            if (!mMoveFunction->canMove(1)) {
                return false;
            }
            mMoveFunction->setForward(1);
        } else {
            return false;
        }

        MR::invalidateClipping(this);
        setNerve(GET_NERVE(HipDropMoveBlock, HipDropMoveBlockNrvMove));
        return true;
    }

    return false;
}

void HipDropMoveBlock::exeWaitSwitch() {
    if (mActiveSwitch == nullptr) {
        setNerve(GET_NERVE(HipDropMoveBlock, HipDropMoveBlockNrvWait));
        return;
    }

    if (MR::isGreaterEqualStep(this, 30) && mActiveSwitch->tryOff()) {
        mActiveSwitch = nullptr;
        setNerve(GET_NERVE(HipDropMoveBlock, HipDropMoveBlockNrvWaitSwitch));
    }
}

void HipDropMoveBlock::exeMove() {
    const char* pNames = "InitHipDropMoveBlockSwitch";
    if (MR::isFirstStep(this)) {
        MR::tryStartAllAnim(this, pNames + 0x60);
        MR::startActionSoundAuto(this, pNames + 0x60, -1, -1, -1);
        MR::tryHitReaction(this, pNames + 0x68);
        if (MR::isValidSwitchA(this) && mFlag96) {
            MR::onSwitchA(this);
        }
    }

    MR::startActionSoundAuto(this, pNames + 0x74, -1, -1, -1);
    if (mMoveFunction->update()) {
        bool bMove = (mMoveFunction->isEndStep() && mMoveFunction->mFlag74) ||
                     (mMoveFunction->isStartStep() && !mMoveFunction->mFlag74);
        if (bMove) {
            if (MR::isValidSwitchA(this) && !mFlag96) {
                MR::onSwitchA(this);
            }
            MR::tryStartAllAnim(this, pNames + 0x80);
            MR::startActionSoundAuto(this, pNames + 0x80, -1, -1, -1);
            MR::tryHitReaction(this, pNames + 0x88);
            setNerve(GET_NERVE(HipDropMoveBlock, HipDropMoveBlockNrvWait));
            return;
        }

        if (MR::isValidSwitchA(this)) {
            MR::offSwitchA(this);
        }
        MR::tryStartAllAnim(this, pNames + 0x98);
        MR::startActionSoundAuto(this, pNames + 0x98, -1, -1, -1);
        MR::tryHitReaction(this, pNames + 0xA0);

        if (mActiveSwitch != nullptr) {
            if (mActiveSwitch->tryOff()) {
                mActiveSwitch = nullptr;
                goto block_20;
            }
        } else {
        block_20:
            setNerve(GET_NERVE(HipDropMoveBlock, HipDropMoveBlockNrvWait));
        }
    }
}

HipDropMoveBlock::~HipDropMoveBlock() {
}

void NrvHipDropMoveBlock::HipDropMoveBlockNrvWait::execute(Spine* pSpine) const {
    LiveActor* pActor = reinterpret_cast<LiveActor*>(pSpine->mExecutor);
    if (MR::isFirstStep(pActor)) {
        MR::validateClipping(pActor);
    }
}
