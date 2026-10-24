// NONMATCHING: Remaining nonmatches are callback temporary allocation, registers, and literal/nerve addressing.
#include "Game/NPC/TicoShopDice.hpp"
#include "Game/NPC/TicoStateEatCoin.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/MapObj/ItemDice.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util.hpp"


namespace NrvTicoShopDice {
    class EatCoin : public Nerve {
    public:
        virtual void execute(Spine* spine) const { MR::updateActorState(static_cast<TicoShopDice*>(spine->mExecutor)); }
        static EatCoin sInstance;
    };
    EatCoin EatCoin::sInstance;
    NEW_NERVE(Disappear, TicoShopDice, Disappear);
    NEW_NERVE(DiceAppear, TicoShopDice, DiceAppear);
    class DelayRecover : public Nerve {
    public:
        virtual void execute(Spine* spine) const;
        static DelayRecover sInstance;
    };
    DelayRecover DelayRecover::sInstance;
    NEW_NERVE(Recover, TicoShopDice, Recover);
    class DelayWait : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            TicoShopDice* actor = static_cast<TicoShopDice*>(spine->mExecutor);
            if (MR::isStep(actor, 60)) actor->setNerve(actor->mWaitNerve);
        }
        static DelayWait sInstance;
    };
    DelayWait DelayWait::sInstance;
    void DelayRecover::execute(Spine* spine) const {
        TicoShopDice* actor = static_cast<TicoShopDice*>(spine->mExecutor);
        if (MR::isStep(actor, 60)) actor->setNerve(&Recover::sInstance);
    }
}

TicoShopDice::TicoShopDice(const char* name) : TicoEat(name), mUseCoins(false), mPrice(30), mDiceCount(0), mDice(nullptr), mEatCoinState(nullptr) {}
TicoShopDice::~TicoShopDice() {}

void TicoShopDice::init(const JMapInfoIter& iter) {
    MR::getJMapInfoArg7NoInit(iter, &mUseCoins);
    NPCActorCaps caps("TicoShopDice");
    caps.setDefault();
    caps.mMessage = false;
    if (mUseCoins) caps.mNerveStep = 1;
    initialize(iter, caps, "TicoShopDice", nullptr, nullptr);
    mPrice = 30;
    MR::getJMapInfoArg0NoInit(iter, &mPrice);
    NPCActorItem item("TicoShopDice");
    MR::getNPCItemData(&item, 0);
    equipment(item, false);
    MR::startAction(mGoods0, "Shake");
    MR::startAction(mGoods1, "Shake");
    mDiceCount = 0;
    s32 childCount = MR::getChildObjNum(iter);
    for (s32 i = 0; i < childCount; i++) {
        const char* name = nullptr;
        MR::getChildObjName(&name, iter, i);
        if (MR::isEqualString(name, "LuckyDice")) mDiceCount++;
    }
    if (mDiceCount >= 0) {
        mDice = new LuckyDice*[mDiceCount];
        s32 diceIndex = 0;
        for (s32 i = 0; i < childCount; i++) {
            const char* name = nullptr;
            MR::getChildObjName(&name, iter, i);
            if (MR::isEqualString(name, "LuckyDice")) {
                mDice[diceIndex] = new LuckyDice("ラッキーダイス");
                MR::initChildObj(mDice[diceIndex], iter, i);
                mDice[diceIndex]->makeActorDead();
                diceIndex++;
            }
        }
    }
    initMessage(mUseCoins ? "TicoShopDiceCoin_Request" : "TicoShopDiceStarPiece_Request");
    MR::setMessageArg(mMsgCtrl, mPrice);
    MR::registerBranchFunc(getMsgCtrl(), TalkMessageFunc_Inline(this, &TicoShopDice::branchFunc));
    MR::registerEventFunc(getMsgCtrl(), TalkMessageFunc_Inline(this, &TicoShopDice::eventFunc));
    MR::setDistanceToTalk(mMsgCtrl, 350.0f);
    if (mUseCoins) {
        mEatCoinState = new TicoStateEatCoin(this, mPrice);
        MR::initActorState(this, mEatCoinState, &NrvTicoShopDice::EatCoin::sInstance, "EatDemoCoin");
        MR::invalidateHitSensor(this, "Mouth");
    } else initStarPiece(mPrice);
    MR::startBva(this, "Small0");
    MR::startBrk(this, "Metamorphosis");
    MR::setBrkFrameAndStop(this, 0.0f);
    TicoEat::init(iter);
}

void TicoShopDice::exeDisappear() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Demo");
    if (MR::isActionEnd(this)) {
        MR::deleteEffectAll(this);
        MR::deleteEffectAll(mGoods0);
        MR::deleteEffectAll(mGoods1);
        MR::stopBck(mGoods0);
        MR::stopBck(mGoods1);
        MR::hideModel(this);
        MR::hideModel(mGoods0);
        MR::hideModel(mGoods1);
        MR::invalidateHitSensors(this);
        MR::invalidateShadowAll(this);
        setNerve(&NrvTicoShopDice::DiceAppear::sInstance);
    }
}

void TicoShopDice::exeDiceAppear() {
    if (MR::isFirstStep(this)) for (s32 i = 0; i < mDiceCount; i++) mDice[i]->appear();
    for (s32 i = 0; i < mDiceCount; i++) if (!MR::isDead(mDice[i])) return;
    setNerve(&NrvTicoShopDice::DelayRecover::sInstance);
}

void TicoShopDice::exeRecover() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Smoke");
        MR::startActionSound(this, "Recover", -1, -1, -1);
        MR::showModel(this);
        MR::showModel(mGoods0);
        MR::showModel(mGoods1);
        MR::validateHitSensors(this);
        MR::validateShadowAll(this);
        MR::startAction(mGoods0, "Shake");
        MR::startAction(mGoods1, "Shake");
        MR::startAction(this, "Wait");
        MR::startBva(this, "Small0");
        const char* waitAction = "Wait";
        const char* talkAction = "Talk";
        mParam._14 = waitAction;
        mParam._18 = waitAction;
        mParam._1C = talkAction;
        mParam._20 = talkAction;
        if (!mUseCoins) resetStarPiece();
        setNerve(&NrvTicoShopDice::DelayWait::sInstance);
    }
}

bool TicoShopDice::isReactionNerve() const {
    if (isNerve(mWaitNerve)) return true;
    return isNerve(mTalkNerve);
}

bool TicoShopDice::branchFunc(u32 id) {
    if (id == 0) {
        if (mUseCoins) return mPrice <= MR::getCoinNum();
        return mPrice <= MR::getStarPieceNum();
    }
    return false;
}

bool TicoShopDice::eventFunc(u32 id) {
    if (id < 3) {
        if (!mUseCoins) return TicoEat::eventFunc(id);
        if (id == 0) {
            if (isNerve(&NrvTicoShopDice::EatCoin::sInstance)) return mEatCoinState->isComplete();
            setNerve(&NrvTicoShopDice::EatCoin::sInstance);
            return false;
        }
        if (id == 1) MR::forceAppearCoinCounterForTicoFat();
        if (id == 2) MR::disappearCoinCounterForTicoFat();
        return true;
    }
    if (id == 10) { setNerve(&NrvTicoShopDice::Disappear::sInstance); return true; }
    return true;
}
