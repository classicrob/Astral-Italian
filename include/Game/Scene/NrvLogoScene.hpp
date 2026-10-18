#pragma once

#include "Game/LiveActor/Nerve.hpp"

namespace NrvLogoScene {
    class LogoSceneNrvStrapFadein : public Nerve {
    public:
        LogoSceneNrvStrapFadein();
        virtual void execute(Spine* pSpine) const;

        static LogoSceneNrvStrapFadein sInstance;
    };

    class LogoSceneNrvStrapDisplay : public Nerve {
    public:
        LogoSceneNrvStrapDisplay();
        virtual void execute(Spine* pSpine) const;

        static LogoSceneNrvStrapDisplay sInstance;
    };

    class LogoSceneNrvStrapFadeout : public Nerve {
    public:
        LogoSceneNrvStrapFadeout();
        virtual void execute(Spine* pSpine) const;

        static LogoSceneNrvStrapFadeout sInstance;
    };

    class LogoSceneNrvMountGameData : public Nerve {
    public:
        LogoSceneNrvMountGameData();
        virtual void execute(Spine* pSpine) const;

        static LogoSceneNrvMountGameData sInstance;
    };

    class LogoSceneNrvWaitReadDoneSystemArchive : public Nerve {
    public:
        LogoSceneNrvWaitReadDoneSystemArchive();
        virtual void execute(Spine* pSpine) const;

        static LogoSceneNrvWaitReadDoneSystemArchive sInstance;
    };

    class LogoSceneNrvDeactive : public Nerve {
    public:
        LogoSceneNrvDeactive();
        virtual void execute(Spine* pSpine) const;

        static LogoSceneNrvDeactive sInstance;
    };
};  // namespace NrvLogoScene
