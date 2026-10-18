#pragma once

#include "Game/Screen/LayoutActor.hpp"

class LogoFader : public LayoutActor {
public:
    /// @brief Creates a new `LogoFader`.
    /// @param pName The pointer to the null-terminated name of the object.
    LogoFader(const char* pName);

    virtual void draw() const;

    void exeBlank();
    void exeDisplay();
    void exeFadeIn();
    void exeFadeOut();
    void setBlank();
    void startFadeIn();
    void startFadeOut();
    bool isFadeEnd() const;

    /* 0x20 */ bool _20;
    /* 0x24 */ u32 mMaxStep;
    /* 0x28 */ f32 mRate;
};

#include "Game/LiveActor/Nerve.hpp"

namespace NrvLogoFader {
    class LogoFaderNrvBlank : public Nerve {
    public:
        LogoFaderNrvBlank();
        virtual void execute(Spine*) const;
        static LogoFaderNrvBlank sInstance;
    };
    class LogoFaderNrvDisplay : public Nerve {
    public:
        LogoFaderNrvDisplay();
        virtual void execute(Spine*) const;
        static LogoFaderNrvDisplay sInstance;
    };
    class LogoFaderNrvFadeIn : public Nerve {
    public:
        LogoFaderNrvFadeIn();
        virtual void execute(Spine*) const;
        static LogoFaderNrvFadeIn sInstance;
    };
    class LogoFaderNrvFadeOut : public Nerve {
    public:
        LogoFaderNrvFadeOut();
        virtual void execute(Spine*) const;
        static LogoFaderNrvFadeOut sInstance;
    };
}
