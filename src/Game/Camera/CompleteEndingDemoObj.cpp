#include "Game/Camera/CompleteEndingDemoObj.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Util.hpp"

using namespace NrvCompleteEndingDemoObj;

static const char* sCompleteEndingName = "\x83\x52\x83\x93\x83\x76\x83\x8a\x81\x5b\x83\x67\x83\x47\x83\x93\x83\x66\x83\x42\x83\x93\x83\x4f";
static const char* sDemoEpilogName = "DemoEpilog";
static const char* sReferencePointName =
    "\x83\x52\x83\x93\x83\x76\x83\x8a\x81\x5b\x83\x67\x83\x47\x83\x93\x83\x66\x83\x42\x83\x93\x83\x4f\x83\x66\x83\x82\x8a\xee\x8f\x80\x93\x5f";

CompleteEndingDemoObj::CompleteEndingDemoObj(const char* pName) : LiveActor(pName), mRosetta(NULL), mRosettaChair(NULL), mTicoBaby(NULL), mMarioCap(NULL) {
}

CompleteEndingDemoObj::~CompleteEndingDemoObj() {
}

void CompleteEndingDemoObj::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);

    if (MR::tryRegisterDemoCast(this, sCompleteEndingName, rIter)) {
        MR::registerDemoActionNerveDirect(this, GET_NERVE(CompleteEndingDemoObj, CompleteEndingDemoObjNrvAppear), sCompleteEndingName, NULL);
    }

    mRosetta = MR::createModelObjNpc("\x83\x66\x83\x82\x97\x70\x83\x8d\x83\x5b\x83\x62\x83\x5e", "RosettaReading", NULL);
    mRosetta->mLinkInfo.mZoneID = mLinkInfo.mZoneID;

    MR::tryInitDemoSheetTalkAnim(mRosetta, rIter, sCompleteEndingName, sDemoEpilogName, NULL, TVec3f(0.0f, 0.0f, 0.0f),
                                 MR::getJointMtx(mRosetta, "JointRoot"));
    MR::initLightCtrl(mRosetta);
    MR::invalidateClipping(mRosetta);
    mRosetta->makeActorDead();

    mRosettaChair = MR::createModelObjMapObj("\x83\x66\x83\x82\x97\x70\x83\x8d\x83\x5b\x83\x62\x83\x5e\x88\xd6\x8e\x71", "RosettaChair", NULL);
    mRosettaChair->initWithoutIter();
    MR::initLightCtrl(mRosettaChair);
    MR::invalidateClipping(mRosettaChair);
    MR::startAction(mRosettaChair, "DemoRosettaReading");
    mRosettaChair->makeActorDead();

    mTicoBaby = MR::createDemoObjWithAnimArc("\x83\x66\x83\x82\x97\x70\x83\x78\x83\x72\x83\x60\x83\x52", "TicoBaby", "TicoBabyAnim", NULL);
    MR::initTalkAnimDemoCast(mTicoBaby, rIter, sCompleteEndingName, sDemoEpilogName);
    MR::initLightCtrl(mTicoBaby);
    MR::invalidateClipping(mTicoBaby);
    mTicoBaby->makeActorDead();

    const char* capModelName = MR::isPlayerLuigi() ? "LuigiCap" : "MarioCap";
    mMarioCap = MR::createModelObjNpc("\x83\x66\x83\x82\x97\x70\x83\x7d\x83\x8a\x83\x49\x96\x58\x8e\x71", capModelName, NULL);
    MR::initTalkAnimDemoCast(mMarioCap, rIter, sCompleteEndingName, sDemoEpilogName);
    MR::initLightCtrl(mMarioCap);
    MR::invalidateClipping(mMarioCap);
    mMarioCap->makeActorDead();

    initNerve(GET_NERVE(CompleteEndingDemoObj, CompleteEndingDemoObjNrvWait), 0);
    makeActorAppeared();
}

void CompleteEndingDemoObj::exeAppear() {
    if (MR::isFirstStep(this)) {
        mRosetta->makeActorAppeared();
        mRosettaChair->makeActorAppeared();
        mTicoBaby->makeActorAppeared();
        mMarioCap->makeActorAppeared();

        MR::resetPosition(mRosetta, sReferencePointName);
        MR::resetPosition(mRosettaChair, sReferencePointName);
        MR::resetPosition(mTicoBaby, sReferencePointName);
        MR::resetPosition(mMarioCap, sReferencePointName);
    }
}

void CompleteEndingDemoObj::kill() {
    mRosetta->kill();
    mRosettaChair->kill();
    mTicoBaby->kill();
    mMarioCap->kill();
    LiveActor::kill();
}
