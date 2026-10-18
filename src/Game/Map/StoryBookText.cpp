#include "Game/Map/StoryBookText.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/IconAButton.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/NPC/TalkTextFormer.hpp"
#include <cstdio>

extern "C" {
    extern const char* lbl_807D02F8;
    extern const char lbl_807D02FC[4];
    extern const char lbl_806F8A5C[];
    extern char lbl_806F8A50[];
}

namespace NrvStoryBookText {
    NEW_NERVE(StoryBookTextNrvAppear, StoryBookText, Appear);

    class StoryBookTextNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            StoryBookText* actor = reinterpret_cast< StoryBookText* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                MR::startAnim(actor, "Wait", 0);
            }
        }
        static StoryBookTextNrvWait sInstance;
    };
    StoryBookTextNrvWait StoryBookTextNrvWait::sInstance ATTRIBUTE_WEAK;

    class StoryBookTextNrvOpenIcon : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            StoryBookText* actor = reinterpret_cast< StoryBookText* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                actor->mIconAButton->openWithoutMessage();
            }
        }
        static StoryBookTextNrvOpenIcon sInstance;
    };
    StoryBookTextNrvOpenIcon StoryBookTextNrvOpenIcon::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(StoryBookTextNrvClose, StoryBookText, Close);
    NEW_NERVE(StoryBookTextNrvNextPage, StoryBookText, NextPage);
    NEW_NERVE(StoryBookTextNrvShowText, StoryBookText, ShowText);
}  // namespace NrvStoryBookText

StoryBookText::StoryBookText() : LayoutActor(lbl_806F8A5C, true) {
    mIconAButton = NULL;
    mMessage = NULL;
    mHasIcon = false;
}

void StoryBookText::init(const JMapInfoIter&) {
    initLayoutManager("StoryBookText", 2);
    mIconAButton = MR::createAndSetupIconAButton(this, true, false);
    mIconAButton->kill();
    MR::connectToSceneLayout(this);
    initNerve(&NrvStoryBookText::StoryBookTextNrvAppear::sInstance);
    kill();
}

void StoryBookText::kill() {
    mIconAButton->kill();
    LayoutActor::kill();
}

void StoryBookText::setMessageWithoutIcon(s32 pageNo, s32 msgNo) {
    mHasIcon = false;
    char buf[0x100];
    snprintf(buf, 0x100, "%s%03d", lbl_807D02F8, pageNo);
    setMessage(buf, msgNo);
}

void StoryBookText::setMessageWithIcon(s32 msgNo) {
    mHasIcon = true;
    char buf[0x100];
    snprintf(buf, 0x100, "%s_%s%03d", lbl_807D02F8, MR::getCurrentDemoPartName(), msgNo);
    setMessage(buf, -1);
    MR::requestMovementOn(mIconAButton);
}

// NONMATCHING: register allocation / prologue (target keeps the table base and args in r28-r31)
void StoryBookText::setMessage(const char* pMsgName, s32 msgNo) {
    mMessage = MR::getSceneMessageDirect(pMsgName, msgNo);
    MR::initTagProcessorRecursive(this, lbl_806F8A50 + 0x20, 2);
    MR::setTextBoxMessageRecursive(this, lbl_806F8A50 + 0x20, mMessage);
    MR::hidePaneRecursive(this, lbl_806F8A50 + 0x20);
    MR::showPane(this, lbl_806F8A50 + 0x20);
    if (MR::isStoryBookPageKindA(pMsgName, msgNo) != 0) {
        const char* pPane = lbl_806F8A50 + 0x6C;
        if (mHasIcon) {
            pPane = lbl_806F8A50 + 0x60;
        }
        MR::showPaneRecursive(this, pPane);
    } else if (MR::isStoryBookPageKindB(pMsgName, msgNo) != 0) {
        MR::showPaneRecursive(this, lbl_806F8A50 + 0x74);
    } else if (MR::isStoryBookPageKindC(pMsgName, msgNo) != 0) {
        MR::showPaneRecursive(this, lbl_806F8A50 + 0x7C);
    }
    MR::startAnimAndSetFrameAndStop(this, lbl_806F8A50 + 0x54, (f32)MR::getTextLineNumMaxRecursive(this, lbl_806F8A50 + 0x20), 1);
    setNerve(&NrvStoryBookText::StoryBookTextNrvAppear::sInstance);
    LayoutActor::appear();
}

void StoryBookText::requestClose() {
    setNerve(&NrvStoryBookText::StoryBookTextNrvClose::sInstance);
}

void StoryBookText::nextPage() {
    mMessage = MR::getNextMessagePage(mMessage);
    setNerve(&NrvStoryBookText::StoryBookTextNrvNextPage::sInstance);
}

bool StoryBookText::tryOpenIcon() {
    if (!mHasIcon) {
        return false;
    }
    if (!isNerve(&NrvStoryBookText::StoryBookTextNrvWait::sInstance)) {
        return false;
    }
    setNerve(&NrvStoryBookText::StoryBookTextNrvOpenIcon::sInstance);
    return true;
}

bool StoryBookText::hasNextPage() const {
    return MR::getNextMessagePage(mMessage) != NULL;
}

bool StoryBookText::isNerveOpenIcon() const {
    return isNerve(&NrvStoryBookText::StoryBookTextNrvOpenIcon::sInstance);
}

void StoryBookText::openWithIcon() {
    mHasIcon = true;
    MR::hidePaneRecursive(this, "StoryBookText");
    MR::requestMovementOn(mIconAButton);
    setNerve(&NrvStoryBookText::StoryBookTextNrvOpenIcon::sInstance);
    LayoutActor::appear();
}

void StoryBookText::termIcon() {
    MR::tryRumblePadWeak(this, 0);
    mIconAButton->term();
}

bool StoryBookText::isIconDead() {
    return MR::isDead(mIconAButton);
}

void StoryBookText::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "Appear", 0);
    }
    if (MR::isAnimStopped(this, 0)) {
        setNerve(&NrvStoryBookText::StoryBookTextNrvWait::sInstance);
    }
}

void StoryBookText::exeClose() {
    if (MR::isFirstStep(this)) {
        if (mHasIcon) {
            MR::tryRumblePadWeak(this, 0);
            mIconAButton->term();
        }
        MR::startAnim(this, lbl_807D02FC, 0);
    }
    if (MR::isAnimStopped(this, 0)) {
        kill();
    }
}

void StoryBookText::exeNextPage() {
    if (MR::isFirstStep(this)) {
        if (mHasIcon) {
            MR::tryRumblePadVeryWeak(this, 0);
            mIconAButton->term();
        }
        MR::startAnim(this, lbl_807D02FC, 0);
    }
    if (MR::isAnimStopped(this, 0)) {
        setNerve(&NrvStoryBookText::StoryBookTextNrvShowText::sInstance);
    }
}

// NONMATCHING: register allocation (target hoists the table base into r31 before the isFirstStep branch)
void StoryBookText::exeShowText() {
    const char* pTable = lbl_806F8A50;
    if (MR::isFirstStep(this)) {
        MR::initTagProcessorRecursive(this, pTable + 0x20, 2);
        MR::setTextBoxMessageRecursive(this, pTable + 0x20, mMessage);
        MR::startAnim(this, pTable + 0x44, 0);
        MR::startAnimAndSetFrameAndStop(this, pTable + 0x54, (f32)MR::getTextLineNumMaxRecursive(this, pTable + 0x20), 1);
    }
    if (MR::isAnimStopped(this, 0)) {
        setNerve(&NrvStoryBookText::StoryBookTextNrvWait::sInstance);
    }
}

StoryBookText::~StoryBookText() {}
