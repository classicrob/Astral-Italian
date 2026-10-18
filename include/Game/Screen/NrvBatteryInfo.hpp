#pragma once

#include "Game/LiveActor/Nerve.hpp"

namespace NrvBatteryInfo {
    class BatteryInfoEnought : public Nerve {
    public:
        BatteryInfoEnought();
        virtual void execute(Spine*) const;
        static BatteryInfoEnought sInstance;
    };
    class BatteryInfoLowAppear : public Nerve {
    public:
        BatteryInfoLowAppear();
        virtual void execute(Spine*) const;
        static BatteryInfoLowAppear sInstance;
    };
    class BatteryInfoLow : public Nerve {
    public:
        BatteryInfoLow();
        virtual void execute(Spine*) const;
        static BatteryInfoLow sInstance;
    };
    class BatteryInfoLowDisappear : public Nerve {
    public:
        BatteryInfoLowDisappear();
        virtual void execute(Spine*) const;
        static BatteryInfoLowDisappear sInstance;
    };
    class BatteryInfoLowHide : public Nerve {
    public:
        BatteryInfoLowHide();
        virtual void execute(Spine*) const;
        static BatteryInfoLowHide sInstance;
    };
    class BatteryInfoDisappear : public Nerve {
    public:
        BatteryInfoDisappear();
        virtual void execute(Spine*) const;
        static BatteryInfoDisappear sInstance;
    };
    class BatteryInfoCriticalAppear : public Nerve {
    public:
        BatteryInfoCriticalAppear();
        virtual void execute(Spine*) const;
        static BatteryInfoCriticalAppear sInstance;
    };
    class BatteryInfoCritical : public Nerve {
    public:
        BatteryInfoCritical();
        virtual void execute(Spine*) const;
        static BatteryInfoCritical sInstance;
    };
    class BatteryInfoNoneAppear : public Nerve {
    public:
        BatteryInfoNoneAppear();
        virtual void execute(Spine*) const;
        static BatteryInfoNoneAppear sInstance;
    };
    class BatteryInfoNone : public Nerve {
    public:
        BatteryInfoNone();
        virtual void execute(Spine*) const;
        static BatteryInfoNone sInstance;
    };
    class BatteryInfoNoneDisappear : public Nerve {
    public:
        BatteryInfoNoneDisappear();
        virtual void execute(Spine*) const;
        static BatteryInfoNoneDisappear sInstance;
    };
    class BatteryInfoNoneHide : public Nerve {
    public:
        BatteryInfoNoneHide();
        virtual void execute(Spine*) const;
        static BatteryInfoNoneHide sInstance;
    };
}
