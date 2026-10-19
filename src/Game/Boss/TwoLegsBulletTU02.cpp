#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"

// TU02 "HorinoHillPart" (0xC0) and "HorinoHill" (0xB0) actors.

namespace NrvHorinoHillPart {
    class HorinoHillPartNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static HorinoHillPartNrvWait sInstance;
    };
    HorinoHillPartNrvWait HorinoHillPartNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(HorinoHillPartNrvShrink, HorinoHillPart, Shrink);
    inline void HorinoHillPartNrvWait::execute(Spine* pSpine) const {
        HorinoHillPart* pActor = reinterpret_cast< HorinoHillPart* >(pSpine->mExecutor);
        MR::setNerveAtBckStopped(pActor, &HorinoHillPartNrvShrink::sInstance);
    }
};  // namespace NrvHorinoHillPart

namespace NrvHorinoHill {
    class HorinoHillNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            HorinoHill* pActor = reinterpret_cast< HorinoHill* >(pSpine->mExecutor);
            if (MR::isFirstStep(pActor)) {
                pActor->tryAppearPart(true);
                return;
            }
            pActor->tryAppearPart(false);
        }
        static HorinoHillNrvWait sInstance;
    };
    HorinoHillNrvWait HorinoHillNrvWait::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvHorinoHill

HorinoHillPart::HorinoHillPart(const char* pName) : LiveActor(pName) {
    mMtx.identity();
}

HorinoHillPart::~HorinoHillPart() {}

void HorinoHillPart::init(const JMapInfoIter&) {
    MR::initActor(this, "HorinoHill", false);
    initNerve(&NrvHorinoHillPart::HorinoHillPartNrvWait::sInstance, 0);
    makeActorDead();
}

void HorinoHillPart::appear() {
    MR::setScale(this, 1.0f);
    MR::startAction(this, "HorinoHill");
    LiveActor::appear();
    setNerve(&NrvHorinoHillPart::HorinoHillPartNrvWait::sInstance);
}

// NONMATCHING: vector temporary evaluation order and register scheduling differ; calls and constants agree.
bool HorinoHillPart::tryPlace(const TVec3f& rPos, const TVec3f& rUp) {
    Triangle tri;
    TVec3f offs(static_cast<const Vec&>(rUp));
    offs.scale(500.0f);
    TVec3f start = rPos + offs;
    TVec3f end;
    end.setLength(-rUp, 1000.0f);
    if (!MR::getFirstPolyOnLineToMap(&mPosition, &tri, start, end)) {
        return false;
    }
    TVec3f front;
    MR::makeAxisVerticalZXFront(&front, *tri.getNormal(0));
    MR::makeMtxUpFrontPos(&mMtx, *tri.getNormal(0), front, mPosition);
    return true;
}

void HorinoHillPart::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mMtx);
}

void HorinoHillPart::exeShrink() {
    f32 rate = MR::calcNerveEaseOutRate(this, 45);
    MR::setScale(this, 1.0f - rate);
    if (MR::isStep(this, 45)) {
        kill();
    }
}

HorinoHill::HorinoHill(LiveActor* pHost, s32 num, const char* pName, f32 radius) : LiveActor(pName) {
    mHost = pHost;
    mParts = nullptr;
    mPartNum = num;
    mRadius = radius;
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mUpDir = nullptr;
}

HorinoHill::~HorinoHill() {}

void HorinoHill::init(const JMapInfoIter&) {
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    initNerve(&NrvHorinoHill::HorinoHillNrvWait::sInstance, 0);
    mParts = new HorinoHillPart*[mPartNum];
    for (s32 i = 0; i < mPartNum; i++) {
        HorinoHillPart* pPart = new HorinoHillPart("\x83\x7a\x83\x8a\x81\x5b\x83\x6d\x92\xcb");
        mParts[i] = pPart;
        mParts[i]->initWithoutIter();
    }
    makeActorDead();
}

void HorinoHill::appear() {
    LiveActor::appear();
    setNerve(&NrvHorinoHill::HorinoHillNrvWait::sInstance);
}

void HorinoHill::tryAppearPart(bool isForce) {
    HorinoHillPart* pPart = nullptr;
    for (s32 i = 0; i < mPartNum; i++) {
        if (MR::isDead(mParts[i])) {
            pPart = mParts[i];
            break;
        }
    }
    if (pPart == nullptr) {
        return;
    }
    TVec3f pos;
    MR::calcPositionFrontOffset(&pos, mHost, 150.0f);
    if (isForce || !(PSVECDistance(&mPos, &pos) < mRadius)) {
        mPos.set(pos);
        TVec3f up;
        if (mUpDir != nullptr) {
            up.set(*mUpDir);
        } else {
            MR::calcUpVec(&up, mHost);
        }
        if (pPart->tryPlace(pos, up)) {
            pPart->appear();
        }
    }
}

