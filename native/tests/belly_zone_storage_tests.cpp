// Actual canonical LiveActor/NameObj/JMapLinkInfo storage; inert lifecycle fixtures.
// The extracted assignment is exercised, not the complete BellyDragon::init lifecycle.
#include "Game/Util/BellyDragon.hpp"
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
static void apply(LiveActor* mDemoB, const LiveActor* pSource) {
    const JMapLinkInfo& mLinkInfo = pSource->mLinkInfo;
    // __PRODUCTION_ASSIGNMENT__
}
int main(){
    static_assert(sizeof(void*)==8,"Native layout regression");
    static_assert(std::is_same<decltype(BellyDragon::mDemoB),LiveActor*>::value,"Actual receiver field type");
    static_assert(offsetof(NameObj,mLinkInfo)+offsetof(JMapLinkInfo,mZoneID)==0x16,"Rebind changed canonical zone offset");
    LiveActor source("source"),dest("destination");
    const s16 cases[]={-32768,-1,0,1,7,32767};
    for(s16 value:cases)for(s16 previous:cases){
        source.mLinkInfo.mFollowID=0x1234;source.mLinkInfo.mZoneID=value;source.mLinkInfo.mContext=-123;
        dest.mLinkInfo.mFollowID=-234;dest.mLinkInfo.mZoneID=previous;dest.mLinkInfo.mContext=0x2345;
        std::array<unsigned char,sizeof(LiveActor)> before,srcBefore;
        std::memcpy(before.data(),static_cast<const void*>(&dest),before.size());
        std::memcpy(srcBefore.data(),static_cast<const void*>(&source),srcBefore.size());
        apply(&dest,&source);
        require(dest.mLinkInfo.mZoneID==value,"canonical signed16 zone is copied; old raw14 addresses native name pointer");
        require(std::memcmp(static_cast<const void*>(&source),srcBefore.data(),srcBefore.size())==0,"source retained");
        const auto* bytes=reinterpret_cast<const unsigned char*>(&dest);
        const auto zone=offsetof(NameObj,mLinkInfo)+offsetof(JMapLinkInfo,mZoneID);
        for(std::size_t i=0;i<before.size();++i)if(i<zone||i>=zone+sizeof(s16))require(bytes[i]==before[i],"all non-zone destination bytes retained");
        apply(&dest,&source);require(dest.mLinkInfo.mZoneID==value,"repeat copy idempotent");
    }
    std::printf("PASS: %u canonical zone storage checks\n",checks);
}
