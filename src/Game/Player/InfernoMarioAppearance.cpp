#include "Game/Player/InfernoMarioAppearance.hpp"
#include "Game/Player/InfernoMario.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvInfernoMarioAppearance {
    NEW_NERVE(InfernoMarioAppearanceNrvJump,InfernoMarioAppearance,Jump);
}
InfernoMarioAppearance::InfernoMarioAppearance(InfernoMario* host) : ActorStateBase<InfernoMario>("インフェルノ登場演出",host),mStartPos(0.0f,0.0f,0.0f),mUp(0.0f,1.0f,0.0f) {
    initNerve(&NrvInfernoMarioAppearance::InfernoMarioAppearanceNrvJump::sInstance);
}
InfernoMarioAppearance::~InfernoMarioAppearance() {}
void InfernoMarioAppearance::appear() { mIsDead=false;setNerve(&NrvInfernoMarioAppearance::InfernoMarioAppearanceNrvJump::sInstance); }
void InfernoMarioAppearance::exeJump() {
    if(MR::isFirstStep(this)) {
        MR::startAction(mHost,"Jump");
        mStartPos.x=mHost->mPosition.x;
        mStartPos.y=mHost->mPosition.y;
        mStartPos.z=mHost->mPosition.z;
        MR::makeQuatFromRotate(&mHost->mCurrentQuat,mHost);
        const TQuat4f& q=mHost->mCurrentQuat;
        mUp.set<f32>(2.0f*(q.x*q.y)-2.0f*(q.w*q.z),(1.0f-2.0f*(q.x*q.x))-2.0f*(q.z*q.z),2.0f*(q.y*q.z)+2.0f*(q.w*q.x));
    }
    f32 height=MR::fn_80048480(this,60,0.0f,150.0f);
    TVec3f offset(mUp);
    offset *= height;
    TVec3f position(mStartPos);
    position += offset;
    MR::resetPosition(mHost,position);
    if(MR::isGreaterStep(this,60)) { MR::tryHitReaction(mHost,"登場終了");kill(); }
}
