#include "Game/MapObj/CoinGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/Coin.hpp"
#include "Game/Util.hpp"

namespace {
    static const s32 sAppearCameraTime = 30;
    static const s32 sAppearDelayTime = 3;
    static const s32 sDemoAppearTime = 90;
};  // namespace

namespace NrvCoinGroup {
    NEW_NERVE(CoinGroupNrvWait, CoinGroup, Wait);
    NEW_NERVE(CoinGroupNrvAppear, CoinGroup, Appear);
    NEW_NERVE(CoinGroupNrvTryStartDemo, CoinGroup, TryStartDemo);
    NEW_NERVE(CoinGroupNrvDemoAppear, CoinGroup, DemoAppear);
    NEW_NERVE(CoinGroupNrvKill, CoinGroup, Kill);
};  // namespace NrvCoinGroup

CoinGroup::CoinGroup(const char* pName) : LiveActor(pName), mCoinArray(), mCameraInfo(), mCoinCount(), mTimeLimit(-1), mIsPurpleCoinGroup() {
}

void CoinGroup::initCoinCount(const JMapInfoIter& rIter) {
    s32 count = 0;
    MR::getJMapInfoArg0NoInit(rIter, &count);
    mCoinCount = count;
    if (mCoinCount == 0) {
        mCoinCount = 8;
    }
}

// NONMATCHING: TVec3f(0.0f) sound temp inlined (target calls __ct__TVec3<f>Ff out of line; shared TVec.hpp issue)
void CoinGroup::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    initCoinCount(rIter);
    initSound(2, "CoinGroup", nullptr, TVec3f(0.0f));
    mTimeLimit = -1;
    MR::getJMapInfoArg1NoInit(rIter, &mTimeLimit);
    mCoinArray = new Coin*[mCoinCount];

    for (u32 i = 0; i < mCoinCount; i++) {
        if (mIsPurpleCoinGroup) {
            mCoinArray[i] = static_cast< Coin* >(MR::createPurpleCoin(this, getCoinName()));
        } else {
            mCoinArray[i] = static_cast< Coin* >(MR::createCoin(this, getCoinName()));
        }

        mCoinArray[i]->setShadowAndPoseModeFromJMapIter(rIter);

        if (mCoinArray[i]->mIgnoreGravity) {
            MR::initDefaultPos(mCoinArray[i], rIter);
        }

        MR::setScale(mCoinArray[i], 1.0f);
        mCoinArray[i]->initWithoutIter();
    }

    initCoinArray(rIter);
    placementCoin();
    MR::useStageSwitchAwake(this, rIter);
    MR::invalidateClipping(this);

    if (MR::tryRegisterDemoCast(this, rIter)) {
        MR::registerDemoActionFunctor(this, MR::Functor(this, &CoinGroup::appearCoinAll), "コイン出現");
        makeActorDead();
    } else if (MR::useStageSwitchSyncAppear(this, rIter)) {
        MR::initActorCamera(this, rIter, &mCameraInfo);

        if (MR::isExistActorCamera(mCameraInfo)) {
            initNerve(GET_NERVE(CoinGroup, CoinGroupNrvDemoAppear));
        } else {
            initNerve(GET_NERVE(CoinGroup, CoinGroupNrvAppear));
        }
    } else {
        initNerve(GET_NERVE(CoinGroup, CoinGroupNrvWait));
    }

    MR::initUseStageSwitchWriteA(this, rIter);

    if (MR::initUseStageSwitchReadB(this, rIter)) {
        MR::listenStageSwitchOnOffB(this, MR::Functor(this, &CoinGroup::appearCoinAll), MR::Functor(this, &CoinGroup::killCoinAll));
        killCoinAll();
    }
}

void CoinGroup::killCoinAll() {
    for (u32 i = 0; i < mCoinCount; i++) {
        mCoinArray[i]->kill();
    }
}

void CoinGroup::appearCoinAll() {
    if (mTimeLimit < 0) {
        appearCoinFix();
    } else {
        appearCoinAllTimer();
    }
}

void CoinGroup::appearCoinFix() {
    for (u32 i = 0; i < mCoinCount; i++) {
        mCoinArray[i]->appear();
    }
}

void CoinGroup::appearCoinAllTimer() {
    for (u32 i = 0; i < mCoinCount; i++) {
        mCoinArray[i]->appearFixTimer(mTimeLimit, -1);
    }
}

void CoinGroup::setCoinTrans(s32 coinIndex, const TVec3f& rPos) {
    MR::setPosition(mCoinArray[coinIndex], rPos);
    mCoinArray[coinIndex]->calcAndSetBaseMtx();
}

void CoinGroup::appear() {
    LiveActor::appear();

    if (isNerve(GET_NERVE(CoinGroup, CoinGroupNrvDemoAppear))) {
        MR::requestStartDemo(this, "出現", GET_NERVE(CoinGroup, CoinGroupNrvDemoAppear), GET_NERVE(CoinGroup, CoinGroupNrvTryStartDemo));
    }
}

void CoinGroup::exeAppear() {
    if (MR::isStep(this, ::sAppearDelayTime)) {
        if (mIsPurpleCoinGroup) {
            MR::startSystemSE("SE_SY_PURPLE_COIN_APPEAR", -1, -1);
        } else {
            MR::startSystemSE("SE_SY_COIN_APPEAR", -1, -1);
        }

        appearCoinAll();
        setNerve(GET_NERVE(CoinGroup, CoinGroupNrvKill));
    }
}

void CoinGroup::exeTryStartDemo() {
}

void CoinGroup::exeWait() {
    if (MR::isFirstStep(this)) {
        appearCoinFix();
        setNerve(GET_NERVE(CoinGroup, CoinGroupNrvKill));
    }
}

void CoinGroup::exeDemoAppear() {
    if (MR::isFirstStep(this)) {
        MR::startActorCameraTargetSelf(this, mCameraInfo, ::sAppearCameraTime);
        MR::startSystemSE("SE_SY_COIN_APPEAR", -1, -1);
        appearCoinAll();
    }

    if (MR::isGreaterStep(this, ::sDemoAppearTime)) {
        MR::endDemo(this, "出現");
        MR::endActorCamera(this, mCameraInfo, false, -1);
        setNerve(GET_NERVE(CoinGroup, CoinGroupNrvKill));
    }
}

void CoinGroup::exeKill() {
    if (!MR::isValidSwitchA(this)) {
        kill();
    } else if (MR::getUncollectedCoinCount(this) == 0) {
        MR::onSwitchA(this);
        kill();
    }
}

const char* CoinGroup::getCoinName() const {
    return "コイン(グループ配置)";
}

void CoinGroup::placementCoin() {
}
