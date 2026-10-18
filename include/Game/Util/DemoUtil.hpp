#pragma once

#include "JSystem/JGeometry/TVec.hpp"

class ActorCameraInfo;
class NameObj;
class LiveActor;

namespace MR {
    const char* getCurrentDemoPartName();
    void registerDemoSimpleCastAll(NameObj*);
    void registerDemoSimpleCastAll(LiveActor*);
    bool tryStartDemo(LiveActor*, const char*);
    bool isDemoEndWaitCameraInterpolating();
    bool tryEndDemoAndKillEffects(LiveActor*, const char*);
    bool tryEndDemoAndAnimCamera(LiveActor*, ActorCameraInfo*, const char*, s32);
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
    bool tryRegisterDemoActionFunctorDirect(const LiveActor*, const MR::FunctorBase&, const char*, const char*);
    bool tryStartDemoRegistered(LiveActor*, const char*);
    bool tryStartDemoRegisteredMarioPuppetable(LiveActor*, const char*);
    bool isDemoCast(const LiveActor*, const char*);
    bool isRegisteredDemoActionAppear(const LiveActor*);
    bool isRegisteredDemoActionNerve(const LiveActor*);
    bool tryStartDemoWithoutCinemaFrame(LiveActor*, const char*);
    bool tryStartDemoWithoutCinemaFrameValidStarPointer(LiveActor*, const char*);
    bool tryStartDemoWithoutCinemaFrameValidHandPointerFinger(NameObj*, const char*);
    bool tryStartDemoMarioPuppetable(LiveActor*, const char*);
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
    void startTimeKeepDemo(NameObj*, const char*, const char*);
    void startTimeKeepDemoMarioPuppetable(NameObj*, const char*, const char*);
    bool isDemoExist(const char*);
    bool canStartDemo();
    bool isTimeKeepDemoActive();
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
