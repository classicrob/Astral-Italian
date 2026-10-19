#include "Game/MapObj/ItemBlock.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

namespace {
    static const s32 cAppearItemStep = 1;
    static const f32 cAppearItemOffsetY = 100.0f;
    static const s32 cTimerLimit = 240;
    static const s32 hNumTimerLastPiece = 11;
    static const f32 hLastBonusSplashRangeH = 10.0f;
    static const f32 hPointLightOffsetY = 100.0f;
    static const f32 hShadowClippingMargin = 200.0f;
    static const f32 hBrightness = 0.998646f;
};  // namespace

namespace NrvItemBlock {
    NEW_NERVE(ItemBlockNrvStandby, ItemBlock, Standby);
    NEW_NERVE(ItemBlockNrvWait, ItemBlock, Wait);
    NEW_NERVE(ItemBlockNrvAppearItem, ItemBlock, AppearItem);
    NEW_NERVE(ItemBlockNrvAppearItemSplash, ItemBlock, AppearItemSplash);
};  // namespace NrvItemBlock

namespace {
    static Color8 hPointLight(0xFF, 0xE6, 0, 0xFF);
};  // namespace

ItemBlock::ItemBlock(const char* pName) : LiveActor(pName), mKind(0), mClippingRange(0.0f, 0.0f, 0.0f), mTimer(0), mCooldownTimer(0), mItemCount(8), _AC(0), mCalcShadow(false) {
}

void ItemBlock::init(const JMapInfoIter& rIter) {
    checkKind(rIter);
    MR::initActor(this, rIter, mKind == 6 ? "CoinBlock2D" : "CoinBlock", false);
    initNerve(GET_NERVE(ItemBlock, ItemBlockNrvStandby));

    s32 arg = -1;
    MR::getJMapInfoArg4NoInit(rIter, &arg);
    if (arg != -1) {
        MR::setShadowDropLength(this, nullptr, arg);
    }

    MR::excludeCalcShadowToMyCollision(this, nullptr);

    if (mCalcShadow) {
        MR::onCalcShadow(this, nullptr);
    } else {
        MR::onCalcShadowOneTime(this, nullptr);
    }

    if (arg == 0) {
        MR::invalidateShadow(this, nullptr);
    }

    if (MR::isZeroGravity(this)) {
        TVec3f upVec;
        MR::calcUpVec(&upVec, this);
        MR::setShadowDropDirection(this, nullptr, -upVec);
    } else {
        MR::calcGravity(this);
    }

    makeActorAppeared();
}

void ItemBlock::startClipped() {
    LiveActor::startClipped();
    MR::forceDeleteEffect(this, "Glow");
}

void ItemBlock::endClipped() {
    LiveActor::endClipped();
    MR::emitEffect(this, "Glow");
}

void ItemBlock::checkKind(const JMapInfoIter& rIter) {
    const char* name = nullptr;
    MR::getObjectName(&name, rIter);

    s32 itemCount = -1;
    MR::getJMapInfoArg0NoInit(rIter, &itemCount);
    if (itemCount < 0) {
        itemCount = 8;
    }

    if (MR::isEqualString(name, "ItemBlockSwitch")) {
        MR::needStageSwitchWriteDead(this, rIter);
        mKind = 5;
    } else if (MR::isEqualString(name, "TimerCoinBlock")) {
        mKind = 1;
        MR::declareCoin(this, 10);
    } else if (MR::isEqualString(name, "SplashCoinBlock")) {
        mKind = 2;
        mItemCount = itemCount;
        MR::declareCoin(this, itemCount);
    } else if (MR::isEqualString(name, "TimerPieceBlock")) {
        mKind = 3;
        MR::declareStarPiece(this, 20);
    } else if (MR::isEqualString(name, "SplashPieceBlock")) {
        mKind = 4;
        mItemCount = itemCount;
        MR::declareStarPiece(this, itemCount);
    } else if (MR::isEqualString(name, "CoinBlock2D")) {
        mKind = 6;
        mItemCount = 1;
        MR::declareCoin(this, 1);
    }

    s32 arg = -1;

    if (mKind == 1 || mKind == 3) {
        MR::getJMapInfoArg1NoInit(rIter, &arg);

        if (arg == -1) {
            mTimer = ::cTimerLimit;
        } else {
            mTimer = arg;
        }
    }

    arg = -1;
    MR::getJMapInfoArg2NoInit(rIter, &arg);
    mCalcShadow = !(arg - 1);
}

void ItemBlock::initAfterPlacement() {
    MR::setClippingRangeIncludeShadow(this, &mClippingRange, ::hShadowClippingMargin);
}

void ItemBlock::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    setNerve(GET_NERVE(ItemBlock, ItemBlockNrvStandby));

    switch (mKind) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        MR::startBrk(this, "CoinBlock");
    }
}

void ItemBlock::kill() {
    MR::startActionSound(this, "EmExplodeS");
    MR::emitEffect(this, "Break");
    LiveActor::kill();
}

// NONMATCHING: point-light vector temporaries use different registers and stack slots.
void ItemBlock::control() {
    TVec3f upVec;
    MR::calcUpVec(&upVec, this);

    TVec3f offset = upVec * ::hPointLightOffsetY;
    MR::requestPointLight(this, mPosition + offset, ::hPointLight, ::hBrightness, -1);

    if (mCalcShadow) {
        MR::setClippingRangeIncludeShadow(this, &mClippingRange, ::hShadowClippingMargin);
    }
}

bool ItemBlock::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerUpperPunch(msg) && MR::isPlayerExistDown(this, 0.0f, 0.25f) && tryStartJumpPunch()) {
        MR::sendMsgReflectV(pSender, pReceiver);
        return true;
    }

    if (isNerve(GET_NERVE(ItemBlock, ItemBlockNrvStandby)) || (isNerve(GET_NERVE(ItemBlock, ItemBlockNrvWait)) && MR::isMsgSupportTicoSpin(msg))) {
        if (mCooldownTimer == 0 && tryStartJumpPunch()) {
            mCooldownTimer = 40;
        }
    }

    return false;
}

bool ItemBlock::tryStartJumpPunch() {
    if (isNerve(GET_NERVE(ItemBlock, ItemBlockNrvStandby))) {
        switch (mKind) {
        case 1:
        case 3:
            MR::startAction(this, "Appear");
            calcAnim();
            mNoCalcAnim = true;
            setNerve(GET_NERVE(ItemBlock, ItemBlockNrvAppearItem));
            break;
        case 2:
        case 4:
        case 5:
        case 6:
            setNerve(GET_NERVE(ItemBlock, ItemBlockNrvAppearItemSplash));
            break;
        }

        return true;
    }

    if (isNerve(GET_NERVE(ItemBlock, ItemBlockNrvWait))) {
        if (mKind == 1 || mKind == 3) {
            if (mKind == 1) {
                if (MR::getDeclareRemnantCoinCount(this) == 1) {
                    mTimer = 0;
                }
            } else {
                if (MR::getDeclareRemnantStarPieceCountNotExist(this) - ::hNumTimerLastPiece == 0) {
                    mTimer = 0;
                }
            }

            MR::startAction(this, "Appear");
            calcAnim();
            mNoCalcAnim = true;
            setNerve(GET_NERVE(ItemBlock, ItemBlockNrvAppearItem));

            return true;
        }
    }

    return false;
}

void ItemBlock::exeStandby() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    MR::attachSupportTicoToTarget(this);
}

void ItemBlock::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::invalidateClipping(this);
    }

    MR::attachSupportTicoToTarget(this);
    decTimer();
}

void ItemBlock::exeAppearItem() {
    if (MR::isFirstStep(this)) {
        if (!mTimer) {
            MR::forceDeleteEffect(this, "Glow");
            MR::startAction(this, "Bomb");
            MR::startActionSound(this, "Punched");
        }

        MR::invalidateClipping(this);
    }

    if (MR::isStep(this, ::cAppearItemStep)) {
        TVec3f gravVec;
        TVec3f position = mPosition;
        MR::calcGravityVector(this, &gravVec, nullptr, 0);
        position -= gravVec * ::cAppearItemOffsetY;

        switch (mKind) {
        case 1:
            MR::hopCoin(this, position, -gravVec);
            break;

        case 3:
            if (!mTimer && MR::getDeclareRemnantStarPieceCountNotExist(this) - ::hNumTimerLastPiece == 0) {
                MR::appearStarPieceToDirection(this, position, -gravVec, 11, ::hLastBonusSplashRangeH, 40.0f, false);

            } else {
                MR::hopStarPiece(this, position, -gravVec);
            }

            MR::startActionSound(this, "SyItemAppear");
            break;
        case 2:
        default:
            break;
        }
    }

    decTimer();

    if (!MR::isActionEnd(this)) {
        return;
    }

    if (!mTimer) {
        kill();
    } else {
        setNerve(GET_NERVE(ItemBlock, ItemBlockNrvWait));
    }
}

void ItemBlock::exeAppearItemSplash() {
    if (MR::isFirstStep(this)) {
        MR::forceDeleteEffect(this, "Glow");
        MR::startAction(this, "Bomb");
    }

    if (!MR::isActionEnd(this)) {
        return;
    }

    if (mKind == 2 || mKind == 6) {
        if (mItemCount == 1) {
            MR::appearCoinPop(this, mPosition, 1);
        } else {
            MR::appearCoinCircle(this, mPosition, mItemCount);
        }
    } else if (mKind == 4) {
        MR::appearStarPiece(this, mPosition, mItemCount, ::hLastBonusSplashRangeH, 40.0f, false);
    } else if (mKind == 5) {
        MR::onSwitchDead(this);
    }

    mTimer = 0;
    kill();
}

void ItemBlock::decTimer() {
    --mTimer;
    if (mTimer <= 0) {
        mTimer = 0;
    }

    --mCooldownTimer;
    if (mCooldownTimer > 0) {
        return;
    }
    mCooldownTimer = 0;
}

void ItemBlock::calcAnim() {
    if (!mNoCalcAnim) {
        LiveActor::calcAnim();
    }
    mNoCalcAnim = false;
}
