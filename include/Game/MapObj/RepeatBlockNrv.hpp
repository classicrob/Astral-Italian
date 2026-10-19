#pragma once

#include "Game/MapObj/RepeatBlock.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvRepeatBlock {
    class RepeatBlockNrvWait : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<RepeatBlock*>(spine->mExecutor)->exeWait(); }
        static RepeatBlockNrvWait sInstance;
    };

    class RepeatBlockNrvAtomA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast< RepeatBlockAtom* >(pSpine->mExecutor)->exeAppear();
        }
        static RepeatBlockNrvAtomA sInstance;
    };

    class RepeatBlockNrvAtomWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* pActor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            if (MR::isFirstStep(pActor)) {
                MR::startAction(pActor, "Wait");
            }
        }
        static RepeatBlockNrvAtomWait sInstance;
    };

    class RepeatBlockNrvAtomDisappear : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* pActor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            f32 rate = MR::getBtkRate(pActor);
            if (rate + MR::getBtkFrame(pActor) <= 0.0f) {
                pActor->kill();
            }
        }
        static RepeatBlockNrvAtomDisappear sInstance;
    };
}
