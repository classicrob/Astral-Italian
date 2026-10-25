#include "Game/MapObj/CoinLinkGroup.hpp"
#include "Game/MapObj/Coin.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Functor.hpp"


namespace {
    extern char sCoinAppearDemoName[];
}

namespace NrvCoinLinkGroup {
    class CoinLinkGroupNrvDemoWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;

        static CoinLinkGroupNrvDemoWait sInstance;
    };
    CoinLinkGroupNrvDemoWait CoinLinkGroupNrvDemoWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(CoinLinkGroupNrvDemo, CoinLinkGroup, Demo);

    class CoinLinkGroupNrvStep3 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;

        static CoinLinkGroupNrvStep3 sInstance;
    };
    CoinLinkGroupNrvStep3 CoinLinkGroupNrvStep3::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(CoinLinkGroupNrvMain, CoinLinkGroup, Main);

    class CoinLinkGroupNrvEmpty : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }

        static CoinLinkGroupNrvEmpty sInstance;
    };
    CoinLinkGroupNrvEmpty CoinLinkGroupNrvEmpty::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvCoinLinkGroup

inline void NrvCoinLinkGroup::CoinLinkGroupNrvDemoWait::execute(Spine* pSpine) const {
    CoinLinkGroup* pActor = reinterpret_cast< CoinLinkGroup* >(pSpine->mExecutor);

    if (MR::tryStartDemo(pActor, sCoinAppearDemoName)) {
        pActor->setNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvDemo::sInstance);
    }
}

inline void NrvCoinLinkGroup::CoinLinkGroupNrvStep3::execute(Spine* pSpine) const {
    CoinLinkGroup* pActor = reinterpret_cast< CoinLinkGroup* >(pSpine->mExecutor);

    if (MR::isStep(pActor, 3)) {
        pActor->appearCoinsOnSwitch();
        pActor->setNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvMain::sInstance);
    }
}

CoinLinkGroup::CoinLinkGroup(const char* pName) : LiveActor(pName) {
    mCameraInfo = nullptr;
    mPlacements = nullptr;
    mPlacementCount = 0;
    mCoins = nullptr;
    mCoinCount = 0;
    mCoinLifetime = -1;
}

void CoinLinkGroup::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    MR::initDefaultPos(this, rIter);
    initNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvMain::sInstance, 0);
    initCoins(rIter);
    createCoins();
    mCoinLifetime = -1;
    MR::getJMapInfoArg1NoInit(rIter, &mCoinLifetime);
    MR::invalidateClipping(this);
    MR::initActorCamera(this, rIter, &mCameraInfo);
    initSound(3, "CoinLinkGroup", nullptr, TVec3f(0.0f));
    MR::useStageSwitchAwake(this, rIter);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initUseStageSwitchWriteDead(this, rIter);
    if (MR::initUseStageSwitchReadB(this, rIter)) {
        MR::listenStageSwitchOnOffB(this, MR::Functor(this, &CoinLinkGroup::appearCoinsOnSwitch), MR::Functor(this, &CoinLinkGroup::killCoins));
        killCoins();
    }
    if (MR::useStageSwitchSyncAppear(this, rIter)) {
        if (MR::isExistActorCamera(mCameraInfo)) {
            setNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvDemoWait::sInstance);
        } else {
            setNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvStep3::sInstance);
        }
    } else {
        setNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvMain::sInstance);
        appearCoins();
    }
}

void CoinLinkGroup::initCoins(const JMapInfoIter& rIter) {
    s32 childNum = MR::getChildObjNum(rIter);
    mPlacements = new PlacementObj*[childNum];
    mPlacementCount = 0;
    for (s32 i = 0; i < childNum; i++) {
        const char* pName;
        MR::getChildObjName(&pName, rIter, i);
        PlacementObj* pChild = nullptr;
        if (MR::isEqualString(pName, "CircleCoinGroup")) {
            pChild = new PlacementCircleObj();
        } else if (MR::isEqualString(pName, "RailCoin")) {
            pChild = new PlacementRailObj();
        }
        if (pChild != nullptr) {
            MR::initChildObj(pChild, rIter, i);
            mPlacements[mPlacementCount] = pChild;
            mPlacementCount++;
        }
    }
}

void CoinLinkGroup::createCoins() {
    s32 total = 0;
    for (s32 i = 0; i < mPlacementCount; i++) {
        total += mPlacements[i]->getPlacementNum();
    }
    if (total > 0) {
        mCoins = new Coin*[total];
        mCoinCount = 0;
        for (s32 i = 0; i < mPlacementCount; i++) {
            for (s32 k = 0; k < mPlacements[i]->getPlacementNum(); k++) {
                TVec3f pos;
                mPlacements[i]->calcPlacementPosition(k, &pos);
                Coin* pCoin = MR::createCoin(this, "リンクグループ用コイン");
                MR::setPosition(pCoin, pos);
                pCoin->initWithoutIter();
                mCoins[mCoinCount] = pCoin;
                mCoinCount++;
            }
        }
    }
}

void CoinLinkGroup::killCoins() {
    for (u32 i = 0; i < mCoinCount; i++) {
        mCoins[i]->kill();
    }
}

void CoinLinkGroup::appearCoinsOnSwitch() {
    if (mCoinLifetime < 0) {
        appearCoins();
    } else {
        appearCoinsFixTimer();
    }
}

void CoinLinkGroup::appearCoins() {
    for (u32 i = 0; i < mCoinCount; i++) {
        mCoins[i]->appear();
    }
}

void CoinLinkGroup::appearCoinsFixTimer() {
    for (u32 i = 0; i < mCoinCount; i++) {
        mCoins[i]->appearFixTimer(mCoinLifetime, -1);
    }
}

namespace {
    char sCoinAppearDemoName[] = "出現";
}

void CoinLinkGroup::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::startActorCameraTargetSelf(this, mCameraInfo, 30);
        appearCoinsOnSwitch();
    }
    if (MR::isGreaterStep(this, 90)) {
        MR::endDemo(this, sCoinAppearDemoName);
        MR::endActorCamera(this, mCameraInfo, false, -1);
        setNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvMain::sInstance);
    }
}

void CoinLinkGroup::exeMain() {
    if (!MR::isGreaterEqualStep(this, mCoinLifetime)) {
        if (MR::isGreaterEqualStep(this, mCoinLifetime - 90)) {
            MR::startActionSound(this, "TimerFast", -1, -1, -1);
        } else if (MR::isGreaterEqualStep(this, mCoinLifetime - 180)) {
            MR::startActionSound(this, "TimerMedium", -1, -1, -1);
        } else {
            MR::startActionSound(this, "TimerSlow", -1, -1, -1);
        }
    }
    if (MR::getUncollectedCoinCount(this) == 0) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
        MR::addPlayerLeft(1);
        MR::startActionSound(this, "Applause", -1, -1, -1);
        MR::startCSSound("CS_PERC_AUD_GOOD", nullptr, -1, 22, 1.0f);
        kill();
    } else {
        if (getNerveStep() == mCoinLifetime) {
            MR::startActionSound(this, "TimerUp", -1, -1, -1);
            if (MR::isValidSwitchDead(this)) {
                MR::onSwitchDead(this);
            }
            setNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvEmpty::sInstance);
        }
    }
}

CoinLinkGroup::~CoinLinkGroup() {}
