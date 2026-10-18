#pragma once

#include "Game/LiveActor/Nerve.hpp"

namespace NrvErrorMessageWindow {
    class ErrorMessageWindowNrvHide : public Nerve {
    public:
        ErrorMessageWindowNrvHide();
        virtual void execute(Spine*) const;
        static ErrorMessageWindowNrvHide sInstance;
    };
    class ErrorMessageWindowNrvAppear : public Nerve {
    public:
        ErrorMessageWindowNrvAppear();
        virtual void execute(Spine*) const;
        static ErrorMessageWindowNrvAppear sInstance;
    };
    class ErrorMessageWindowNrvDisplay : public Nerve {
    public:
        ErrorMessageWindowNrvDisplay();
        virtual void execute(Spine*) const;
        static ErrorMessageWindowNrvDisplay sInstance;
    };
    class ErrorMessageWindowNrvDisappear : public Nerve {
    public:
        ErrorMessageWindowNrvDisappear();
        virtual void execute(Spine*) const;
        static ErrorMessageWindowNrvDisappear sInstance;
    };
    class ErrorMessageWindowNrvDisappearBeforeAppear : public Nerve {
    public:
        ErrorMessageWindowNrvDisappearBeforeAppear();
        virtual void execute(Spine*) const;
        static ErrorMessageWindowNrvDisappearBeforeAppear sInstance;
    };
}
