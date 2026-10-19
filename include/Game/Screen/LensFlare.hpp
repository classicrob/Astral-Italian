#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/System/DrawSyncManager.hpp"
#include "Game/Util/Array.hpp"

class BrightCamInfo;
class BrightObjBase;
class TriggerChecker;

class LensFlareModel : public LiveActor {
public:
    /// @brief Creates a new `LensFlareModel`.
    /// @param pName The pointer to the null-terminated name of the object.
    /// @param pArcName The pointer to the null-terminated base name of the resource archive.
    LensFlareModel(const char* pName, const char* pArcName);

    virtual void appear();
    virtual void control();
    virtual void appearAnim() {
    }
    virtual void controlAnim() {
    }

    void update(bool, bool);

    void exeKill();
    void exeHide();
    void exeShow();
    void exeFadeIn();
    void exeFadeOut();

    void notifyInArea();

    /* 0x8C */ f32 mBrightRate;
    /* 0x90 */ f32 mFadeLevel;
    /* 0x94 */ f32 mFadeSpeed;
    /* 0x98 */ TriggerChecker* mAreaTrigger;
    /* 0x9C */ TriggerChecker* mBrightTrigger;
};

class LensFlareRing : public LensFlareModel {
public:
    /// @brief Creates a new `LensFlareRing`.
    LensFlareRing();

    virtual void appearAnim();
    virtual void controlAnim();

    /* 0xA0 */ f32 mRingFrame;
};

class LensFlareGlow : public LensFlareModel {
public:
    /// @brief Creates a new `LensFlareGlow`.
    LensFlareGlow();

    virtual void appearAnim();
    virtual void controlAnim();
};

class LensFlareLine : public LensFlareModel {
public:
    /// @brief Creates a new `LensFlareLine`.
    LensFlareLine();

    virtual void appearAnim();
    virtual void controlAnim();
};

class LensFlareDirector : public NameObj, public DrawSyncCallback {
public:
    /// @brief Creates a new `LensFlareDirector`.
    LensFlareDirector();

    virtual ~LensFlareDirector();

    /// @brief Intializes the `LensFlareDirector` while being placed into a scene.
    /// @param rIter The reference to an iterator over a `JMapInfo`.
    virtual void init(const JMapInfoIter& rIter);

    virtual void movement();
    virtual void drawSyncCallback(u16);

    void pauseOff();
    void setDrawSyncToken();
    s32 checkArea();
    bool checkBrightObj(bool);
    void controlFlare(s32, bool);

    /* 0x10 */ LensFlareRing* mRing;
    /* 0x14 */ LensFlareGlow* mGlow;
    /* 0x18 */ LensFlareLine* mLine;
    /* 0x1C */ BrightObjBase* mBrightObjArray[16];
    /* 0x5C */ u32 mBrightObjCount;
    /* 0x60 */ TVec2f mBrightCenter;
    /* 0x68 */ f32 mBright;
    /* 0x6C */ TVec2f mCenter;
    /* 0x74 */ TVec2f mNowCenter;
    /* 0x7C */ u16 mDrawSyncId;
    /* 0x7E */ u16 mDrawSyncTokenIndex;
    /* 0x80 */ BrightCamInfo* mBrightCamInfo;
};

namespace MR {
    void addBrightObj(BrightObjBase* pBrightObj);
    void setLensFlareDrawSyncToken();
    u32 getLensFlareDrawSyncTokenIndex();
};  // namespace MR

#include "Game/LiveActor/Nerve.hpp"

namespace NrvLensFlareModel {
    class LensFlareModelNrvKill : public Nerve {
    public:
        LensFlareModelNrvKill();
        virtual void execute(Spine*) const;
        static LensFlareModelNrvKill sInstance;
    };
    class LensFlareModelNrvHide : public Nerve {
    public:
        LensFlareModelNrvHide();
        virtual void execute(Spine*) const;
        static LensFlareModelNrvHide sInstance;
    };
    class LensFlareModelNrvShow : public Nerve {
    public:
        LensFlareModelNrvShow();
        virtual void execute(Spine*) const;
        static LensFlareModelNrvShow sInstance;
    };
    class LensFlareModelNrvFadeIn : public Nerve {
    public:
        LensFlareModelNrvFadeIn();
        virtual void execute(Spine*) const;
        static LensFlareModelNrvFadeIn sInstance;
    };
    class LensFlareModelNrvFadeOut : public Nerve {
    public:
        LensFlareModelNrvFadeOut();
        virtual void execute(Spine*) const;
        static LensFlareModelNrvFadeOut sInstance;
    };
}
