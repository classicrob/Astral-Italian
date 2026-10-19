#define JGEOMETRY_VEC3_SET_NOINLINE
#include "Game/Util/DemoUtil.hpp"
#include "Game/Demo/PrepareKoopaFinalDemoObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Demo/StoryBookDemo.hpp"
#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/ActorMovementUtil.hpp"

// Data table (demo object params, 0x18 bytes of names per object) and names.
extern "C" f32 lbl_807D9810;
extern "C" f32 lbl_807D9814;
extern "C" f32 lbl_807D9818;
extern "C" f32 lbl_807D981C;
extern "C" f32 lbl_807D9820;
extern "C" f32 lbl_807D9824;
extern "C" f32 lbl_807D9828;
extern "C" f32 lbl_807D982C;


namespace NrvPrepareKoopaFinalDemoObj {
    class PrepareKoopaFinalDemoObjNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static PrepareKoopaFinalDemoObjNrvWait sInstance;
    };
    class PrepareKoopaFinalDemoObjNrvStopShakeEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static PrepareKoopaFinalDemoObjNrvStopShakeEnd sInstance;
    };
    class PrepareKoopaFinalDemoObjNrvShake : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static PrepareKoopaFinalDemoObjNrvShake sInstance;
    };
    class PrepareKoopaFinalDemoObjNrvStartPuppet : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static PrepareKoopaFinalDemoObjNrvStartPuppet sInstance;
    };
    class PrepareKoopaFinalDemoObjNrvEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static PrepareKoopaFinalDemoObjNrvEnd sInstance;
    };
};  // namespace NrvPrepareKoopaFinalDemoObj

PrepareKoopaFinalDemoObj::PrepareKoopaFinalDemoObj(const char* pName) : LiveActor(pName), mKoopaSign(nullptr), mTalkActor(nullptr), mFinalKoopa(nullptr) {
}

// NONMATCHING: functor/register scheduling
void PrepareKoopaFinalDemoObj::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    if (MR::tryRegisterDemoCast(this, rIter)) {
        MR::FunctorV0M< PrepareKoopaFinalDemoObj*, void (PrepareKoopaFinalDemoObj::*)() > functor(this, &PrepareKoopaFinalDemoObj::requestWorldMapAfterWipe);
        MR::registerDemoActionFunctor(this, functor, nullptr);
    }
    mKoopaSign = MR::createDemoObjWithAnimArc("マイスター", "Meister", "MeisterAnim", "Demo", nullptr);
    MR::tryInitDemoSheetAnim(mKoopaSign, rIter, "クッパ最終戦突入", "DemoKoopaCastle");
    MR::invalidateClipping(mKoopaSign);
    mKoopaSign->makeActorAppeared();
    mTalkActor = MR::createDemoObjWithAnimArc("ベビチコ", "TicoBaby", "TicoBabyAnim", nullptr);
    MR::initTalkAnimDemoCast(mTalkActor, rIter, "クッパ最終戦突入", "DemoKoopaCastle");
    MR::invalidateClipping(mTalkActor);
    mTalkActor->makeActorAppeared();
    mFinalKoopa = MR::createDemoObj("クッパ城", "MiniVsKoopaLv3Galaxy", "Demo", nullptr);
    MR::invalidateClipping(mFinalKoopa);
    mFinalKoopa->makeActorAppeared();
    initNerve(GET_NERVE(PrepareKoopaFinalDemoObj, PrepareKoopaFinalDemoObjNrvWait), 0);
    makeActorAppeared();
}

void PrepareKoopaFinalDemoObj::exeStopShakeEnd() {
    bool shake = false;
    if (MR::isStep(this, 360) || (MR::isGreaterEqualStep(this, 240) && MR::getRandom(s32(0), s32(10)) == 0)) shake = true;
    if (shake) setNerve(GET_NERVE(PrepareKoopaFinalDemoObj, PrepareKoopaFinalDemoObjNrvShake));
}

void PrepareKoopaFinalDemoObj::exeShake() {
    if (MR::isFirstStep(this)) {
        MR::shakeCameraInfinity(this, lbl_807D981C, lbl_807D9810);
        MR::tryRumblePadMiddle(this, 0);
        return;
    }
    if (MR::isIntervalStep(this, 0x14)) {
        MR::tryRumblePadWeak(this, 0);
    }
    if (MR::isIntervalStep(this, 0x64)) {
        MR::tryRumblePadMiddle(this, 0);
        MR::shakeCameraWeak();
    }
    if (MR::isStep(this, 0xB4)) {
        MR::stopShakingCamera(this);
        setNerve(GET_NERVE(PrepareKoopaFinalDemoObj, PrepareKoopaFinalDemoObjNrvStopShakeEnd));
    }
}

void PrepareKoopaFinalDemoObj::exeStopShake() {
    if (MR::isFirstStep(this)) {
        MR::stopShakingCamera(this);
    }
    if (MR::isStoryBookTurnPageEnd() != 0) {
        setNerve(GET_NERVE(PrepareKoopaFinalDemoObj, PrepareKoopaFinalDemoObjNrvShake));
    }
}

// NONMATCHING: FP/register load scheduling
void PrepareKoopaFinalDemoObj::initAfterPlacement() {
    MR::fn_80011F80(this, lbl_807D9818, lbl_807D9818);
    mKoopaSign->mPosition.set(mPosition);
    mTalkActor->mPosition.set(mPosition);
    JGeometry::TMatrix34< JGeometry::SMatrix34C< f32 > > mtx;
    mtx.identity();
    MR::makeMtxRotate(mtx.mMtx, mRotation);
    TVec3f offset;
    offset.x = lbl_807D9814;
    offset.y = lbl_807D9814;
    offset.z = lbl_807D9810;
    offset.set<f32>(mtx.mMtx[0][2], mtx.mMtx[1][2], mtx.mMtx[2][2]);
    JMAVECScaleAdd(&offset, &mPosition, &mFinalKoopa->mPosition, lbl_807D9820);
    mFinalKoopa->mPosition.y += lbl_807D9824;
    mFinalKoopa->mRotation.set(lbl_807D9828, lbl_807D982C, lbl_807D9814);
}

// NONMATCHING: equivalent branch layout
void PrepareKoopaFinalDemoObj::control() {
    if (isNerve(GET_NERVE(PrepareKoopaFinalDemoObj, PrepareKoopaFinalDemoObjNrvStopShakeEnd)) || isNerve(GET_NERVE(PrepareKoopaFinalDemoObj, PrepareKoopaFinalDemoObjNrvShake))) {
        if (!MR::isDemoActive("クッパ最終戦突入")) {
            setNerve(GET_NERVE(PrepareKoopaFinalDemoObj, PrepareKoopaFinalDemoObjNrvEnd));
            return;
        }
        if (MR::isStoryBookDemoActive() != 0 && MR::isStoryBookTurnPageEnd() == 0) {
            setNerve(GET_NERVE(PrepareKoopaFinalDemoObj, PrepareKoopaFinalDemoObjNrvStartPuppet));
        }
    }
}

PrepareKoopaFinalDemoObj::~PrepareKoopaFinalDemoObj() {
}

void PrepareKoopaFinalDemoObj::requestWorldMapAfterWipe() {
    MR::closeSystemWipeFadeWithCaptureScreen(0);
    GameSequenceFunction::requestChangeStageWorldMap();
}

void PrepareKoopaFinalDemoObj::setRandomBckFrame(s32 offset) {
    s32 frameMax = static_cast< s32 >(MR::getBckFrameMax(this));
    MR::setBckFrame(this, static_cast< f32 >(frameMax - (offset % frameMax)));
}

// Nerve execute bodies (each nerve's sInstance is defined below).
namespace NrvPrepareKoopaFinalDemoObj {
    void PrepareKoopaFinalDemoObjNrvWait::execute(Spine* pSpine) const {
        reinterpret_cast< PrepareKoopaFinalDemoObj* >(pSpine->mExecutor)->exeStopShake();
    }
    void PrepareKoopaFinalDemoObjNrvShake::execute(Spine* pSpine) const {
        reinterpret_cast< PrepareKoopaFinalDemoObj* >(pSpine->mExecutor)->exeShake();
    }
    void PrepareKoopaFinalDemoObjNrvStopShakeEnd::execute(Spine* pSpine) const {
        reinterpret_cast< PrepareKoopaFinalDemoObj* >(pSpine->mExecutor)->exeStopShakeEnd();
    }
    void PrepareKoopaFinalDemoObjNrvStartPuppet::execute(Spine* pSpine) const {
        PrepareKoopaFinalDemoObj* pActor = reinterpret_cast< PrepareKoopaFinalDemoObj* >(pSpine->mExecutor);
        if (MR::tryStartDemoRegisteredMarioPuppetable(pActor, nullptr)) {
            MR::setPlayerPosAndWait(pActor->mPosition, false);
            pActor->setNerve(GET_NERVE(PrepareKoopaFinalDemoObj, PrepareKoopaFinalDemoObjNrvShake));
        }
    }
    void PrepareKoopaFinalDemoObjNrvEnd::execute(Spine* pSpine) const {
        PrepareKoopaFinalDemoObj* pActor = reinterpret_cast< PrepareKoopaFinalDemoObj* >(pSpine->mExecutor);
        if (MR::isFirstStep(pActor)) {
            MR::stopShakingCamera(pActor);
        }
    }
    PrepareKoopaFinalDemoObjNrvWait PrepareKoopaFinalDemoObjNrvWait::sInstance ATTRIBUTE_WEAK;
    PrepareKoopaFinalDemoObjNrvStopShakeEnd PrepareKoopaFinalDemoObjNrvStopShakeEnd::sInstance ATTRIBUTE_WEAK;
    PrepareKoopaFinalDemoObjNrvShake PrepareKoopaFinalDemoObjNrvShake::sInstance ATTRIBUTE_WEAK;
    PrepareKoopaFinalDemoObjNrvStartPuppet PrepareKoopaFinalDemoObjNrvStartPuppet::sInstance ATTRIBUTE_WEAK;
    PrepareKoopaFinalDemoObjNrvEnd PrepareKoopaFinalDemoObjNrvEnd::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvPrepareKoopaFinalDemoObj
