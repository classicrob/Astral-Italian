#include "Game/MapObj/RepeatBlock.hpp"
#include "Game/Util.hpp"
namespace NrvRepeatBlock {
    class RepeatBlockNrvTailWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static RepeatBlockNrvTailWait sInstance;
    };
}
RepeatBlockHead::RepeatBlockHead(const char* name,RepeatBlock* parent) : LiveActor(name),mParent(parent),mAtoms(nullptr) {
    MR::setPosition(this,parent->mPosition);
    MR::setRotation(this,parent->mRotation);
    MR::offCalcGravity(this);
    MR::makeMtxTR(mBaseMtx,mPosition,mRotation);
    f32 z=mBaseMtx.mMtx[2][1];
    f32 y=mBaseMtx.mMtx[1][1];
    f32 x=mBaseMtx.mMtx[0][1];
    mGravity.set<f32>(x,y,z);
    mGravity.negate();
    mAtoms=new RepeatBlockAtom*[parent->getBlockNum()];
    for (s32 i=0;i<parent->getBlockNum();i++) {
        mAtoms[i]=new RepeatBlockAtom(parent);
        mAtoms[i]->makeActorDead();
    }
    updateAtomPositions();
}
RepeatBlockHead::~RepeatBlockHead() {}
void RepeatBlockHead::appearWithAtoms() {
    LiveActor::appear();
    for (s32 i=0;i<mParent->getBlockNum();i++) mAtoms[i]->appear();
}
void RepeatBlockHead::kill() {
    LiveActor::kill();
    for (s32 i=0;i<mParent->getBlockNum();i++) mAtoms[i]->kill();
}
void RepeatBlockHead::updateAtomPositions() {
    TVec3f center;
    mParent->mInvMtx.mult(mPosition,center);
    TVec3f pos;
    s32 index=0;
    pos.x=center.x + 0.5f * (200.0f * -(mParent->mNumX-1));
    for (s32 x=0;x<mParent->mNumX;x++,pos.x+=200.0f) {
        pos.y=center.y + 0.5f * (200.0f * -(mParent->mNumY-1));
        for (s32 y=0;y<mParent->mNumY;y++,pos.y+=200.0f) {
            pos.z=center.z + 0.5f * (200.0f * -(mParent->mNumZ-1));
            for (s32 z=0;z<mParent->mNumZ;) {
                mParent->mMtx.mult(pos,mAtoms[index]->mPosition);
                z++;
                index++;
                pos.z+=200.0f;
            }
        }
    }
    MR::makeMtxTR(mBaseMtx,mPosition,mRotation);
}
RepeatBlockTail::RepeatBlockTail(RepeatBlock* parent,const char* name) : RepeatBlockHead(name,parent),mAppearDir(0),mMoveDir(0),mAppearProgress(0.0f),mAppearStep(0) {
    initNerve(&NrvRepeatBlock::RepeatBlockNrvTailWait::sInstance);
}
