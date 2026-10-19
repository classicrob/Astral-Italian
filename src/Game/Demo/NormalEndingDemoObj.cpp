#include "Game/LiveActor/DemoObj.hpp"
#include "Game/Demo/StoryBookDemo.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Demo/NormalEndingDemoObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/MapObj/PowerStar.hpp"
#include "Game/Demo/AstroDemoFunction.hpp"
#include "Game/Demo/DemoAstroPlanet.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/AreaObj/AreaObj.hpp"
#include "Game/Util/AreaObjUtil.hpp"

const char* sNormalEndingDemoName1 = "ノーマルエンディング１";
const char* sNormalEndingDemoName2 = "ノーマルエンディング２";
const char* sNormalEndingDemoName3 = "ノーマルエンディング３";
const char* sNormalEndingDemoName4 = "ノーマルエンディング４";
const char* sNormalEndingPartName1 = "DemoEnding1st";
const char* sNormalEndingPartName2 = "DemoEnding2nd";
const char* sNormalEndingPartName3 = "DemoEnding3rd";
const char* sNormalEndingPartName4 = "DemoEnding4th";
const char* sNormalEndingBasePointName = "ノーマルエンディングデモ基準点";
namespace NrvNormalEndingDemoObj {
    class NormalEndingDemoObjNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
        }
        static NormalEndingDemoObjNrvWait sInstance;
    };
    NormalEndingDemoObjNrvWait NormalEndingDemoObjNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(NormalEndingDemoObjNrvA, NormalEndingDemoObj, A);
    NEW_NERVE(NormalEndingDemoObjNrvB, NormalEndingDemoObj, B);
    NEW_NERVE(NormalEndingDemoObjNrvC, NormalEndingDemoObj, C);

    class NormalEndingDemoObjNrvStaffRoll : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< NormalEndingDemoObj* >(pSpine->mExecutor)->checkStaffRollStart();
        }
        static NormalEndingDemoObjNrvStaffRoll sInstance;
    };
    NormalEndingDemoObjNrvStaffRoll NormalEndingDemoObjNrvStaffRoll::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvNormalEndingDemoObj

LiveActor* NormalEndingDemoObj::createDemoModel(const char* pName, const char* pModelName, MtxPtr pMtx) {
    LiveActor* pActor = MR::createDemoObj(pName, pModelName, pMtx);
    pActor->mLinkInfo.mZoneID = mLinkInfo.mZoneID;
    MR::invalidateClipping(pActor);
    pActor->makeActorDead();
    return pActor;
}

LiveActor* NormalEndingDemoObj::createDemoModel(const char* pName, const char* pModelName, const char* pFuncName, MtxPtr pMtx) {
    LiveActor* pActor = MR::createDemoObj(pName, pModelName, pFuncName, pMtx);
    pActor->mLinkInfo.mZoneID = mLinkInfo.mZoneID;
    MR::invalidateClipping(pActor);
    pActor->makeActorDead();
    return pActor;
}

LiveActor* NormalEndingDemoObj::createDemoModelWithAnimArc(const char* pName, const char* pModelName, const char* pAnimArcName, MtxPtr pMtx) {
    LiveActor* pActor = MR::createDemoObjWithAnimArc(pName, pModelName, pAnimArcName, pMtx);
    pActor->mLinkInfo.mZoneID = mLinkInfo.mZoneID;
    MR::invalidateClipping(pActor);
    pActor->makeActorDead();
    return pActor;
}

LiveActor* NormalEndingDemoObj::createDemoModel(const char* pName, const char* pModelName) {
    LiveActor* pActor = MR::createDemoObj(pName, pModelName, nullptr);
    pActor->mLinkInfo.mZoneID = mLinkInfo.mZoneID;
    MR::invalidateClipping(pActor);
    pActor->makeActorDead();
    return pActor;
}

void NormalEndingDemoObj::setEndingAreaIndex(s32 index) {
    AreaObjMgr* pMgr = MR::getAreaObjManager("LightArea");

    for (s32 i = 0; i < pMgr->getNumAreaObj(); i++) {
        AreaObj* pArea = pMgr->getAreaObj(i);

        if (index == pArea->mObjArg0) {
            pArea->validate();
        } else {
            pArea->invalidate();
        }
    }
}

NormalEndingDemoObj::NormalEndingDemoObj(const char* pName) : LiveActor(pName), mField90(0), mActorA(nullptr), mActorB(nullptr), mActorC(nullptr), mStar(nullptr), mActorD(nullptr), mActorE(nullptr), mActorF(nullptr), mActorG(nullptr), mActorH(nullptr), mPlanet(nullptr), mActorI(nullptr), mActorJ(nullptr) {
}

void NormalEndingDemoObj::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    MR::registerDemoCast(this, sNormalEndingDemoName1, rIter);
    MR::registerDemoCast(this, sNormalEndingDemoName2, rIter);
    MR::registerDemoCast(this, sNormalEndingDemoName3, rIter);
    MR::registerDemoCast(this, sNormalEndingDemoName4, rIter);
    MR::registerDemoActionNerveDirect(this, GET_NERVE(NormalEndingDemoObj, NormalEndingDemoObjNrvA), sNormalEndingDemoName1, nullptr);
    mActorI = createDemoModel("デモ用顔惑星ヌルオブジェ", "MarioShipTakeOff");
    MR::tryRegisterDemoCast(mActorI, sNormalEndingDemoName1, rIter);
    MR::tryRegisterDemoCast(mActorI, sNormalEndingDemoName2, rIter);
    MR::tryRegisterDemoCast(mActorI, sNormalEndingDemoName3, rIter);
    MR::initTalkAnimDemoCast(mActorI, rIter, sNormalEndingDemoName4, sNormalEndingPartName4);
    mActorJ = createDemoModel("デモ用天文台ヌルオブジェ", "DemoAstroPlanetNull");
    MR::initTalkAnimDemoCast(mActorJ, rIter, sNormalEndingDemoName1, sNormalEndingPartName1);
    MR::tryRegisterDemoCast(mActorJ, sNormalEndingDemoName2, rIter);
    MR::initTalkAnimDemoCast(mActorJ, rIter, sNormalEndingDemoName3, sNormalEndingPartName3);
    MR::tryRegisterDemoCast(mActorJ, sNormalEndingDemoName4, rIter);
    mActorA = createDemoModelWithAnimArc("デモ用ベビチコ", "TicoBaby", "TicoBabyAnim", 0);
    MR::initLightCtrl(mActorA);
    MR::tryInitDemoSheetAnim(mActorA, rIter, sNormalEndingDemoName1, sNormalEndingPartName1);
    MR::tryInitDemoSheetAnim(mActorA, rIter, sNormalEndingDemoName2, sNormalEndingPartName2);
    MR::tryInitDemoSheetAnim(mActorA, rIter, sNormalEndingDemoName3, sNormalEndingPartName3);
    MR::tryInitDemoSheetAnim(mActorA, rIter, sNormalEndingDemoName4, sNormalEndingPartName4);
    mActorB = createDemoModelWithAnimArc("デモ用ピーチ姫", "Peach", "PeachAnimPeachCastleGalaxy2", 0);
    MR::initTalkAnimDemoCast(mActorB, rIter, sNormalEndingDemoName1, sNormalEndingPartName1);
    MR::initTalkAnimDemoCast(mActorB, rIter, sNormalEndingDemoName2, sNormalEndingPartName2);
    MR::initTalkAnimDemoCast(mActorB, rIter, sNormalEndingDemoName3, sNormalEndingPartName3);
    MR::initTalkAnimDemoCast(mActorB, rIter, sNormalEndingDemoName4, sNormalEndingPartName4);
    mActorC = createDemoModel("デモ用ロゼッタ", "Rosetta", "NormalEnding", nullptr);
    MR::initTalkAnimDemoCast(mActorC, rIter, sNormalEndingDemoName2, sNormalEndingPartName2);
    MR::initTalkAnimDemoCast(mActorC, rIter, sNormalEndingDemoName3, sNormalEndingPartName3);
    mStar = createDemoModel("デモ用グランドスター", "GrandStar", "Demo", nullptr);
    MR::initTalkAnimDemoCast(mStar, rIter, sNormalEndingDemoName1, sNormalEndingPartName1);
    mActorD = createDemoModelWithAnimArc("デモ用マイスター", "Meister", "MeisterAnimPeachCastleGalaxy2", 0);
    MR::initTalkAnimDemoCast(mActorD, rIter, sNormalEndingDemoName2, sNormalEndingPartName2);
    MR::initTalkAnimDemoCast(mActorD, rIter, sNormalEndingDemoName3, sNormalEndingPartName3);
    MR::initTalkAnimDemoCast(mActorD, rIter, sNormalEndingDemoName4, sNormalEndingPartName4);
    mActorE = createDemoModel("デモ用顔惑星", "DemoMarioFaceShipPlanet", "NormalEnding", MR::getJointMtx(mActorI, "MarioShipTakeOff"));
    mActorF = createDemoModel("デモ用顔惑星パネル", "WorldMapAccessPanel", nullptr);
    MR::registerDemoSimpleCastAll(mActorF);
    MR::initTalkAnimDemoCast(mActorF, rIter, sNormalEndingDemoName4, sNormalEndingPartName4);
    mActorG = MR::createDemoObj("デモ用顔惑星舵", "MarioFaceShipRudder", "Demo", nullptr);
    MR::invalidateClipping(mActorG);
    mActorG->makeActorDead();
    MR::registerDemoSimpleCastAll(mActorG);
    MR::initTalkAnimDemoCast(mActorG, rIter, sNormalEndingDemoName4, sNormalEndingPartName4);
    mActorH = createDemoModel("デモ用彗星", "DemoAstroCocoon", MR::getJointMtx(mActorJ, "AstroCocoon"));
    MR::initTalkAnimDemoCast(mActorH, rIter, sNormalEndingDemoName1, sNormalEndingPartName1);
    MR::initTalkAnimDemoCast(mActorH, rIter, sNormalEndingDemoName3, sNormalEndingPartName3);
    mPlanet = new DemoAstroPlanet("デモ用天文台", MR::getJointMtx(mActorJ, "AstroPlanet"));
    mPlanet->initWithoutIter();
    mPlanet->mLinkInfo.mZoneID = mLinkInfo.mZoneID;
    MR::invalidateClipping(mPlanet);
    mPlanet->makeActorDead();
    MR::registerDemoSimpleCastAll(mPlanet);
    MR::initTalkAnimDemoCast(mPlanet, rIter, sNormalEndingDemoName1, sNormalEndingPartName1);
    MR::initTalkAnimDemoCast(mPlanet, rIter, sNormalEndingDemoName3, sNormalEndingPartName3);
    initNerve(GET_NERVE(NormalEndingDemoObj, NormalEndingDemoObjNrvWait), 0);
    makeActorAppeared();
}

void NormalEndingDemoObj::initAfterPlacement() {
    MR::resetPosition(mActorA, sNormalEndingBasePointName);
    MR::resetPosition(mActorB, sNormalEndingBasePointName);
    MR::resetPosition(mActorC, sNormalEndingBasePointName);
    MR::resetPosition(mStar, sNormalEndingBasePointName);
    MR::resetPosition(mActorD, sNormalEndingBasePointName);
    MR::resetPosition(mActorE, sNormalEndingBasePointName);
    MR::resetPosition(mActorF, sNormalEndingBasePointName);
    MR::resetPosition(mActorG, sNormalEndingBasePointName);
    MR::resetPosition(mActorH, sNormalEndingBasePointName);
    MR::resetPosition(mPlanet, sNormalEndingBasePointName);
    MR::resetPosition(mActorI, sNormalEndingBasePointName);
    MR::resetPosition(mActorJ, sNormalEndingBasePointName);
    setEndingAreaIndex(1);
    MR::endStartPosCamera();
}

// Nerve 1E0C.. : shows the cast, then hands off to exeB.
// NONMATCHING: address-load scheduling and equivalent SDA references
void NormalEndingDemoObj::exeA() {
    if (MR::isFirstStep(this)) {
        mActorA->makeActorAppeared();
        mActorB->makeActorAppeared();
        mActorC->makeActorAppeared();
        PowerStar::setupColorAtResultSequence(mStar, true);
        mStar->makeActorAppeared();
        mActorD->makeActorAppeared();
        mActorE->makeActorAppeared();
        mActorF->makeActorAppeared();
        MR::startAction(mActorF, "DemoEndingWait");
        mActorG->makeActorAppeared();
        MR::startAction(mActorG, "DemoEndingWait");
        mActorJ->makeActorAppeared();
        mActorH->makeActorAppeared();
        MR::startBtk(mActorH, "DemoEnding1stLoop");
        mPlanet->makeActorAppeared();
        MR::startBtk(mPlanet, "DemoEnding1stLoop");
        MR::startBrk(mPlanet, "DemoEnding1stLoop");
        mActorI->makeActorAppeared();
        MR::startAction(mActorI, "DemoEndingWait");
        setEndingAreaIndex(1);
        MR::openSystemWipeWhiteFade(0xB4);
    }
    PowerStar::requestPointLightAtResultSequence(mStar);
    if (MR::isDemoActive(sNormalEndingDemoName1)) {
        if (MR::isDemoPartStep("マリオ顔惑星に帰還", 0x1A2) && MR::isStoryBookTurningPage() != 0) {
            setEndingAreaIndex(2);
        }
        if (MR::isDemoPartStep("彗星が天文台に変化", 0x104)) {
            mPlanet->mIsRotating = true;
            setEndingAreaIndex(1);
        }
    }
    if (MR::isDemoLastStep()) {
        MR::pauseStoryBookTurnPage();
        MR::requestStartTimeKeepDemoMarioPuppetableWithoutCinemaFrame(this, sNormalEndingDemoName2, 0);
        setNerve(GET_NERVE(NormalEndingDemoObj, NormalEndingDemoObjNrvB));
    }
}

void NormalEndingDemoObj::exeB() {
    if (MR::isFirstStep(this)) {
        mStar->makeActorDead();
    }
    if (MR::isDemoActive(sNormalEndingDemoName2)) {
        if (MR::isDemoPartStep("ロゼッタ出現", 0)) {
            setEndingAreaIndex(3);
        }
    }
    if (MR::isDemoLastStep()) {
        MR::pauseStoryBookTurnPage();
        MR::requestStartTimeKeepDemoMarioPuppetableWithoutCinemaFrame(this, sNormalEndingDemoName3, 0);
        setNerve(GET_NERVE(NormalEndingDemoObj, NormalEndingDemoObjNrvC));
    }
}

void NormalEndingDemoObj::exeC() {
    if (MR::isFirstStep(this)) {
        mPlanet->mRotationY = 0.0f;
        mPlanet->mIsRotating = false;
        MR::startBtk(mActorH, "DemoEnding3rdLoop");
        MR::startBtk(mPlanet, "DemoEnding3rdLoop");
        MR::startBrk(mPlanet, "DemoEnding3rdLoop");
        setEndingAreaIndex(3);
    }
    if (MR::isDemoActive(sNormalEndingDemoName3)) {
        if (MR::isDemoPartStep("チコ帰還", 0x159)) {
            setEndingAreaIndex(1);
        }
        if (MR::isDemoPartStep("チコ帰還", 0x29D)) {
            setEndingAreaIndex(4);
        }
        if (MR::isDemoPartStep("チコ帰還", 0x30B)) {
            setEndingAreaIndex(1);
        }
    }
    if (MR::isDemoLastStep()) {
        MR::pauseStoryBookTurnPage();
        MR::requestStartTimeKeepDemoMarioPuppetableWithoutCinemaFrame(this, sNormalEndingDemoName4, 0);
        setNerve(GET_NERVE(NormalEndingDemoObj, NormalEndingDemoObjNrvStaffRoll));
    }
}

void NormalEndingDemoObj::checkStaffRollStart() {
    const char* part = "マリオ惑星出発";
    if (MR::isDemoPartActive(part)) {
        if (MR::isDemoPartStep(part, MR::getDemoPartTotalStep(part) - 2)) {
            GameSequenceFunction::requestChangeStageStaffRoll();
            MR::startSystemWipeCircleWithCaptureScreen(180);
        }
    }
}

void NormalEndingDemoObj::kill() {
    mActorB->kill();
    mActorC->kill();
    mStar->kill();
    mActorD->kill();
    mActorE->kill();
    mActorH->kill();
    mPlanet->kill();
    mActorI->kill();
    mActorJ->kill();
    LiveActor::kill();
}

DemoAstroPlanet::DemoAstroPlanet(const char* pName, MtxPtr pMtx)
    : LiveActor(pName), mJointController(NULL), mBaseMtx(pMtx), mEaseTimer(0.0f), mRotationY(0.0f), mIsRotating(false) {
    if (pMtx != NULL) {
        mPosition.set<f32>(pMtx[0][3], pMtx[1][3], pMtx[2][3]);
    }
}

void DemoAstroPlanet::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "DemoAstroPlanet", false);

    JointControlDelegator< DemoAstroPlanet >::func calcFunc = &DemoAstroPlanet::calcJointMatrix;
    JointControlDelegator< DemoAstroPlanet >* pDelegator = new JointControlDelegator< DemoAstroPlanet >(this, calcFunc, NULL);
    MR::setJointControllerParam(pDelegator, this, "Model");
    mJointController = pDelegator;

    makeActorAppeared();
}

void DemoAstroPlanet::control() {
    if (mIsRotating && !MR::isStoryBookDemoActive()) {
        f32 ease = 1.0f;
        s32 limit = 60;
        if (mEaseTimer < (f32)limit) {
            f32 t = MR::normalize(mEaseTimer, 0.0f, (f32)limit);
            ease = MR::getEaseInValue(t, 0.0f, 1.0f, 1.0f);
            mEaseTimer += 1.0f;
        }

        mRotationY += 0.05f * ease;
        mRotationY = MR::repeatDegree(mRotationY);
    }
}

void DemoAstroPlanet::calcAndSetBaseMtx() {
    if (mBaseMtx != NULL) {
        mPosition.set<f32>(mBaseMtx[0][3], mBaseMtx[1][3], mBaseMtx[2][3]);
        MR::setBaseTRMtx(this, mBaseMtx);
    } else {
        LiveActor::calcAndSetBaseMtx();
    }

    mJointController->registerCallBack();
}

bool DemoAstroPlanet::calcJointMatrix(TPos3f* pMtx, const JointControllerInfo&) {
    MR::rotateMtxLocalYDegree(pMtx->toMtxPtr(), mRotationY);
    return true;
}

NormalEndingDemoObj::~NormalEndingDemoObj() {
}

DemoAstroPlanet::~DemoAstroPlanet() {
}
