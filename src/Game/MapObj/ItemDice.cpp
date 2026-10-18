#pragma auto_inline off
#include "Game/MapObj/ItemDiceFunction.hpp"

#include "Game/MapObj/ItemDice.hpp"
#include "Game/Enemy/Kuribo.hpp"
#include "Game/MapObj/BenefitItemObj.hpp"
#pragma dont_inline on
#include "Game/MapObj/BenefitItemOneUp.hpp"
#pragma dont_inline off
#include "Game/Util.hpp"
#include "JSystem/JGeometry/TMatrix.hpp"
#include "JSystem/JGeometry/TVec.hpp"
#include <new>

// String/data labels (.data) and float constants (.sdata2) used by the dice classes.


// Unnamed SMG2 helpers (DiceBase/BenefitItem unit, not ours to name here).
// PLACEHOLDER(objects): Kuribo appear-from-box pose helper; awaiting the enemies owner declaration.
extern "C" void fn_801B9BD0(Kuribo*, const TVec3f*, const TQuat4f*);

namespace {
    inline const char* diceName(const char* pLabel) {
        return pLabel;
    }

    // The 0x154-byte BenefitItemOneUp is built by calling the BenefitItemObj ctor and then storing the
    // BenefitItemOneUp vtable (the inline 2-arg ctor of BenefitItemOneUp reproduces that shape).
}  // namespace

// ItemDice --------------------------------------------------------------

ItemDice::ItemDice(const char* pName) : DiceBase(pName) {
    mMainItem = nullptr;
    mSubItem = nullptr;
}

ItemDice::~ItemDice() {
}

const char* ItemDice::getModelName() {
    return diceName("ItemDice");
}

void ItemDice::initItems(const JMapInfoIter&) {
    mMainItem = new Kuribo("クリボー");
    MR::setPosition(mMainItem, mPosition);
    mMainItem->initWithoutIter();
    mMainItem->makeActorDead();

    BenefitItemObj* pSub = new BenefitItemOneUp("１ＵＰキノコ");
    mSubItem = pSub;
    MR::setPosition(mSubItem, mPosition);
    mSubItem->initWithoutIter();
    mSubItem->makeActorDead();
    mSubItem->_DE = 2;

    MR::declareCoin(this, 10);
}

void ItemDice::setAnimFrame() {
    MR::startBtp(this, diceName("ItemDice"));
    MR::setBtpFrameAndStop(this, 0.0f);
}

void ItemDice::setResultFrame(u32 result) {
    switch (result) {
    case 0:
    case 1:
        MR::setBtpFrameAndStop(this, 2.0f);
        break;
    case 2:
    case 3:
        MR::setBtpFrameAndStop(this, 1.0f);
        break;
    case 4:
    case 5:
        MR::setBtpFrameAndStop(this, 3.0f);
        break;
    }
}

bool ItemDice::makeResult(u32 result) {
    switch (result) {
    case 0:
    case 1:
        MR::appearCoinPop(this, mPosition, 10);
        return true;
    case 2:
    case 3:
        popSubItem();
        return true;
    case 4:
    case 5:
        placeMainItem();
        break;
    }
    return false;
}

bool ItemDice::isGoodResult(u32 result) const {
    return result <= 3;
}

// unnamed fn_802B4B50: orient the main item by the dice rotation and place it
void ItemDice::placeMainItem() {
    TRot3f rot;
    ItemDiceFunction::initItemPose(mMainItem, &mPosition, &rot, 0.0f);
    TQuat4f quat;
    rot.getQuat(quat);
    fn_801B9BD0(mMainItem, &mPosition, &quat);
}

// unnamed fn_802B4BB0: pop the sub item out of the dice with a random speed
void ItemDice::popSubItem() {
    ItemDiceFunction::initItemPose(mSubItem, &mPosition, nullptr, 3.1415927f * MR::getRandom(-1.0f, 1.0f));
    mSubItem->_DE = 2;
    mSubItem->appearPop();
}

// MultipleChoiceDice -------------------------------------------------------

MultipleChoiceDice::~MultipleChoiceDice() {
}

const char* MultipleChoiceDice::getModelName() {
    return diceName("ItemDice");
}

void MultipleChoiceDice::initItems(const JMapInfoIter& rIter) {
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initUseStageSwitchWriteB(this, rIter);
}

void MultipleChoiceDice::setAnimFrame() {
    MR::startBtp(this, diceName("ItemDice"));
    MR::setBtpFrameAndStop(this, 4.0f);
}

void MultipleChoiceDice::setResultFrame(u32 result) {
    switch (result) {
    case 0:
    case 1:
        MR::setBtpFrameAndStop(this, 6.0f);
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        MR::setBtpFrameAndStop(this, 5.0f);
        break;
    }
}

bool MultipleChoiceDice::makeResult(u32 result) {
    switch (result) {
    case 0:
    case 1:
        if (MR::isValidSwitchB(this)) {
            MR::onSwitchB(this);
        }
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
        return true;
    }
    return false;
}

bool MultipleChoiceDice::isGoodResult(u32 result) const {
    return result - 2 <= 3;
}

// PositiveDice -------------------------------------------------------------

PositiveDice::PositiveDice(const char* pName) : DiceBase(pName) {
    mItem = nullptr;
}

PositiveDice::~PositiveDice() {
}

const char* PositiveDice::getModelName() {
    return diceName("PositiveDice");
}

void PositiveDice::initItems(const JMapInfoIter&) {
    BenefitItemObj* pItem = new BenefitItemOneUp("１ＵＰキノコ");
    mItem = pItem;
    MR::setPosition(mItem, mPosition);
    mItem->initWithoutIter();
    mItem->makeActorDead();
    mItem->_DE = 2;
    MR::declareStarPiece(this, 6);
}

void PositiveDice::setAnimFrame() {
    MR::startBtp(this, diceName("PositiveDice"));
    MR::setBtpFrameAndStop(this, 0.0f);
}

void PositiveDice::setResultFrame(u32 result) {
    if (result <= 1) goto item;
    if (result - 2 <= 1) goto piece;
    if ((s32)result == 4) goto item;
    if ((s32)result == 5) goto piece;
    return;
item:
    MR::setBtpFrameAndStop(this, 1.0f);
    return;
piece:
    MR::setBtpFrameAndStop(this, 2.0f);
}

bool PositiveDice::makeResult(u32 result) {
    if (result <= 1) goto item;
    if (result - 2 <= 1) goto piece;
    if ((s32)result == 4) goto item;
    if ((s32)result == 5) goto piece;
    goto failed;
item:
    popItem();
    return true;
piece:
    MR::appearStarPiece(this, mPosition, 1, 10.0f, 40.0f, false);
    return true;
failed:
    return false;
}

bool PositiveDice::isGoodResult(u32 result) const {
    return result <= 5;
}

// unnamed fn_802B4F90
void PositiveDice::popItem() {
    ItemDiceFunction::initItemPose(mItem, &mPosition, nullptr, 3.1415927f * MR::getRandom(-1.0f, 1.0f));
    mItem->_DE = 2;
    mItem->appearPop();
}

// LuckyDice ----------------------------------------------------------------

LuckyDice::LuckyDice(const char* pName) : DiceBase(pName) {
}

const char* LuckyDice::getModelName() {
    return diceName("LuckyDice");
}

void LuckyDice::initItems(const JMapInfoIter&) {
    for (u32 i = 0; i < 5; i++) {
        BenefitItemObj* pItem = new BenefitItemOneUp("１ＵＰキノコ");
        mItems[i] = pItem;
        MR::setPosition(mItems[i], mPosition);
        mItems[i]->initWithoutIter();
        mItems[i]->_DE = 2;
        mItems[i]->makeActorDead();
    }
    MR::declareStarPiece(this, 1000);
}

void LuckyDice::setAnimFrame() {
    MR::startBtp(this, diceName("LuckyDice"));
    MR::setBtpFrameAndStop(this, 0.0f);
}

void LuckyDice::setResultFrame(u32 result) {
    if (result <= 1) goto one;
    if (result - 4 <= 1) goto piece;
    if ((s32)result == 2) goto five;
    if ((s32)result == 3) goto three;
    return;
one:
    MR::setBtpFrameAndStop(this, 1.0f);
    return;
five:
    MR::setBtpFrameAndStop(this, 3.0f);
    return;
three:
    MR::setBtpFrameAndStop(this, 2.0f);
    return;
piece:
    MR::setBtpFrameAndStop(this, 4.0f);
}

bool LuckyDice::makeResult(u32 result) {
    if (result <= 1) goto one;
    if (result - 4 <= 1) goto piece;
    if ((s32)result == 2) goto five;
    if ((s32)result == 3) goto three;
    goto failed;
one:
    shootItems(1);
    return true;
five:
    shootItems(5);
    return true;
three:
    shootItems(3);
    return true;
piece:
    MR::appearStarPiece(this, mPosition, 1, 10.0f, 40.0f, false);
    return true;
failed:
    return false;
}

bool LuckyDice::isGoodResult(u32 result) const {
    return result <= 5;
}

// unnamed fn_802B5240 (0x268 bytes): spread `count` benefit items around the dice toward the player.
// NONMATCHING: empty SDK default constructors remain outlined; vector temporary stack placement differs.
#pragma dont_inline on
void LuckyDice::shootItems(s32 count) {
    TVec3f toPlayer = *MR::getPlayerPos() - mPosition;
    MR::vecKillElement(toPlayer, mGravity, &toPlayer);
    if (MR::normalizeOrZero(&toPlayer)) {
        toPlayer.set<f32>(0.0f, 0.0f, 1.0f);
        TQuat4f q;
        q.setRotate(TVec3f(0.0f, -1.0f, 0.0f), mGravity);
        q.transform(toPlayer);
    }

    TVec3f negGravity = -mGravity;
    TVec3f axis;
    PSVECCrossProduct(&negGravity, &toPlayer, &axis);
    MR::normalizeOrZero(&axis);

    TRot3f rot;
    rot.setXDir(axis);
    TVec3f negGravity2 = -mGravity;
    rot.setYDir(negGravity2);
    rot.setZDir(toPlayer);
    reinterpret_cast<TPos3f&>(rot).setTrans(TVec3f(0.0f, 0.0f, 0.0f));

    TPos3f spin;
    spin.makeRotate(TVec3f(0.0f, 1.0f, 0.0f), 6.2831855f / (f32)count);

    TVec3f yDir;
    rot.getYDir(yDir);
    for (s32 i = 0; i < count; i++) {
        TVec3f ang;
        rot.getEulerXYZ(ang);
        ang.scale(57.295776f);
        MR::setRotation(mItems[i], ang);

        TVec3f zDir;
        rot.getZDir(zDir);
        TVec3f off(zDir);
        off.scale(80.0f);
        TVec3f pos(mPosition);
        pos.add(off);
        MR::setPosition(mItems[i], pos);

        TVec3f up(yDir);
        up.scale(20.0f);
        TVec3f out(zDir);
        out.scale(10.0f);
        TVec3f vel(out);
        vel.add(up);
        mItems[i]->appearThrow(vel);

        if (i != 4) {
            rot.concat(spin);
        } else {
            break;
        }
    }
}

#pragma dont_inline reset

// Destructor helper for the ItemDice TU -----------------------------------
