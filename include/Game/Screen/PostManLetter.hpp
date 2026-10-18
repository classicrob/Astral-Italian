#pragma once
#include "Game/Screen/LayoutActor.hpp"
class LayoutPager;
class PostManLetterList {
public:
    PostManLetterList() : mCount(0) {}
    void push_back(const s32& id);
    s32 mIds[18];
    s32 mCount;
};
class PostManLetter;
namespace PostmanEvent { s32 getTalkType(const PostManLetter*); }

class PostManLetter : public LayoutActor {
    friend s32 PostmanEvent::getTalkType(const PostManLetter*);
public:
    PostManLetter(s32);
    virtual ~PostManLetter();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();
    void updateLetter();
    void setLetterMessage(s32);
    bool canShowTwoPrevious() const;
    bool canShowPrevious() const;
    bool canShowNext() const;
    bool canShowTwoNext() const;
    bool containsLetter(s32) const;
    bool isNewLetter(s32) const;
    bool isUnreadLetter(s32) const;
    bool isEnd() const;
    s32 getCurrentLetterId() const;
    void exeAppear();
    void exeWait();
    void exeDisappear();
private:
    LayoutPager* mPager;
    PostManLetterList mLetters;
    s32* mCurrentLetter;
    bool mHasUnreadLetter;
    bool mHasReadNewLetter;
};
