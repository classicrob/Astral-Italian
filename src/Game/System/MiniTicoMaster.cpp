#define JGEOMETRY_VEC3_SET_NOINLINE
#include "Game/System/MiniTicoMaster.hpp"
#include "Game/System/MiniWorldMapMark.hpp"
#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

class TalkMessageCtrl;



namespace { extern char sMiniTicoMasterModel[]; }
namespace NrvMiniTicoMaster {
    class MiniTicoMasterNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* pActor = reinterpret_cast<LiveActor*>(pSpine->mExecutor);
            if (MR::isFirstStep(pActor)) {
                MR::startAction(pActor, sMiniTicoMasterModel);
            }
        };
        static MiniTicoMasterNrvWait sInstance;
    };
    MiniTicoMasterNrvWait MiniTicoMasterNrvWait::sInstance ATTRIBUTE_WEAK;

    class MiniTicoMasterNrvGoMarioPlanet : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast<MiniTicoMaster*>(pSpine->mExecutor)->exeGoMarioPlanet();
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

// NONMATCHING: regalloc (FP order/grouping checked: no FP instructions).
MiniTicoMaster::MiniTicoMaster(WorldMapResultWorld* world, s32 partsIndex, s32* count, s32 pointIndex, s32 nextPoint)
    : LiveActor("ミニチコ（ルート大工）") {
    mUnk90 = world;
    mUnk94 = count;
    mUnk98 = 0;
    mUnk9C = 0;
    mUnkA0 = partsIndex;
    mModelA = nullptr;
    mModelB = nullptr;
    mChild = nullptr;
    mTalkCtrl = nullptr;
    mShadow = nullptr;
    mUnkA4 = pointIndex;
    mUnkA8 = nextPoint;
    mUnk9C = GameDataFunction::getTicoFatStarPieceAlmsFromWorld(world->getWorldId(), mUnkA0);
    mUnk98 = MR::max(0, *mUnk94 - mUnk9C);
    if (mUnk9C < *mUnk94) {
        mUnk90->setLinkProgressFull(mUnkA4, mUnkA8);
    }
}

MiniTicoMaster::~MiniTicoMaster() {}

namespace { char sMiniTicoMasterModel[] = "MiniTicoMaster"; }
void MiniTicoMaster::init(const JMapInfoIter& rIter) {
    TVec3f base(mUnk90->getNode(mUnkA4)->mPos);
    TVec3f pos = base + sPlanetOffset;
    mPosition.x = pos.x;
    mPosition.y = pos.y;
    mPosition.z = pos.z;

    MR::initActor(this, rIter, sMiniTicoMasterModel, false);

    mModelA = WorldMapFunction::createPartsModel(this, "左手", "TicoFatGoodsStarPiece", "StarPieceL");
    mModelB = WorldMapFunction::createPartsModel(this, "右手", "TicoFatGoodsStarPiece", "StarPieceR");
    MR::setScale(mModelA, 11.0f);
    MR::setScale(mModelB, 11.0f);
    MR::setBpkFrame(mModelA, 0.0f);
    MR::setBpkFrame(mModelB, 100.0f);
    MR::registerDemoSimpleCastAll(mModelA);
    MR::registerDemoSimpleCastAll(mModelB);
    MR::registerDemoSimpleCastAll(this);

    if (mStarPointerTarget == nullptr) {
        TVec3f pointerOffset;
        pointerOffset.set(0.0f, 0.0f, 0.0f);
        MR::initStarPointerTarget(this, 800.0f, pointerOffset);
    }

    mShadow = new MiniGalaxyShadow(this, "影", "MiniGalaxyShadow");

    mTalkCtrl = MR::createTalkCtrlDirect(this, "WorldMapEventTicoRequestStarPiece", TVec3f(0.0f, 0.0f, 0.0f), nullptr);
    MR::setMessageArg(mTalkCtrl, mUnk98);
    MR::setMessageBalloonFollowOffset(mTalkCtrl, sTalkOffset);
    MR::onRootNodeAutomatic(mTalkCtrl);

    mChild = new MiniWorldMapMark(this, "MiniTicoMasterMark");

    mChild->initWithoutIter();
    MR::setPosition(mChild, base);
    MR::setScale(mChild, TVec3f(1.0f, 1.0f, 1.0f));
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

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void MiniTicoMaster::control() {
    WorldMapFunction::returnScale(this, 0.05f, 1.0f);
    TPos3f joint;
    TPos3f scaleMtx;
    MR::separateMtxScale(joint.toMtxPtr(), scaleMtx.toMtxPtr(), MR::getJointMtx(this, "Center"));
    TVec3f scale;
    scaleMtx.getScale(scale);
    TVec3f pos;
    joint.getTrans(pos);
    MR::setScale(mModelA, (scale * 11.0f) * mScale.y);
    MR::setScale(mModelB, (scale * 11.0f) * mScale.y);
    TVec3f groundPos(pos.x, mPosition.y, pos.z);
    if ((mPosition - groundPos).length() < 2000.0f) {
        MR::setPosition(mShadow, TVec3f(pos.x, mPosition.y, pos.z) + sShadowOffset);
        MR::setScale(mShadow, scale);
        MR::showModel(mShadow);
    } else {
        MR::hideModel(mShadow);
    }
}

bool MiniTicoMaster::isFull() const {
    return mUnk9C >= *mUnk94;
}

void MiniTicoMaster::startGoMarioPlanet() {
    setNerve(&NrvMiniTicoMaster::MiniTicoMasterNrvGoMarioPlanet::sInstance);
}

void MiniTicoMaster::exeGoMarioPlanet() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "GoMarioPlanet");
    }
    if (MR::isActionEnd(this)) {
        kill();
    }
}

// NONMATCHING: register/stack or instruction scheduling (FP order/grouping proven by default rdclass a).
void MiniTicoMaster::setPosOnRoute(f32 t) {
    TVec3f a = mUnk90->getNode(mUnkA4)->mPos;
    TVec3f b = mUnk90->getNode(mUnkA8)->mPos;
    MR::setPosition(this, a * (1.0f - t) + b * t);
}

bool MiniTicoMaster::tryTalk() {
    return MR::tryTalkForce(mTalkCtrl);
}

bool MiniTicoMaster::isPointing() {
    return WorldMapFunction::isPointing(this);
}

bool MiniTicoMaster::tryPointTalk() {
    WorldMapFunction::approachScale(this, 0.1f, 1.4f);
    MR::tryTalkForce(mTalkCtrl);
    return true;
}

void MiniTicoMaster::setEffectDrawOrder(s32 drawOrder) {
    MR::setAllEffectDrawOrder(this, drawOrder);
    MR::setAllEffectDrawOrder(mModelA, drawOrder);
    MR::setAllEffectDrawOrder(mModelB, drawOrder);
}
