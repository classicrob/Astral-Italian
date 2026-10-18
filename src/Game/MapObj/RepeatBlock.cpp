#pragma auto_inline off

#include "Game/MapObj/RepeatBlock.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <revolution.h>

// Functions of RepeatBlockAtom / RepeatBlockHead (Game/MapObj/RepeatBlockHead) that are not named yet.
extern "C" {
void fn_802EC450(RepeatBlockAtom*, RepeatBlock*, s32);
void fn_802EC4E0(RepeatBlockAtom*, RepeatBlock*, s32);
void fn_802EC590(LiveActor*);
RepeatBlockHead* fn_802EC610(RepeatBlockHead*, const char*, RepeatBlock*);
void fn_802EC740(RepeatBlockHead*);
void fn_802EC820(RepeatBlockHead*);
RepeatBlockTail* fn_802ECA00(RepeatBlockTail*, RepeatBlock*, const char*);
}

namespace {
    // direction -> opposite direction
    const s32 sOppositeDir[6] = {1, 0, 3, 2, 5, 4};
    // direction -> cell offset
    RepeatBlockCell sDirTable[6] = {
        RepeatBlockCell(1, 0, 0), RepeatBlockCell(-1, 0, 0), RepeatBlockCell(0, 1, 0), RepeatBlockCell(0, -1, 0), RepeatBlockCell(0, 0, 1), RepeatBlockCell(0, 0, -1),
    };
};  // namespace

namespace NrvRepeatBlock {
    NEW_NERVE(RepeatBlockNrvWait, RepeatBlock, Wait);
    NEW_NERVE(RepeatBlockNrvMove, RepeatBlock, Move);
    NEW_NERVE(RepeatBlockNrvStop, RepeatBlock, Stop);

    class RepeatBlockNrvEnd : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< LiveActor* >(pSpine->mExecutor)->kill();
        }
        static RepeatBlockNrvEnd sInstance;
    };
    RepeatBlockNrvEnd RepeatBlockNrvEnd::sInstance ATTRIBUTE_WEAK;

    class RepeatBlockNrvAtomA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            fn_802EC590(reinterpret_cast< LiveActor* >(pSpine->mExecutor));
        }
        static RepeatBlockNrvAtomA sInstance;
    };
    RepeatBlockNrvAtomA RepeatBlockNrvAtomA::sInstance ATTRIBUTE_WEAK;

    class RepeatBlockNrvAtomWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* pActor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            if (MR::isFirstStep(pActor)) {
                MR::startAction(pActor, "Wait");
            }
        }
        static RepeatBlockNrvAtomWait sInstance;
    };
    RepeatBlockNrvAtomWait RepeatBlockNrvAtomWait::sInstance ATTRIBUTE_WEAK;

    class RepeatBlockNrvAtomDisappear : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* pActor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            f32 rate = MR::getBtkRate(pActor);
            if (MR::getBtkFrame(pActor) + rate <= 0.0f) {
                pActor->kill();
            }
        }
        static RepeatBlockNrvAtomDisappear sInstance;
    };
    RepeatBlockNrvAtomDisappear RepeatBlockNrvAtomDisappear::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(RepeatBlockNrvTailAppear, RepeatBlockTail, Appear);
    NEW_NERVE(RepeatBlockNrvTailWait, RepeatBlockTail, Wait);
    NEW_NERVE(RepeatBlockNrvTailAppearLoop, RepeatBlockTail, AppearLoop);
};  // namespace NrvRepeatBlock

void RepeatBlockTail::appear() {
    LiveActor::appear();
    setNerve(GET_NERVE(RepeatBlock, RepeatBlockNrvTailAppear));
}

void RepeatBlockTail::kill() {
    RepeatBlockHead::kill();
}

void RepeatBlockTail::exeAppear() {
    if (MR::isFirstStep(this)) {
        mAppearDir = mParent->mDir;
        mAppearStep = 0;
        mAppearProgress = mParent->mCellRate + (f32)mParent->mCellStep;
    }

    s32 num = calcAppearNum(mAppearDir);
    if (num >= mAppearStep) {
        while (mAppearStep <= num && mAppearStep < getCellNum(mAppearDir)) {
            void (RepeatBlockTail::*pFunc)(RepeatBlockAtom*) = &RepeatBlockTail::appearAtom;
            forEachAtom(mAppearDir, mAppearStep, pFunc);
            mAppearStep++;
        }
    }

    if (mAppearStep >= getCellNum(mAppearDir)) {
        setNerve(GET_NERVE(RepeatBlock, RepeatBlockNrvTailWait));
    }
}

void RepeatBlockTail::exeAppearLoop() {
    if (MR::isFirstStep(this)) {
        mMoveDir = mParent->mDirHistory[mParent->mHistoryRead];
        mAppearStep = 0;
        mAppearProgress = mParent->mCellRate + (f32)mParent->mCellStep;
    }

    s32 num = calcAppearNum(mMoveDir);
    if (num >= mAppearStep) {
        while (mAppearStep <= num && mAppearStep < getCellNum(mMoveDir)) {
            void (RepeatBlockTail::*pFunc)(RepeatBlockAtom*) = &RepeatBlockTail::startAtom;
            forEachAtom(mMoveDir, mAppearStep, pFunc);
            mAppearStep++;
        }
    }

    if (mAppearStep >= getCellNum(mMoveDir)) {
        setNerve(GET_NERVE(RepeatBlock, RepeatBlockNrvTailWait));
    }
}

void RepeatBlockTail::forEachAtom(s32 dir, s32 idx, void (RepeatBlockTail::*pFunc)(RepeatBlockAtom*)) {
    if (dir == 0 || dir == 2 || dir == 4) {
        idx = (getCellNum(dir) - 1) - idx;
    }

    s32 xBegin = 0;
    s32 yBegin = 0;
    s32 zBegin = 0;
    s32 xEnd = mParent->mNumX;
    s32 yEnd = mParent->mNumY;
    s32 zEnd = mParent->mNumZ;

    switch (dir) {
    case 0:
    case 1:
        xBegin = idx;
        xEnd = idx + 1;
        break;
    case 2:
    case 3:
        yBegin = idx;
        yEnd = idx + 1;
        break;
    case 4:
    case 5:
        zBegin = idx;
        zEnd = idx + 1;
        break;
    }

    for (s32 x = xBegin; x < xEnd; x++) {
        for (s32 y = yBegin; y < yEnd; y++) {
            for (s32 z = zBegin; z < zEnd; z++) {
                (this->*pFunc)(mAtoms[(x * mParent->mNumY + y) * mParent->mNumZ + z]);
            }
        }
    }
}

void RepeatBlockTail::appearAtom(RepeatBlockAtom* pAtom) {
    pAtom->appear();
    fn_802EC450(pAtom, mParent, mAppearDir);
}

void RepeatBlockTail::startAtom(RepeatBlockAtom* pAtom) {
    fn_802EC4E0(pAtom, mParent, mMoveDir);
}

s32 RepeatBlockTail::calcAppearNum(s32 dir) const {
    f32 progress = (mParent->mCellRate + (f32)mParent->mCellStep) - mAppearProgress;
    switch (dir) {
    case 0:
    case 1:
        return progress * mParent->mNumX;
    case 2:
    case 3:
        return progress * mParent->mNumY;
    case 4:
    case 5:
        return progress * mParent->mNumZ;
    }
    return 0;
}

s32 RepeatBlockTail::getCellNum(s32 dir) const {
    switch (dir) {
    case 0:
    case 1:
        return mParent->mNumX;
    case 2:
    case 3:
        return mParent->mNumY;
    case 4:
    case 5:
        return mParent->mNumZ;
    }
    return 0;
}

RepeatBlock::RepeatBlock(const char* pName) : LiveActor(pName), mNumX(2), mNumY(2), mNumZ(2), mTailNum(3), mHead(nullptr), mTail(nullptr), mTails(nullptr), mTailIdx(0), _10C(0), mSpeed(0.0f), _114(0.0f), _118(0.0f), _11C(0.0f), mCellLength(5.0f), mCellSize(0.0f), mMaxNum(0), mDir(1), mCell(0), mRailCell(0), mNextCell(0), mCellStep(0), mCellRate(0.0f), mDirHistory(nullptr), mPosHistory(nullptr), mHistoryWrite(0), mHistoryRead(0), mHistoryFull(false), mTailsAppeared(false), mParent(nullptr), mChildren(nullptr), mChildNum(0) {
    mMtx.identity();
    mInvMtx.identity();
}

RepeatBlockCell::RepeatBlockCell(s16 v) {
    mX = v;
    mY = v;
    mZ = v;
}

void RepeatBlock::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    s32 arg = 0;
    if (MR::getJMapInfoArg0NoInit(rIter, &arg)) {
        mNumX = arg;
    }
    if (MR::getJMapInfoArg1NoInit(rIter, &arg)) {
        mNumY = arg;
    }
    if (MR::getJMapInfoArg2NoInit(rIter, &arg)) {
        mNumZ = arg;
    }
    MR::getJMapInfoArg3NoInit(rIter, &mTailNum);
    MR::getJMapInfoArg4NoInit(rIter, &mCellLength);

    if (mNumX > mNumY) {
        if (mNumX > mNumZ) {
            mMaxNum = mNumX;
        } else {
            mMaxNum = mNumZ;
        }
    } else {
        if (mNumY > mNumZ) {
            mMaxNum = mNumY;
        } else {
            mMaxNum = mNumZ;
        }
    }

    mSpeed = 200.0f;
    mCellSize.set(mNumX, mNumY, mNumZ);
    f32 scale = mCellLength / mMaxNum;
    mCellSize.x *= scale;
    mCellSize.y *= scale;
    mCellSize.z *= scale;
    mDirHistory = new s32[mTailNum + 1];
    mPosHistory = new TVec3f[mTailNum + 1];
    MR::connectToSceneMapObjMovement(this);
    initHitSensor(1);
    initSound(3, "RepeatBlock", nullptr, TVec3f(0.0f));
    TVec3f sensorOffset(0.0f);
    MR::addHitSensorMapObj(this, "Body", 32, 0.0f, sensorOffset);
    if (MR::isConnectedWithRail(rIter)) {
        initRailRider(rIter);
        MR::moveTransToCurrentRailPos(this);
    }
    initNerve(GET_NERVE(RepeatBlock, RepeatBlockNrvWait), 0);
    MR::makeMtxTR(mMtx, this);
    mInvMtx.invert(mInvMtx);
    initHead();
    initTail();
    initTails();
    initHistory();
    initGrid();
    initShadow();
    initChildren(rIter);
    mGrid->worldToCell(&mCell, mPosition);
    mHistory->push(mCell);
    MR::useStageSwitchAwake(this, rIter);
    makeActorAppeared();
}

void RepeatBlock::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    fn_802EC740(mHead);
    mShadowDrawer->makeVertexBuffer();
}

void RepeatBlock::kill() {
    LiveActor::makeActorDead();
    mHead->kill();
    mTail->kill();
    killTails();
    mShadowDrawer->mPointNum = 0;
}

void RepeatBlock::makeActorDead() {
    LiveActor::makeActorDead();
    mHead->kill();
    mTail->kill();
    killTails();
}

void RepeatBlock::exeMove() {
    if (MR::isFirstStep(this)) {
        fn_802EC820(mTail);
        mTail->appear();
        stepForward();
    }
    MR::startActionSound(this, "Move", -1, -1, -1);
    updateHead();
    updateTail();
    moveTails();
    if (MR::isRailReachedGoal(this)) {
        setNerve(GET_NERVE(RepeatBlock, RepeatBlockNrvStop));
    }
}

f32 RepeatBlock::getCellSize(s32 dir) const {
    switch (dir) {
    case 0:
    case 1:
        return mCellSize.x;
    case 2:
    case 3:
        return mCellSize.y;
    case 4:
    case 5:
        return mCellSize.z;
    }
    return 0.0f;
}

bool RepeatBlock::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorPlayerOrRide(pReceiver)) {
        return false;
    }
    if (MR::isMsgFloorTouch(msg)) {
        if (mParent != nullptr) {
            return mParent->startMove();
        }
        return startMove();
    }
    return false;
}

void RepeatBlock::initHead() {
    RepeatBlockHead* pHead = static_cast< RepeatBlockHead* >(::operator new(0xC8));
    if (pHead != nullptr) {
        pHead = fn_802EC610(pHead, "頭ブロック", this);
    }
    mHead = pHead;
    fn_802EC740(mHead);
}

void RepeatBlock::updateHead() {
    calcMovedPos(&mPosition, mDir);
    bool shadowChanged = checkShadowArea();
    if (calcArrive(&mPosition, &mNextCell, mDir)) {
        if (stepForward()) {
            appearNextTail();
            startTailMove();
            shadowChanged = false;
        } else {
            mCellRate = 1.0f;
        }
    }
    if (shadowChanged) {
        mShadowDrawer->makeVertexBuffer();
    }
    mShadowDrawer->mOffset = mCellRate;
    mShadowDrawer->makeFace();
    MR::setPosition(mHead, mPosition);
    fn_802EC820(mHead);
}

void RepeatBlock::initTail() {
    RepeatBlockTail* pTail = static_cast< RepeatBlockTail* >(::operator new(0xD8));
    if (pTail != nullptr) {
        pTail = fn_802ECA00(pTail, this, "尻ブロック");
    }
    mTail = pTail;
    mTail->kill();
}

void RepeatBlock::updateTail() {
    mTail->movement();
    if (mHistoryFull) {
        calcMovedPos(&mTail->mPosition, mDirHistory[mHistoryRead]);
        fn_802EC820(mTail);
    }
}

void RepeatBlock::initTails() {
    mTails = new RepeatBlockTail*[mTailNum];
    for (s32 i = 0; i < mTailNum; i++) {
        RepeatBlockTail* pTail = static_cast< RepeatBlockTail* >(::operator new(0xD8));
        if (pTail != nullptr) {
            pTail = fn_802ECA00(pTail, this, "リピートブロック本体");
        }
        mTails[i] = pTail;
        mTails[i]->kill();
    }
}

void RepeatBlock::initHistory() {
    mHistory = new RepeatBlockHistory(mTailNum + 2);
}

void RepeatBlock::initGrid() {
    mGrid = new RepeatBlockGrid(mMtx, 0.5f * mNumX, 0.5f * mNumY, 0.5f * mNumZ);
}

void RepeatBlock::initShadow() {
    mShadowDrawer = new RepeatBlockShadowDrawer(mHistory, mGrid);
    mShadowDrawer->makeVertexBuffer();
}

void RepeatBlock::appearNextTail() {
    RepeatBlockTail* pTail = mTails[mTailIdx];
    MR::setPosition(pTail, mPosition);
    fn_802EC820(pTail);
    pTail->appear();
    mTailIdx++;
    if (mTailIdx >= mTailNum) {
        mTailIdx = 0;
        mTailsAppeared = true;
    }
}

void RepeatBlock::startTailMove() {
    if (mTailsAppeared) {
        mTails[mTailIdx]->setNerve(GET_NERVE(RepeatBlock, RepeatBlockNrvTailAppearLoop));
    }
}

void RepeatBlock::killTails() {
    for (s32 i = 0; i < mTailNum; i++) {
        mTails[i]->kill();
    }
}

void RepeatBlock::moveTails() {
    for (s32 i = 0; i < mTailNum; i++) {
        mTails[i]->movement();
    }
}

bool RepeatBlock::stepForward() {
    mGrid->worldToCell(&mRailCell, mPosition);
    if (mRailCell.isEqual(mCell)) {
        do {
            MR::moveCoord(this, mSpeed);
            mGrid->worldToCell(&mRailCell, MR::getRailPos(this));
            if (!mRailCell.isEqual(mCell)) {
                break;
            }
            if (MR::isRailReachedGoal(this)) {
                return false;
            }
        } while (true);
    }

    REPEAT_BLOCK_CELL_SUB(diff, mRailCell, mCell);
    if (diff.mX != 0) {
        mDir = diff.mX < 0 ? 0 : 1;
    } else if (diff.mY != 0) {
        mDir = diff.mY < 0 ? 2 : 3;
    } else if (diff.mZ != 0) {
        mDir = diff.mZ < 0 ? 4 : 5;
    }

    mNextCell.mX = mCell.mX;
    mNextCell.mY = mCell.mY;
    mNextCell.mZ = mCell.mZ;
    switch (mDir) {
    case 0:
        mNextCell.mX -= 1;
        break;
    case 1:
        mNextCell.mX += 1;
        break;
    case 2:
        mNextCell.mY -= 1;
        break;
    case 3:
        mNextCell.mY += 1;
        break;
    case 4:
        mNextCell.mZ -= 1;
        break;
    case 5:
        mNextCell.mZ += 1;
        break;
    }

    mHistory->push(mNextCell);
    mShadowDrawer->makeVertexBuffer();
    if (!mHistoryFull) {
        if (mHistoryWrite == mTailNum) {
            mHistoryFull = true;
        }
    } else {
        mHistoryRead++;
        if (mHistoryRead >= mTailNum + 1) {
            mHistoryRead = 0;
        }
        MR::setPosition(mTail, mPosHistory[mHistoryRead]);
    }
    mDirHistory[mHistoryWrite] = mDir;
    mPosHistory[mHistoryWrite] = mPosition;
    mPosHistory[mHistoryWrite].z = mPosition.z;
    mHistoryWrite++;
    if (mHistoryWrite >= mTailNum + 1) {
        mHistoryWrite = 0;
    }
    return true;
}

bool RepeatBlockCell::isEqual(const RepeatBlockCell& rOther) const {
    bool ret = false;
    if (mX == rOther.mX && mY == rOther.mY && mZ == rOther.mZ) {
        ret = true;
    }
    return ret;
}

void RepeatBlock::calcMovedPos(TVec3f* pPos, s32 dir) const {
    TVec3f local;
    mInvMtx.mult(*pPos, local);
    switch (dir) {
    case 0:
        local.x += -mCellSize.x;
        break;
    case 1:
        local.x += mCellSize.x;
        break;
    case 2:
        local.y += -mCellSize.y;
        break;
    case 3:
        local.y += mCellSize.y;
        break;
    case 4:
        local.z += -mCellSize.z;
        break;
    case 5:
        local.z += mCellSize.z;
        break;
    }
    mMtx.mult(local, *pPos);
}

bool RepeatBlock::calcArrive(TVec3f* pPos, RepeatBlockCell* pCell, s32 dir) {
    TVec3f local;
    mInvMtx.mult(*pPos, local);
    TVec3f target;
    mGrid->calcCellPos(&target, *pCell);
    bool arrived = false;
    switch (dir) {
    case 0:
        arrived = local.x <= target.x;
        if (arrived) {
            local.x = target.x;
        }
        break;
    case 1:
        arrived = local.x >= target.x;
        if (arrived) {
            local.x = target.x;
        }
        break;
    case 2:
        arrived = local.y <= target.y;
        if (arrived) {
            local.y = target.y;
        }
        break;
    case 3:
        arrived = local.y >= target.y;
        if (arrived) {
            local.y = target.y;
        }
        break;
    case 4:
        arrived = local.z <= target.z;
        if (arrived) {
            local.z = target.z;
        }
        break;
    case 5:
        arrived = local.z >= target.z;
        if (arrived) {
            local.z = target.z;
        }
        break;
    }
    if (arrived) {
        mMtx.mult(local, *pPos);
    }
    TVec3f from;
    mGrid->calcCellPos(&from, mCell);
    TVec3f total = target - from;
    f32 totalLen = PSVECMag(&total);
    if (totalLen > 0.0f) {
        TVec3f done = local - from;
        mCellRate = PSVECMag(&done) / totalLen;
    }
    if (arrived) {
        mCellStep++;
        mCellRate = 0.0f;
    }
    return arrived;
}

void RepeatBlock::initChildren(const JMapInfoIter& rIter) {
    mChildNum = MR::getChildObjNum(rIter);
    mChildren = new RepeatBlock*[mChildNum];
    for (s32 i = 0; i < mChildNum; i++) {
        mChildren[i] = new RepeatBlock(mName);
        if (mParent != nullptr) {
            mChildren[i]->mParent = mParent;
        } else {
            mChildren[i]->mParent = this;
        }
        MR::initChildObj(mChildren[i], rIter, i);
    }
}

bool RepeatBlock::startMove() {
    if (!isNerve(GET_NERVE(RepeatBlock, RepeatBlockNrvWait))) {
        return false;
    }
    if (MR::isExistRail(this)) {
        MR::invalidateClipping(this);
        setNerve(GET_NERVE(RepeatBlock, RepeatBlockNrvMove));
    }
    for (s32 i = 0; i < mChildNum; i++) {
        mChildren[i]->startMove();
    }
    return true;
}

bool RepeatBlock::checkShadowArea() {
    AreaObj* pArea = MR::getAreaObj("RepeatBlockShadowChangeArea", mPosition);
    if (pArea == nullptr) {
        return false;
    }
    s32 arg = MR::getAreaObjArg(pArea, 0);
    s32 dir = mShadowDrawer->mDirection;
    switch (arg) {
    case 0:
        dir = 0;
        break;
    case 1:
        dir = 1;
        break;
    case 2:
        dir = 2;
        break;
    case 3:
        dir = 3;
        break;
    case 4:
        dir = 4;
        break;
    case 5:
        dir = 5;
        break;
    }
    if (dir == mShadowDrawer->mDirection) {
        return false;
    }
    mShadowDrawer->setDirection(dir);
    return true;
}

s32 RepeatBlock::getBlockNum() const {
    return mNumZ * (mNumX * mNumY);
}

RepeatBlock::~RepeatBlock() {
}

RepeatBlockAtom::~RepeatBlockAtom() {
}

RepeatBlockTail::~RepeatBlockTail() {
}

void RepeatBlock::exeStop() {
    if (MR::isGreaterEqualStep(this, 0x78)) {
        setNerve(GET_NERVE(RepeatBlock, RepeatBlockNrvEnd));
    }
}

RepeatBlockGrid::RepeatBlockGrid(const TPos3f& rMtx, f32 x, f32 y, f32 z) : mMtx(rMtx), mInvMtx(rMtx), mCellSize(x, y, z) {
    mInvMtx.invert(mInvMtx);
}

void RepeatBlockGrid::calcCellPos(TVec3f* pDst, const RepeatBlockCell& rCell) const {
    pDst->set(mCellSize.x * rCell.mX, mCellSize.y * rCell.mY, mCellSize.z * rCell.mZ);
}

void RepeatBlockGrid::calcWorldCellPos(TVec3f* pDst, const RepeatBlockCell& rCell) const {
    calcCellPos(pDst, rCell);
    mMtx.mult(*pDst, *pDst);
}

void RepeatBlockGrid::calcCell(RepeatBlockCell* pCell, const TVec3f& rPos) const {
    TVec3f pos(rPos);
    TVec3f half(mCellSize);
    half *= 0.5f;
    pos += half;
    pos.x /= mCellSize.x;
    pos.y /= mCellSize.y;
    pos.z /= mCellSize.z;
    pCell->mX = pos.x;
    pCell->mY = pos.y;
    pCell->mZ = pos.z;
    if (pos.x < 0.0f) {
        pCell->mX--;
    }
    if (pos.y < 0.0f) {
        pCell->mY--;
    }
    if (pos.z < 0.0f) {
        pCell->mZ--;
    }
}

void RepeatBlockGrid::localToWorld(TVec3f* pDst, const TVec3f& rSrc) const {
    pDst->set(rSrc);
    mMtx.mult(*pDst, *pDst);
}

void RepeatBlockGrid::worldToCell(RepeatBlockCell* pCell, const TVec3f& rPos) const {
    TVec3f local(rPos);
    mInvMtx.mult(local, local);
    calcCell(pCell, local);
}

void RepeatBlockGrid::getAxisX(TVec3f* pDst) const {
    pDst->set(mMtx.mMtx[0][0], mMtx.mMtx[1][0], mMtx.mMtx[2][0]);
}

void RepeatBlockGrid::getAxisY(TVec3f* pDst) const {
    pDst->set(mMtx.mMtx[0][1], mMtx.mMtx[1][1], mMtx.mMtx[2][1]);
}

void RepeatBlockGrid::getAxisZ(TVec3f* pDst) const {
    pDst->set(mMtx.mMtx[0][2], mMtx.mMtx[1][2], mMtx.mMtx[2][2]);
}

RepeatBlockHistory::RepeatBlockHistory(s32 capacity) : mCapacity(capacity), mCount(0), mHead(0) {
    mNodes = new RepeatBlockHistoryNode[capacity];
}

void RepeatBlockHistory::push(const RepeatBlockCell& rCell) {
    if (mCount == mCapacity) {
        propagate();
    }
    RepeatBlockHistoryNode* pNode = &mNodes[mHead];
    pNode->mCell.mX = rCell.mX;
    pNode->mCell.mY = rCell.mY;
    pNode->mCell.mZ = rCell.mZ;
    connect(pNode);
    mHead++;
    if (mHead >= mCapacity) {
        mHead = 0;
    }
    if (mCount < mCapacity) {
        mCount++;
    }
}

bool RepeatBlockHistory::hasNeighbor(s32 n, s32 dir) const {
    return mNodes[calcIndex(n)].mNeighbor[dir] != -1;
}

s32 RepeatBlockHistory::getDistance(s32 n, s32 dir) const {
    return mNodes[calcIndex(n)].mDistance[dir];
}

RepeatBlockHistoryNode* RepeatBlockHistory::getNode(s32 n) const {
    return &mNodes[calcIndex(n)];
}

s32 RepeatBlockHistory::calcIndex(s32 n) const {
    s32 idx = mHead - (n + 1);
    if (idx < 0) {
        idx += mCapacity;
    }
    return idx;
}

void RepeatBlockHistory::propagate() {
    RepeatBlockHistoryNode* pHead = &mNodes[mHead];
    for (s32 i = 0; i < 6; i++) {
        s32 neighbor = pHead->mNeighbor[i];
        if (neighbor >= 0) {
            s32 opp = sOppositeDir[i];
            s32 dist = pHead->mDistance[i] + pHead[0].mDistance[opp];
            RepeatBlockHistoryNode* pOppHead = &pHead[0];
            if (dist < 10000) {
                mNodes[neighbor].mNeighbor[opp] = pOppHead->mNeighbor[opp];
                mNodes[pHead->mNeighbor[i]].mDistance[opp] = dist;
            } else {
                mNodes[neighbor].mNeighbor[opp] = -1;
                mNodes[pHead->mNeighbor[i]].mDistance[opp] = 10000;
            }
        }
    }
}

void RepeatBlockHistory::connect(RepeatBlockHistoryNode* pNode) {
    for (s32 i = 0; i < 6; i++) {
        pNode->mNeighbor[i] = -1;
        pNode->mDistance[i] = 10000;
    }
    for (u32 i = 0; i < mCount; i++) {
        if (i != mHead) {
            REPEAT_BLOCK_CELL_SUB(diff, mNodes[i].mCell, pNode->mCell);
            RepeatBlockCell sgn = RepeatBlockCell::sign(diff);
            s32 dist = RepeatBlockCell::absSum(diff);
            for (s32 dir = 0; dir < 6; dir++) {
                if (sgn == sDirTable[dir] && dist < 10000) {
                    RepeatBlockHistoryNode* pOther = &mNodes[i];
                    s32 opp = sOppositeDir[dir];
                    if (pOther->mNeighbor[opp] == -1 || pOther->mDistance[opp] > dist) {
                        pNode->mNeighbor[dir] = i;
                        pNode->mDistance[dir] = dist;
                        mNodes[i].mNeighbor[opp] = mHead;
                        mNodes[i].mDistance[opp] = dist;
                    }
                }
            }
        }
    }
}

RepeatBlockCell::RepeatBlockCell(s16 x, s16 y, s16 z) {
    mX = x;
    mY = y;
    mZ = z;
}

RepeatBlockShadowDrawer::RepeatBlockShadowDrawer(RepeatBlockHistory* pHistory, RepeatBlockGrid* pGrid) : ShadowVolumeDrawer("リピートブロックの影描画") {
    mHistory = pHistory;
    mGrid = pGrid;
    mPointNum = 0;
    mDirection = 3;
    mMaxLength = 16;
    mVolume = new RepeatBlockVolume(pGrid);
    mOffset = 0.5f;
    mHeadPoints = new TVec3f[8];
    mHasHead = false;
    mTailPoints = new TVec3f[8];
    mHasTail = false;
    mPoints = new TVec3f[pHistory->mCapacity * 8];
}

void RepeatBlockShadowDrawer::loadModelDrawMtx() const {
    GXLoadPosMtxImm(MR::getCameraViewMtx(), 0);
    GXSetCurrentMtx(0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
}

void RepeatBlockShadowDrawer::drawShape() const {
    if (mHasHead) {
        drawBox(mHeadPoints);
    }
    if (mHasTail) {
        drawBox(mTailPoints);
    }
    for (s32 i = 0; i < mPointNum; i++) {
        drawBox(&mPoints[i * 8]);
    }
}

bool RepeatBlockShadowDrawer::isDraw() const {
    return mPointNum != 0;
}

void RepeatBlockShadowDrawer::setDirection(s32 dir) {
    mDirection = dir;
}

void RepeatBlockShadowDrawer::makeVertexBuffer() {
    mPointNum = 0;
    TVec3f axisX, axisY, axisZ;
    mGrid->getAxisX(&axisX);
    mGrid->getAxisY(&axisY);
    mGrid->getAxisZ(&axisZ);
    f32 sizeX = mGrid->mCellSize.x;
    TVec3f X = (axisX * 0.5f) * sizeX;
    f32 sizeY = mGrid->mCellSize.y;
    TVec3f Y = (axisY * 0.5f) * sizeY;
    f32 sizeZ = mGrid->mCellSize.z;
    TVec3f Z = (axisZ * 0.5f) * sizeZ;
    TVec3f c0, c1, c2, c3;
    switch (mDirection) {
    case 0:
        c0 = (X + Y) + Z;
        c1 = (X - Y) + Z;
        c2 = (X - Y) - Z;
        c3 = (X + Y) - Z;
        break;
    case 1:
        c0 = (-X + Y) - Z;
        c1 = (-X - Y) - Z;
        c2 = (-X - Y) + Z;
        c3 = (-X + Y) + Z;
        break;
    case 2:
        c0 = (X + Y) + Z;
        c1 = (X + Y) - Z;
        c2 = (-X + Y) - Z;
        c3 = (-X + Y) + Z;
        break;
    case 3:
        c0 = (X - Y) - Z;
        c1 = (X - Y) + Z;
        c2 = (-X - Y) + Z;
        c3 = (-X - Y) - Z;
        break;
    case 4:
        c0 = (-X + Y) + Z;
        c1 = (-X - Y) + Z;
        c2 = (X - Y) + Z;
        c3 = (X + Y) + Z;
        break;
    case 5:
        c0 = (X + Y) - Z;
        c1 = (X - Y) - Z;
        c2 = (-X - Y) - Z;
        c3 = (-X + Y) - Z;
        break;
    }

    TVec3f drop;
    calcDropOffset(&drop);
    s32 end = mHistory->mCount;
    if (end == mHistory->mCapacity) {
        end--;
    }
    s32 start = 1;
    if (end == 1) {
        start = 0;
    }
    for (s32 i = start; i < end; i++) {
        if (!mHistory->hasNeighbor(i, mDirection) || mHistory->getDistance(i, mDirection) != 1) {
            f32 len = calcFaceLength(i, mDirection);
            TVec3f pos;
            mGrid->calcWorldCellPos(&pos, mHistory->getNode(i)->mCell);
            s32 base = mPointNum * 8;
            mPoints[base] = pos + c0;
            mPoints[base + 1] = pos + c1;
            mPoints[base + 2] = pos + c2;
            mPoints[base + 3] = pos + c3;
            mPoints[base + 4] = (pos + c0) + drop * len;
            mPoints[base + 5] = (pos + c1) + drop * len;
            mPoints[base + 6] = (pos + c2) + drop * len;
            mPoints[base + 7] = (pos + c3) + drop * len;
            mPointNum++;
        }
    }
}

void RepeatBlockShadowDrawer::makeFace() {
    makeHeadFace();
    makeTailFace();
}

void RepeatBlockShadowDrawer::calcDropOffset(TVec3f* pDst) const {
    TVec3f axisX, axisY, axisZ;
    mGrid->getAxisX(&axisX);
    mGrid->getAxisY(&axisY);
    mGrid->getAxisZ(&axisZ);
    switch (mDirection) {
    case 0:
        pDst->set(axisX * mGrid->mCellSize.x);
        break;
    case 1:
        pDst->set(-axisX * mGrid->mCellSize.x);
        break;
    case 2:
        pDst->set(axisY * mGrid->mCellSize.y);
        break;
    case 3:
        pDst->set(-axisY * mGrid->mCellSize.y);
        break;
    case 4:
        pDst->set(axisZ * mGrid->mCellSize.z);
        break;
    case 5:
        pDst->set(-axisZ * mGrid->mCellSize.z);
        break;
    }
}

s32 RepeatBlockShadowDrawer::calcDirection(const RepeatBlockCell* pFrom, const RepeatBlockCell* pTo) const {
    REPEAT_BLOCK_CELL_SUB(diff, *pTo, *pFrom);
    if (diff.mX < 0) {
        return 1;
    }
    if (diff.mX > 0) {
        return 0;
    }
    if (diff.mY < 0) {
        return 3;
    }
    if (diff.mY > 0) {
        return 2;
    }
    s32 ret = 4;
    if (diff.mZ < 0) {
        ret = 5;
    }
    return ret;
}

void RepeatBlockShadowDrawer::drawBox(const TVec3f* pPoints) const {
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 10);
    GXPosition3f32(pPoints[0].x, pPoints[0].y, pPoints[0].z);
    GXPosition3f32(pPoints[4].x, pPoints[4].y, pPoints[4].z);
    GXPosition3f32(pPoints[1].x, pPoints[1].y, pPoints[1].z);
    GXPosition3f32(pPoints[5].x, pPoints[5].y, pPoints[5].z);
    GXPosition3f32(pPoints[2].x, pPoints[2].y, pPoints[2].z);
    GXPosition3f32(pPoints[6].x, pPoints[6].y, pPoints[6].z);
    GXPosition3f32(pPoints[3].x, pPoints[3].y, pPoints[3].z);
    GXPosition3f32(pPoints[7].x, pPoints[7].y, pPoints[7].z);
    GXPosition3f32(pPoints[0].x, pPoints[0].y, pPoints[0].z);
    GXPosition3f32(pPoints[4].x, pPoints[4].y, pPoints[4].z);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    GXPosition3f32(pPoints[0].x, pPoints[0].y, pPoints[0].z);
    GXPosition3f32(pPoints[1].x, pPoints[1].y, pPoints[1].z);
    GXPosition3f32(pPoints[3].x, pPoints[3].y, pPoints[3].z);
    GXPosition3f32(pPoints[2].x, pPoints[2].y, pPoints[2].z);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    GXPosition3f32(pPoints[7].x, pPoints[7].y, pPoints[7].z);
    GXPosition3f32(pPoints[6].x, pPoints[6].y, pPoints[6].z);
    GXPosition3f32(pPoints[4].x, pPoints[4].y, pPoints[4].z);
    GXPosition3f32(pPoints[5].x, pPoints[5].y, pPoints[5].z);
}

s32 RepeatBlockShadowDrawer::calcFaceLength(s32 n, s32 dir) const {
    s32 len = mHistory->getDistance(n, dir);
    if (len > mMaxLength) {
        len = mMaxLength;
    }
    return len;
}

void RepeatBlockShadowDrawer::makeHeadFace() {
    mVolume->mIsValid = false;
    if (mHistory->mCount < 2) {
        mHasHead = false;
        return;
    }
    s32 len = calcFaceLength(0, mDirection);
    if (len == 1) {
        mHasHead = false;
        return;
    }
    TVec3f drop;
    calcDropOffset(&drop);
    RepeatBlockCell* pFirst = &mHistory->getNode(0)->mCell;
    mVolume->setup(pFirst, calcDirection(pFirst, &mHistory->getNode(1)->mCell), mOffset);
    TVec3f offset = drop * (f32)len;
    mHasHead = mVolume->calcFace(mHeadPoints, mDirection, offset);
}

void RepeatBlockShadowDrawer::makeTailFace() {
    mVolume->mIsValid = false;
    if (mHistory->mCount < mHistory->mCapacity) {
        mHasTail = false;
        return;
    }
    s32 len = calcFaceLength(mHistory->mCount - 1, mDirection);
    if (len == 1) {
        mHasTail = false;
        return;
    }
    TVec3f drop;
    calcDropOffset(&drop);
    RepeatBlockCell* pLast = &mHistory->getNode(mHistory->mCount - 1)->mCell;
    mVolume->setup(pLast, calcDirection(pLast, &mHistory->getNode(mHistory->mCount - 2)->mCell), 1.0f - mOffset);
    TVec3f offset = drop * (f32)len;
    mHasTail = mVolume->calcFace(mTailPoints, mDirection, offset);
}

RepeatBlockVolume::RepeatBlockVolume(RepeatBlockGrid* pGrid) : mGrid(pGrid), mIsValid(false) {
}

void RepeatBlockVolume::setup(const RepeatBlockCell* pCell, s32 dir, f32 rate) {
    TVec3f shift;
    TVec3f scale;
    switch (dir) {
    case 0:
        scale.x = rate;
        scale.y = 1.0f;
        scale.z = 1.0f;
        shift.set(-0.5f, 0.0f, 0.0f);
        break;
    case 1:
        scale.x = rate;
        scale.y = 1.0f;
        scale.z = 1.0f;
        shift.set(0.5f, 0.0f, 0.0f);
        break;
    case 2:
        scale.y = rate;
        scale.x = 1.0f;
        scale.z = 1.0f;
        shift.set(0.0f, -0.5f, 0.0f);
        break;
    case 3:
        scale.y = rate;
        scale.x = 1.0f;
        scale.z = 1.0f;
        shift.set(0.0f, 0.5f, 0.0f);
        break;
    case 4:
        scale.z = rate;
        scale.x = 1.0f;
        scale.y = 1.0f;
        shift.set(0.0f, 0.0f, -0.5f);
        break;
    case 5:
        scale.z = rate;
        scale.x = 1.0f;
        scale.y = 1.0f;
        shift.set(0.0f, 0.0f, 0.5f);
        break;
    }
    mPoints[0].x = 0.5f;
    mPoints[0].y = 0.5f;
    mPoints[0].z = -0.5f;
    mPoints[1].x = 0.5f;
    mPoints[1].y = 0.5f;
    mPoints[1].z = 0.5f;
    mPoints[2].x = -0.5f;
    mPoints[2].y = 0.5f;
    mPoints[2].z = 0.5f;
    mPoints[3].x = -0.5f;
    mPoints[3].y = 0.5f;
    mPoints[3].z = -0.5f;
    mPoints[4].x = 0.5f;
    mPoints[4].y = -0.5f;
    mPoints[4].z = -0.5f;
    mPoints[5].x = 0.5f;
    mPoints[5].y = -0.5f;
    mPoints[5].z = 0.5f;
    mPoints[6].x = -0.5f;
    mPoints[6].y = -0.5f;
    mPoints[6].z = 0.5f;
    mPoints[7].x = -0.5f;
    mPoints[7].y = -0.5f;
    mPoints[7].z = -0.5f;
    TVec3f center;
    mGrid->calcCellPos(&center, *pCell);
    TVec3f size(mGrid->mCellSize.x, mGrid->mCellSize.y, mGrid->mCellSize.z);
    for (s32 i = 0; i < 8; i++) {
        TVec3f* pPoint = &mPoints[i];
        pPoint->add(shift);
        pPoint->mul(scale);
        pPoint->sub(shift);
        pPoint->mul(size);
        pPoint->add(center);
        mGrid->localToWorld(pPoint, *pPoint);
    }
    mIsValid = true;
}

bool RepeatBlockVolume::calcFace(TVec3f* pDst, s32 dir, const TVec3f& rOffset) {
    if (!mIsValid) {
        return false;
    }
    if (MR::isNearZero(rOffset, 0.001f)) {
        return false;
    }
    switch (dir) {
    case 0:
        pDst[0].set(mPoints[0]);
        pDst[1].set(mPoints[1]);
        pDst[2].set(mPoints[5]);
        pDst[3].set(mPoints[4]);
        break;
    case 1:
        pDst[0].set(mPoints[2]);
        pDst[1].set(mPoints[3]);
        pDst[2].set(mPoints[7]);
        pDst[3].set(mPoints[6]);
        break;
    case 2:
        pDst[0].set(mPoints[1]);
        pDst[1].set(mPoints[0]);
        pDst[2].set(mPoints[3]);
        pDst[3].set(mPoints[2]);
        break;
    case 3:
        pDst[0].set(mPoints[4]);
        pDst[1].set(mPoints[5]);
        pDst[2].set(mPoints[6]);
        pDst[3].set(mPoints[7]);
        break;
    case 4:
        pDst[0].set(mPoints[2]);
        pDst[1].set(mPoints[6]);
        pDst[2].set(mPoints[5]);
        pDst[3].set(mPoints[1]);
        break;
    case 5:
        pDst[0].set(mPoints[0]);
        pDst[1].set(mPoints[4]);
        pDst[2].set(mPoints[7]);
        pDst[3].set(mPoints[3]);
        break;
    }
    for (s32 i = 0; i < 4; i++) {
        TVec3f tmp(pDst[i]);
        tmp.add(rOffset);
        pDst[i + 4] = tmp;
    }
    return true;
}

RepeatBlockShadowDrawer::~RepeatBlockShadowDrawer() {
}
