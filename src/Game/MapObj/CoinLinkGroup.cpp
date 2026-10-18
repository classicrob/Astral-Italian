#pragma auto_inline off
// NONMATCHING (logic written from asm, 43% overall): init (85%, Functor ptmf build order/regalloc),
// fn_8028FA10 exeMain (fn_8028FA10 not byte-identical yet), fn_8028FD60 Step3 nerve (execute inlined differently),
// fn_8028FDC0 DemoWait nerve (execute inlined differently), fn_8028F8A0/fn_8028F910 (loops; register diffs).
#include "Game/MapObj/CoinLinkGroup.hpp"
#include "Game/MapObj/Coin.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Functor.hpp"


namespace MR {
    void initChildObj(NameObj*, const JMapInfoIter&, s32);
    Coin* createCoin(const NameObj*, const char*);
    s32 getUncollectedCoinCount(const NameObj*);
    void startCSSound(const char*, const char*, s32, s32, f32);
}

namespace NrvCoinLinkGroup {
    class CoinLinkGroupNrvDemoWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static CoinLinkGroupNrvDemoWait sInstance;
    };
    CoinLinkGroupNrvDemoWait CoinLinkGroupNrvDemoWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(CoinLinkGroupNrvDemo, CoinLinkGroup, Demo);

    class CoinLinkGroupNrvStep3 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static CoinLinkGroupNrvStep3 sInstance;
    };
    CoinLinkGroupNrvStep3 CoinLinkGroupNrvStep3::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(CoinLinkGroupNrvMain, CoinLinkGroup, Main);

    class CoinLinkGroupNrvEmpty : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static CoinLinkGroupNrvEmpty sInstance;
    };
    CoinLinkGroupNrvEmpty CoinLinkGroupNrvEmpty::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvCoinLinkGroup

// Demo name is a Shift-JIS string in the target data pool.
static const char* const sDemoName = "\x8F\x6F\x8C\xBB";

void NrvCoinLinkGroup::CoinLinkGroupNrvDemoWait::execute(Spine* pSpine) const {
    CoinLinkGroup* pActor = reinterpret_cast< CoinLinkGroup* >(pSpine->mExecutor);
    if (MR::tryStartDemo(pActor, sDemoName)) {
        pActor->setNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvDemo::sInstance);
    }
}

void NrvCoinLinkGroup::CoinLinkGroupNrvStep3::execute(Spine* pSpine) const {
    CoinLinkGroup* pActor = reinterpret_cast< CoinLinkGroup* >(pSpine->mExecutor);
    if (MR::isStep(pActor, 3)) {
        pActor->appearCoinsOnSwitch();
        pActor->setNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvMain::sInstance);
    }
}

CoinLinkGroup::CoinLinkGroup(const char* pName) : LiveActor(pName) {
    mCameraInfo = nullptr;
    mChildren = nullptr;
    mChildCount = 0;
    mCoins = nullptr;
    mCoinCount = 0;
    mTime = -1;
}

void CoinLinkGroup::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    MR::initDefaultPos(this, rIter);
    initNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvMain::sInstance, 0);
    initCoins(rIter);
    createCoins();
    mTime = -1;
    MR::getJMapInfoArg1NoInit(rIter, &mTime);
    MR::invalidateClipping(this);
    MR::initActorCamera(this, rIter, &mCameraInfo);
    initSound(3, "CoinLinkGroup", nullptr, TVec3f(0.0f));
    MR::useStageSwitchAwake(this, rIter);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initUseStageSwitchWriteDead(this, rIter);
    if (MR::initUseStageSwitchReadB(this, rIter)) {
        MR::FunctorV0M< CoinLinkGroup*, void (CoinLinkGroup::*)() > killFunctor = MR::Functor(this, &CoinLinkGroup::killCoins);
        MR::FunctorV0M< CoinLinkGroup*, void (CoinLinkGroup::*)() > appearFunctor = MR::Functor(this, &CoinLinkGroup::appearCoinsOnSwitch);
        MR::listenStageSwitchOnOffB(this, appearFunctor, killFunctor);
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
    mChildren = new PlacementObj*[childNum];
    mChildCount = 0;
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
            mChildren[mChildCount] = pChild;
            mChildCount++;
        }
    }
}

void CoinLinkGroup::createCoins() {
    s32 total = 0;
    for (s32 i = 0; i < mChildCount; i++) {
        total += mChildren[i]->getPlacementNum();
    }
    if (total > 0) {
        mCoins = new Coin*[total];
        mCoinCount = 0;
        for (s32 i = 0; i < mChildCount; i++) {
            for (s32 k = 0; k < mChildren[i]->getPlacementNum(); k++) {
                TVec3f pos;
                mChildren[i]->calcPlacementPosition(k, &pos);
                Coin* pCoin = MR::createCoin(this, "\x83\x8A\x83\x93\x83\x4E\x83\x4F\x83\x8B\x81\x5B\x83\x76\x97\x70\x83\x52\x83\x43\x83\x93");
                MR::setPosition(pCoin, pos);
                pCoin->initWithoutIter();
                mCoins[mCoinCount] = pCoin;
                mCoinCount++;
            }
        }
    }
}

void CoinLinkGroup::killCoins() {
    for (s32 i = 0; i < mCoinCount; i++) {
        mCoins[i]->kill();
    }
}

void CoinLinkGroup::appearCoinsOnSwitch() {
    if (mTime >= 0) {
        appearCoinsFixTimer();
    } else {
        appearCoins();
    }
}

void CoinLinkGroup::appearCoins() {
    for (s32 i = 0; i < mCoinCount; i++) {
        mCoins[i]->appear();
    }
}

void CoinLinkGroup::appearCoinsFixTimer() {
    for (s32 i = 0; i < mCoinCount; i++) {
        mCoins[i]->appearFixTimer(mTime, -1);
    }
}

void CoinLinkGroup::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::startActorCameraTargetSelf(this, mCameraInfo, 30);
        appearCoinsOnSwitch();
    }
    if (MR::isGreaterStep(this, 90)) {
        MR::endDemo(this, "\x8F\x6F\x8C\xBB");
        MR::endActorCamera(this, mCameraInfo, false, -1);
        setNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvMain::sInstance);
    }
}

void CoinLinkGroup::exeMain() {
    if (!MR::isGreaterEqualStep(this, mTime)) {
        if (MR::isGreaterEqualStep(this, mTime - 90)) {
            MR::startActionSound(this, "TimerFast", -1, -1, -1);
        } else if (MR::isGreaterEqualStep(this, mTime - 180)) {
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
        if (getNerveStep() == mTime) {
            MR::startActionSound(this, "TimerUp", -1, -1, -1);
            if (MR::isValidSwitchDead(this)) {
                MR::onSwitchDead(this);
            }
            setNerve(&NrvCoinLinkGroup::CoinLinkGroupNrvEmpty::sInstance);
        }
    }
}

CoinLinkGroup::~CoinLinkGroup() {}
