#include "Game/Player/InfernoMarioGenerator.hpp"
#include "Game/Player/InfernoMario.hpp"
#include "Game/Player/InfernoMarioManager.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/ActorStateDemoAppearance.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/Functor.hpp"
namespace NrvInfernoMarioGenerator {
    NEW_NERVE(InfernoMarioGeneratorNrvWait,InfernoMarioGenerator,Wait);
    NEW_NERVE(InfernoMarioGeneratorNrvPrepare,InfernoMarioGenerator,Prepare);
    NEW_NERVE(InfernoMarioGeneratorNrvDemo,InfernoMarioGenerator,Demo);
    NEW_NERVE(InfernoMarioGeneratorNrvGenerate,InfernoMarioGenerator,Generate);
    NEW_NERVE(InfernoMarioGeneratorNrvEnd,InfernoMarioGenerator,End);
}
InfernoMarioGenerator::InfernoMarioGenerator(const char* name) : LiveActor(name),mGroup(nullptr),mGenerated(0),mCount(40),mSpacing(90),mFrame(0),mAlreadyDoneFlag(-1),mAlreadyDone(false) { MR::createInfernoMarioManager(); }
InfernoMarioGenerator::~InfernoMarioGenerator() {}
InfernoMarioGroup::~InfernoMarioGroup() {}
InfernoMario* InfernoMarioGroup::getDeadMember() const { if(getDeadActor()) return static_cast<InfernoMario*>(getDeadActor()); return nullptr; }
void InfernoMarioGenerator::makeActorAppeared() { if(!isNerve(&NrvInfernoMarioGenerator::InfernoMarioGeneratorNrvEnd::sInstance)) { LiveActor::makeActorAppeared();mGenerated=0; } }
void InfernoMarioGenerator::makeActorDead() { LiveActor::makeActorDead();mGroup->killAll();MR::deleteEffectAll(this); }
void InfernoMarioGenerator::init(const JMapInfoIter& iter) {
    MR::initDefaultPos(this,iter);
    MR::connectToSceneEnemyMovement(this);
    initEffectKeeper(0,"InfernoMario",false);
    initSound(3,"InfernoMarioGenerator",nullptr,TVec3f(0.0f));
    MR::getJMapInfoArg1NoInit(iter,&mCount);
    MR::getJMapInfoArg3NoInit(iter,&mSpacing);
    mGroup=new InfernoMarioGroup("インフェルノマリオ管理",mCount);
    for(s32 i=0;i<mCount;i++) {
        InfernoMario* actor=new InfernoMario("インフェルノマリオ");
        if(i==0) actor->mPlayExtraSound=true;
        actor->mFrameSpacing=mSpacing;
        MR::setPosition(actor,mPosition);
        actor->initWithoutIter();
        mGroup->registerActor(actor);
    }
    initNerve(&NrvInfernoMarioGenerator::InfernoMarioGeneratorNrvGenerate::sInstance,1);
    ActorStateDemoAppearance* state=new ActorStateDemoAppearance(this,iter,nullptr);
    state->_1C=0x8C;
    MR::initActorState(this,state,&NrvInfernoMarioGenerator::InfernoMarioGeneratorNrvDemo::sInstance,nullptr);
    MR::invalidateClipping(this);
    mAlreadyDone=MR::setupAlreadyDoneFlag(&mAlreadyDoneFlag,"インフェルノ登場デモ再生済み",iter,1);
    if(!state->mIsInitCamera) mAlreadyDone=true;
    if(MR::useStageSwitchSyncAppear(this,iter)) setNerve(&NrvInfernoMarioGenerator::InfernoMarioGeneratorNrvPrepare::sInstance);
    else setNerve(&NrvInfernoMarioGenerator::InfernoMarioGeneratorNrvWait::sInstance);
    if(MR::initUseStageSwitchReadB(this,iter)) {
        MR::FunctorV0M<InfernoMarioGenerator*, void (InfernoMarioGenerator::*)()> functor(this,&InfernoMarioGenerator::end);
        MR::listenStageSwitchOnB(this,functor);
    }
}
void InfernoMarioGenerator::end() {
    setNerve(&NrvInfernoMarioGenerator::InfernoMarioGeneratorNrvEnd::sInstance);
    s32 count=mGroup->getObjNum();
    for(s32 i=0;i<count;i++) {
        LiveActor* actor=mGroup->getActor(i);
        if(!MR::isDead(actor)) { MR::emitItem(actor,"特別報酬");MR::startActionSound(actor,"Dead",-1,-1,-1); }
    }
    makeActorDead();
}
void InfernoMarioGenerator::control() {}
void InfernoMarioGenerator::exeWait() { if(MR::isNearPlayer(this,1500.0f)) setNerve(&NrvInfernoMarioGenerator::InfernoMarioGeneratorNrvPrepare::sInstance); }
void InfernoMarioGenerator::exePrepare() {
    if(MR::isFirstStep(this)) {
        if(mAlreadyDone) MR::emitEffect(this,"Appear");
        MR::resetPosition(this,*MR::getPlayerPos());
        MR::copyPlayerTR(this);
        mFrame=0;
    }
    MR::startActionSound(this,"Appear",-1,-1,-1);
    mFrame++;
    if(MR::isGreaterStep(this,mSpacing/2)) {
        if(mAlreadyDone) { setNerve(&NrvInfernoMarioGenerator::InfernoMarioGeneratorNrvGenerate::sInstance);return; }
        MR::emitEffect(this,"Appear");
        setNerve(&NrvInfernoMarioGenerator::InfernoMarioGeneratorNrvDemo::sInstance);
    }
}
void InfernoMarioGenerator::exeDemo() {
    if(MR::isStep(this,40)) {
        InfernoMario* actor=mGroup->getDeadMember();
        if(actor) {
            actor->fn_80381670();
            MR::copyTransRotate(this,actor);
            MR::resetPosition(actor);
            MR::requestMovementOn(actor);
            MR::startActionSound(actor,"AppearFromGenerator",-1,-1,-1);
            MR::startActionSound(actor,"BvGhostMarioAppear",-1,-1,-1);
        }
    }
    if(MR::updateActorStateAndNextNerve(this,&NrvInfernoMarioGenerator::InfernoMarioGeneratorNrvGenerate::sInstance)) MR::updateAlreadyDoneFlag(mAlreadyDoneFlag);
}
void InfernoMarioGenerator::exeGenerate() {
    MR::isFirstStep(this);
    if(MR::isIntervalStep(this,mSpacing)) {
        InfernoMario* actor=mGroup->getDeadMember();
        if(actor && mGenerated<mCount && MR::addInfernoMario(actor)) {
            actor->fn_803816C0(mFrame);
            mGenerated++;
            if(mGenerated>=mCount) { setNerve(&NrvInfernoMarioGenerator::InfernoMarioGeneratorNrvEnd::sInstance);return; }
        }
    }
    if(mGenerated<mGroup->getObjNum()) mFrame++;
}
void InfernoMarioGenerator::exeEnd() { if(MR::isFirstStep(this)) MR::deleteEffectAll(this); }
