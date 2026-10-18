#include "Game/Screen/TitleSequenceProduct.hpp"
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

namespace {
    static const s32 sPressABAppearFrame = 25;
};  // namespace

namespace TitleSequenceProductSub {
    LogoLayout::LogoLayout() : SimpleLayout("ロゴ", "TitleLogo", 2, -1) {
        initEffectKeeper(1, "TitleLogo", nullptr);
        kill();
    }
};  // namespace TitleSequenceProductSub

class DemoTitleAnim : public LiveActor {
public:
    DemoTitleAnim(const char*);
    u8 _pad[168 - sizeof(LiveActor)];
};

extern "C" void fn_800085C0(LayoutActor*, const char*, s32, s32);
extern "C" void fn_804A4EF0(TitleSequenceProduct*);
extern "C" void fn_804A4F60(TitleSequenceProduct*);
extern "C" void fn_804A4FB0(TitleSequenceProduct*);
extern "C" void fn_8013D530(DemoTitleAnim*);
extern "C" bool fn_8013D240(DemoTitleAnim*);
extern "C" void fn_8013D540(DemoTitleAnim*);
extern "C" bool fn_8013D220(DemoTitleAnim*);

namespace GameDataFunction {
    bool getSysConfigFileEncouragedPal16();
    void setSysConfigFileEncouragedPal16();
}  // namespace GameDataFunction

namespace {
    // NONMATCHING: SMG2 has no symbols for the first three nerves and the final one (fn_804A5730/5740/5750/57B0)
    class TitleSequenceProductNrvPal60 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            fn_804A4EF0(reinterpret_cast< TitleSequenceProduct* >(pSpine->mExecutor));
        }
        static TitleSequenceProductNrvPal60 sInstance;
    };
    TitleSequenceProductNrvPal60 TitleSequenceProductNrvPal60::sInstance ATTRIBUTE_WEAK;

    class TitleSequenceProductNrvBgmStop : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            fn_804A4F60(reinterpret_cast< TitleSequenceProduct* >(pSpine->mExecutor));
        }
        static TitleSequenceProductNrvBgmStop sInstance;
    };
    TitleSequenceProductNrvBgmStop TitleSequenceProductNrvBgmStop::sInstance ATTRIBUTE_WEAK;

    class TitleSequenceProductNrvDemoAnim : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            fn_804A4FB0(reinterpret_cast< TitleSequenceProduct* >(pSpine->mExecutor));
        }
        static TitleSequenceProductNrvDemoAnim sInstance;
    };
    TitleSequenceProductNrvDemoAnim TitleSequenceProductNrvDemoAnim::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(TitleSequenceProductLogoFadein, TitleSequenceProduct, LogoFadein);
    NEW_NERVE(TitleSequenceProductLogoWait, TitleSequenceProduct, LogoWait);
    NEW_NERVE(TitleSequenceProductLogoDisplay, TitleSequenceProduct, LogoDisplay);
    NEW_NERVE(TitleSequenceProductDecide, TitleSequenceProduct, Decide);
    NEW_NERVE(TitleSequenceProductDead, TitleSequenceProduct, Dead);

    class TitleSequenceProductNrvEnd : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static TitleSequenceProductNrvEnd sInstance;
    };
    TitleSequenceProductNrvEnd TitleSequenceProductNrvEnd::sInstance ATTRIBUTE_WEAK;
};  // namespace

TitleSequenceProduct::TitleSequenceProduct()
    : NerveExecutor(nullptr), mLogoLayout(nullptr), mPressStartLayout(nullptr), mEncouragePal60Window(nullptr), mAButtonChecker(nullptr),
      mBButtonChecker(nullptr), mDemoTitleAnim(nullptr) {
    bool isPal60 = VIGetTvFormat() == VI_PAL && !GameDataFunction::getSysConfigFileEncouragedPal16();
    if (isPal60) {
        mEncouragePal60Window = new EncouragePal60Window();
        mEncouragePal60Window->initWithoutIter();
        mEncouragePal60Window->kill();
        initNerve(&TitleSequenceProductNrvPal60::sInstance, 0);
    } else {
        initNerve(&TitleSequenceProductNrvBgmStop::sInstance, 0);
    }

    mLogoLayout = new TitleSequenceProductSub::LogoLayout();
    mPressStartLayout = new SimpleLayout("Press[A][B]", "PressStart", 1, -1);
    mPressStartLayout->initWithoutIter();
    mAButtonChecker = new TriggerChecker();
    mBButtonChecker = new TriggerChecker();
    mDemoTitleAnim = new DemoTitleAnim("タイトルアニメ");
    mDemoTitleAnim->initWithoutIter();
}

extern "C" void fn_804A4EF0(TitleSequenceProduct* pThis) {
    if (MR::isFirstStep(pThis)) {
        pThis->mEncouragePal60Window->appear();
        GameDataFunction::setSysConfigFileEncouragedPal16();
    }

    if (MR::isDead(pThis->mEncouragePal60Window)) {
        pThis->setNerve(&TitleSequenceProductNrvBgmStop::sInstance);
    }
}

extern "C" void fn_804A4F60(TitleSequenceProduct* pThis) {
    if (MR::isFirstStep(pThis)) {
        MR::forceCloseWipeFade();
        MR::stopStageBGM(0);
    }
    pThis->setNerve(&TitleSequenceProductNrvDemoAnim::sInstance);
}

extern "C" void fn_804A4FB0(TitleSequenceProduct* pThis) {
    if (MR::isFirstStep(pThis)) {
        fn_8013D530(pThis->mDemoTitleAnim);
        MR::openWipeFade(-1);
    }
    if (fn_8013D240(pThis->mDemoTitleAnim)) {
        pThis->setNerve(GET_NERVE_ANON(TitleSequenceProductLogoFadein));
    }
}

void TitleSequenceProduct::exeLogoFadein() {
    if (MR::isFirstStep(this)) {
        mLogoLayout->appear();
        MR::startAnim(mLogoLayout, "Appear", 0);
    }

    if (MR::isAnimStopped(mLogoLayout, 0)) {
        setNerve(GET_NERVE_ANON(TitleSequenceProductLogoWait));
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
        setNerve(GET_NERVE_ANON(TitleSequenceProductLogoDisplay));
    }
}

void TitleSequenceProduct::exeLogoDisplay() {
    if (MR::isFirstStep(this)) {
        mPressStartLayout->appear();
        MR::startAnim(mPressStartLayout, "Appear", 0);
    }

    if (MR::isAnimStopped(mPressStartLayout, 0)) {
        MR::startAnim(mPressStartLayout, "Wait", 0);
    }

    mAButtonChecker->update(MR::testCorePadButtonA(WPAD_CHAN0));
    mBButtonChecker->update(MR::testCorePadButtonB(WPAD_CHAN0));

    if (mAButtonChecker->getLevel() && mBButtonChecker->getLevel()) {
        fn_800085C0(mPressStartLayout, "PressAB", -1, -1);
        MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        setNerve(GET_NERVE_ANON(TitleSequenceProductDecide));
    } else {
        if (mAButtonChecker->getOnTrigger()) {
            fn_800085C0(mPressStartLayout, "PressA", -1, -1);
        }
        if (mBButtonChecker->getOnTrigger()) {
            fn_800085C0(mPressStartLayout, "PressB", -1, -1);
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
        setNerve(GET_NERVE_ANON(TitleSequenceProductDead));
    }
}

void TitleSequenceProduct::exeDead() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeWhiteFade(-1);
    }

    if (MR::isWipeBlank()) {
        MR::openWipeWhiteFade(-1);
        fn_8013D540(mDemoTitleAnim);
        mDemoTitleAnim->kill();
        mLogoLayout->kill();
        mPressStartLayout->kill();
        if (mEncouragePal60Window != nullptr) {
            mEncouragePal60Window->kill();
        }
        setNerve(&TitleSequenceProductNrvEnd::sInstance);
    }
}

void TitleSequenceProduct::appear() {
    if (mEncouragePal60Window != nullptr) {
        setNerve(&TitleSequenceProductNrvPal60::sInstance);
    } else {
        setNerve(&TitleSequenceProductNrvBgmStop::sInstance);
    }
}

void TitleSequenceProduct::kill() {
    setNerve(GET_NERVE_ANON(TitleSequenceProductDead));
}

bool TitleSequenceProduct::isActive() const {
    return !isNerve(GET_NERVE_ANON(TitleSequenceProductDead));
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

extern "C" bool fn_804A54D0(TitleSequenceProduct* pThis) {
    if (pThis->mDemoTitleAnim == nullptr) {
        return false;
    }
    return fn_8013D220(pThis->mDemoTitleAnim);
}
