#include "Game/Util/Functor.hpp"
#include "Game/Map/Air.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

namespace NrvAir {
    NEW_NERVE(HostTypeIn, Air, In);
    NEW_NERVE(HostTypeOut, Air, Out);
    class HostTypeWait : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }

        static HostTypeWait sInstance;
    };
    HostTypeWait HostTypeWait::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvAir

Air::Air(const char* pName) : LiveActor(pName) {
    mHasChanged = 0;
    mWaitForOpeningCamera = 0;
    mDistance = 70.0f;
}

void Air::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    const char* object_name = 0;
    MR::getObjectName(&object_name, rIter);
    initModel(object_name);
    connectToScene();
    MR::initUseStageSwitchReadA(this, rIter);
    MR::initUseStageSwitchReadB(this, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mDistance);
    s32 arg_1 = -1;
    MR::getJMapInfoArg1NoInit(rIter, &arg_1);
    mWaitForOpeningCamera = arg_1 + 1;
    s32 arg_2 = -1;
    MR::getJMapInfoArg2NoInit(rIter, &arg_2);
    bool isAlwaysVisible = arg_2 + 1;
    MR::tryStartAllAnim(this, object_name);
    MR::setClippingTypeSphereContainsModelBoundingBox(this, 100.0f);
    setFarClipping();
    MR::setGroupClipping(this, rIter, 0x10);
    if (isAlwaysVisible) {
        initNerve(GET_NERVE(Air, HostTypeWait));
    } else {
        initNerve(GET_NERVE(Air, HostTypeOut));
    }
    MR::registerDemoSimpleCastAll(this);
    MR::useStageSwitchAwake(this, rIter);
    bool registered = MR::tryRegisterDemoCast(this, rIter);

    if (registered) {
        bool val = true;

        bool ret = MR::tryRegisterDemoActionFunctor(this, MR::Functor(this, &Air::appearFadeOut), "消滅");

        if (!ret) {
            ret = MR::tryRegisterDemoActionFunctor(this, MR::Functor(this, &Air::appearFadeIn), nullptr);

            if (!ret) {
                val = false;
            }
        }

        if (val) {
            makeActorDead();
        }
    } else {
        appear();
    }
}

void Air::appear() {
    LiveActor::appear();

    if (isNerve(GET_NERVE(Air, HostTypeWait))) {
        MR::tryStartAllAnim(this, "Appear");
        MR::setAllAnimFrameAtEnd(this, "Appear");
        return;
    }

    f32 val = 100.0f * mDistance;
    f32 dist = MR::calcDistanceToPlayer(this);

    if (dist < val) {
        MR::tryStartAllAnim(this, "Appear");
        MR::setAllAnimFrameAtEnd(this, "Appear");
        setNerve(GET_NERVE(Air, HostTypeIn));
    } else {
        MR::tryStartAllAnim(this, "Disappear");
        MR::setAllAnimFrameAtEnd(this, "Disappear");
        MR::hideModel(this);
        setNerve(GET_NERVE(Air, HostTypeOut));
    }
}

bool Air::isDrawing() const {
    if (!MR::isDead(this)) {
        return !MR::isHiddenModel(this);
    }

    return false;
}

void Air::initModel(const char* pModelName) {
    MR::initActor(this, pModelName, false);
}

bool Air::tryChange() {
    if (!MR::isValidSwitchA(this)) {
        return false;
    }

    if (mHasChanged) {
        return false;
    }

    if (MR::isOnSwitchA(this)) {
        MR::tryStartAllAnim(this, "Change");
        mHasChanged = 1;
        return true;
    }

    return false;
}

void Air::appearFadeIn() {
    makeActorAppeared();
    MR::tryStartAllAnim(this, "Appear");
    setNerve(GET_NERVE(Air, HostTypeIn));
}

void Air::appearFadeOut() {
    makeActorAppeared();
    MR::tryStartAllAnim(this, "Disappear");
    setNerve(GET_NERVE(Air, HostTypeOut));
}

void Air::exeIn() {
    bool isCameraNotEnd = mWaitForOpeningCamera && !MR::isStartAnimCameraEnd();

    if (!isCameraNotEnd) {
        tryChange();
        f32 val = (100.0f * (20.0f + mDistance));
        if (val < MR::calcDistanceToPlayer(this) || (MR::isValidSwitchB(this) && MR::isOnSwitchB(this))) {
            MR::tryStartAllAnim(this, "Disappear");
            setNerve(GET_NERVE(Air, HostTypeOut));
        }
    }
}

void Air::exeOut() {
    bool flag = mWaitForOpeningCamera && !MR::isStartAnimCameraEnd();

    if (!flag) {
        tryChange();

        if (!MR::isHiddenModel(this) && MR::isAnyAnimStopped(this, "Disappear")) {
            MR::hideModel(this);
        }

        f32 distMult = (100.0f * mDistance);

        if (MR::calcDistanceToPlayer(this) < distMult) {
            if (!MR::isValidSwitchB(this) || !MR::isOnSwitchB(this)) {
                MR::showModel(this);
                MR::tryStartAllAnim(this, "Appear");
                setNerve(GET_NERVE(Air, HostTypeIn));
            }
        }
    }
}

ProjectionMapAir::ProjectionMapAir(const char* pName) : Air(pName) {
}

void ProjectionMapAir::initModel(const char* pModelName) {
    MR::initActor(this, pModelName, true);
    ProjmapEffectMtxSetter* mtxSetter = MR::initDLMakerProjmapEffectMtxSetter(this);
    MR::newDifferedDLBuffer(this);
    mtxSetter->updateMtxUseBaseMtx();
}

void Air::setFarClipping() {
    MR::setClippingFarMax(this);
}

void Air::connectToScene() {
    MR::connectToSceneAir(this);
}

AirFar100m::AirFar100m(const char* pName) : Air(pName) {
}

void AirFar100m::setFarClipping() {
    MR::setClippingFar100m(this);
}

AfterMapObjDrawAir::AfterMapObjDrawAir(const char* pName) : Air(pName) {
}

void AfterMapObjDrawAir::connectToScene() {
    MR::connectToScene(this, 0x23, 5, 0x14, -1);
}

PriorDrawAir::PriorDrawAir(const char* pName) : Air(pName) {
    MR::createSceneObj(SceneObj_PriorDrawAirHolder);
    MR::getSceneObj< PriorDrawAirHolder >(SceneObj_PriorDrawAirHolder)->add(this);
}

PriorDrawAirHolder::PriorDrawAirHolder() : NameObj("先描画大気保持") {
    mAirCount = 0;
}

void PriorDrawAirHolder::add(PriorDrawAir* pAir) {
    s32 airCnt = mAirCount++;
    mAirs[airCnt] = pAir;
}

bool PriorDrawAirHolder::isExistValidDrawAir() const {
    PriorDrawAir** curAir = (PriorDrawAir**)&mAirs;

    for (; curAir != &mAirs[mAirCount]; *curAir++) {
        if ((*curAir)->isDrawing()) {
            return true;
        }
    }

    return false;
}

namespace MR {
    bool isExistPriorDrawAir() {
        if (!MR::isExistSceneObj(SceneObj_PriorDrawAirHolder)) {
            return false;
        }

        return MR::getSceneObj< PriorDrawAirHolder >(SceneObj_PriorDrawAirHolder)->isExistValidDrawAir();
    }
};  // namespace MR
