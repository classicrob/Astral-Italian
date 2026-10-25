#include "Game/Demo/StoryBookDemo.hpp"
#include "Game/Demo/DemoDirector.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/AreaObj/AreaObj.hpp"
#include "Game/Map/StoryBookText.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>
#include <JSystem/JUtility/JUTVideo.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <cstdio>

class StoryBookShadow : public LiveActor {
public:
    StoryBookShadow(const char* name) : LiveActor(name) {}
    virtual ~StoryBookShadow() {}
    virtual void calcAndSetBaseMtx() {}
};
namespace NrvStoryBookDemo {
    NEW_NERVE(Wait, StoryBookDemo, Wait);
    NEW_NERVE(AreaText, StoryBookDemo, AreaText);
    NEW_NERVE(WarpWait, StoryBookDemo, WarpWait);
    NEW_NERVE(WarpStart, StoryBookDemo, WarpStart);
    NEW_NERVE(WarpPlayer, StoryBookDemo, WarpPlayer);
    NEW_NERVE(WarpCapture, StoryBookDemo, WarpCapture);
    NEW_NERVE(WarpTurn, StoryBookDemo, WarpTurn);
    NEW_NERVE(WarpEnd, StoryBookDemo, WarpEnd);
    NEW_NERVE(WaitIcon, StoryBookDemo, WaitIcon);
    NEW_NERVE(Text, StoryBookDemo, Text);
    NEW_NERVE(CloseText, StoryBookDemo, CloseText);
    NEW_NERVE(DemoText, StoryBookDemo, Text);
    NEW_NERVE(DemoCloseText, StoryBookDemo, CloseText);
    NEW_NERVE(WaitDemo, StoryBookDemo, WaitDemo);
    NEW_NERVE(TurnStart, StoryBookDemo, TurnStart);
    NEW_NERVE(TurnCapture, StoryBookDemo, TurnCapture);
    NEW_NERVE(TurnEnd, StoryBookDemo, TurnEnd);
    NEW_NERVE(TurnPaused, StoryBookDemo, TurnStart);
}
namespace {
    const char* sWarpDemo = "絵本瞬間移動デモ";
    const char* sCurrentPage = "DummyPageCurrent";
    const char* sNextPage = "DummyPageNext";
    const char* sWarpPoint = "絵本移動ポイント";
}
// NONMATCHING: regalloc
StoryBookDemo::StoryBookDemo() : LiveActor("遊べる絵本"), mArea(0), mText(0), mCurrentTexture(0), mNextTexture(0), mShadow(0), mCaptureRequested(false), mMessageIndex(-1), mTextFrames(-1) {
    {
        MR::CurrentHeapRestorer restore(MR::getSceneHeapGDDR3());
        mCurrentTexture = new JUTTexture(MR::getFrameBufferWidth(), MR::getFrameBufferHeight(), GX_TF_RGB565);
        mNextTexture = new JUTTexture(MR::getFrameBufferWidth(), MR::getFrameBufferHeight(), GX_TF_RGB565);
    }
    s32 height = MR::getFrameBufferHeight();
    s32 width = MR::getFrameBufferWidth();
    width *= height;
    s32 size = width * 2;
    MR::zeroMemory(mCurrentTexture->mImage, size);
    MR::zeroMemory(mNextTexture->mImage, size);
    MR::createDrawAdaptor("絵本用キャプチャ", 0x33, MR::FunctorV0M<StoryBookDemo*, void (StoryBookDemo::*)()>(this, &StoryBookDemo::capture));
}
s32 MR::getFrameBufferHeight() { return JUTVideo::getManager()->getEfbHeight(); }
void StoryBookDemo::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "StoryBook", true);
    MR::initDLMakerChangeTex(this, sCurrentPage);
    MR::initDLMakerChangeTex(this, sNextPage);
    MR::newDifferedDLBuffer(this);
    mText = new StoryBookText;
    mText->initWithoutIter();
    MR::registerStoryBookDemoCast(this);
    if (MR::isScreen16Per9()) MR::startBtkAndSetFrameAndStop(this, "Ratio", 0.0f);
    else MR::startBtkAndSetFrameAndStop(this, "Ratio", 1.0f);
    MR::startAction(this, "StartPage");
    if (MR::isEqualStageName("StaffRollGalaxy")) {
        mShadow = new StoryBookShadow("遊べる絵本スタッフロールフィルター");
        MR::initActor(mShadow, iter, "StoryBookShadow", false);
        MR::invalidateClipping(mShadow);
        mShadow->makeActorAppeared();
    }
    initNerve(&NrvStoryBookDemo::Wait::sInstance, 0);
    makeActorAppeared();
}
void StoryBookDemo::startText(bool demo) {
    if (demo) { ++mMessageIndex; setNerve(&NrvStoryBookDemo::DemoText::sInstance); }
    else setNerve(&NrvStoryBookDemo::WaitIcon::sInstance);
}
void StoryBookDemo::startTurnPage() {
    if (isNerve(&NrvStoryBookDemo::TurnPaused::sInstance)) {
        MR::setBckRate(this, 1.0f);
        MR::setBvaRate(this, 1.0f);
        MR::resetCameraMan();
        setNerve(&NrvStoryBookDemo::TurnCapture::sInstance);
        return;
    }
    mText->kill();
    setNerve(&NrvStoryBookDemo::TurnStart::sInstance);
}
bool StoryBookDemo::isActive() const {
    bool active = false;
    if (isNerve(&NrvStoryBookDemo::DemoCloseText::sInstance) || isNerve(&NrvStoryBookDemo::TurnStart::sInstance) || isNerve(&NrvStoryBookDemo::TurnCapture::sInstance) || isNerve(&NrvStoryBookDemo::TurnEnd::sInstance)) active = true;
    return active;
}
bool StoryBookDemo::isTurnPageEnd() const {
    if (isNerve(&NrvStoryBookDemo::TurnEnd::sInstance)) {
        f32 frame = MR::getBckFrame(this);
        return MR::getBckFrameMax(this) - 1.0f <= frame;
    }
    return false;
}
bool StoryBookDemo::isTurningPage() const {
    if (isNerve(&NrvStoryBookDemo::TurnCapture::sInstance)) return true;
    return isNerve(&NrvStoryBookDemo::TurnEnd::sInstance);
}
bool StoryBookDemo::isTurnPageStart() const {
    if (isNerve(&NrvStoryBookDemo::TurnCapture::sInstance)) return MR::isStep(this, 1);
    return false;
}
void StoryBookDemo::exeWait() {
    mArea = MR::getAreaObjAtPlayerPos("StoryBookAreaText");
    if (mArea) { setNerve(&NrvStoryBookDemo::AreaText::sInstance); return; }
    mArea = MR::getAreaObjAtPlayerPos("StoryBookAreaWarp");
    if (mArea) setNerve(&NrvStoryBookDemo::WarpWait::sInstance);
}
void StoryBookDemo::exeAreaText() {
    if (MR::isFirstStep(this)) {
        AreaObj* area = mArea;
        mText->setMessageWithoutIcon(MR::getAreaObjArg(area, 0), area->mLinkInfo.mZoneID);
    }
    if (MR::getAreaObjAtPlayerPos("StoryBookAreaText") != mArea) {
        mText->requestClose(); setNerve(&NrvStoryBookDemo::Wait::sInstance);
    }
}
void StoryBookDemo::exeWarpStart() {
    if (MR::isFirstStep(this)) {
        DemoFunction::getDemoDirector()->movementOffAllCasts();
        MR::requestMovementOnSoundAreas();
        MR::requestPlayerMovementOnDuringDemo();
        MR::requestSupportTicoMovementOn();
        MR::startAction(this, "TurnPage");
        mCaptureRequested = true;
    }
    if (MR::isStep(this, 1)) {
        MR::changeModelDataTexAll(this, sCurrentPage, *mCurrentTexture->mTIMG);
        setNerve(&NrvStoryBookDemo::WarpPlayer::sInstance);
    }
}
void StoryBookDemo::exeWarpPlayer() {
    char name[0x40];
    snprintf(name, sizeof(name), "%s%03d", sWarpPoint, MR::getAreaObjArg(mArea, 0));
    MR::setPlayerPosAndWait(name, false);
    MR::requestSupportTicoHide();
    s32 warpPlayer = MR::getAreaObjArg(mArea, 1) == 0;
    if (warpPlayer) {
        MR::startBckPlayer("GoThrough", 0);
        MR::setBckRatePlayer(0.0f);
    }
    setNerve(&NrvStoryBookDemo::WarpCapture::sInstance);
}
void StoryBookDemo::exeWarpCapture() {
    if (MR::isFirstStep(this)) MR::resetCameraMan();
    if (MR::isStep(this, 4)) { mCaptureRequested = true; MR::requestEffectStopSceneStartAndDelete(); }
    if (MR::isStep(this, 5)) {
        MR::changeModelDataTexAll(this, sNextPage, *mNextTexture->mTIMG);
        setNerve(&NrvStoryBookDemo::WarpTurn::sInstance);
    }
}
void StoryBookDemo::exeWarpTurn() {
    if (MR::isActionEnd(this)) {
        s32 warpPlayer = MR::getAreaObjArg(mArea, 1) == 0;
        if (warpPlayer) { setNerve(&NrvStoryBookDemo::WarpEnd::sInstance); return; }
        DemoFunction::getDemoDirector()->movementOnAllCasts();
        MR::requestSupportTicoShow();
        MR::requestEffectStopSceneEnd();
        MR::endDemo(this, sWarpDemo);
        setNerve(&NrvStoryBookDemo::Wait::sInstance);
    }
}
void StoryBookDemo::exeWarpEnd() {
    if (MR::isFirstStep(this)) MR::startBckPlayer("GoThrough", 0);
    if (MR::isBckStoppedPlayer()) {
        DemoFunction::getDemoDirector()->movementOnAllCasts();
        MR::requestSupportTicoShow(); MR::requestEffectStopSceneEnd();
        MR::endDemo(this, sWarpDemo); setNerve(&NrvStoryBookDemo::Wait::sInstance);
    }
}
void StoryBookDemo::exeWaitIcon() {
    if (MR::isPauseTimeKeepDemo(this)) {
        if (mText->isIconDead()) mText->openWithIcon();
        if (mText->isNerveOpenIcon()) {
            if (MR::testCorePadTriggerA(0)) { MR::startActionSound(this, "SelectOK", -1, -1, -1); setNerve(&NrvStoryBookDemo::DemoCloseText::sInstance); }
        } else mText->tryOpenIcon();
    }
}
void StoryBookDemo::exeText() {
    if (MR::isFirstStep(this)) mText->setMessageWithIcon(mMessageIndex);
    if (mTextFrames != -1 && MR::isStep(this, mTextFrames)) {
        mTextFrames = -1;
        if (isNerve(&NrvStoryBookDemo::Text::sInstance)) setNerve(&NrvStoryBookDemo::CloseText::sInstance);
        else setNerve(&NrvStoryBookDemo::DemoCloseText::sInstance);
    }
    if (mText->hasNextPage()) {
        if (mText->isNerveOpenIcon()) {
            if (MR::testCorePadTriggerA(0)) { MR::startActionSound(this, "SelectNext", -1, -1, -1); mText->nextPage(); }
            return;
        }
        mText->tryOpenIcon();
    }
    if (MR::isPauseTimeKeepDemo(this)) {
        if (mText->isNerveOpenIcon()) {
            if (MR::testCorePadTriggerA(0)) {
                MR::startActionSound(this, "SelectOK", -1, -1, -1);
                if (isNerve(&NrvStoryBookDemo::Text::sInstance)) { setNerve(&NrvStoryBookDemo::CloseText::sInstance); return; }
                setNerve(&NrvStoryBookDemo::DemoCloseText::sInstance);
            }
        } else mText->tryOpenIcon();
    }
}
void StoryBookDemo::exeCloseText() {
    if (MR::isFirstStep(this)) mText->requestClose();
    if (MR::isDead(mText)) {
        if (isNerve(&NrvStoryBookDemo::CloseText::sInstance)) { setNerve(&NrvStoryBookDemo::Wait::sInstance); return; }
        setNerve(&NrvStoryBookDemo::TurnStart::sInstance);
    }
}
// NONMATCHING: regalloc
namespace {
    inline bool isShortTurnPageDemo() {
        if (MR::isDemoActive("ピーチ誘拐") && MR::isDemoSubPartLastStep()) {
            return true;
        }
        return MR::isDemoActive("スピンドライバ初出");
    }
}  // namespace

void StoryBookDemo::exeTurnStart() {
    if (MR::isFirstStep(this)) {
        DemoFunction::getDemoDirector()->movementOffAllCasts();
        MR::requestMovementOnSoundAreas(); MR::requestPlayerMovementOnDuringDemo(); MR::requestSupportTicoMovementOn();
        if (MR::isEqualStageName("StaffRollGalaxy")) MR::startAction(this, "TurnPageChargeMore");
        else if (MR::isPauseTimeKeepDemo("ベビチコ出会い")) MR::startAction(this, "TurnPageCharge");
        else {
            if (isShortTurnPageDemo()) MR::startAction(this, "TurnPageShort");
            else MR::startAction(this, "TurnPage");
        }
        mCaptureRequested = true;
    }
    if (MR::isStep(this, 1)) {
        MR::changeModelDataTexAll(this, sCurrentPage, *mCurrentTexture->mTIMG);
        if (isNerve(&NrvStoryBookDemo::TurnPaused::sInstance)) { MR::stopBck(this); MR::stopBva(this); return; }
        setNerve(&NrvStoryBookDemo::TurnCapture::sInstance);
    }
}
void StoryBookDemo::exeTurnCapture() {
    if (MR::isStep(this, 4)) { MR::requestEffectStopSceneStartAndDelete(); MR::killCenterScreenBlur(); mCaptureRequested = true; }
    if (MR::isStep(this, 5)) {
        MR::changeModelDataTexAll(this, sNextPage, *mNextTexture->mTIMG);
        setNerve(&NrvStoryBookDemo::TurnEnd::sInstance);
    }
}
void StoryBookDemo::exeTurnEnd() {
    if (MR::isActionEnd(this)) {
        DemoFunction::getDemoDirector()->movementOnAllCasts(); MR::requestEffectStopSceneEnd();
        setNerve(&NrvStoryBookDemo::Wait::sInstance);
    }
}
void StoryBookDemo::calcAnim() { LiveActor::calcAnim(); if (mShadow) mShadow->calcAnim(); }
// NONMATCHING: regalloc
namespace {
    inline void calcStoryBookCameraPosition(TVec3f* pos, const TVec3f& camZ, const TVec3f& camPos) {
        JMAVECScaleAdd(&camZ, &camPos, pos, 174.6f);
    }
    inline void addStoryBookCameraUp(TVec3f* pos, const TVec3f& camY) {
        JMAVECScaleAdd(&camY, pos, pos, 0.05f);
    }
}

void StoryBookDemo::calcAndSetBaseMtx() {
    TPos3f mtx;
    TVec3f pos;
    pos.x = 0.0f; pos.y = 0.0f; pos.z = 0.0f;
    calcStoryBookCameraPosition(&pos, MR::getCamZdir(), MR::getCamPos());
    addStoryBookCameraUp(&pos, MR::getCamYdir());
    mtx.setTrans(pos);
    MR::makeMtxUpFrontPos(&mtx, MR::getCamYdir(), -MR::getCamZdir(), pos);
    MR::setBaseTRMtx(this, mtx);
    if (mShadow) MR::setBaseTRMtx(mShadow, mtx);
}
bool StoryBookDemo::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgStartDemo(msg)) { mMessageIndex = -1; return true; }
    return false;
}
void StoryBookDemo::capture() {
    if (mCaptureRequested) {
        if (isNerve(&NrvStoryBookDemo::WarpStart::sInstance) || isNerve(&NrvStoryBookDemo::TurnStart::sInstance) || isNerve(&NrvStoryBookDemo::TurnPaused::sInstance)) mCurrentTexture->capture(0, 0, GX_TF_RGB565, false, 0);
        else if (isNerve(&NrvStoryBookDemo::WarpCapture::sInstance) || isNerve(&NrvStoryBookDemo::TurnCapture::sInstance)) mNextTexture->capture(0, 0, GX_TF_RGB565, false, 0);
        mCaptureRequested = false;
    }
}
// NONMATCHING: regalloc
bool MR::registerStoryBookDemoCast(LiveActor* pActor) {
    JMapInfoIter iter(0, -1);
    bool registered = false;
    registered |= MR::tryRegisterDemoCast(pActor, "ピーチからの手紙", iter);
    registered |= MR::tryRegisterDemoCast(pActor, "ベビチコ出会い", iter);
    registered |= MR::tryRegisterDemoCast(pActor, "ピーチ誘拐", iter);
    registered |= MR::tryRegisterDemoCast(pActor, "スピンドライバ初出", iter);
    registered |= MR::tryRegisterDemoCast(pActor, "クッパ最終戦突入", iter);
    registered |= MR::tryRegisterDemoCast(pActor, "ノーマルエンディング１", iter);
    registered |= MR::tryRegisterDemoCast(pActor, "ノーマルエンディング２", iter);
    registered |= MR::tryRegisterDemoCast(pActor, "ノーマルエンディング３", iter);
    registered |= MR::tryRegisterDemoCast(pActor, "ノーマルエンディング４", iter);
    return registered;
}
void MR::stopStoryBookDemoCasts() {
    DemoFunction::getDemoDirector()->movementOffAllCasts(); MR::requestMovementOnSoundAreas(); MR::requestPlayerMovementOnDuringDemo(); MR::requestSupportTicoMovementOn();
}
void MR::requestMovementOnStoryBook() {
    if (MR::isExistSceneObj(0x89)) {
        StoryBookDemo* book = MR::getSceneObj<StoryBookDemo*>(0x89);
        MR::requestMovementOn(book); MR::requestMovementOn(book->mText);
    }
}
void MR::startStoryBookText(bool demo) { MR::getSceneObj<StoryBookDemo*>(0x89)->startText(demo); }
void MR::waitStoryBookDemo() { MR::getSceneObj<StoryBookDemo*>(0x89)->setNerve(&NrvStoryBookDemo::WaitDemo::sInstance); }
void MR::startStoryBookTimedText(s32 frames) {
    StoryBookDemo* book = MR::getSceneObj<StoryBookDemo*>(0x89);
    book->mTextFrames = frames; ++book->mMessageIndex; book->setNerve(&NrvStoryBookDemo::Text::sInstance);
}
bool MR::isStoryBookDemoName(const char* name) {
    if (!MR::isStageStoryBook()) return false;
    if (MR::isEqualString(name, "ピーチからの手紙")) return false;
    if (MR::isEqualString(name, "クッパ最終戦突入")) return false;
    return !MR::isEqualString(name, "ノーマルエンディング１");
}
bool MR::isStoryBookEndingDemoName(const char* name) {
    if (!MR::isStageStoryBook()) return false;
    if (MR::isEqualString(name, "ノーマルエンディング１")) return true;
    if (MR::isEqualString(name, "ノーマルエンディング２")) return true;
    if (MR::isEqualString(name, "ノーマルエンディング３")) return true;
    return MR::isEqualString(name, "ノーマルエンディング４");
}
bool MR::isStoryBookTextAppeared() { if (!MR::isExistSceneObj(0x89)) return false; return !MR::isDead(MR::getSceneObj<StoryBookDemo*>(0x89)->mText); }
bool MR::isStoryBookDemoActive() { if (!MR::isExistSceneObj(0x89)) return false; return MR::getSceneObj<StoryBookDemo*>(0x89)->isActive(); }
bool MR::isStoryBookTurnPaused() { if (!MR::isExistSceneObj(0x89)) return false; return MR::getSceneObj<StoryBookDemo*>(0x89)->isNerve(&NrvStoryBookDemo::TurnPaused::sInstance); }
bool MR::isStoryBookTurnPageEnd() { if (!MR::isExistSceneObj(0x89)) return false; return MR::getSceneObj<StoryBookDemo*>(0x89)->isTurnPageEnd(); }
bool MR::isStoryBookTurningPage() { if (!MR::isExistSceneObj(0x89)) return false; return MR::getSceneObj<StoryBookDemo*>(0x89)->isTurningPage(); }
bool MR::isStoryBookTurnPageStart() { if (!MR::isExistSceneObj(0x89)) return false; return MR::getSceneObj<StoryBookDemo*>(0x89)->isTurnPageStart(); }
void MR::openStoryBookTextIcon() { MR::getSceneObj<StoryBookDemo*>(0x89)->mText->openWithIcon(); }
void MR::termStoryBookTextIcon() { MR::getSceneObj<StoryBookDemo*>(0x89)->mText->termIcon(); }
void MR::startStoryBookTurnPage() { MR::getSceneObj<StoryBookDemo*>(0x89)->startTurnPage(); }
void MR::pauseStoryBookTurnPage() {
    StoryBookDemo* book = MR::getSceneObj<StoryBookDemo*>(0x89);
    book->mText->kill(); book->setNerve(&NrvStoryBookDemo::TurnPaused::sInstance);
}
void MR::startStoryBookAppear() {
    if (MR::isDead(MR::getSceneObj<StoryBookDemo*>(0x89))) MR::getSceneObj<StoryBookDemo*>(0x89)->appear();
    MR::startBrk(MR::getSceneObj<StoryBookDemo*>(0x89), "Appear");
}
// NONMATCHING: regalloc
void MR::endStoryBookAppear() {
    MR::startStoryBookAppear();
    StoryBookDemo* a = MR::getSceneObj<StoryBookDemo*>(0x89);
    StoryBookDemo* b = MR::getSceneObj<StoryBookDemo*>(0x89);
    MR::setBrkFrameAndStop(b, MR::getBrkFrameMax(a));
}
void MR::resetStoryBookAppear() { MR::startBrkAndSetFrameAndStop(MR::getSceneObj<StoryBookDemo*>(0x89), "Appear", 0.0f); }
StoryBookDemo::~StoryBookDemo() {}
void StoryBookDemo::exeWarpWait() { if (MR::tryStartDemoMarioPuppetableWithoutCinemaFrame(this, sWarpDemo)) setNerve(&NrvStoryBookDemo::WarpStart::sInstance); }
void StoryBookDemo::exeWaitDemo() { if (MR::isPauseTimeKeepDemo(this)) setNerve(&NrvStoryBookDemo::TurnStart::sInstance); }
