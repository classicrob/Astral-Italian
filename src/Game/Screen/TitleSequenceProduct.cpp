#include "Game/Screen/TitleSequenceProduct.hpp"
#include "Game/Demo/DemoTitleAnim.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util.hpp"
#include "Game/Screen/EncouragePal60Window.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/SystemUtil.hpp"
#include "Game/Util/TriggerChecker.hpp"

#ifdef GALAXY_NATIVE
// Progress telemetry and the automated smoke run (native/app); observation only.
#include <petari/milestone.hpp>
#define TITLE_SEQUENCE_MILESTONE(name) petari_milestone(name)
// Tells the keyboard layer the A+B prompt is up, so Return can press both.
#include <petari/input.hpp>
#define TITLE_SEQUENCE_AB_PROMPT() PetariNative::Input::titlePromptShown()
#else
#define TITLE_SEQUENCE_MILESTONE(name)
#define TITLE_SEQUENCE_AB_PROMPT()
#endif

namespace {
    static const s32 sPressABAppearFrame = 25;
};  // namespace

namespace TitleSequenceProductSub {
    LogoLayout::LogoLayout() : SimpleLayout("ロゴ", "TitleLogo", 2, -1) {
        initEffectKeeper(1, "TitleLogo", nullptr);
        kill();
    }
};  // namespace TitleSequenceProductSub

namespace NrvTitleSequenceProduct {
    NEW_NERVE(TitleSequenceProductNrvPal60, TitleSequenceProduct, Pal60);
    NEW_NERVE(TitleSequenceProductNrvBgmStop, TitleSequenceProduct, BgmStop);
    NEW_NERVE(TitleSequenceProductNrvDemoAnim, TitleSequenceProduct, DemoAnim);
    NEW_NERVE(TitleSequenceProductNrvLogoFadein, TitleSequenceProduct, LogoFadein);
    NEW_NERVE(TitleSequenceProductNrvLogoWait, TitleSequenceProduct, LogoWait);
    NEW_NERVE(TitleSequenceProductNrvLogoDisplay, TitleSequenceProduct, LogoDisplay);
    NEW_NERVE(TitleSequenceProductNrvDecide, TitleSequenceProduct, Decide);
    NEW_NERVE(TitleSequenceProductNrvDead, TitleSequenceProduct, Dead);
    class TitleSequenceProductNrvEnd : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static TitleSequenceProductNrvEnd sInstance;
    };
    TitleSequenceProductNrvEnd TitleSequenceProductNrvEnd::sInstance ATTRIBUTE_WEAK;
}

TitleSequenceProduct::TitleSequenceProduct()
    : NerveExecutor(""), mLogoLayout(nullptr), mPressStartLayout(nullptr), mEncouragePal60Window(nullptr), mAButtonChecker(nullptr),
      mBButtonChecker(nullptr), mDemoTitleAnim(nullptr) {
    bool isPal60 = VIGetTvFormat() == VI_PAL && !GameDataFunction::getSysConfigFileEncouragedPal16();
    if (isPal60) {
        mEncouragePal60Window = new EncouragePal60Window();
        mEncouragePal60Window->initWithoutIter();
        mEncouragePal60Window->kill();
        initNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvPal60), 0);
    } else {
        initNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvBgmStop), 0);
    }

    mLogoLayout = new TitleSequenceProductSub::LogoLayout();
    mPressStartLayout = new SimpleLayout("Press[A][B]", "PressStart", 1, -1);
    mPressStartLayout->initWithoutIter();
    mAButtonChecker = new TriggerChecker();
    mBButtonChecker = new TriggerChecker();
    mDemoTitleAnim = new DemoTitleAnim("タイトルアニメ");
    mDemoTitleAnim->initWithoutIter();
}

void TitleSequenceProduct::exePal60() {
    if (MR::isFirstStep(this)) {
        mEncouragePal60Window->appear();
        GameDataFunction::setSysConfigFileEncouragedPal16();
    }

    if (MR::isDead(mEncouragePal60Window)) {
        setNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvBgmStop));
    }
}

void TitleSequenceProduct::exeBgmStop() {
    if (MR::isFirstStep(this)) {
        MR::forceCloseWipeFade();
        MR::stopStageBGM(0);
    }
    setNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvDemoAnim));
}

void TitleSequenceProduct::exeDemoAnim() {
    if (MR::isFirstStep(this)) {
        mDemoTitleAnim->start();
        MR::openWipeFade(-1);
    }
    if (mDemoTitleAnim->isTitleLogoStep()) {
        setNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvLogoFadein));
    }
}

void TitleSequenceProduct::exeLogoFadein() {
    if (MR::isFirstStep(this)) {
        mLogoLayout->appear();
        MR::startAnim(mLogoLayout, "Appear", 0);
    }

    if (MR::isAnimStopped(mLogoLayout, 0)) {
        setNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvLogoWait));
    }
}

void TitleSequenceProduct::exeLogoWait() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(mLogoLayout, "Wait", 0);
        MR::emitEffect(mLogoLayout, "TitleLogoLightB");
        MR::emitEffect(mLogoLayout, "TitleLogoLightC");
        MR::emitEffect(mLogoLayout, "TitleLogoLightD");
        MR::emitEffect(mLogoLayout, "TitleLogoLightE");
        MR::emitEffect(mLogoLayout, "TitleLogoLightF");
        MR::emitEffect(mLogoLayout, "TitleLogoLightG");
        MR::emitEffect(mLogoLayout, "TitleLogoTicoLightA");
        MR::emitEffect(mLogoLayout, "TitleLogoTicoLightB");
        MR::emitEffect(mLogoLayout, "TitleLogoTicoLightC");
        MR::emitEffect(mLogoLayout, "TitleLogoTicoLightD");
    }

    if (MR::isStep(this, ::sPressABAppearFrame)) {
        setNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvLogoDisplay));
    }
}

void TitleSequenceProduct::exeLogoDisplay() {
    if (MR::isFirstStep(this)) {
        TITLE_SEQUENCE_MILESTONE("TitleSequence.LogoDisplay");
        mPressStartLayout->appear();
        MR::startAnim(mPressStartLayout, "Appear", 0);
    }

    if (MR::isAnimStopped(mPressStartLayout, 0)) {
        MR::startAnim(mPressStartLayout, "Wait", 0);
    }

    TITLE_SEQUENCE_AB_PROMPT();
    mAButtonChecker->update(MR::testCorePadButtonA(WPAD_CHAN0));
    mBButtonChecker->update(MR::testCorePadButtonB(WPAD_CHAN0));

    if (mAButtonChecker->getLevel() && mBButtonChecker->getLevel()) {
        MR::startActionSound(mPressStartLayout, "PressAB", -1, -1);
        MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        setNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvDecide));
    } else {
        if (mAButtonChecker->getOnTrigger()) {
            MR::startActionSound(mPressStartLayout, "PressA", -1, -1);
        }
        if (mBButtonChecker->getOnTrigger()) {
            MR::startActionSound(mPressStartLayout, "PressB", -1, -1);
        }
        updateButtonReaction(mAButtonChecker, "ReactionA");
        updateButtonReaction(mBButtonChecker, "ReactionB");
        updatePressStartReaction();
    }
}

void TitleSequenceProduct::exeDecide() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(mLogoLayout, "Decide", 0);
        MR::deleteEffectAll(mLogoLayout);
        MR::startAnim(mPressStartLayout, "End", 0);
    }

    if (MR::isAnimStopped(mLogoLayout, 0) && MR::isAnimStopped(mPressStartLayout, 0)) {
        setNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvDead));
    }
}

// NONMATCHING: compiler retains the optional PAL window pointer instead of reloading it.
inline bool hasPal60Window(const TitleSequenceProduct* sequence) { return sequence->mEncouragePal60Window != nullptr; }

void TitleSequenceProduct::exeDead() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeWhiteFade(-1);
    }

    if (MR::isWipeBlank()) {
        MR::openWipeWhiteFade(-1);
        mDemoTitleAnim->endAll();
        mDemoTitleAnim->kill();
        mLogoLayout->kill();
        mPressStartLayout->kill();
        if (hasPal60Window(this)) {
            mEncouragePal60Window->kill();
        }
        setNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvEnd));
    }
}

void TitleSequenceProduct::appear() {
    if (mEncouragePal60Window != nullptr) {
        setNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvPal60));
    } else {
        setNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvBgmStop));
    }
}

void TitleSequenceProduct::kill() {
    setNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvEnd));
}

bool TitleSequenceProduct::isActive() const {
    return !isNerve(GET_NERVE(TitleSequenceProduct, TitleSequenceProductNrvEnd));
}

void TitleSequenceProduct::updateButtonReaction(TriggerChecker* pButtonChecker, const char* pAnimName) {
    if (pButtonChecker->getOnTrigger()) {
        MR::startAnim(mLogoLayout, pAnimName, 1);
        MR::setAnimFrameAndStop(mLogoLayout, 0.0f, 1);
    } else if (pButtonChecker->getOffTrigger()) {
        MR::startAnim(mLogoLayout, pAnimName, 1);
    }
}

void TitleSequenceProduct::updatePressStartReaction() {
    if (mAButtonChecker->getOnTrigger()) {
        MR::startAnim(mPressStartLayout, "ButtonReaction", 0);
    } else if (mBButtonChecker->getOnTrigger()) {
        MR::startAnim(mPressStartLayout, "ButtonReaction", 0);
    }
}

bool TitleSequenceProduct::isBgmStartStep() const {
    if (mDemoTitleAnim == nullptr) {
        return false;
    }
    return mDemoTitleAnim->isBgmStartStep();
}
