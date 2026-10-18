#include "Game/Screen/SimpleLayout.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TalkTextFormer.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ObjUtil.hpp"

SimpleLayout::SimpleLayout(const char* pName, const char* pLayoutName, u32 a3, int drawType) : LayoutActor(pName, true) {
    int type = drawType >= 0 ? drawType : MR::DrawType_Layout;

    MR::connectToScene(this, MR::MovementType_Layout, MR::CalcAnimType_Layout, MR::DrawBufferType_None, type);
    initLayoutManager(pLayoutName, a3);
    initActionSound(4, nullptr, nullptr);
}

SimpleEffectLayout::SimpleEffectLayout(const char* pName, const char* pLayoutName, u32 a3, int drawType)
    : SimpleLayout(pName, pLayoutName, a3, drawType) {
    initEffectKeeper(0, nullptr, nullptr);
}


inline void SimpleTalkBalloon::exeWait() {
    mTextFormer->updateTalking();
}

namespace NrvSimpleTalkBalloon {
    NEW_NERVE(SimpleTalkBalloonNrvAppear, SimpleTalkBalloon, Appear);
    NEW_NERVE(SimpleTalkBalloonNrvWait, SimpleTalkBalloon, Wait);
    NEW_NERVE(SimpleTalkBalloonNrvDisappear, SimpleTalkBalloon, Disappear);
}

SimpleTalkBalloon::SimpleTalkBalloon(const char* name, const char* layout, u32 animCount)
    : SimpleLayout(name, layout, animCount, 0x51), mTextFormer(nullptr) {
    mTextFormer = new TalkTextFormer(this, "Text00");
    initNerve(&NrvSimpleTalkBalloon::SimpleTalkBalloonNrvAppear::sInstance);
    kill();
}

SimpleTalkBalloon::~SimpleTalkBalloon() {}

void SimpleTalkBalloon::appearMessage(const char* message) {
    MR::startAnimAndSetFrameAndStop(this, "Appear", 0.0f, 0);
    MR::hidePane(this, "ShaBeak");
    MR::hidePane(this, "PicBeak");
    if (message != nullptr) mTextFormer->formMessage(MR::getGameMessageDirect(message), 1);
    LayoutActor::appear();
    setNerve(&NrvSimpleTalkBalloon::SimpleTalkBalloonNrvAppear::sInstance);
}

void SimpleTalkBalloon::disappear() {
    setNerve(&NrvSimpleTalkBalloon::SimpleTalkBalloonNrvDisappear::sInstance);
}

void SimpleTalkBalloon::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "Appear", 0);
        MR::startActionSoundOneShot(this, "Appear", -1, -1);
    }
    if (MR::isAnimStopped(this, 0)) setNerve(&NrvSimpleTalkBalloon::SimpleTalkBalloonNrvWait::sInstance);
}


void SimpleTalkBalloon::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "End", 0);
        MR::startActionSoundOneShot(this, "End", -1, -1);
    }
    if (MR::isAnimStopped(this, 0)) kill();
}
