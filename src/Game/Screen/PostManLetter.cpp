#include "Game/Screen/PostManLetter.hpp"
#include "Game/Screen/LayoutPager.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <cstdio>

namespace {
    struct LetterInfo {
        s32 mId;
        const char* mTo;
        const char* mFrom;
        const char* mBody;
        f32 mColorFrame;
        const char* mVoice;
    };
    LetterInfo cLetterInfo[] = {
        {0, "Mario", "Peach", "Peach", 1.0f, "MailVoicePeach"},
        {1, "Luigi", "Luigi", "Luigi", 3.0f, "MailVoiceLuigi"},
        {2, "Mario", "Peach", "Peach", 1.0f, "MailVoicePeach"},
        {3, 0, 0, "Witch", 2.0f, "MailVoiceHelperWitch"},
        {4, "Bee", "Bee", "Bee", 5.0f, "MailVoiceHoneyBee"},
        {5, "Rabbit", "Rabbit", "Rabbit", 4.0f, "MailVoiceRabbit"},
        {6, "Request", "Caretaker", "Caretaker1", 6.0f, "MailVoiceCareTaker"},
        {7, "Request", "Caretaker", "Caretaker2", 6.0f, "MailVoiceCareTaker"},
        {8, "Challenge", "Pichan", "Pichan1", 7.0f, "MailVoicePichan"},
        {9, "Challenge", "Pichan", "Pichan2", 7.0f, "MailVoicePichan"},
        {10, "Invitation", "ScoreAttack", "ScoreAttack1", 8.0f, "MailVoiceSasurai"},
        {11, "Invitation", "ScoreAttack", "ScoreAttack2", 8.0f, "MailVoiceSasurai"},
        {12, "Invitation", "ScoreAttack", "ScoreAttack3", 8.0f, "MailVoiceSasurai"},
        {13, "Tico", 0, "Rosetta1", 2.0f, "MailVoiceHelperWitch"},
        {14, "Tico", 0, "Rosetta2", 2.0f, "MailVoiceHelperWitch"},
        {15, "Tico", 0, "Rosetta3", 2.0f, "MailVoiceHelperWitch"},
        {16, "Tico", 0, "Rosetta4", 2.0f, "MailVoiceHelperWitch"},
        {17, "Tico", 0, "Rosetta5", 2.0f, "MailVoiceHelperWitch"},
    };
}

namespace NrvPostManLetter {
    class PostManLetterNrvDelay : public Nerve {
    public:
        static PostManLetterNrvDelay sInstance;
        virtual void execute(Spine*) const;
    };
    PostManLetterNrvDelay PostManLetterNrvDelay::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(PostManLetterNrvAppear, PostManLetter, Appear);
    NEW_NERVE(PostManLetterNrvWait, PostManLetter, Wait);
    NEW_NERVE(PostManLetterNrvDisappear, PostManLetter, Disappear);
    class PostManLetterNrvEnd : public Nerve {
    public:
        static PostManLetterNrvEnd sInstance;
        virtual void execute(Spine*) const {}
    };
    PostManLetterNrvEnd PostManLetterNrvEnd::sInstance ATTRIBUTE_WEAK;
}

namespace {
    const s32 cFirstLetterId = 0;
}

void PostManLetter::setLetterMessage(s32 id) {
    char toMessage[256];
    char fromMessage[256];
    char bodyMessage[256];
    MR::hidePane(this, "To");
    MR::hidePane(this, "From");
    const LetterInfo* info = &cLetterInfo[id];
    if (!MR::isNullOrEmptyString(info->mTo)) {
        snprintf(toMessage, sizeof(toMessage), "PostmanLetter%s_To", info->mTo);
        MR::showPane(this, "To");
        MR::setTextBoxGameMessageRecursive(this, "To", toMessage);
    }
    if (!MR::isNullOrEmptyString(info->mFrom)) {
        snprintf(fromMessage, sizeof(fromMessage), "PostmanLetter%s_From", info->mFrom);
        MR::showPane(this, "From");
        MR::setTextBoxGameMessageRecursive(this, "From", fromMessage);
    }
    snprintf(bodyMessage, sizeof(bodyMessage), "PostmanLetter%s_Body", info->mBody);
    MR::setTextBoxGameMessageRecursive(this, "Body", bodyMessage);
}

PostManLetter::PostManLetter(s32 newLetter)
    : LayoutActor("郵便屋さんの手紙", true), mPager(0), mLetters(), mHasUnreadLetter(false), mHasReadNewLetter(false) {
    if (newLetter != 18 && isNewLetter(newLetter)) mHasUnreadLetter = true;
    if (GameDataFunction::isOnPostmanImportantLetterRead(0) && newLetter != 2) {
        mLetters.push_back(cFirstLetterId);
    }
    for (s32 i = 0; i < 18; ++i) {
        if (GameDataFunction::isOnPostmanTaskLetterRead(i)) {
            s32 id = i;
            if (i != newLetter && !containsLetter(i)) mLetters.push_back(id);
        }
    }
    for (s32 i = 0; i < 18; ++i) {
        if (GameDataFunction::isOnPostmanImportantLetterRead(i)) {
            s32 id = i;
            if (i != 0 && (!containsLetter(0) || id != 2) && (!containsLetter(2) || id != 0) && id != newLetter && !containsLetter(id)) {
                mLetters.push_back(id);
            }
        }
    }
    if (newLetter != 18) mLetters.push_back(newLetter);
    mCurrentLetter = mLetters.mIds + mLetters.mCount - 1;
}

void PostManLetter::init(const JMapInfoIter&) {
    initLayoutManagerWithTextBoxBufferLength("PostLetter", 512, 2);
    MR::createAndAddPaneCtrl(this, "PeachLetter", 1);
    mPager = new LayoutPager(this, "PeachLetter", canShowTwoPrevious(), canShowPrevious(), canShowNext(), canShowTwoNext());
    MR::connectToScene(this, -1, 13, -1, 80);
    initNerve(&NrvPostManLetter::PostManLetterNrvAppear::sInstance);
    initActionSound(3, "PostLetter", 0);
    setLetterMessage(*mCurrentLetter);
    kill();
}

void PostManLetter::control() {
    mPager->updateButtons();
    MR::setSeVolumeSetLevelForMenu();
}

void PostManLetter::updateLetter() {
    s32 id = *mCurrentLetter;
    MR::startAnim(this, "Color", 1);
    MR::setAnimFrameAndStop(this, cLetterInfo[id].mColorFrame - 1.0f, 1);
    setLetterMessage(*mCurrentLetter);
    mPager->setPageAvailability(canShowTwoPrevious(), canShowPrevious(), canShowNext(), canShowTwoNext());
}

bool PostManLetter::canShowTwoPrevious() const {
    if (isUnreadLetter(*mCurrentLetter)) return false;
    if (mLetters.mCount < 3) return false;
    return &mLetters.mIds[1] < mCurrentLetter;
}

bool PostManLetter::canShowPrevious() const {
    if (isUnreadLetter(*mCurrentLetter)) return false;
    if (mLetters.mCount < 2) return false;
    return mCurrentLetter != mLetters.mIds;
}

bool PostManLetter::canShowNext() const {
    if (isUnreadLetter(*mCurrentLetter)) return false;
    if (mLetters.mCount < 2) return false;
    bool result = false;
    const s32* end = mLetters.mIds + mLetters.mCount;
    if (mCurrentLetter != end && mCurrentLetter + 1 != end) result = true;
    return result;
}

// NONMATCHING: current and end pointer registers are interchanged.
bool PostManLetter::canShowTwoNext() const {
    if (isUnreadLetter(*mCurrentLetter)) return false;
    if (mLetters.mCount < 3) return false;
    bool result = false;
    const s32* end = mLetters.mIds + mLetters.mCount;
    const s32* current = mCurrentLetter;
    bool next = false;
    if (current != end && current + 1 != end) next = true;
    if (next && current + 2 != end) result = true;
    return result;
}

bool PostManLetter::containsLetter(s32 id) const {
    const s32* item = mLetters.mIds;
    const s32* end = mLetters.mIds + mLetters.mCount;
    while (item != end && *item != id) ++item;
    return item != end;
}

bool PostManLetter::isNewLetter(s32 id) const {
    if (id == 18) return false;
    if (MR::isPostmanSpecialLetter(id)) return !mHasReadNewLetter;
    if (GameDataFunction::isOnPostmanTaskLetterRead(id)) return false;
    return !GameDataFunction::isOnPostmanImportantLetterRead(id);
}

bool PostManLetter::isUnreadLetter(s32 id) const {
    if (id == 18) return false;
    if (mHasUnreadLetter && MR::isPostmanSpecialLetter(id) && !mHasReadNewLetter) return true;
    if (GameDataFunction::isOnPostmanTaskLetterRead(id)) return false;
    return !GameDataFunction::isOnPostmanImportantLetterRead(id);
}

void PostManLetter::appear() {
    LayoutActor::appear();
    MR::hideScreen(this);
    MR::getGameSceneLayoutHolder()->deactivateDefaultGameLayout();
    MR::tryFrameToScreenCinemaFrame();
    mCurrentLetter = mLetters.mIds + mLetters.mCount - 1;
    setNerve(&NrvPostManLetter::PostManLetterNrvDelay::sInstance);
}

bool PostManLetter::isEnd() const {
    return isNerve(&NrvPostManLetter::PostManLetterNrvEnd::sInstance);
}

s32 PostManLetter::getCurrentLetterId() const {
    return *mCurrentLetter;
}

void PostManLetter::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showScreen(this);
        MR::requestPauseBlurMovementOn();
        MR::incrementPauseBlurDelay();
        updateLetter();
        MR::startAnim(this, "Appear", 0);
        MR::startActionSound(this, "LetterAppear", -1, -1);
    }
    if (MR::isAnimStopped(this, 0)) {
        s32 id = *mCurrentLetter;
        if (isUnreadLetter(id)) {
            const LetterInfo* letter = cLetterInfo + id;
            MR::startActionSound(this, letter->mVoice, -1, -1);
        }
        mPager->appear();
        setNerve(&NrvPostManLetter::PostManLetterNrvWait::sInstance);
    }
}

void PostManLetter::exeWait() {
    if (mPager->update()) {
        setNerve(&NrvPostManLetter::PostManLetterNrvDisappear::sInstance);
        return;
    }
    if (mPager->isNextSelected()) {
        MR::startActionSound(this, "PageChange", -1, -1);
        ++mCurrentLetter;
        updateLetter();
    }
    if (mPager->isPreviousSelected()) {
        MR::startActionSound(this, "PageChange", -1, -1);
        --mCurrentLetter;
        updateLetter();
    }
}

void PostManLetter::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "End", 0);
    }
    if (MR::isAnimStopped(this, 0)) {
        mHasReadNewLetter = true;
        MR::tryScreenToFrameCinemaFrame();
        MR::decrementPauseBlurDelay();
        MR::getGameSceneLayoutHolder()->activateDefaultGameLayout();
        setNerve(&NrvPostManLetter::PostManLetterNrvEnd::sInstance);
    }
}

void PostManLetterList::push_back(const s32& id) {
    s32 index = mCount;
    mCount = index + 1;
    mIds[index] = id;
}

PostManLetter::~PostManLetter() {
}

void NrvPostManLetter::PostManLetterNrvDelay::execute(Spine* spine) const {
    PostManLetter* actor = reinterpret_cast<PostManLetter*>(spine->mExecutor);
    if (MR::isGreaterEqualStep(actor, 30)) {
        actor->setNerve(&PostManLetterNrvAppear::sInstance);
    }
}
