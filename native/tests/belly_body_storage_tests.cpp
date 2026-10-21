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

BellyDragonBodyPart::BellyDragonBodyPart(s32,BellyDragonBodyPart*,bool,f32,f32,f32):LiveActor("part"){}
BellyDragonBodyPart::~BellyDragonBodyPart(){}
void BellyDragonBodyPart::init(const JMapInfoIter&){}
MtxPtr BellyDragonBodyPart::getBaseMtx()const{return const_cast<MtxPtr>(mMtx.mMtx);}
EMPTY0(BellyDragonBodyPart,control)
void BellyDragonBodyPart::attackSensor(HitSensor*,HitSensor*){}
bool BellyDragonBodyPart::receiveMsgPush(HitSensor*,HitSensor*){return false;}
bool BellyDragonBodyPart::receiveMsgPlayerAttack(u32,HitSensor*,HitSensor*){return false;}
// __PRODUCTION_SITE__
int main(){
 static_assert(offsetof(BellyDragonBodyPart,mMtx)==0xF0,"actual native matrix");static_assert(offsetof(LiveActor,mPosition)==0x1C,"actual native position");
 BellyDragonBodyPart dest(0,nullptr,false,1,2,3);TPos3f src;
 for(unsigned t=0;t<12;++t){matrix(src,t);matrix(dest.mMtx,t+1);dest.mPosition.set(4.0f,5.0f,6.0f);auto before=snapshot(dest);auto input=snapshot(src);BellyDragonUtil::setBodyPartMtx(&dest,src.mMtx);
 require(std::memcmp(&dest.mMtx,&src,sizeof(TPos3f))==0,"canonical matrix copy");const float expected[]={src.mMtx[0][3],src.mMtx[1][3],src.mMtx[2][3]};require(std::memcmp(&dest.mPosition,expected,12)==0,"translation from copied matrix in x y z order");retained(dest,before,offsetof(BellyDragonBodyPart,mMtx),48,offsetof(LiveActor,mPosition),12);require(snapshot(src)==input,"source matrix retained");
 // The actual operation also supports its own field as source; no arithmetic/reassociation.
 auto stable=snapshot(dest);BellyDragonUtil::setBodyPartMtx(&dest,dest.mMtx.mMtx);require(snapshot(dest)==stable,"self matrix source retained");}
 std::printf("PASS: %u body typed storage checks\n",checks);
}
