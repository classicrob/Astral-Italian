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

// Real string bytes (SJIS) from the target data.
static const char lbl_collisionName[] = "\x83\x88\x83\x62\x83\x56\x81\x5B\x83\x75\x83\x8D\x83\x62\x83\x4E\x83\x52\x83\x8A\x83\x57\x83\x87\x83\x93";

// SMG2 DynamicCollisionObj fields sit 4 bytes past the Petari offsets (see the header note).
#define YBC_FIELD(T, ofs) (*reinterpret_cast<T*>(reinterpret_cast<u8*>(this) + (ofs)))

static CollisionParts* getCollisionParts(const YoshiBlockCollision* pCollision) {
    return *reinterpret_cast<CollisionParts* const*>(reinterpret_cast<const u8*>(pCollision) + 0x94);
}

YoshiBlockCollision::YoshiBlockCollision(YoshiBlock* pBlock) : DynamicCollisionObj(lbl_collisionName) {
    mYoshiBlock = pBlock;
}

// NONMATCHING: TVec3(f,f,f) ctor is inlined here (target calls __ct__..Ffff out of line); array-count reloads and register order differ.
void YoshiBlockCollision::init(const JMapInfoIter& rIter) {
    DynamicCollisionObj::init(rIter);
    TVec3f sp20(mScale);
    sp20.scale(160.0f);
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    initHitSensor(1);
    TVec3f sp14(0.0f, 0.0f, 0.0f);
    MR::addHitSensorMapObj(this, "Body", 8, 80.0f, sp14);
    YBC_FIELD(u16, 0x98) = 12;
    YBC_FIELD(s16, 0x9A) = 8;
    YBC_FIELD(TVec3f*, 0x9C) = new TVec3f[8];
    TVec3f sp8(0.5f, 1.0f, 0.5f);
    sp20.x *= sp8.x;
    sp20.y *= sp8.y;
    sp20.z *= sp8.z;
    TVec3f sp = sp20;
    YBC_FIELD(TVec3f*, 0x9C)[0].set<f32>(-sp.x, sp.y, sp.z);
    YBC_FIELD(TVec3f*, 0x9C)[1].set<f32>(sp.x, sp.y, sp.z);
    YBC_FIELD(TVec3f*, 0x9C)[2].set<f32>(sp.x, sp.y, -sp.z);
    YBC_FIELD(TVec3f*, 0x9C)[3].set<f32>(-sp.x, sp.y, -sp.z);
    YBC_FIELD(TVec3f*, 0x9C)[4].set<f32>(-sp.x, 0.0f, sp.z);
    YBC_FIELD(TVec3f*, 0x9C)[5].set<f32>(sp.x, 0.0f, sp.z);
    YBC_FIELD(TVec3f*, 0x9C)[6].set<f32>(sp.x, 0.0f, -sp.z);
    YBC_FIELD(TVec3f*, 0x9C)[7].set<f32>(-sp.x, 0.0f, -sp.z);
    u16 triNum = YBC_FIELD(u16, 0x98);
    YBC_FIELD(TriangleIndexing*, 0xA4) = new TriangleIndexing[triNum];
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[0] = 0;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[1] = 1;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[2] = 2;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[3] = 0;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[4] = 2;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[5] = 3;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[6] = 4;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[7] = 5;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[8] = 1;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[9] = 4;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[10] = 1;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[11] = 0;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[12] = 5;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[13] = 6;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[14] = 2;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[15] = 5;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[16] = 2;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[17] = 1;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[18] = 6;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[19] = 7;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[20] = 3;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[21] = 6;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[22] = 3;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[23] = 2;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[24] = 7;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[25] = 4;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[26] = 0;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[27] = 7;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[28] = 0;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[29] = 3;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[30] = 7;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[31] = 6;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[32] = 5;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[33] = 7;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[34] = 5;
    reinterpret_cast<u16*>(YBC_FIELD(TriangleIndexing*, 0xA4))[35] = 4;
    YBC_FIELD(TVec3f*, 0xA0) = new TVec3f[triNum];
    createCollision();
}

void YoshiBlockCollision::appear() {
    LiveActor::appear();
    CollisionParts* pParts = *reinterpret_cast< CollisionParts** >(reinterpret_cast< u8* >(this) + 0x94);
    *reinterpret_cast< u8* >(reinterpret_cast< u8* >(pParts) + 0xCC) = 1;
    *reinterpret_cast< u8* >(reinterpret_cast< u8* >(pParts) + 0xCD) = 1;
}

void YoshiBlockCollision::kill() {
    LiveActor::kill();
    CollisionParts* pParts = *reinterpret_cast< CollisionParts** >(reinterpret_cast< u8* >(this) + 0x94);
    *reinterpret_cast< u8* >(reinterpret_cast< u8* >(pParts) + 0xCC) = 0;
}

bool YoshiBlockCollision::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgDashYoshiAttack(msg)) {
        return mYoshiBlock->receiveOtherMsg(msg, pSender, pReceiver);
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

// NONMATCHING: same TVec3 ctor inlining as YoshiBlockCollision::init; stack frame and vertex scheduling differ.
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

    DiceMover* pMover = new DiceMover(8, 0);
    mCollision = pMover;

    f32 hx = 0.5f * mSize.x;
    f32 hy = 0.5f * mSize.y;
    f32 hz = 0.5f * mSize.z;
    TVec3f sp5C(-hx, hy, hz);
    mCollision->addCollisionPoint(&sp5C);
    TVec3f sp50(hx, hy, hz);
    mCollision->addCollisionPoint(&sp50);
    TVec3f sp44(hx, hy, -hz);
    mCollision->addCollisionPoint(&sp44);
    TVec3f sp38(-hx, hy, -hz);
    mCollision->addCollisionPoint(&sp38);
    TVec3f sp2C(-hx, -hy, hz);
    mCollision->addCollisionPoint(&sp2C);
    TVec3f sp20(hx, -hy, hz);
    mCollision->addCollisionPoint(&sp20);
    TVec3f sp14(hx, -hy, -hz);
    mCollision->addCollisionPoint(&sp14);
    TVec3f sp8(-hx, -hy, -hz);
    mCollision->addCollisionPoint(&sp8);
    mCollision->calcRadius();
    mCollision->mSpeedLimit *= 1.5f;
    updateCollisionMtx();

    mCollisionActor = new YoshiBlockCollision(this);
    mCollisionActor->init(rIter);
    MR::invalidateClipping(mCollisionActor);
    mCollisionActor->makeActorAppeared();
    getCollisionParts(mCollisionActor)->setMtx(mMtx);
    makeActorAppeared();
}

void YoshiBlock::control() {
}

// NONMATCHING: JUTTexture local gets its ctor inlined (target stores nothing before storeTIMG); otherwise the draw math matches.
void YoshiBlock::draw() const {
    ResTIMG* pImg = static_cast< ResTIMG* >(mResource->mFileInfoTable->getRes("YoshiBlockTex.bti"));
    JUTTexture tex;
    tex.storeTIMG(pImg, static_cast<u8>(0));
    tex.mFlag &= 2;

    TVec3f sp1A0;
    mMtx.getTrans(sp1A0);
    TVec3f sp194;
    sp194.set<f32>(mMtx.mMtx[0][2], mMtx.mMtx[1][2], mMtx.mMtx[2][2]);
    TVec3f sp188;
    mMtx.getYDir(sp188);
    TVec3f sp17C;
    sp17C.set<f32>(mMtx.mMtx[0][0], mMtx.mMtx[1][0], mMtx.mMtx[2][0]);
    TDDraw::setup(1, 0, 1);
    ::GXSetZMode(1, GX_LEQUAL, 1);

    TVec3f sp110(sp188);
    sp110.scale(mSize.y);
    TVec3f sp11C(sp110);
    sp11C.scale(0.5f);
    TVec3f spF8(sp194);
    spF8.scale(mSize.z);
    TVec3f sp104(spF8);
    sp104.scale(0.5f);
    TVec3f sp128 = sp1A0 + sp11C;
    TVec3f sp170 = sp128 + sp104;
    TVec3f spD4(sp188);
    spD4.scale(mSize.y);
    TVec3f spE0(spD4);
    spE0.scale(0.5f);
    TVec3f spBC(sp17C);
    spBC.scale(mSize.x);
    TVec3f spC8(spBC);
    spC8.scale(0.5f);
    TVec3f spEC = sp1A0 + spE0;
    TVec3f sp164 = spEC + spC8;
    TVec3f sp80(sp194);
    sp80.scale(mSize.z);
    TVec3f sp8C(sp80);
    sp8C.scale(0.5f);
    TVec3f sp98(sp188);
    sp98.scale(mSize.y);
    TVec3f spA4(sp98);
    spA4.scale(0.5f);
    TVec3f spB0 = sp1A0 + spA4;
    TVec3f sp158(spB0);
    sp158.sub(sp8C);
    TVec3f sp44(sp17C);
    sp44.scale(mSize.x);
    TVec3f sp50(sp44);
    sp50.scale(0.5f);
    TVec3f sp5C(sp188);
    sp5C.scale(mSize.y);
    TVec3f sp68(sp5C);
    sp68.scale(0.5f);
    TVec3f sp74 = sp1A0 + sp68;
    TVec3f sp14C(sp74);
    sp14C.sub(sp50);
    TVec3f sp38(sp188);
    sp38.scale(mSize.y);
    TVec3f sp140 = sp1A0 + sp38;
    TVec3f sp134(sp1A0);

    TDDraw::drawTexture3D(sp170, sp194, sp188, 0.5f * mSize.y, 0.5f * mSize.x, &tex, false, false);
    TDDraw::drawTexture3D(sp164, sp17C, sp188, 0.5f * mSize.y, 0.5f * mSize.z, &tex, false, false);
    TVec3f sp2C = -sp194;
    TDDraw::drawTexture3D(sp158, sp2C, sp188, 0.5f * mSize.y, 0.5f * mSize.x, &tex, false, false);
    TVec3f sp20 = -sp17C;
    TDDraw::drawTexture3D(sp14C, sp20, sp188, 0.5f * mSize.y, 0.5f * mSize.z, &tex, false, false);
    TVec3f sp14 = -sp194;
    TDDraw::drawTexture3D(sp140, sp188, sp14, 0.5f * mSize.z, 0.5f * mSize.x, &tex, false, false);
    TVec3f sp8 = -sp188;
    TDDraw::drawTexture3D(sp134, sp8, sp194, 0.5f * mSize.z, 0.5f * mSize.x, &tex, false, false);
}

void YoshiBlock::appear() {
    LiveActor::appear();
    mPosition.set(mBasePos);
    mRotation.set(mBaseRot);
    MR::makeMtxTR(reinterpret_cast< MtxPtr >(&mMtx), mPosition, mRotation);
    CollisionParts* pParts = *reinterpret_cast< CollisionParts** >(reinterpret_cast< u8* >(mCollisionActor) + 0x94);
    pParts->setMtx(mMtx);
    mCollisionActor->appear();
    updateCollisionMtx();
    setNerve(GET_NERVE(YoshiBlock, YoshiBlockNrvWait));
}

void YoshiBlock::exeDashed() {
    TVec3f sp44;
    MR::calcGravityVector(this, &sp44, nullptr, 0);
    TVec3f sp20(sp44);
    sp20.x *= 0.4f;
    sp20.y *= 0.4f;
    sp20.z *= 0.4f;
    mCollision->setGravity(&sp20);
    mCollision->update();
    TPos3f sp50;
    mCollision->calcMtx(&sp50);
    TVec3f sp38;
    sp50.getYDir(sp38);
    TVec3f sp2C;
    sp50.getTrans(sp2C);
    TVec3f sp8(sp38);
    sp8.scale(mSize.y);
    TVec3f sp14(sp8);
    sp14.scale(0.5f);
    sp2C.sub(sp14);
    sp50.setTrans(sp2C);
    mPosition.set(sp2C);
    mMtx = sp50;
    mMtx.getTrans(mPosition);

    f32 m20 = mMtx.mMtx[2][0];
    if (m20 - 1.0f >= -3.8147e-06f) {
        mRotation.x = JMath::sAtanTable.atan2_(-mMtx.mMtx[0][1], mMtx.mMtx[1][1]);
        mRotation.y = -1.5707964f;
        mRotation.z = 0.0f;
    } else if (m20 + 1.0f <= 3.8147e-06f) {
        mRotation.x = JMath::sAtanTable.atan2_(mMtx.mMtx[0][1], mMtx.mMtx[1][1]);
        mRotation.y = 1.5707964f;
        mRotation.z = 0.0f;
    } else {
        mRotation.x = JMath::sAtanTable.atan2_(mMtx.mMtx[2][1], mMtx.mMtx[2][2]);
        mRotation.z = JMath::sAtanTable.atan2_(mMtx.mMtx[1][0], mMtx.mMtx[0][0]);
        mRotation.y = JGeometry::TUtil<f32>::asin(-mMtx.mMtx[2][0]);
    }
    mRotation.x *= 57.29578f;
    mRotation.y *= 57.29578f;
    mRotation.z *= 57.29578f;
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
    f32 sizeY = mSize.y;
    TVec3f sp14(sp44);
    sp14.scale(sizeY);
    TVec3f sp20(sp14);
    sp20.scale(0.5f);
    TVec3f sp8(sp44);
    sp8.scale(10.0f);
    TVec3f sp2C = mPosition + sp20;
    TVec3f sp38 = sp2C + sp8;
    sp50.setTrans(sp38);
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
