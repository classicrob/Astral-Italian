#define JGEOMETRY_VEC3_WMR_OUTLINE
#define JGEOMETRY_VEC3_SET_NOINLINE
#include "Game/Player/Yoshi.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/Player/YoshiArrow.hpp"
#include "Game/Player/YoshiTongue.hpp"
#include "Game/Player/YoshiTongueAction.hpp"
#include "Game/Player/YoshiEgg.hpp"
#include "Game/Player/YoshiFruitCounter.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Player/YoshiHolder.hpp"
#include "Game/Player/YoshiTongueTarget.hpp"
#include "Game/Player/YoshiLockOnManager.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util.hpp"

// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
YoshiTongueTarget::YoshiTongueTarget(YoshiLockOnManager* manager)
    : mCapacity(0), mCount(0), mIndex(0), mTargets(nullptr), mManager(manager), mSensor(nullptr) {
    mCapacity = manager->getTargetCapacity();
    mTargets = new YoshiLockOnTargetDesc[mCapacity];
}
void YoshiTongueTarget::takeLockOnTargets() { mCount=mManager->takeActiveTargets(mTargets,mCapacity);mIndex=0; }
bool YoshiTongueTarget::sendStartTongueMessage() {return sendMessageToCurrentTarget(0xF8);}
bool YoshiTongueTarget::sendLoopTongueMessage() {return sendMessageToCurrentTarget(0xF9);}
void YoshiTongueTarget::sendStartEatenMessage() {sendMessageToCurrentTarget(0xFB);}
void YoshiTongueTarget::sendReleaseEatenMessage() {sendMessageToCurrentTarget(0xFA);}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiTongueTarget::advanceTargetIfMore() {if(mIndex<mCount-1)++mIndex;}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiTongueTarget::cancelTargetAndAdvance() {sendMessageToCurrentTarget(0xFC);++mIndex;}
bool YoshiTongueTarget::hasMultipleRemainingTargets()const{return mCount-mIndex>1;}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
bool YoshiTongueTarget::hasCurrentTarget()const{return mIndex<mCount;}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
bool YoshiTongueTarget::hasNextTarget()const{return mCount-1>mIndex;}
bool YoshiTongueTarget::fn_8041C120()const{return mTargets[mIndex].mType==YoshiLockOnTargetType::Eat;}
bool YoshiTongueTarget::fn_8041C140()const{return mTargets[mIndex].mType==YoshiLockOnTargetType::Lick;}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
bool YoshiTongueTarget::fn_8041C170()const{return mTargets[mIndex].mType==YoshiLockOnTargetType::Capture||mTargets[mIndex].mType==YoshiLockOnTargetType::CaptureAlternate;}
bool YoshiTongueTarget::fn_8041C1B0()const{return mTargets[mIndex].mType==YoshiLockOnTargetType::Capture;}
bool YoshiTongueTarget::fn_8041C1E0()const{return mTargets[mIndex].mType==YoshiLockOnTargetType::Pull;}
bool YoshiTongueTarget::fn_8041C210()const{return mTargets[mIndex].mType==YoshiLockOnTargetType::Projectile;}
void YoshiTongueTarget::calcCurrentTargetPosition(TVec3f* out)const {
    MtxPtr source=getCurrentTargetMatrix();
    if(!source)out->set<f32>(mTargets[mIndex].mActor->mPosition);
    else out->set<f32>(source[0][3],source[1][3],source[2][3]);
    TVec3f x,y,z;
    if(source){x.set<f32>(source[0][0],source[1][0],source[2][0]);y.set<f32>(source[0][1],source[1][1],source[2][1]);z.set<f32>(source[0][2],source[1][2],source[2][2]);}
    else if(mTargets[mIndex].mActor->getBaseMtx()){
        TPos3f matrix;matrix.set(mTargets[mIndex].mActor->getBaseMtx());matrix.getXDir(x);matrix.getYDir(y);matrix.getZDir(z);
    }else{TPos3f matrix;MR::makeMtxTR(matrix.toMtxPtr(),mTargets[mIndex].mActor);matrix.getXDir(x);matrix.getYDir(y);matrix.getZDir(z);}
    const TVec3f& offset = mTargets[mIndex].mOffset;
    x.scale(offset.x);y.scale(offset.y);z.scale(offset.z);out->add(x+y+z);
}
void YoshiTongueTarget::fn_8041C440(TVec3f* out)const {
    MtxPtr matrix=mTargets[mIndex].mActor->getBaseMtx();out->set<f32>(matrix[0][2],matrix[1][2],matrix[2][2]);
}
LiveActor* YoshiTongueTarget::getCurrentTargetActor()const{return mTargets[mIndex].mActor;}
MtxPtr YoshiTongueTarget::getCurrentTargetMatrix()const{return mTargets[mIndex].mMtx;}
bool YoshiTongueTarget::sendMessageToCurrentTarget(u32 message){
    LiveActor* actor = mTargets[mIndex].mActor;
    HitSensor* sensor = MR::getMessageSensor();
    return actor->receiveMessage(message,mSensor,sensor);
}

YoshiLockOnTargetDesc::YoshiLockOnTargetDesc():mActor(nullptr),mMtx(nullptr){
    mOffset.x = 0.0f;
    mOffset.y = 0.0f;
    mOffset.z = 0.0f;
    mType = 0;
}
void* YoshiHolder::fn_8041B6F0(){return mYoshi->mTongue;}
// NONMATCHING: index and array pointer registers are swapped.
void YoshiHolder::fn_8041B820(LiveActor* egg){
    YoshiEgg** eggs;
    s32 index = mCount;
    eggs = mEggs;
    mCount = index + 1;
    eggs[index] = static_cast<YoshiEgg*>(egg);
}
void YoshiHolder::fn_8041B840(LiveActor* egg){_34=false;for(s32 i=0;i<mCount;++i)if(mEggs[i]!=egg)mEggs[i]->fn_80416FC0();}
void YoshiHolder::fn_8041B8B0(LiveActor* egg){_34=true;for(s32 i=0;i<mCount;++i)if(mEggs[i]!=egg)mEggs[i]->fn_80416F60();}
void YoshiHolder::fn_8041B920(){if(mCurrentEgg){mCurrentEgg->fn_80416D30();mCurrentEgg=nullptr;}}
void YoshiHolder::fn_8041B970(){if(mCurrentEgg){mCurrentEgg->fn_80416E60();mCurrentEgg=nullptr;}}
void YoshiHolder::fn_8041B9C0(){if(mCurrentEgg){mCurrentEgg->fn_80416EE0();mCurrentEgg=nullptr;}}
void YoshiHolder::fn_8041BA10(){mFruitCounter->increment();}
void YoshiHolder::fn_8041BAE0(){mCountdown=60;}
void YoshiHolder::fn_8041BAF0(){mCountdown=0;}
bool YoshiHolder::fn_8041BB30()const{return mCountdown>0;}
f32 YoshiHolder::fn_8041BB50()const{return mCountdown/60.0f;}
void YoshiHolder::fn_8041BB90(){mTimer=38;}
void YoshiHolder::fn_8041BBA0(){mTimer=0;}
void YoshiHolder::fn_8041BBB0(){if(mTimer>0)--mTimer;}
bool YoshiHolder::fn_8041BBD0()const{return mTimer>0;}
void YoshiHolder::fn_8041BD30(){mHistory->fn_80415DA0();}
namespace YoshiParam {
s32 fn_8041BD40(){return 0;}s32 fn_8041BD50(){return 1;}s32 fn_8041BD60(){return 1;}s32 fn_8041BD70(){return 0;}
f32 fn_8041BD80(){return 26.0f;}f32 fn_8041BD90(){return 3.5f;}f32 fn_8041BDA0(){return 13.0f;}f32 fn_8041BDB0(){return .65f;}
f32 fn_8041BDC0(){return 60.0f;}f32 fn_8041BDD0(){return 40.0f;}f32 fn_8041BDE0(){return .999f;}s32 fn_8041BDF0(){return 30;}
// NONMATCHING: gravity temporary address lifetime, stack slots and registers differ.
void fn_8041BE00(TVec3f* out){const TVec3f& gravity=*MR::getPlayerGravity();TVec3f gravityPart=gravity*40.0f;const TVec3f& velocity=*MR::getPlayerVelocity();TVec3f velocityPart=velocity*2.0f;TVec3f result(velocityPart);result.sub(gravityPart);out->x=result.x;out->y=result.y;out->z=result.z;}
}
namespace MR {bool isPlayerInRushOrDemoOrTalk(){return isPlayerInRush()||isDemoActive()||isSystemTalking();}}
namespace {s32 sAppearValue=1;}
YoshiHolder::YoshiHolder(Mario* mario)
    :mYoshi(nullptr),mTongueAction(nullptr),_C(nullptr),_10(nullptr),mArrow(nullptr),mRideModelActor(nullptr),mHistory(nullptr),mEggs(nullptr),mCapacity(0),mCount(0),mCurrentEgg(nullptr),_34(true),_35(false),_38(0),_3C(0),mCountdown(0),mTimer(0),_48(0),mMario(mario){
    mYoshi=new Yoshi("ヨッシー");mYoshi->initWithoutIter();
    _C=new YoshiLockOnManager(mYoshi);_C->initWithoutIter();
    _10=new YoshiTongueTarget(_C);
    mArrow=new YoshiArrow("ヨッシーアロー");mArrow->initWithoutIter();
    mTongueAction=new YoshiTongueAction(mYoshi,mYoshi->mTongue,mario,_10,mArrow);
    mEggs=new YoshiEgg*[16];mCapacity=16;mHistory=new YoshiHistory15C50();mFruitCounter=new YoshiFruitCounter();
}
// NONMATCHING: Equivalent inline helpers, register allocation, or instruction scheduling.
void YoshiHolder::fn_8041B700(LiveActor* actor){
    if(MR::isCreatePlayerResourceYoshiBalloon()){
        mRideModelActor=new PartsModel(actor,"ヨッシーバルーン","YoshiBalloon",nullptr,0x16,true);
        MR::initDLMakerFog(mRideModelActor,true);MR::newDifferedDLBuffer(mRideModelActor);
        if(MR::getLightNumMax(mRideModelActor)>0)MR::initLightCtrl(mRideModelActor);
        mRideModelActor->initWithoutIter();mRideModelActor->kill();
        static_cast<PartsModel*>(mRideModelActor)->initFixedPosition(TVec3f(0.0f,0.0f,0.0f),TVec3f(0.0f,0.0f,0.0f),nullptr);
        MR::initShadowFromCSVWithInitShadowVolumeSphere(mRideModelActor,"Shadow");MR::offCalcAnim(mRideModelActor);MR::onCalcGravity(mRideModelActor);
    }
}
void YoshiHolder::fn_8041BB00(){
    if (mCountdown > 0) --mCountdown;
    MR::startActionSound(mYoshi,"Flutter",-1,-1,-1);
}
void YoshiHolder::fn_8041BA20(MtxPtr matrix,LiveActor* egg){
    mCurrentEgg=static_cast<YoshiEgg*>(egg);
    Yoshi* yoshi = mYoshi;
    MR::setPosition(yoshi,TVec3f(matrix[0][3],matrix[1][3],matrix[2][3]));
    MR::setPosition(mYoshi->mTongue,mYoshi->mPosition);
    mYoshi->setTransformFromMtx(matrix);
    mYoshi->appear();mYoshi->mTongue->appear();MR::showModel(mYoshi);MR::showModel(mYoshi->mTongue);_38=sAppearValue;
}
// NONMATCHING: only verified scale literal relocation labels differ.
void YoshiHolder::fn_8041BBF0(const TVec3f& position,const TVec3f& up,const TVec3f& front){
    TVec3f x,y;PSVECCrossProduct(&front,&up,&x);
    if(MR::normalizeOrZero(&x)){TVec3f fallback(front.z,front.x,front.y);PSVECCrossProduct(&front,&fallback,&x);MR::normalize(&x);}
    PSVECCrossProduct(&x,&front,&y);MR::normalizeOrZero(&y);
    TPos3f matrix;matrix.setTrans(position);matrix.setXDir(x*2.0f);matrix.setYDir(front*2.0f);matrix.setZDir(y*2.0f);
    mHistory->fn_80415CF0(matrix.toMtxPtr());
}

