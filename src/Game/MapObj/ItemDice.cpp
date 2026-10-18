#pragma auto_inline off

#include "Game/MapObj/ItemDice.hpp"
#include "Game/Enemy/Kuribo.hpp"
#include "Game/MapObj/BenefitItemObj.hpp"
#include "Game/MapObj/BenefitItemOneUp.hpp"
#include "Game/Util.hpp"
#include "JSystem/JGeometry/TMatrix.hpp"
#include "JSystem/JGeometry/TVec.hpp"
#include <new>

// String/data labels (.data) and float constants (.sdata2) used by the dice classes.
extern "C" char lbl_806A2EB8[];  // ItemDice model / btp name (table, +0 used)
extern "C" char lbl_806A2F68[];  // name of the 0xD0 benefit object (Kuribo-typed ctor in the target)
extern "C" char lbl_806A2F74[];  // name of the 0x154 BenefitItemOneUp
extern "C" char lbl_8065EE30[];  // model name for BenefitItemOneUp
extern "C" char lbl_806A2F84[];  // PositiveDice model name
extern "C" char lbl_806A2F94[];  // LuckyDice model name

extern "C" f32 lbl_807DD7C0;
extern "C" f32 lbl_807DD7C4;
extern "C" f32 lbl_807DD7CC;
extern "C" f32 lbl_807DD7D0;
extern "C" f32 lbl_807DD7D4;
extern "C" f32 lbl_807DD7D8;
extern "C" f32 lbl_807DD7DC;
extern "C" f32 lbl_807DD7E4;
extern "C" f32 lbl_807DD808;
extern "C" f32 lbl_807DD820;
extern "C" f32 lbl_807DD824;
extern "C" f32 lbl_807DD828;
extern "C" f32 lbl_807DD82C;
extern "C" f32 lbl_807DD830;
extern "C" f32 lbl_807DD834;

// Unnamed SMG2 helpers (DiceBase/BenefitItem unit, not ours to name here).
extern "C" void fn_802B30F0(BenefitItemObj*, const TVec3f*, TRot3f*, f32);
extern "C" void fn_801B9BD0(BenefitItemObj*, const TVec3f*, TQuat4f*);

namespace {
    inline const char* diceName(const char* pLabel) {
        return pLabel;
    }

    // The 0x154-byte BenefitItemOneUp is built by calling the BenefitItemObj ctor and then storing the
    // BenefitItemOneUp vtable (the inline 2-arg ctor of BenefitItemOneUp reproduces that shape).
}  // namespace

// unnamed fn_802B49F0 in the target (kept out of line). NONMATCHING: our placement new adds a null check
// that the target only does in its callers.
BenefitItemObj* createOneUpItem(void* pMem, const char* pName) NO_INLINE;
BenefitItemObj* createOneUpItem(void* pMem, const char* pName) {
    return new (pMem) BenefitItemOneUp(pName, diceName(lbl_8065EE30));
}

namespace {
}  // namespace

// ItemDice --------------------------------------------------------------

ItemDice::ItemDice(const char* pName) : DiceBase(pName) {
    mMainItem = nullptr;
    mSubItem = nullptr;
}

ItemDice::~ItemDice() {
}

const char* ItemDice::getModelName() {
    return diceName(lbl_806A2EB8);
}

void ItemDice::initItems(const JMapInfoIter&) {
    // main item: 0xD0-byte object built from the Kuribo-named ctor
    void* pMem = ::operator new(0xD0);
    BenefitItemObj* pMain = static_cast<BenefitItemObj*>(pMem);
    if (pMem != nullptr) {
        pMain = reinterpret_cast<BenefitItemObj*>(new (pMem) Kuribo(diceName(lbl_806A2F68)));
    }
    mMainItem = pMain;
    MR::setPosition(mMainItem, mPosition);
    mMainItem->initWithoutIter();
    mMainItem->makeActorDead();

    // sub item: 0x154-byte BenefitItemOneUp
    void* pSubMem = ::operator new(0x154);
    BenefitItemObj* pSub = static_cast<BenefitItemObj*>(pSubMem);
    if (pSubMem != nullptr) {
        pSub = createOneUpItem(pSubMem, diceName(lbl_806A2F74));
    }
    mSubItem = pSub;
    MR::setPosition(mSubItem, mPosition);
    mSubItem->initWithoutIter();
    mSubItem->makeActorDead();
    mSubItem->_DE = 2;

    MR::declareCoin(this, 10);
}

void ItemDice::setAnimFrame() {
    MR::startBtp(this, diceName(lbl_806A2EB8));
    MR::setBtpFrameAndStop(this, lbl_807DD7C4);
}

void ItemDice::setResultFrame(u32 result) {
    switch (result) {
    case 0:
    case 1:
        MR::setBtpFrameAndStop(this, lbl_807DD7CC);
        break;
    case 2:
    case 3:
        MR::setBtpFrameAndStop(this, lbl_807DD7C0);
        break;
    case 4:
    case 5:
        MR::setBtpFrameAndStop(this, lbl_807DD7D0);
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
        return false;
    }
    return false;
}

bool ItemDice::isGoodResult(u32 result) const {
    return result <= 3;
}

// unnamed fn_802B4B50: orient the main item by the dice rotation and place it
void ItemDice::placeMainItem() {
    TRot3f rot;
    fn_802B30F0(mMainItem, &mPosition, &rot, lbl_807DD7C4);
    TQuat4f quat;
    rot.getQuat(quat);
    fn_801B9BD0(mMainItem, &mPosition, &quat);
}

// unnamed fn_802B4BB0: pop the sub item out of the dice with a random speed
void ItemDice::popSubItem() {
    fn_802B30F0(mSubItem, &mPosition, nullptr, lbl_807DD7D8 * MR::getRandom(lbl_807DD7D4, lbl_807DD7C0));
    mSubItem->_DE = 2;
    mSubItem->appearPop();
}

// MultipleChoiceDice -------------------------------------------------------

MultipleChoiceDice::~MultipleChoiceDice() {
}

const char* MultipleChoiceDice::getModelName() {
    return diceName(lbl_806A2EB8);
}

void MultipleChoiceDice::initItems(const JMapInfoIter& rIter) {
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initUseStageSwitchWriteB(this, rIter);
}

void MultipleChoiceDice::setAnimFrame() {
    MR::startBtp(this, diceName(lbl_806A2EB8));
    MR::setBtpFrameAndStop(this, lbl_807DD820);
}

void MultipleChoiceDice::setResultFrame(u32 result) {
    switch (result) {
    case 0:
    case 1:
        MR::setBtpFrameAndStop(this, lbl_807DD824);
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        MR::setBtpFrameAndStop(this, lbl_807DD828);
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
        return false;
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
    return diceName(lbl_806A2F84);
}

void PositiveDice::initItems(const JMapInfoIter&) {
    void* pMem = ::operator new(0x154);
    BenefitItemObj* pItem = static_cast<BenefitItemObj*>(pMem);
    if (pMem != nullptr) {
        pItem = createOneUpItem(pMem, diceName(lbl_806A2F74));
    }
    mItem = pItem;
    MR::setPosition(mItem, mPosition);
    mItem->initWithoutIter();
    mItem->makeActorDead();
    mItem->_DE = 2;
    MR::declareStarPiece(this, 6);
}

void PositiveDice::setAnimFrame() {
    MR::startBtp(this, diceName(lbl_806A2F84));
    MR::setBtpFrameAndStop(this, lbl_807DD7C4);
}

void PositiveDice::setResultFrame(u32 result) {
    switch (result) {
    case 0:
    case 1:
    case 4:
        MR::setBtpFrameAndStop(this, lbl_807DD7C0);
        break;
    case 2:
    case 3:
    case 5:
        MR::setBtpFrameAndStop(this, lbl_807DD7CC);
        break;
    }
}

bool PositiveDice::makeResult(u32 result) {
    switch (result) {
    case 0:
    case 1:
    case 4:
        popItem();
        return true;
    case 2:
    case 3:
    case 5:
        MR::appearStarPiece(this, mPosition, 1, lbl_807DD7E4, lbl_807DD82C, false);
        return true;
    }
    return false;
}

bool PositiveDice::isGoodResult(u32 result) const {
    return result <= 5;
}

// unnamed fn_802B4F90
void PositiveDice::popItem() {
    fn_802B30F0(mItem, &mPosition, nullptr, lbl_807DD7D8 * MR::getRandom(lbl_807DD7D4, lbl_807DD7C0));
    mItem->_DE = 2;
    mItem->appearPop();
}

// LuckyDice ----------------------------------------------------------------

LuckyDice::LuckyDice(const char* pName) : DiceBase(pName) {
}

const char* LuckyDice::getModelName() {
    return diceName(lbl_806A2F94);
}

void LuckyDice::initItems(const JMapInfoIter&) {
    for (s32 i = 0; i < 5; i++) {
        void* pMem = ::operator new(0x154);
        BenefitItemObj* pItem = static_cast<BenefitItemObj*>(pMem);
        if (pMem != nullptr) {
            pItem = createOneUpItem(pMem, diceName(lbl_806A2F74));
        }
        mItems[i] = pItem;
        MR::setPosition(mItems[i], mPosition);
        mItems[i]->initWithoutIter();
        mItems[i]->_DE = 2;
        mItems[i]->makeActorDead();
    }
    MR::declareStarPiece(this, 1000);
}

void LuckyDice::setAnimFrame() {
    MR::startBtp(this, diceName(lbl_806A2F94));
    MR::setBtpFrameAndStop(this, lbl_807DD7C4);
}

void LuckyDice::setResultFrame(u32 result) {
    switch (result) {
    case 0:
    case 1:
        MR::setBtpFrameAndStop(this, lbl_807DD7C0);
        break;
    case 2:
        MR::setBtpFrameAndStop(this, lbl_807DD7D0);
        break;
    case 3:
        MR::setBtpFrameAndStop(this, lbl_807DD7CC);
        break;
    case 4:
    case 5:
        MR::setBtpFrameAndStop(this, lbl_807DD820);
        break;
    }
}

bool LuckyDice::makeResult(u32 result) {
    switch (result) {
    case 0:
    case 1:
        shootItems(1);
        return true;
    case 2:
        shootItems(5);
        return true;
    case 3:
        shootItems(3);
        return true;
    case 4:
    case 5:
        MR::appearStarPiece(this, mPosition, 1, lbl_807DD7E4, lbl_807DD82C, false);
        return true;
    }
    return false;
}

bool LuckyDice::isGoodResult(u32 result) const {
    return result <= 5;
}

// unnamed fn_802B5240 (0x268 bytes): spread `count` benefit items around the dice toward the player.
// NONMATCHING: TVec3f operator-, TQuat4f::transform and TRotation3 helpers are inline in our headers (target calls __mi/transform out of line); constants and call order match.
void LuckyDice::shootItems(s32 count) {
    TVec3f toPlayer = *MR::getPlayerPos() - mPosition;
    MR::vecKillElement(toPlayer, mGravity, &toPlayer);
    if (MR::normalizeOrZero(&toPlayer)) {
        toPlayer.set<f32>(lbl_807DD7C4, lbl_807DD7C4, lbl_807DD7C0);
        TQuat4f q;
        q.setRotate(TVec3f(lbl_807DD7C4, lbl_807DD7D4, lbl_807DD7C4), mGravity);
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
    reinterpret_cast<TPos3f&>(rot).setTrans(TVec3f(lbl_807DD7C4, lbl_807DD7C4, lbl_807DD7C4));

    TPos3f spin;
    spin.makeRotate(TVec3f(lbl_807DD7C4, lbl_807DD7C0, lbl_807DD7C4), lbl_807DD830 / (f32)count);

    TVec3f yDir;
    rot.getYDir(yDir);
    for (s32 i = 0; i < count; i++) {
        TVec3f ang;
        rot.getEulerXYZ(ang);
        ang.scale(lbl_807DD7DC);
        MR::setRotation(mItems[i], ang);

        TVec3f zDir;
        rot.getZDir(zDir);
        TVec3f off(zDir);
        off.scale(lbl_807DD834);
        TVec3f pos(mPosition);
        pos.add(off);
        MR::setPosition(mItems[i], pos);

        TVec3f up(yDir);
        up.scale(lbl_807DD808);
        TVec3f out(zDir);
        out.scale(lbl_807DD7E4);
        TVec3f vel(out);
        vel.add(up);
        mItems[i]->appearThrow(vel);

        if (i != 4) {
            rot.concat(spin);
        }
    }
}

// Destructor helper for the ItemDice TU -----------------------------------
