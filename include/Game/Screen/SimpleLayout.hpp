#pragma once

#include "Game/Screen/LayoutActor.hpp"

class SimpleLayout : public LayoutActor {
public:
    SimpleLayout(const char*, const char*, u32, int);
    virtual ~SimpleLayout();
};

class SimpleEffectLayout : public SimpleLayout {
public:
    SimpleEffectLayout(const char*, const char*, u32, int);
};

class TalkTextFormer;
class SimpleTalkBalloon : public SimpleLayout {
public:
    SimpleTalkBalloon(const char*, const char*, u32);
    virtual ~SimpleTalkBalloon();
    void exeAppear();
    void exeWait();
    void exeDisappear();
    void appearMessage(const char*);
    void disappear();
private:
    TalkTextFormer* mTextFormer;
};
