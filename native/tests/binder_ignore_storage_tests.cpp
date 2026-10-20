// Canonical production types and extracted production functions. Constructors and
// unrelated virtuals below are inert lifecycle fixtures, not a Binder::bind test.
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Binder.hpp"
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
JMapLinkInfo::JMapLinkInfo() : mFollowID(-1), mZoneID(-1), mContext(-1) {}
LiveActorFlag::LiveActorFlag() : mIsDead(false), mIsNotReleasedAnimFrame(false), mIsOnCalcAnim(false),
    mIsNoCalcView(false), mIsNoEntryDrawBuffer(false), mIsOnBind(false), mIsCalcGravity(false),
    mIsClipped(false), mIsClippingInvalid(false) {}
NameObj::NameObj(const char* name) : mName(name), mFlags(0), mExecutorIdx(-1), mLinkInfo() {}
NameObj::~NameObj() {}
#define EMPTY0(cls, name) void cls::name() {}
void NameObj::init(const JMapInfoIter&) {}
EMPTY0(NameObj, initAfterPlacement)
EMPTY0(NameObj, movement)
void NameObj::draw() const {}
EMPTY0(NameObj, calcAnim)
EMPTY0(NameObj, calcViewAndEntry)
EMPTY0(NameObj, startMovement)
EMPTY0(NameObj, endMovement)
LiveActor::LiveActor(const char* name) : NameObj(name), mPosition(0.0f), mRotation(0.0f), mScale(1.0f),
    mVelocity(0.0f), mGravity(0.0f), mActionKeeper(), mModelManager(), mSpine(), mSensorKeeper(), mBinder(),
    mRailRider(), mEffectKeeper(), mSoundHolder(), mFlags(), mShadowControllerList(), mCollisionParts(),
    mStageSwitchCtrl(), mStarPointerTarget(), mActorLightCtrl() {}
void LiveActor::init(const JMapInfoIter&) {}
EMPTY0(LiveActor, movement)
EMPTY0(LiveActor, calcAnim)
EMPTY0(LiveActor, calcViewAndEntry)
EMPTY0(LiveActor, appear)
EMPTY0(LiveActor, makeActorAppeared)
EMPTY0(LiveActor, kill)
EMPTY0(LiveActor, makeActorDead)
bool LiveActor::receiveMessage(u32, HitSensor*, HitSensor*) { return false; }
MtxPtr LiveActor::getBaseMtx() const { return nullptr; }
MtxPtr LiveActor::getTakingMtx() const { return nullptr; }
EMPTY0(LiveActor, startClipped)
EMPTY0(LiveActor, endClipped)
EMPTY0(LiveActor, calcAndSetBaseMtx)
void LiveActor::updateHitSensor(HitSensor*) {}
void LiveActor::attackSensor(HitSensor*, HitSensor*) {}
bool LiveActor::receiveMsgPush(HitSensor*, HitSensor*) { return false; }
bool LiveActor::receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*) { return false; }

Triangle::Triangle() : mParts(), mIdx(), mSensor(), mNormals(), mPos() {}
HitInfo::HitInfo() : mParentTriangle(), mDistance(), mHitPos(), mQueryPos(), mNormalMovement(), mCollisionType(), mReserved() {}
Binder::Binder(MtxPtr m, const TVec3f* p, const TVec3f* g, f32 r, f32 y, u32 n)
    : mTriangleFilter(), mCollisionPartsFilter(), mExCollisionParts(), _C(m), _10(p), _14(g), mRadius(r), mOffsetY(y),
      mOffsetVec(), _24(n), mPlaneNum(), mPlane(), mFixReactionVector(), mGroundInfo(), _C8(), mWallInfo(), _158(),
      mRoofInfo(), _1E8(), _1EC() {}

namespace MR {
// __PRODUCTION_FUNCTION__
}
static bool movingCollisionGate(const Binder& binder) {
    // __PRODUCTION_READER__
}
static unsigned checks;
static void require(bool ok,const char* why){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
int main(){
    static_assert(sizeof(void*)==8,"Native layout regression");
    static_assert(offsetof(Binder,_1EC)==0x254,"Rebind changed native Binder layout");
    LiveActor actor("fixture"); Binder binder(nullptr,&actor.mPosition,&actor.mGravity,1.0f,0.0f,0);actor.mBinder=&binder;
    for(unsigned seed=0;seed<256;++seed){
        const unsigned char initial=static_cast<unsigned char>(seed);std::memcpy(&binder._1EC,&initial,1);
        require(movingCollisionGate(binder)==((initial&0x01)!=0),"native moving-collision reader matches canonical _0 seed");
        std::array<unsigned char,sizeof(Binder)> before;std::array<unsigned char,sizeof(LiveActor)> actorBefore;
        std::memcpy(before.data(),&binder,before.size());std::memcpy(actorBefore.data(),static_cast<const void*>(&actor),actorBefore.size());
        const unsigned externalBefore=binder._1EC._2,stopBefore=binder._1EC._5;
        MR::setBinderIgnoreMovingCollision(&actor);
        require(!movingCollisionGate(binder),"native moving-collision reader disabled; old0x80 leaves _0 enabled");
        unsigned char value;std::memcpy(&value,&binder._1EC,1);require(value==(initial&0xFE),"only native _0 bit changes; other named/padding bits retained");
        require(binder._1EC._2==externalBefore&&binder._1EC._5==stopBefore,"external collision and stopAtFirstHit flags retained");
        const auto* bytes=reinterpret_cast<const unsigned char*>(&binder);
        for(std::size_t i=0;i<before.size();++i)if(i!=offsetof(Binder,_1EC))require(bytes[i]==before[i],"unrelated Binder bytes retained");
        require(std::memcmp(static_cast<const void*>(&actor),actorBefore.data(),actorBefore.size())==0,"actor bytes retained");
        MR::setBinderIgnoreMovingCollision(&actor);unsigned char repeated;std::memcpy(&repeated,&binder._1EC,1);require(repeated==value,"repeat disable idempotent");
    }
    std::printf("PASS: %u canonical Binder setter/reader storage checks\n",checks);
}
