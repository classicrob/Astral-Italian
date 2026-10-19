#include "Game/MapObj/BigRepeatWarpLift.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/Util.hpp"

namespace NrvBigRepeatWarpLift {
    class BigRepeatWarpLiftNrvHide : public Nerve {
    public:
        virtual void execute(Spine*) const;

        static BigRepeatWarpLiftNrvHide sInstance;
    };
    BigRepeatWarpLiftNrvHide BigRepeatWarpLiftNrvHide::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(BigRepeatWarpLiftNrvAppear, BigRepeatWarpLift, Appear);
    NEW_NERVE(BigRepeatWarpLiftNrvMove, BigRepeatWarpLift, Move);
    NEW_NERVE(BigRepeatWarpLiftNrvSign, BigRepeatWarpLift, Sign);
    NEW_NERVE(BigRepeatWarpLiftNrvDisappear, BigRepeatWarpLift, Disappear);

    void BigRepeatWarpLiftNrvHide::execute(Spine* pSpine) const {
        BigRepeatWarpLift* actor = reinterpret_cast<BigRepeatWarpLift*>(pSpine->mExecutor);
        if (MR::isGreaterEqualStep(actor, 30)) {
            MR::moveCoordAndTransToRailStartPoint(actor);
            actor->setNerve(&BigRepeatWarpLiftNrvAppear::sInstance);
        }
    }
};  // namespace NrvBigRepeatWarpLift

BigRepeatWarpLift::BigRepeatWarpLift(const char* pName) : MapObjActor(pName) {
    mRailClipCenter.x = 0.0f;
    mRailClipCenter.y = 0.0f;
    mRailClipCenter.z = 0.0f;
}

BigRepeatWarpLift::~BigRepeatWarpLift() {
}

void BigRepeatWarpLift::init(const JMapInfoIter& rIter) {
    MapObjActor::init(rIter);

    MapObjActorInitInfo info;
    info.setupHioNode("\222n\214`\203I\203u\203W\203F");
    info.setupDefaultPos();
    info.setupConnectToScene();
    info.setupEffect(nullptr, 0);
    info.setupSound(6, nullptr);
    info.setupSoundPos(nullptr);
    info.setupGroupClipping(0x20);
    info.setupSeAppear();
    info.setupRailMover();
    info.setupRailPosture();
    info.setupShadow(nullptr);
    info.setupBaseMtxFollowTarget();
    info.setupNerve(&NrvBigRepeatWarpLift::BigRepeatWarpLiftNrvMove::sInstance);
    MapObjActorUtil::setupInitInfoTypical(&info, mObjectName);
    initialize(rIter, info);

    if (MR::isConnectedWithRail(rIter)) {
        f32 radius = 0.0f;
        MR::calcModelBoundingRadius(&radius, this);
        MR::initAndSetRailClipping(&mRailClipCenter, this, 100.0f, radius);
    }
}

void BigRepeatWarpLift::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showModelIfHidden(this);
        MR::resetPosition(this);
        MR::validateCollisionParts(this);
        MR::startAction(this, "Appear");
    }

    if (MR::isActionEnd(this)) {
        setNerve(&NrvBigRepeatWarpLift::BigRepeatWarpLiftNrvMove::sInstance);
    }
}

void BigRepeatWarpLift::exeMove() {
    if (MR::isFirstStep(this)) {
        MapObjActorUtil::endRotator(this);
    }

    MR::startActionSound(this, "Move", -1, -1, -1);

    if (mRailMover->isReachedEnd()) {
        MR::startActionSound(this, "Stop", -1, -1, -1);
        mRailMover->end();
        setNerve(&NrvBigRepeatWarpLift::BigRepeatWarpLiftNrvSign::sInstance);
    }
}

void BigRepeatWarpLift::exeSign() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "Sign");
    }

    if (MR::isStep(this, 0x37) || MR::isStep(this, 0x5F)) {
        MR::startActionSound(this, "Sign", -1, -1, -1);
    }

    if (MR::isBrkStopped(this) && MR::isGreaterEqualStep(this, 0x78)) {
        setNerve(&NrvBigRepeatWarpLift::BigRepeatWarpLiftNrvDisappear::sInstance);
    }
}

void BigRepeatWarpLift::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Disappear");
    }

    if (MR::isActionEnd(this)) {
        MR::hideModel(this);
        MR::invalidateCollisionParts(this);
        setNerve(&NrvBigRepeatWarpLift::BigRepeatWarpLiftNrvHide::sInstance);
    }
}
