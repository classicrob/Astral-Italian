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
template<class T> static auto snapshot(const T& object){std::array<unsigned char,sizeof(T)> out;std::memcpy(out.data(),static_cast<const void*>(&object),sizeof(T));return out;}
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

static unsigned appeared,helperCalls;static TPos3f* expectedDest;static NameObj* expectedHost;static const TVec3f* expectedVec;static const float (*expectedBase)[4];static float expectedScale;static TPos3f writeValue;static LiveActor* available;
BellyDragonFireGround::BellyDragonFireGround():LiveActor("ground"),mBloom(nullptr){}
BellyDragonFireGround::~BellyDragonFireGround(){}
void BellyDragonFireGround::init(const JMapInfoIter&){}
void BellyDragonFireGround::appear(){require(helperCalls==1,"matrix helper before appear");++appeared;}
EMPTY0(BellyDragonFireGround,kill) EMPTY0(BellyDragonFireGround,calcAndSetBaseMtx)
void BellyDragonFireGround::attackSensor(HitSensor*,HitSensor*){}
NameObjGroup::NameObjGroup(const char* n,int count):NameObj(n),mObjNumMax(count),mObjNum(0),mObjArray(nullptr){}
NameObjGroup::~NameObjGroup(){}
LiveActorGroup::LiveActorGroup(const char* n,int c):NameObjGroup(n,c){}
LiveActor* LiveActorGroup::getDeadActor()const{return available;}
namespace BellyDragonUtil{
s32 isLv2(BellyDragon* p){return p->mIsLv2;}
// Matrix generators are explicit recording fixtures. Production caller is unmodified/extracted.
void calcGroundUpMtx(TPos3f* out,NameObj* h,const TVec3f* v){require(out==expectedDest,"canonical matrix destination");require(h==expectedHost&&v==expectedVec,"up helper arguments retained");++helperCalls;out->set(writeValue.mMtx);}
void calcGroundHillMtx(TPos3f* out,NameObj* h,const float (*base)[4],float scale){require(out==expectedDest,"canonical matrix destination");require(h==expectedHost&&base==expectedBase&&scale==expectedScale,"hill helper arguments retained");++helperCalls;out->set(writeValue.mMtx);}
}
// Exact source scalar definitions for this full caller are inserted by the extractor.
// __PRODUCTION_SCALARS__
// __PRODUCTION_SITE__
int main(){
 static_assert(offsetof(BellyDragonFireGround,mBaseMtx)==0xD0,"actual native ground matrix");
 BellyDragon source("host");BellyDragonPod pod(&source,nullptr);BellyDragonFireGround dest;LiveActorGroup group("group",1);pod.mGroupA=&group;TVec3f vec(1,2,3);expectedDest=&dest.mBaseMtx;expectedHost=&source;expectedBase=source.mBaseMtx.mMtx;
 for(unsigned t=0;t<12;++t)for(int path=0;path<3;++path){available=&dest;source.mIsLv2=true;matrix(writeValue,t);matrix(dest.mBaseMtx,t+1);helperCalls=appeared=0;expectedVec=path==0?&vec:nullptr;expectedScale=path==2?lbl_807D7850:lbl_807D784C;auto before=snapshot(dest);auto src=snapshot(source);pod.appearFireGround(path==2,expectedVec);
 require(helperCalls==1&&appeared==1,"one helper then one appear");require(std::memcmp(&dest.mBaseMtx,&writeValue,48)==0,"canonical matrix copy");retained(dest,before,offsetof(BellyDragonFireGround,mBaseMtx),48);require(snapshot(source)==src,"host retained");}
 for(int no=0;no<2;++no){source.mIsLv2=no!=0;available=nullptr;helperCalls=appeared=0;auto before=snapshot(dest);pod.appearFireGround(0,&vec);require(!helperCalls&&!appeared&&snapshot(dest)==before,"level/dead-actor guard retains storage");}
 std::printf("PASS: %u fire typed storage checks\n",checks);
}
