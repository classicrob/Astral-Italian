#pragma auto_inline off

#include "Game/MapObj/RepeatBlock.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <revolution.h>
#include "Game/MapObj/RepeatBlockNrv.hpp"




namespace NrvRepeatBlock {
    RepeatBlockNrvWait RepeatBlockNrvWait::sInstance ATTRIBUTE_WEAK;
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


    RepeatBlockNrvAtomA RepeatBlockNrvAtomA::sInstance ATTRIBUTE_WEAK;


    RepeatBlockNrvAtomWait RepeatBlockNrvAtomWait::sInstance ATTRIBUTE_WEAK;


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
        mAppearProgress = mParent->mCellRate + (f32)mParent->mCellStep;
        mAppearStep = 0;
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
        mAppearProgress = mParent->mCellRate + (f32)mParent->mCellStep;
        mAppearStep = 0;
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
    pAtom->startAppear(mParent, mAppearDir);
}

void RepeatBlockTail::startAtom(RepeatBlockAtom* pAtom) {
    pAtom->startDisappear(mParent, mMoveDir);
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

#pragma push
#pragma auto_inline on
RepeatBlock::RepeatBlock(const char* pName) : LiveActor(pName), mNumX(2), mNumY(2), mNumZ(2), mTailNum(3), mHead(nullptr), mTail(nullptr), mTails(nullptr), mTailIdx(0), _10C(0), mSpeed(0.0f), _114(0.0f), _118(0.0f), _11C(0.0f), mCellLength(5.0f), mCellSize(0.0f, 0.0f, 0.0f), mMaxNum(0), mDir(1), mCell(0), mRailCell(0), mNextCell(0), mCellStep(0), mCellRate(0.0f), mDirHistory(nullptr), mPosHistory(nullptr), mHistoryWrite(0), mHistoryRead(0), mHistoryFull(false), mTailsAppeared(false), mParent(nullptr), mChildren(nullptr), mChildNum(0) {
    mMtx.identity();
    mInvMtx.identity();
}

#pragma pop

RepeatBlockCell::RepeatBlockCell(s16 v) {
    mX = v;
    mY = v;
    mZ = v;
}

void RepeatBlock::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    s32 arg;
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
    mCellSize.set<f32>(mNumX, mNumY, mNumZ);
    f32 scale = mCellLength / mMaxNum;
    mCellSize.x *= scale;
    mCellSize.y *= scale;
    mCellSize.z *= scale;
    mDirHistory = new s32[mTailNum + 1];
    mPosHistory = new TVec3f[mTailNum + 1];
    MR::connectToSceneMapObjMovement(this);
    initHitSensor(1);
    initSound(3, "RepeatBlock", nullptr, TVec3f(0.0f));
    MR::addHitSensorMapObj(this, "Body", 32, 0.0f, TVec3f(0, 0, 0));
    if (MR::isConnectedWithRail(rIter)) {
        initRailRider(rIter);
        MR::moveTransToCurrentRailPos(this);
    }
    initNerve(GET_NERVE(RepeatBlock, RepeatBlockNrvWait), 0);
    MR::makeMtxTR(mMtx, mPosition, mRotation);
    mInvMtx.set(mMtx);
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
    mHead->RepeatBlockHead::appearWithAtoms();
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
        mTail->updateAtomPositions();
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
    if (!MR::isSensorPlayerOrRide(pSender)) {
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
    RepeatBlockHead* pHead = new RepeatBlockHead("頭ブロック", this);
    mHead = pHead;
    mHead->RepeatBlockHead::appearWithAtoms();
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
    mHead->updateAtomPositions();
}

void RepeatBlock::initTail() {
    RepeatBlockTail* pTail = new RepeatBlockTail(this, "尻ブロック");
    mTail = pTail;
    mTail->kill();
}

void RepeatBlock::updateTail() {
    mTail->movement();
    if (mHistoryFull) {
        calcMovedPos(&mTail->mPosition, mDirHistory[mHistoryRead]);
        mTail->updateAtomPositions();
    }
}

void RepeatBlock::initTails() {
    mTails = new RepeatBlockTail*[mTailNum];
    for (s32 i = 0; i < mTailNum; i++) {
        RepeatBlockTail* pTail = new RepeatBlockTail(this, "リピートブロック本体");
        mTails[i] = pTail;
        mTails[i]->kill();
    }
}

void RepeatBlock::initHistory() {
    mHistory = new RepeatBlockHistory(mTailNum + 2);
}

void RepeatBlock::initGrid() {
    mGrid = new RepeatBlockGrid(mMtx, 200.0f * mNumX, 200.0f * mNumY, 200.0f * mNumZ);
}

void RepeatBlock::initShadow() {
    mShadowDrawer = new RepeatBlockShadowDrawer(mHistory, mGrid);
    mShadowDrawer->makeVertexBuffer();
}

void RepeatBlock::appearNextTail() {
    MR::setPosition(mTails[mTailIdx], mPosition);
    mTails[mTailIdx]->updateAtomPositions();
    mTails[mTailIdx]->appear();
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

// NONMATCHING: target retains more by-value RepeatBlockCell temporaries (5 struct copies of the cell difference, frame 0x50), `!isEqual` as cntlzw/srwi., and outlines the position-history assignment.
bool RepeatBlock::stepForward() {
    mGrid->worldToCell(&mCell, mPosition);
    if (mRailCell.isEqual(mCell)) {
        do {
            MR::moveCoord(this, mSpeed);
            mGrid->worldToCell(&mRailCell, MR::getRailPos(this));
            bool isDifferent = !mRailCell.isEqual(mCell);
            if (isDifferent) {
                break;
            }
            if (MR::isRailReachedGoal(this)) {
                return false;
            }
        } while (true);
    }

    REPEAT_BLOCK_CELL_SUB(diff, mRailCell, mCell);
    if (diff.mX != 0) {
        if (diff.mX < 0) {
            mDir = 0;
        } else {
            mDir = 1;
        }
    } else if (diff.mY != 0) {
        if (diff.mY < 0) {
            mDir = 2;
        } else {
            mDir = 3;
        }
    } else if (diff.mZ != 0) {
        if (diff.mZ < 0) {
            mDir = 4;
        } else {
            mDir = 5;
        }
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
    s32 currentDirection = mShadowDrawer->mDirection;
    s32 dir = 0;
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
    if (dir == currentDirection) {
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

// The inverse-matrix copy is outlined here; target expands its paired-single copy.
