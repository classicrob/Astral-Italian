#include "Game/MapObj/GravityTimerChangeWall.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/RhythmLib/AudRhythmTiming.hpp"
#include "Game/Util.hpp"
namespace NrvGravityTimerChangeWall {
NEW_NERVE(GravityTimerChangeWallNrvWait, GravityTimerChangeWall, Wait);
NEW_NERVE(GravityTimerChangeWallNrvUpToDown, GravityTimerChangeWall, UpToDown);
NEW_NERVE(GravityTimerChangeWallNrvDownToUp, GravityTimerChangeWall, DownToUp);
}
extern const f32 lbl_807DD6F0;
extern const f32 lbl_807E3098;
extern const s32 lbl_807E309C;
GravityTimerChangeWall::GravityTimerChangeWall(const char* name) : LiveActor(name), mWaitFrames(0), mIsChanging(false), mUseProjmap(false), mProjmap(0) {}
void GravityTimerChangeWall::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, true);
    initNerve(&NrvGravityTimerChangeWall::GravityTimerChangeWallNrvWait::sInstance);
    mWaitFrames = lbl_807E309C;
    const char* objName = 0;
    MR::getObjectName(&objName, iter);
    if (MR::isEqualString(objName, "GravityTimerChangeWall")) mUseProjmap = true;
    if (mUseProjmap) mProjmap = MR::initDLMakerProjmapEffectMtxSetter(this);
    MR::newDifferedDLBuffer(this);
    MR::tryStartAllAnim(this, objName);
    MR::initUseStageSwitchReadA(this, iter);
    if (MR::isValidSwitchA(this)) MR::listenStageSwitchOnA(this, MR::FunctorV0M<GravityTimerChangeWall*, void (GravityTimerChangeWall::*)()>(this, &GravityTimerChangeWall::listenSwitchOnA));
    makeActorAppeared();
}
void GravityTimerChangeWall::exeWait() {
    MR::startAction(this, "UpToDown");
    MR::setAllAnimFrameAtEnd(this, "UpToDown");
    setNerve(&NrvGravityTimerChangeWall::GravityTimerChangeWallNrvDownToUp::sInstance);
    if (mUseProjmap) mProjmap->updateMtxUseBaseMtx();
}
void GravityTimerChangeWall::exeUpToDown() {
    if (tryChange()) { mIsChanging = true; MR::startAction(this, "UpToDown"); }
    bool done = false;
    if (mIsChanging && MR::isBvaStopped(this)) done = true;
    if (done) { mIsChanging = false; setNerve(&NrvGravityTimerChangeWall::GravityTimerChangeWallNrvDownToUp::sInstance); }
}
void GravityTimerChangeWall::exeDownToUp() {
    if (tryChange()) { mIsChanging = true; MR::startAction(this, "DownToUp"); }
    bool done = false;
    if (mIsChanging && MR::isBvaStopped(this)) done = true;
    if (done) { mIsChanging = false; setNerve(&NrvGravityTimerChangeWall::GravityTimerChangeWallNrvUpToDown::sInstance); }
}
bool GravityTimerChangeWall::tryChange() { if (MR::isGreaterEqualStep(this, mWaitFrames)) return AudRhythmWrap::isBeatTiming(s32(lbl_807DD6F0), lbl_807E3098); return false; }
void GravityTimerChangeWall::listenSwitchOnA() { MR::startAction(this, "UpToDown"); MR::setAllAnimFrameAtEnd(this, "UpToDown"); setNerve(&NrvGravityTimerChangeWall::GravityTimerChangeWallNrvDownToUp::sInstance); }
GravityTimerChangeWall::~GravityTimerChangeWall() {}
