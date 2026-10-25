#pragma auto_inline off

#include "Game/MapObj/YoshiBlock.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/MapObj/DiceBase.hpp"
#include <JSystem/JGeometry/TUtil.hpp>
#include "Game/Util/DirectDraw.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>
#include "Game/System/ResourceHolder.hpp"

namespace MR {
    bool isMsgDashYoshiAttack(u32);
}


// SMG2: LiveActor is 0x90 bytes, so YoshiBlock's own fields start at 0x90.
namespace NrvYoshiBlock {
    class YoshiBlockNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }
        static YoshiBlockNrvWait sInstance;
    };
    YoshiBlockNrvWait YoshiBlockNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(YoshiBlockNrvDashed, YoshiBlock, Dashed);
};  // namespace NrvYoshiBlock

YoshiBlockCollision::YoshiBlockCollision(YoshiBlock* pBlock) : DynamicCollisionObj("ヨッシーブロックコリジョン") {
    mYoshiBlock = pBlock;
}

// NONMATCHING: regalloc only (copy-ctor argument registers set r4 before r3 in the target)
void YoshiBlockCollision::init(const JMapInfoIter& rIter) {
    DynamicCollisionObj::init(rIter);
    TVec3f sp20(mScale);
    sp20.scale(160.0f);
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    initHitSensor(1);
    MR::addHitSensorMapObj(this, "Body", 8, 80.0f, TVec3f(0.0f, 0.0f, 0.0f));
    mTriangleNum = 12;
    mPositionNum = 8;
    mPositions = new TVec3f[mPositionNum];
    sp20.mul(TVec3f(0.5f, 1.0f, 0.5f));
    mPositions[0].set<f32>(-sp20.x, sp20.y, sp20.z);
    mPositions[1].set<f32>(sp20.x, sp20.y, sp20.z);
    mPositions[2].set<f32>(sp20.x, sp20.y, -sp20.z);
    mPositions[3].set<f32>(-sp20.x, sp20.y, -sp20.z);
    mPositions[4].set<f32>(-sp20.x, 0.0f, sp20.z);
    mPositions[5].set<f32>(sp20.x, 0.0f, sp20.z);
    mPositions[6].set<f32>(sp20.x, 0.0f, -sp20.z);
    mPositions[7].set<f32>(-sp20.x, 0.0f, -sp20.z);
    mIndices = new TriangleIndexing[mTriangleNum];
    mIndices[0].set(0, 1, 2);
    mIndices[1].set(0, 2, 3);
    mIndices[2].set(4, 5, 1);
    mIndices[3].set(4, 1, 0);
    mIndices[4].set(5, 6, 2);
    mIndices[5].set(5, 2, 1);
    mIndices[6].set(6, 7, 3);
    mIndices[7].set(6, 3, 2);
    mIndices[8].set(7, 4, 0);
    mIndices[9].set(7, 0, 3);
    mIndices[10].set(7, 6, 5);
    mIndices[11].set(7, 5, 4);
    mFaceNormals = new TVec3f[mTriangleNum];
    createCollision();
}

void YoshiBlockCollision::appear() {
    LiveActor::appear();
    CollisionParts* pParts = mParts;
    pParts->mIsRegistered = 1;
    pParts->mNeedsZoneUpdate = 1;
}

void YoshiBlockCollision::kill() {
    LiveActor::kill();
    CollisionParts* pParts = mParts;
    pParts->mIsRegistered = 0;
}

bool YoshiBlockCollision::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgDashYoshiAttack(msg)) {
        return mYoshiBlock->receiveMessage(msg, pSender, pReceiver);
    }
    return false;
}

YoshiBlockCollision::~YoshiBlockCollision() {
}

YoshiBlock::YoshiBlock(const char* pName) : LiveActor(pName) {
    mResource = nullptr;
    mCollision = nullptr;
    mMtx.identity();
}

void YoshiBlock::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    mBasePos = mPosition;
    mBaseRot = mRotation;
    TVec3f sp68(mScale);
    sp68.scale(160.0f);
    mSize = sp68;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    MR::connectToScene(this, 0x21, 5, -1, 5);
    MR::makeMtxTR(reinterpret_cast< MtxPtr >(&mMtx), mPosition, mRotation);
    mResource = MR::createAndAddResourceHolder("YoshiBlock.arc");
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(YoshiBlock, YoshiBlockNrvWait), 0);

    mCollision = new DiceMover(8, 0);
#ifdef GALAXY_NATIVE
    {
        const TVec3f point(0.5f * -mSize.x, 0.5f * mSize.y, 0.5f * mSize.z);
        mCollision->addCollisionPoint(&point);
    }
    {
        const TVec3f point(0.5f * mSize.x, 0.5f * mSize.y, 0.5f * mSize.z);
        mCollision->addCollisionPoint(&point);
    }
    {
        const TVec3f point(0.5f * mSize.x, 0.5f * mSize.y, 0.5f * -mSize.z);
        mCollision->addCollisionPoint(&point);
    }
    {
        const TVec3f point(0.5f * -mSize.x, 0.5f * mSize.y, 0.5f * -mSize.z);
        mCollision->addCollisionPoint(&point);
    }
    {
        const TVec3f point(0.5f * -mSize.x, 0.5f * -mSize.y, 0.5f * mSize.z);
        mCollision->addCollisionPoint(&point);
    }
    {
        const TVec3f point(0.5f * mSize.x, 0.5f * -mSize.y, 0.5f * mSize.z);
        mCollision->addCollisionPoint(&point);
    }
    {
        const TVec3f point(0.5f * mSize.x, 0.5f * -mSize.y, 0.5f * -mSize.z);
        mCollision->addCollisionPoint(&point);
    }
    {
        const TVec3f point(0.5f * -mSize.x, 0.5f * -mSize.y, 0.5f * -mSize.z);
        mCollision->addCollisionPoint(&point);
    }
#else
    mCollision->addCollisionPoint(&TVec3f(0.5f * -mSize.x, 0.5f * mSize.y, 0.5f * mSize.z));
    mCollision->addCollisionPoint(&TVec3f(0.5f * mSize.x, 0.5f * mSize.y, 0.5f * mSize.z));
    mCollision->addCollisionPoint(&TVec3f(0.5f * mSize.x, 0.5f * mSize.y, 0.5f * -mSize.z));
    mCollision->addCollisionPoint(&TVec3f(0.5f * -mSize.x, 0.5f * mSize.y, 0.5f * -mSize.z));
    mCollision->addCollisionPoint(&TVec3f(0.5f * -mSize.x, 0.5f * -mSize.y, 0.5f * mSize.z));
    mCollision->addCollisionPoint(&TVec3f(0.5f * mSize.x, 0.5f * -mSize.y, 0.5f * mSize.z));
    mCollision->addCollisionPoint(&TVec3f(0.5f * mSize.x, 0.5f * -mSize.y, 0.5f * -mSize.z));
    mCollision->addCollisionPoint(&TVec3f(0.5f * -mSize.x, 0.5f * -mSize.y, 0.5f * -mSize.z));
#endif
    mCollision->calcRadius();
    f32 radius = mCollision->mRadius;
    mCollision->mRadius = 1.5f * radius;
    updateCollisionMtx();

    mCollisionActor = new YoshiBlockCollision(this);
    mCollisionActor->init(rIter);
    MR::invalidateClipping(mCollisionActor);
    mCollisionActor->makeActorAppeared();
    mCollisionActor->mParts->setMtx(mMtx);
    makeActorAppeared();
}

void YoshiBlock::control() {
}

void YoshiBlock::draw() const {
    ResTIMG* pImg = static_cast< ResTIMG* >(mResource->mFileInfoTable->getRes("YoshiBlockTex.bti"));
    JUTTexture tex(pImg, 0);

    TVec3f sp1A0;
    mMtx.getTrans(sp1A0);
    TVec3f sp194;
    mMtx.getZDir(sp194);
    TVec3f sp188;
    mMtx.getYDir(sp188);
    TVec3f sp17C;
    mMtx.getXDir(sp17C);
    TDDraw::setup(1, 0, 1);
    ::GXSetZMode(1, GX_LESS, 1);

    TVec3f sp170 = sp1A0 + sp188.scaleInline(mSize.y).scaleInline(0.5f) + sp194.scaleInline(mSize.z).scaleInline(0.5f);
    TVec3f sp164 = sp1A0 + sp188.scaleInline(mSize.y).scaleInline(0.5f) + sp17C.scaleInline(mSize.x).scaleInline(0.5f);
    TVec3f sp158 = sp1A0 + sp188.scaleInline(mSize.y).scaleInline(0.5f) - sp194.scaleInline(mSize.z).scaleInline(0.5f);
    TVec3f sp14C = sp1A0 + sp188.scaleInline(mSize.y).scaleInline(0.5f) - sp17C.scaleInline(mSize.x).scaleInline(0.5f);
    TVec3f sp140 = sp1A0 + sp188.scaleInline(mSize.y);
    TVec3f sp134(sp1A0);

    TDDraw::drawTexture3D(sp170, sp194, sp188, 0.5f * mSize.y, 0.5f * mSize.x, &tex, false, false);
    TDDraw::drawTexture3D(sp164, sp17C, sp188, 0.5f * mSize.y, 0.5f * mSize.z, &tex, false, false);
    TDDraw::drawTexture3D(sp158, -sp194, sp188, 0.5f * mSize.y, 0.5f * mSize.x, &tex, false, false);
    TDDraw::drawTexture3D(sp14C, -sp17C, sp188, 0.5f * mSize.y, 0.5f * mSize.z, &tex, false, false);
    TDDraw::drawTexture3D(sp140, sp188, -sp194, 0.5f * mSize.z, 0.5f * mSize.x, &tex, false, false);
    TDDraw::drawTexture3D(sp134, -sp188, sp194, 0.5f * mSize.z, 0.5f * mSize.x, &tex, false, false);
}

void YoshiBlock::appear() {
    LiveActor::appear();
    mPosition.set(mBasePos);
    mRotation.set(mBaseRot);
    MR::makeMtxTR(reinterpret_cast< MtxPtr >(&mMtx), mPosition, mRotation);
    CollisionParts* pParts = mCollisionActor->mParts;
    pParts->setMtx(mMtx);
    mCollisionActor->appear();
    updateCollisionMtx();
    setNerve(GET_NERVE(YoshiBlock, YoshiBlockNrvWait));
}

void YoshiBlock::exeDashed() {
    TVec3f sp44;
    MR::calcGravityVector(this, &sp44, nullptr, 0);
#ifdef GALAXY_NATIVE
    const TVec3f gravity = sp44.multInLine(0.4f);
    mCollision->setGravity(&gravity);
#else
    mCollision->setGravity(&sp44.multInLine(0.4f));
#endif
    mCollision->update();
    TPos3f sp50;
    mCollision->calcMtx(&sp50);
    TVec3f sp38;
    sp50.getYDir(sp38);
    TVec3f sp2C;
    sp50.getTrans(sp2C);
    sp2C.sub(sp38.scaleInline(mSize.y).scaleInline(0.5f));
    sp50.setTrans(sp2C);
    mPosition.set(sp2C);
    mMtx.setInline(sp50);
    mMtx.getTrans(mPosition);

    mMtx.getEuler(mRotation);
    mRotation.x *= 57.295776f;
    mRotation.y *= 57.295776f;
    mRotation.z *= 57.295776f;
    if (MR::isGreaterStep(this, 180)) {
        kill();
    }
}

bool YoshiBlock::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(YoshiBlock, YoshiBlockNrvWait))) {
        if (MR::isMsgDashYoshiAttack(msg)) {
            const TVec3f* pVelocity = MR::getPlayerVelocity();
            const TVec3f* pSensorPos = MR::getSensorPos(pSender);
            doYoshiDashed(*pSensorPos, *pVelocity);
            return true;
        }
    }
    return false;
}

void YoshiBlock::updateCollisionMtx() {
    TVec3f sp44;
    mMtx.getYDir(sp44);
    TPos3f sp50(mMtx);
    sp50.setTrans(mPosition + sp44.scaleInline(mSize.y).scaleInline(0.5f) + sp44.scaleInline(10.0f));
    mCollision->setMtx(sp50.toMtxPtr());
}

void YoshiBlock::doYoshiDashed(const TVec3f& rPos, const TVec3f& rVel) {
    mCollision->collide(&rPos, &rVel);
    mCollisionActor->kill();
    MR::shakeCameraNormal();
    MR::tryRumblePadMiddle(this, 0);
    setNerve(GET_NERVE(YoshiBlock, YoshiBlockNrvDashed));
}

YoshiBlock::~YoshiBlock() {
}
