#pragma once

#include "Game/Screen/LayoutActor.hpp"

class IconAButton;

class StoryBookText : public LayoutActor {
public:
    StoryBookText();
    virtual ~StoryBookText();
    virtual void init(const JMapInfoIter&);
    virtual void kill();

    void setMessage(const char*, s32);
    void setMessageWithoutIcon(s32, s32);
    void setMessageWithIcon(s32);
    void requestClose();
    void nextPage();
    bool tryOpenIcon();
    bool hasNextPage() const;
    bool isNerveOpenIcon() const;
    void openWithIcon();
    void termIcon();
    bool isIconDead();

    void exeAppear();
    void exeClose();
    void exeNextPage();
    void exeShowText();

    /* 0x2C */ IconAButton* mIconAButton;
    /* 0x30 */ const wchar_t* mMessage;
    /* 0x34 */ bool mHasIcon;
};
