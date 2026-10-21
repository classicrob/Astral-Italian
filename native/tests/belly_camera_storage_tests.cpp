// Actual canonical LiveActor/NameObj/JMapLinkInfo storage; inert lifecycle fixtures.
// The extracted assignment is exercised, not the complete BellyDragon::init lifecycle.
#include "Game/Util/BellyDragon.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <type_traits>
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



static unsigned checks;
static void require(bool ok,const char* why){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
template<class T> static auto snapshot(const T& object){std::array<unsigned char,sizeof(T)> out;std::memcpy(out.data(),&object,sizeof(T));return out;}
template<class T> static void retained(const T& object,const std::array<unsigned char,sizeof(T)>& before,std::size_t a,std::size_t n,std::size_t b=0,std::size_t m=0){
 auto after=snapshot(object);for(std::size_t i=0;i<after.size();++i)if(!(i>=a&&i<a+n)&&!(i>=b&&i<b+m))require(after[i]==before[i],"every byte outside intended fields retained");
}
static void matrix(TPos3f& m,unsigned turn){
 const unsigned bits[]={0,0x80000000u,0x3f800000u,0xbf800000u,0x7f800000u,0xff800000u,0x7fc12345u,0xffc54321u,1,0x7f7fffffu,0x00800000u,0x40200000u};
 for(unsigned i=0;i<12;++i){unsigned v=bits[(i+turn)%12];std::memcpy(&m.mMtx[i/4][i%4],&v,4);}
}

// Inert lifecycle fixtures instantiate the actual production types; no layout model.
BellyDragon::BellyDragon(const char* n):LiveActor(n),mIsLv2(true),mBody(nullptr),mPod(nullptr),mJointCtrl(nullptr),mArmA(nullptr),mArmB(nullptr),mDemoB(nullptr),mDemoA(nullptr){}
BellyDragon::~BellyDragon(){}
void BellyDragon::init(const JMapInfoIter&){}
EMPTY0(BellyDragon,appear) EMPTY0(BellyDragon,kill) EMPTY0(BellyDragon,control) EMPTY0(BellyDragon,calcAndSetBaseMtx)
void BellyDragon::attackSensor(HitSensor*,HitSensor*){}
bool BellyDragon::receiveMsgPlayerAttack(u32,HitSensor*,HitSensor*){return false;}
MtxPtr BellyDragon::getBaseMtx() const{return const_cast<MtxPtr>(mBaseMtx.mMtx);}
BellyDragonPod::BellyDragonPod(BellyDragon* h,const JMapInfoIter*):mHost(h),mCameraInfo(nullptr),mFlameCtrl(nullptr),mDecoration(nullptr),mCameraTargetMtx(nullptr),mEffectCount(0),mGroupA(nullptr),mGroupB(nullptr){}

CameraTargetObj::CameraTargetObj(const char* n):NameObj(n),mCameraWall(false){}
CameraTargetMtx::CameraTargetMtx(const char* n):CameraTargetObj(n),mInvalidLastMove(false),mCameraArea(nullptr){}
CameraTargetMtx::~CameraTargetMtx(){}
EMPTY0(CameraTargetMtx,movement)
const TVec3f& CameraTargetMtx::getPosition() const{return mPosition;}
const TVec3f& CameraTargetMtx::getUpVec() const{return mUp;}
const TVec3f& CameraTargetMtx::getFrontVec() const{return mFront;}
const TVec3f& CameraTargetMtx::getSideVec() const{return mSide;}
const TVec3f& CameraTargetMtx::getLastMove() const{return mLastMove;}
const TVec3f& CameraTargetMtx::getGroundPos() const{return mPosition;}
const TVec3f& CameraTargetMtx::getGravityVector() const{return mGravityVector;}
static void apply(BellyDragon* pHost){
// __PRODUCTION_SITE__
}
int main(){
 static_assert(offsetof(CameraTargetMtx,mMatrix)==0x1C,"actual native field");
 BellyDragon source("source");BellyDragonPod pod(&source,nullptr);CameraTargetMtx dest("dest");source.mPod=&pod;pod.mCameraTargetMtx=&dest;
 for(unsigned t=0;t<12;++t){matrix(pod.mHostMtx,t);matrix(dest.mMatrix,t+1);auto before=snapshot(dest);auto src=snapshot(pod);apply(&source);
 require(std::memcmp(&dest.mMatrix,&pod.mHostMtx,sizeof(TPos3f))==0,"canonical matrix copy");retained(dest,before,offsetof(CameraTargetMtx,mMatrix),sizeof(TPos3f));require(snapshot(pod)==src,"source pod retained");}
 std::printf("PASS: %u camera typed storage checks\n",checks);
}
