#include "Game/System/MiniTicoMaster.hpp"
#include "Game/System/MiniWorldMapMark.hpp"
#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

class TalkMessageCtrl;


extern "C" TVec3f* fn_804F2550(s32, s32);
extern "C" LiveActor* fn_804E83B0(LiveActor*, const char*, const char*, const char*);
extern "C" void fn_804E8C40(LiveActor*);
extern "C" void fn_804E8C50(LiveActor*, f32, f32);
extern "C" void fn_804E8C80(LiveActor*, f32, f32);
extern "C" void fn_804E7180(MiniTicoMaster*);
extern "C" void fn_804E71F0(MiniTicoMaster*, f32);

namespace NrvMiniTicoMaster {
    class MiniTicoMasterNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* pActor = reinterpret_cast<LiveActor*>(pSpine->mExecutor);
            if (MR::isFirstStep(pActor)) {
                MR::startAction(pActor, "MiniTicoMaster");
            }
        };
        static MiniTicoMasterNrvWait sInstance;
    };
    MiniTicoMasterNrvWait MiniTicoMasterNrvWait::sInstance ATTRIBUTE_WEAK;

    class MiniTicoMasterNrvGoMarioPlanet : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            fn_804E7180(reinterpret_cast<MiniTicoMaster*>(pSpine->mExecutor));
        };
        static MiniTicoMasterNrvGoMarioPlanet sInstance;
    };
    MiniTicoMasterNrvGoMarioPlanet MiniTicoMasterNrvGoMarioPlanet::sInstance ATTRIBUTE_WEAK;

    class MiniTicoMasterNrvNone : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {};
        static MiniTicoMasterNrvNone sInstance;
    };
    MiniTicoMasterNrvNone MiniTicoMasterNrvNone::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvMiniTicoMaster

namespace {
    TVec3f sPlanetOffset(0.0f, 500.0f, 0.0f);
    TVec3f sTalkOffset(0.0f, 2000.0f, 0.0f);
    TVec3f sShadowOffset(0.0f, -250.0f, 0.0f);
};  // namespace

MiniTicoMaster::~MiniTicoMaster() {}

void MiniTicoMaster::init(const JMapInfoIter& rIter) {
    TVec3f base(*fn_804F2550(mUnk90, mUnkA4));
    TVec3f pos = base + sPlanetOffset;
    mPosition = pos;

    MR::initActor(this, rIter, "MiniTicoMaster", false);

    mModelA = fn_804E83B0(this, "左手", "MiniTicoMaster", "StarPieceL");
    mModelB = fn_804E83B0(this, "右手", "MiniTicoMaster", "StarPieceR");
    MR::setScale(mModelA, 11.0f);
    MR::setScale(mModelB, 11.0f);
    MR::setBpkFrame(mModelA, 0.0f);
    MR::setBpkFrame(mModelB, 100.0f);
    MR::registerDemoSimpleCastAll(mModelA);
    MR::registerDemoSimpleCastAll(mModelB);
    MR::registerDemoSimpleCastAll(this);

    if (mPointerTarget == nullptr) {
        TVec3f pointerOffset;
        pointerOffset.set(0.0f, 0.0f, 0.0f);
        MR::initStarPointerTarget(this, 800.0f, pointerOffset);
    }

    mShadow = new MiniGalaxyShadow(this, "MiniGalaxyShadow", "影");

    mTalkCtrl = MR::createTalkCtrlDirect(this, "WorldMapEventTicoRequestStarPiece", TVec3f(0.0f, 0.0f, 0.0f), nullptr);
    MR::setMessageArg(mTalkCtrl, mUnk98);
    MR::setMessageBalloonFollowOffset(mTalkCtrl, sTalkOffset);
    MR::onRootNodeAutomatic(mTalkCtrl);

    mChild = new MiniWorldMapMark(this, "MiniTicoMasterChild");

    initWithoutIter();
    MR::setPosition(mChild, pos);
    TVec3f childScale(1.0f, 1.0f, 1.0f);
    MR::setScale(mChild, childScale);
    initNerve(&NrvMiniTicoMaster::MiniTicoMasterNrvWait::sInstance, 0);

    if (mUnk9C >= *mUnk94) {
        MR::setMessageArg(mTalkCtrl, *mUnk94);
        makeActorDead();
        mChild->appear();
    } else {
        makeActorAppeared();
    }
}

void MiniTicoMaster::makeActorAppeared() {
    if (mUnk9C >= *mUnk94) {
        mChild->appear();
    } else {
        LiveActor::makeActorAppeared();
        mModelA->makeActorAppeared();
        mModelB->makeActorAppeared();
        mShadow->makeActorAppeared();
    }
}

void MiniTicoMaster::makeActorDead() {
    LiveActor::makeActorDead();
    mChild->makeActorDead();
    mModelA->makeActorDead();
    mModelB->makeActorDead();
    mShadow->makeActorDead();
}

// NONMATCHING: only the first call is reconstructed. The rest (joint matrix column lengths via
// fn_80046D50, hand scaling and positioning, shadow placement) uses paired-single ops in the target.
void MiniTicoMaster::control() {
    fn_804E8C80(this, 0.05f, 1.0f);
}

extern "C" bool fn_804E7150(MiniTicoMaster* pActor) {
    return pActor->mUnk9C >= *pActor->mUnk94;
}

extern "C" void fn_804E7170(MiniTicoMaster* pActor) {
    pActor->setNerve(&NrvMiniTicoMaster::MiniTicoMasterNrvGoMarioPlanet::sInstance);
}

extern "C" void fn_804E7180(MiniTicoMaster* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "GoMarioPlanet");
    }
    if (MR::isActionEnd(pActor)) {
        pActor->kill();
    }
}

extern "C" void fn_804E71F0(MiniTicoMaster* pActor, f32 t) {
    TVec3f a = *fn_804F2550(pActor->mUnk90, pActor->mUnkA4);
    TVec3f b = *fn_804F2550(pActor->mUnk90, pActor->mUnkA8);
    TVec3f c = b * t;
    TVec3f d = a * (1.0f - t);
    TVec3f e = d + c;
    MR::setPosition(pActor, e);
}

extern "C" bool fn_804E72A0(MiniTicoMaster* pActor) {
    return MR::tryTalkForce(pActor->mTalkCtrl);
}

extern "C" void fn_804E72B0(LiveActor* pActor) {
    fn_804E8C40(pActor);
}

extern "C" bool fn_804E72C0(MiniTicoMaster* pActor) {
    fn_804E8C50(pActor, 0.1f, 1.4f);
    MR::tryTalkForce(pActor->mTalkCtrl);
    return true;
}

extern "C" void fn_804E7300(MiniTicoMaster* pActor, s32 drawOrder) {
    MR::setAllEffectDrawOrder(pActor, drawOrder);
    MR::setAllEffectDrawOrder(pActor->mModelA, drawOrder);
    MR::setAllEffectDrawOrder(pActor->mModelB, drawOrder);
}
