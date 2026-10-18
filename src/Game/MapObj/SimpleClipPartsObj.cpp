#include "Game/MapObj/SimpleClipPartsObj.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

namespace NrvSimpleClipPartsObj {
    NEW_NERVE(HostTypeWait, SimpleClipPartsObj, Wait);
};  // namespace NrvSimpleClipPartsObj

namespace NrvRailMoveObjClipParts {
    // Waits for switch B, then moves.
    class RailMoveObjClipPartsNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static RailMoveObjClipPartsNrvWait sInstance;
    };

    class RailMoveObjClipPartsNrvMove : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            RailMoveObjClipParts* actor = reinterpret_cast< RailMoveObjClipParts* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                actor->startMapPartsFunctions();
            }
        }
        static RailMoveObjClipPartsNrvMove sInstance;
    };

    RailMoveObjClipPartsNrvWait RailMoveObjClipPartsNrvWait::sInstance ATTRIBUTE_WEAK;
    RailMoveObjClipPartsNrvMove RailMoveObjClipPartsNrvMove::sInstance ATTRIBUTE_WEAK;

    void RailMoveObjClipPartsNrvWait::execute(Spine* pSpine) const {
        RailMoveObjClipParts* actor = reinterpret_cast< RailMoveObjClipParts* >(pSpine->mExecutor);
        if (MR::isOnSwitchB(actor)) {
            actor->setNerve(&RailMoveObjClipPartsNrvMove::sInstance);
        }
    }
};  // namespace NrvRailMoveObjClipParts

namespace NrvRotateMoveObjClipParts {
    class RotateMoveObjClipPartsNrvMove : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            RotateMoveObjClipParts* actor = reinterpret_cast< RotateMoveObjClipParts* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                actor->startMapPartsFunctions();
            }
        }
        static RotateMoveObjClipPartsNrvMove sInstance;
    };
    RotateMoveObjClipPartsNrvMove RotateMoveObjClipPartsNrvMove::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvRotateMoveObjClipParts

SimpleClipPartsObj::SimpleClipPartsObj(const char* pName) : MapObjActor(pName), _D0(0) {
}

void SimpleClipPartsObj::init(const JMapInfoIter& rrIter) {
    MapObjActor::init(rrIter);
    MapObjActorInitInfo info;
    info.setupRailMover();
    info.setupRotator();
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    MapObjActorUtil::setupInitInfoTypical(&info, mObjectName);
    setupInitInfo(info);
    MapObjActor::initialize(rrIter, info);
    _D0 = MR::isExistCollisionResource(this, mObjectName);
    if (_D0) {
        MR::setSensorType(this, "body", 0x43 /* ATYPE_CLIP_FIELD_MAP_PARTS */);
    }
}

void SimpleClipPartsObj::setupInitInfo(const MapObjActorInitInfo& rInfo) {
}

void SimpleClipPartsObj::control() {
    MapObjActor::control();
    if (_D0 && MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this)) {
            MR::validateCollisionParts(this);
        } else {
            MR::invalidateCollisionParts(this);
        }
    }
}

void SimpleClipPartsObj::connectToScene(const MapObjActorInitInfo& rIter) {
    if (rIter.mConnectToScene) {
        MR::connectToClippedMapParts(this);
    }
}

void SimpleClipPartsObj::initCaseUseSwitchA(const MapObjActorInitInfo& rIter) {
}

void SimpleClipPartsObj::initCaseNoUseSwitchA(const MapObjActorInitInfo& rIter) {
}

void SimpleClipPartsObj::initCaseUseSwitchB(const MapObjActorInitInfo& rIter) {
    MR::listenStageSwitchOnOffB(this, MR::Functor< MapObjActor >(this, &MapObjActor::startMapPartsFunctions),
                                MR::Functor< MapObjActor >(this, &MapObjActor::endMapPartsFunctions));
    setStateWait();
}

void SimpleClipPartsObj::initCaseNoUseSwitchB(const MapObjActorInitInfo& rIter) {
    MapObjActor::startMapPartsFunctions();
}

void SimpleClipPartsObj::exeWait() {
    MapObjActor::exeWait();
}

RailMoveObjClipParts::RailMoveObjClipParts(const char* pName) : SimpleClipPartsObj(pName) {
    mRailClipPos.x = 0.0f;
    mRailClipPos.y = 0.0f;
    mRailClipPos.z = 0.0f;
}

RailMoveObjClipParts::~RailMoveObjClipParts() {
}

void RailMoveObjClipParts::init(const JMapInfoIter& rIter) {
    SimpleClipPartsObj::init(rIter);
    bool hasCsv = MR::isValidInitActorCsvParser(mObjectName, nullptr);
    if (MR::isConnectedWithRail(rIter) && !hasCsv) {
        f32 radius = 0.0f;
        MR::calcModelBoundingRadius(&radius, this);
        MR::initAndSetRailClipping(&mRailClipPos, this, 100.0f, radius);
    }
}

void RailMoveObjClipParts::setupInitInfo(const MapObjActorInitInfo& rInfo) {
    MapObjActorInitInfo& info = const_cast< MapObjActorInitInfo& >(rInfo);
    info.setupRailMover();
    info.setupRailPosture();
    info.setupBaseMtxFollowTarget();
    info.setupNerve(&NrvRailMoveObjClipParts::RailMoveObjClipPartsNrvMove::sInstance);
    if (MR::isEqualString(mObjectName, "ClipAppearBattleShip")) {
        info.mFarClipping = 50.0f;
    }
}

void RailMoveObjClipParts::initCaseUseSwitchB(const MapObjActorInitInfo& rInfo) {
    MR::listenStageSwitchOnOffB(this, MR::Functor< MapObjActor >(this, &MapObjActor::startMapPartsFunctions),
                                MR::Functor< MapObjActor >(this, &MapObjActor::endMapPartsFunctions));
    setStateWait();
    setNerve(&NrvRailMoveObjClipParts::RailMoveObjClipPartsNrvWait::sInstance);
}

bool RailMoveObjClipParts::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xDC && isNerve(&NrvRailMoveObjClipParts::RailMoveObjClipPartsNrvMove::sInstance)) {
        kill();
        return true;
    }
    return false;
}

RotateMoveObjClipParts::RotateMoveObjClipParts(const char* pName) : SimpleClipPartsObj(pName) {
}

RotateMoveObjClipParts::~RotateMoveObjClipParts() {
}

void RotateMoveObjClipParts::init(const JMapInfoIter& rIter) {
    SimpleClipPartsObj::init(rIter);
}

void RotateMoveObjClipParts::setupInitInfo(const MapObjActorInitInfo& rInfo) {
    MapObjActorInitInfo& info = const_cast< MapObjActorInitInfo& >(rInfo);
    info.setupRotator();
    info.setupBaseMtxFollowTarget();
    info.setupNerve(&NrvRotateMoveObjClipParts::RotateMoveObjClipPartsNrvMove::sInstance);
}
