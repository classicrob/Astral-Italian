#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"

class FileSelectModel : public LiveActor {
public:
    FileSelectModel(const char*, MtxPtr, const char*);

    virtual void calcAnim();
    virtual void calcAndSetBaseMtx();

    void open();
    void blinkOnce();
    void close();
    void blink();
    bool isOpen() const;
    void emitOpen();
    void emitVanish();
    void emitCopy();
    void emitCompleteEffect();
    void deleteCompleteEffect();

    void exeOpen();
    void exeBlinkOnce();
    void exeClose();
    void exeBlink();

    /* 0x90 */ MtxPtr mHostMtx;
};

namespace NrvFileSelectModel {
    class FileSelectModelNrvOpen : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static FileSelectModelNrvOpen sInstance;
    };
    class FileSelectModelNrvBlinkOnce : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static FileSelectModelNrvBlinkOnce sInstance;
    };
    class FileSelectModelNrvClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static FileSelectModelNrvClose sInstance;
    };
    class FileSelectModelNrvBlink : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static FileSelectModelNrvBlink sInstance;
    };
};  // namespace NrvFileSelectModel
