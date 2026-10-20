#include "Game/MapObj/StarPieceScatter.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#ifdef GALAXY_NATIVE
#include "Game/Util/Color.hpp"
#endif
namespace NrvStarPieceScatter {
    NEW_NERVE(StarPieceScatterNrvScatter,StarPieceScatter,Scatter);
}
StarPieceScatter::StarPieceScatter(const char* name)
 :LiveActor(name),mColor((GXColor){0,0,0,255}),mUp(0.f,0.f,0.f){}
void StarPieceScatter::init(const JMapInfoIter& iter){
    u32 color=StarPieceDirector::getRandomColor();
#ifdef GALAXY_NATIVE
    mColor = static_cast<GXColor>(Color8(color));
#else
    mColor=*reinterpret_cast<const GXColor*>(&color);
#endif
    MR::initActor(this,iter,"StarPiece",true);
    MR::initDLMakerMatColor0(this,"StarPiece1_v",&mColor);
    MR::newDifferedDLBuffer(this);
    MR::connectToSceneNoSilhouettedMapObj(this);
    initEffectKeeper(0,"StarPiece",false);
    initSound(4,"StarPieceScatter",NULL,TVec3f(0.f));
    MR::onCalcGravity(this);
    MR::startBtk(this,"Gift");
    MR::setBtkFrameAndStop(this,5.f);
    initNerve(&NrvStarPieceScatter::StarPieceScatterNrvScatter::sInstance,0);
    makeActorDead();
}
// NONMATCHING: only vector temporary stack slots differ from the target.
void StarPieceScatter::scatter(const TVec3f& position,const TVec3f& up,f32 randomMagnitude,f32 upSpeed){
    TVec3f random(0.f);
    MR::getRandomVector(&random,randomMagnitude);
    random=random*30.f;
    MR::vecKillElement(random,up,&random);
    MR::normalizeOrZero(&mUp,up);
    mPosition=position;
    mVelocity.set(mUp*upSpeed+random);
    MR::setScale(this,8.f);
    MR::emitEffect(this,"WorldMapStarPieceLight");
    MR::invalidateClipping(this);
    setNerve(&NrvStarPieceScatter::StarPieceScatterNrvScatter::sInstance);
    MR::addStarPiece(-1);
    appear();
}
void StarPieceScatter::exeScatter(){
    MR::addVelocityMoveToDirection3D(this,TVec3f(0.f,-1.f,0.f),5.f);
    if(MR::isGreaterEqualStep(this,60))kill();
}
StarPieceScatter::~StarPieceScatter(){}
