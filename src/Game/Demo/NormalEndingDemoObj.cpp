#include "Game/Util/DemoUtil.hpp"
#include "Game/Demo/NormalEndingDemoObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/MapObj/PowerStar.hpp"
#include "Game/Demo/AstroDemoFunction.hpp"
#include "Game/Demo/DemoAstroPlanet.hpp"
#include "Game/Demo/UnnamedDemoCallee.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GalaxyWorldMapInfo.hpp"

// Name table at lbl_80680070 (object/anim names); per-scene name pointers.
extern "C" char lbl_80680070[];
extern "C" char lbl_80680394[];
extern "C" char lbl_806803A4[];
extern "C" char lbl_806803B8[];
extern "C" char lbl_806803C4[];
extern "C" const char* lbl_807CF730;
extern "C" const char* lbl_807CF734;
extern "C" const char* lbl_807CF738;
extern "C" const char* lbl_807CF73C;
extern "C" const char* lbl_807CF740;
extern "C" const char* lbl_807CF744;
extern "C" const char* lbl_807CF748;
extern "C" const char* lbl_807CF74C;
extern "C" const char* lbl_807CF750;
extern "C" char lbl_807D1E0C[];
extern "C" char lbl_807D1E08[];
extern "C" char lbl_807D1E10[];
extern "C" char lbl_807D1E14[];
extern "C" char lbl_807D1E18[];
extern "C" f32 lbl_807D97FC;


NormalEndingDemoObj::NormalEndingDemoObj(const char* pName) : LiveActor(pName), mField90(0), mActorA(nullptr), mActorB(nullptr), mActorC(nullptr), mStar(nullptr), mActorD(nullptr), mActorE(nullptr), mActorF(nullptr), mActorG(nullptr), mActorH(nullptr), mPlanet(nullptr), mActorI(nullptr), mActorJ(nullptr) {
}

// NONMATCHING: several helper arguments inferred from m2c; not verified
void NormalEndingDemoObj::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    MR::registerDemoCast(this, lbl_807CF730, rIter);
    MR::registerDemoCast(this, lbl_807CF734, rIter);
    MR::registerDemoCast(this, lbl_807CF738, rIter);
    MR::registerDemoCast(this, lbl_807CF73C, rIter);
    UnnamedDemoCallee::fn_8001F5B0(this, lbl_807D1E0C, lbl_807CF730, 0);
    mActorI = UnnamedDemoCallee::fn_8013FE00(this, lbl_80680070 + 0xCC, lbl_80680070 + 0xE8);
    MR::tryRegisterDemoCast(mActorI, lbl_807CF730, rIter);
    MR::tryRegisterDemoCast(mActorI, lbl_807CF734, rIter);
    MR::tryRegisterDemoCast(mActorI, lbl_807CF738, rIter);
    MR::initTalkAnimDemoCast(mActorI, rIter, lbl_807CF73C, lbl_807CF74C);
    mActorJ = UnnamedDemoCallee::fn_8013FE00(this, lbl_80680070 + 0xFC, lbl_80680070 + 0x118);
    MR::initTalkAnimDemoCast(mActorJ, rIter, lbl_807CF730, lbl_807CF740);
    MR::tryRegisterDemoCast(mActorJ, lbl_807CF734, rIter);
    MR::initTalkAnimDemoCast(mActorJ, rIter, lbl_807CF738, lbl_807CF748);
    MR::tryRegisterDemoCast(mActorJ, lbl_807CF73C, rIter);
    mActorA = UnnamedDemoCallee::fn_8013FD90(this, lbl_80680070 + 0x12C, lbl_80680070 + 0x13C, lbl_80680070 + 0x148, 0);
    MR::initLightCtrl(mActorA);
    MR::tryInitDemoSheetAnim(mActorA, rIter, lbl_807CF730, lbl_807CF740);
    MR::tryInitDemoSheetAnim(mActorA, rIter, lbl_807CF734, lbl_807CF744);
    MR::tryInitDemoSheetAnim(mActorA, rIter, lbl_807CF738, lbl_807CF748);
    MR::tryInitDemoSheetAnim(mActorA, rIter, lbl_807CF73C, lbl_807CF74C);
    mActorB = UnnamedDemoCallee::fn_8013FD90(this, lbl_80680070 + 0x158, lbl_80680070 + 0x168, lbl_80680070 + 0x170, 0);
    MR::initTalkAnimDemoCast(mActorB, rIter, lbl_807CF730, lbl_807CF740);
    MR::initTalkAnimDemoCast(mActorB, rIter, lbl_807CF734, lbl_807CF744);
    MR::initTalkAnimDemoCast(mActorB, rIter, lbl_807CF738, lbl_807CF748);
    MR::initTalkAnimDemoCast(mActorB, rIter, lbl_807CF73C, lbl_807CF74C);
    mActorC = UnnamedDemoCallee::fn_8013FD20(this, lbl_80680070 + 0x18C, lbl_80680070 + 0x1A0, lbl_80680070 + 0x1A8, nullptr);
    MR::initTalkAnimDemoCast(mActorC, rIter, lbl_807CF734, lbl_807CF744);
    MR::initTalkAnimDemoCast(mActorC, rIter, lbl_807CF738, lbl_807CF748);
    mStar = reinterpret_cast< PowerStar* >(UnnamedDemoCallee::fn_8013FD20(this, lbl_80680070 + 0x1B8, lbl_80680070 + 0x1D0, lbl_80680070 + 0x1DC, nullptr));
    MR::initTalkAnimDemoCast(mStar, rIter, lbl_807CF730, lbl_807CF740);
    mActorD = UnnamedDemoCallee::fn_8013FD90(this, lbl_80680070 + 0x1E4, lbl_80680070 + 0x1F8, lbl_80680070 + 0x200, 0);
    MR::initTalkAnimDemoCast(mActorD, rIter, lbl_807CF734, lbl_807CF744);
    MR::initTalkAnimDemoCast(mActorD, rIter, lbl_807CF738, lbl_807CF748);
    MR::initTalkAnimDemoCast(mActorD, rIter, lbl_807CF73C, lbl_807CF74C);
    mActorE = UnnamedDemoCallee::fn_8013FD20(this, lbl_80680070 + 0x220, lbl_80680070 + 0x230, lbl_80680070 + 0x1A8, MR::getJointMtx(mActorI, lbl_80680070 + 0xE8));
    mActorF = UnnamedDemoCallee::fn_8013FCC0(this, lbl_80680070 + 0x248, lbl_80680070 + 0x25C, nullptr);
    MR::registerDemoSimpleCastAll(mActorF);
    MR::initTalkAnimDemoCast(mActorF, rIter, lbl_807CF73C, lbl_807CF74C);
    mActorG = MR::createDemoObj(lbl_80680070 + 0x270, lbl_80680070 + 0x280, lbl_80680070 + 0x1DC, nullptr);
    MR::invalidateClipping(mActorG);
    MR::registerDemoSimpleCastAll(mActorG);
    MR::initTalkAnimDemoCast(mActorG, rIter, lbl_807CF73C, lbl_807CF74C);
    mActorH = UnnamedDemoCallee::fn_8013FCC0(this, lbl_80680070 + 0x294, lbl_80680070 + 0x2A0, MR::getJointMtx(mActorJ, lbl_80680070 + 0x2B0));
    MR::initTalkAnimDemoCast(mActorH, rIter, lbl_807CF730, lbl_807CF740);
    MR::initTalkAnimDemoCast(mActorH, rIter, lbl_807CF738, lbl_807CF748);
    mPlanet = new DemoAstroPlanet(lbl_80680070 + 0x2BC, MR::getJointMtx(mActorJ, lbl_80680070 + 0x2CC));
    mPlanet->initWithoutIter();
    MR::invalidateClipping(mPlanet);
    MR::registerDemoSimpleCastAll(mPlanet);
    MR::initTalkAnimDemoCast(mPlanet, rIter, lbl_807CF730, lbl_807CF740);
    MR::initTalkAnimDemoCast(mPlanet, rIter, lbl_807CF738, lbl_807CF748);
    initNerve(reinterpret_cast< Nerve* >(lbl_807D1E08), 0);
    makeActorAppeared();
}

void NormalEndingDemoObj::initAfterPlacement() {
    MR::resetPosition(mActorA, lbl_807CF750);
    MR::resetPosition(mActorB, lbl_807CF750);
    MR::resetPosition(mActorC, lbl_807CF750);
    MR::resetPosition(mStar, lbl_807CF750);
    MR::resetPosition(mActorD, lbl_807CF750);
    MR::resetPosition(mActorE, lbl_807CF750);
    MR::resetPosition(mActorF, lbl_807CF750);
    MR::resetPosition(mActorG, lbl_807CF750);
    MR::resetPosition(mActorH, lbl_807CF750);
    MR::resetPosition(mPlanet, lbl_807CF750);
    MR::resetPosition(mActorI, lbl_807CF750);
    MR::resetPosition(mActorJ, lbl_807CF750);
    UnnamedDemoCallee::fn_8013FE60(1);
    MR::endStartPosCamera();
}

// Nerve 1E0C.. : shows the cast, then hands off to exeB.
// NONMATCHING: mixes LiveActor/PowerStar calls whose vtable slots are inferred
void NormalEndingDemoObj::exeA() {
    if (MR::isFirstStep(this)) {
        mActorA->makeActorAppeared();
        mActorB->makeActorAppeared();
        mActorC->makeActorAppeared();
        mStar->setupColorAtResultSequence(this, true);
        mStar->makeActorAppeared();
        mActorD->makeActorAppeared();
        mActorE->makeActorAppeared();
        mActorF->makeActorAppeared();
        MR::startAction(mActorF, lbl_80680070 + 0x2D8);
        mActorG->makeActorAppeared();
        MR::startAction(mActorG, lbl_80680070 + 0x2D8);
        mActorJ->makeActorAppeared();
        mActorH->makeActorAppeared();
        MR::startBtk(mActorH, lbl_80680070 + 0x2E8);
        mPlanet->makeActorAppeared();
        MR::startBtk(mPlanet, lbl_80680070 + 0x2E8);
        MR::startBrk(mPlanet, lbl_80680070 + 0x2E8);
        mActorI->makeActorAppeared();
        MR::startAction(mActorI, lbl_80680070 + 0x2D8);
        UnnamedDemoCallee::fn_8013FE60(1);
        MR::openSystemWipeWhiteFade(0xB4);
    }
    if (MR::isDemoActive(lbl_807CF730)) {
        if (MR::isDemoPartStep(lbl_80680070 + 0x2FC, 0x1A2) && UnnamedDemoCallee::fn_80147730() != 0) {
            UnnamedDemoCallee::fn_8013FE60(2);
        }
        if (MR::isDemoPartStep(lbl_80680070 + 0x310, 0x104)) {
            *reinterpret_cast< s32* >(reinterpret_cast< char* >(mPlanet) + 0xA0) = 1;
            UnnamedDemoCallee::fn_8013FE60(1);
        }
    }
    if (MR::isDemoLastStep()) {
        UnnamedDemoCallee::fn_80147860();
        UnnamedDemoCallee::fn_80020110(this, lbl_807CF734, 0);
        setNerve(reinterpret_cast< Nerve* >(lbl_807D1E10));
    }
}

// NONMATCHING: vtable slot 0x38 inferred as makeActorDead
void NormalEndingDemoObj::exeB() {
    if (MR::isFirstStep(this)) {
        mStar->makeActorDead();
    }
    if (MR::isDemoActive(lbl_807CF734)) {
        if (MR::isDemoPartStep(lbl_80680394, 0)) {
            UnnamedDemoCallee::fn_8013FE60(3);
        }
    }
    if (MR::isDemoLastStep()) {
        UnnamedDemoCallee::fn_80147860();
        UnnamedDemoCallee::fn_80020110(this, lbl_807CF738, 0);
        setNerve(reinterpret_cast< Nerve* >(lbl_807D1E14));
    }
}

// NONMATCHING: planet field offsets written as raw casts (DemoAstroPlanet not fully declared)
void NormalEndingDemoObj::exeC() {
    if (MR::isFirstStep(this)) {
        *reinterpret_cast< f32* >(reinterpret_cast< char* >(mPlanet) + 0x9C) = lbl_807D97FC;
        *reinterpret_cast< s32* >(reinterpret_cast< char* >(mPlanet) + 0xA0) = 0;
        MR::startBtk(mActorB, lbl_806803A4);
        MR::startBtk(mPlanet, lbl_806803A4);
        MR::startBrk(mPlanet, lbl_806803A4);
        UnnamedDemoCallee::fn_8013FE60(3);
    }
    if (MR::isDemoActive(lbl_807CF738)) {
        if (MR::isDemoPartStep(lbl_806803B8, 0x159)) {
            UnnamedDemoCallee::fn_8013FE60(1);
        }
        if (MR::isDemoPartStep(lbl_806803B8, 0x29D)) {
            UnnamedDemoCallee::fn_8013FE60(4);
        }
        if (MR::isDemoPartStep(lbl_806803B8, 0x30B)) {
            UnnamedDemoCallee::fn_8013FE60(1);
        }
    }
    if (MR::isDemoLastStep()) {
        UnnamedDemoCallee::fn_80147860();
        UnnamedDemoCallee::fn_80020110(this, lbl_807CF73C, 0);
        setNerve(reinterpret_cast< Nerve* >(lbl_807D1E18));
    }
}

// NONMATCHING: isDemoPartStep result is not passed through (target passes it to the sequence call)
void NormalEndingDemoObj::checkStaffRollStart(const char* pName) {
    if (MR::isDemoPartActive(pName)) {
        if (MR::isDemoPartStep(pName, MR::getDemoPartTotalStep(pName) - 2)) {
            MR::startSystemWipeCircleWithCaptureScreen(0xB4);
            GameSequenceFunction::requestChangeStageStaffRoll();
        }
    }
}

void NormalEndingDemoObj::kill() {
    mActorG->kill();
    mActorH->kill();
    mActorE->kill();
    mActorF->kill();
    mActorI->kill();
    mStar->kill();
    mActorD->kill();
    mActorJ->kill();
    mActorB->kill();
    LiveActor::kill();
}

NormalEndingDemoObj::~NormalEndingDemoObj() {
}
