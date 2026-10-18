#pragma once

#include "JSystem/JGeometry/TVec.hpp"

class ActorCameraInfo;
class NameObj;
class LiveActor;

namespace MR {
    const char* getCurrentDemoPartName();
    void registerDemoSimpleCastAll(NameObj*);
    void registerDemoSimpleCastAll(LiveActor*);
    bool tryStartDemo(NameObj*, const char*);
    bool tryStartDemo(LiveActor*, const char*);
    bool isDemoEndWaitCameraInterpolating();
    bool tryEndDemoAndKillEffects(LiveActor*, const char*);
    bool tryEndDemoAndAnimCamera(LiveActor*, ActorCameraInfo*, const char*, s32);
    bool tryStartDemoAndAnimCamera(LiveActor*, const ActorCameraInfo*, const char*, s32);  // fn_80020260
    bool endDemoAndAnimCameraIfActionEnd(LiveActor*, const ActorCameraInfo*, const char*, s32);  // fn_800202E0
    bool tryEndDemo(LiveActor*, const char*, bool);
    void endDemo(NameObj*, const char*);
    bool isDemoActive();
};  // namespace MR

// Merged from Petari (core lane): declarations missing here
#include <revolution/types.h>
#include "Game/Util/Functor.hpp"
class JMapInfoIter;
class LayoutActor;
class Nerve;
class NerveExecutor;
class TalkMessageCtrl;

namespace MR {
    bool tryRegisterDemoCast(LiveActor*, const JMapInfoIter&);
    void registerDemoActionFunctor(const LiveActor*, const MR::FunctorBase&, const char*);
    void registerDemoActionNerve(const LiveActor*, const Nerve*, const char*);
    bool tryRegisterDemoActionFunctor(const LiveActor*, const MR::FunctorBase&, const char*);
    bool tryRegisterDemoActionNerve(const LiveActor*, const Nerve*, const char*);
    void registerDemoCast(LiveActor*, const char*, const JMapInfoIter&);
    void initTalkAnimDemoCast(LiveActor*, const JMapInfoIter&, const char*, const char*);
    void registerDemoActionFunctorDirect(const LiveActor*, const MR::FunctorBase&, const char*, const char*);
    void registerDemoActionNerveDirect(const LiveActor*, const Nerve*, const char*, const char*);
    bool tryRegisterDemoActionNerveDirect(const LiveActor*, const Nerve*, const char*, const char*);
    bool tryStartDemoRegistered(LiveActor*, const char*);
    bool tryStartDemoRegisteredMarioPuppetable(LiveActor*, const char*);
    bool isDemoCast(const LiveActor*, const char*);
    bool isRegisteredDemoActionAppear(const LiveActor*);
    bool isRegisteredDemoActionNerve(const LiveActor*);
    bool tryStartDemoWithoutCinemaFrame(NameObj*, const char*);
    bool tryStartDemoWithoutCinemaFrame(LiveActor*, const char*);
    bool tryStartDemoWithoutCinemaFrameValidStarPointer(LiveActor*, const char*);
    bool tryStartDemoWithoutCinemaFrameValidHandPointerFinger(NameObj*, const char*);
    bool tryStartDemoMarioPuppetable(LiveActor*, const char*);
    bool tryStartDemoMarioPuppetableWithoutCinemaFrame(NameObj*, const char*);
    bool tryStartDemoMarioPuppetableWithoutCinemaFrame(LiveActor*, const char*);
    bool tryStartTimeKeepDemo(NameObj*, const char*, const char*);
    bool tryStartTimeKeepDemoMarioPuppetable(NameObj*, const char*, const char*);
    bool requestStartDemo(LiveActor*, const char*, const Nerve*, const Nerve*);
    bool requestStartDemoWithoutCinemaFrame(LiveActor*, const char*, const Nerve*, const Nerve*);
    bool requestStartDemoMarioPuppetable(LiveActor*, const char*, const Nerve*, const Nerve*);
    bool requestStartDemoMarioPuppetableWithoutCinemaFrame(LiveActor*, const char*, const Nerve*, const Nerve*);
    bool requestStartTimeKeepDemo(LiveActor*, const char*, const Nerve*, const Nerve*, const char*);
    bool requestStartTimeKeepDemoMarioPuppetable(LiveActor*, const char*, const Nerve*, const Nerve*, const char*);
    bool requestStartTimeKeepDemoWithoutCinemaFrame(LiveActor*, const char*, const Nerve*, const Nerve*, const char*);
    bool requestStartDemoRegistered(LiveActor*, const Nerve*, const Nerve*, const char*);
    bool requestStartDemoRegisteredMarioPuppetable(LiveActor*, const Nerve*, const Nerve*, const char*);
    void endDemoWaitCameraInterpolating(NameObj*, const char*);
    void initDemoSheetTalkAnim(LiveActor*, const JMapInfoIter&, const char*, const char*, TalkMessageCtrl*);
    void initDemoSheetTalkAnim(LiveActor*, const JMapInfoIter&, const char*, const char*, TalkMessageCtrl*, TVec3f, MtxPtr);
    void initDemoSheetTalkAnimWithMessageName(LiveActor*, const JMapInfoIter&, const char*, const char*, const char*);
    void initDemoSheetTalkAnimFunctor(LiveActor*, const JMapInfoIter&, const char*, const char*, TalkMessageCtrl*, const MR::FunctorBase&);
    bool tryInitDemoSheetTalkAnim(LiveActor*, const JMapInfoIter&, const char*, const char*, TalkMessageCtrl*);
    bool tryInitDemoSheetTalkAnim(LiveActor*, const JMapInfoIter&, const char*, const char*, TalkMessageCtrl*, TVec3f, MtxPtr);
    void startTimeKeepDemo(NameObj*, const char*, const char*);
    void startTimeKeepDemoMarioPuppetable(NameObj*, const char*, const char*);
    bool isDemoExist(const char*);
    bool canStartDemo();
    bool isTimeKeepDemoActive();
    bool isDemoPartWaitUserInputActive();
    bool isDemoActiveRegistered(const LiveActor*);
    bool isDemoPartExist(const LiveActor*, const char*);
    bool isDemoLastStep();
    bool isDemoPartActive(const char*);
    bool isDemoPartStep(const char*, s32);
    bool isDemoPartFirstStep(const char*);
    bool isDemoPartLastStep(const char*);
    bool isDemoPartLessEqualStep(const char*, s32);
    bool isDemoPartGreaterStep(const char*, s32);
    s32 getDemoPartTotalStep(const char*);
    f32 calcDemoPartStepRate(const char*);
    s32 getDemoPartStep(const char*);
    void pauseTimeKeepDemo(LiveActor*);
    void resumeTimeKeepDemo(LiveActor*);
    bool isPauseTimeKeepDemo(LiveActor*);
    bool isPowerStarGetDemoActive();
    const char* getCurrentDemoPartNameMain(const char*);
    void startTalkingSequenceWithoutCinemaFrame(NameObj*);
    void endTalkingSequence(NameObj*);
    bool isSystemTalking();
    bool isNormalTalking();
    LiveActor* getTalkingActor();
    bool isDemoPartTalk(const char*);
    void startTalkingSequence(NameObj*);
};  // namespace MR

// Merged from Petari (core lane): overloads missing here


namespace MR {
    bool tryRegisterDemoCast(LiveActor*, const char*, const JMapInfoIter&);
    void registerDemoSimpleCastAll(LayoutActor*);
    bool tryStartDemo(LayoutActor*, const char*);
    bool tryStartTimeKeepDemoMarioPuppetable(LiveActor*, const char*, const char*);
    bool requestStartDemoWithoutCinemaFrame(LayoutActor*, const char*, const Nerve*, const Nerve*);
    bool requestStartDemoMarioPuppetable(NerveExecutor*, LiveActor*, const char*, const Nerve*, const Nerve*);
    bool requestStartTimeKeepDemo(NameObj*, const char*, const char*);
    bool requestStartTimeKeepDemoMarioPuppetable(NameObj*, const char*, const char*);
    bool isDemoActive(const char*);
};  // namespace MR

namespace MR {
    bool tryInitDemoSheetAnim(LiveActor*, const JMapInfoIter&, const char*, const char*);
    bool isStoryBookDemoActive();
}

namespace MR {
    // PLACEHOLDER(core4): names guessed from the DemoStartRequestUtil::startDemoSystem flags (type, cinema, pointer).
    bool tryStartTimeKeepDemoMarioPuppetableWithoutCinemaFrame(LiveActor*, const char*);  // SMG2 fn_8001FDD0
    bool requestStartDemo(NerveExecutor*, LiveActor*, const char*, const Nerve*, const Nerve*);  // SMG2 fn_8001FEF0
    bool requestStartDemo(NerveExecutor*, NameObj*, const char*, const Nerve*, const Nerve*);  // SMG2 fn_8001FEB0
    bool requestStartDemoMarioPuppetableWithoutCinemaFrame(NerveExecutor*, LiveActor*, const char*, const Nerve*, const Nerve*);  // SMG2 fn_80020090
    bool requestStartTimeKeepDemoMarioPuppetableWithoutCinemaFrame(NameObj*, const char*, const char*);  // SMG2 fn_80020110
    bool endDemoAndSetAlreadyDone(LiveActor*, const char*);  // SMG2 fn_800204E0; PLACEHOLDER(core4): name guessed from behaviour
    bool tryEndDemoAndSetAlreadyDone(LiveActor*, const char*);  // SMG2 fn_800205B0; PLACEHOLDER(core4): name guessed, called from DemoExecutor movement
    bool tryStartDemoEndWaitCameraInterpolating(LiveActor*, const char*);  // SMG2 fn_80020690; PLACEHOLDER(core4): name guessed (sets the director byte 0x13)
    bool isDemoEndWaitCinemaFrameStopped();  // SMG2 fn_80020800; PLACEHOLDER(core4): name guessed
    void initDemoSheetAnim(LiveActor*, const JMapInfoIter&, const char*, const char*);  // SMG2 fn_80020860; PLACEHOLDER(core4): name guessed
    bool isPauseTimeKeepDemo(const char*);  // SMG2 fn_80021120
    bool isDemoSubPartActive(const char*);  // SMG2 fn_80021100; PLACEHOLDER(core4): name guessed
    void resetDemoSubPart();  // SMG2 fn_80021110; PLACEHOLDER(core4): name guessed
    bool tryStartDemoSystemFlags111(NameObj*, const char*);              // SMG2 fn_8001FB40
    bool tryStartDemoMarioPuppetableSystemFlags(NameObj*, const char*);  // SMG2 fn_8001FD30
    bool requestStartDemoMarioPuppetableWithoutCinemaFrame(NameObj*, const char*);  // SMG2 fn_80020030
}

namespace MR { bool isDemoSubPartLastStep(); }
