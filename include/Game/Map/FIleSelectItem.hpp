#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/System/NerveExecutor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class FileSelectIconID;
class FileSelectItem;
class FileSelectItemDelegatorBase;
class FileSelectModel;
class FileSelectNumber;
class MiiFaceParts;
class PartsModel;

namespace FileSelectItemSub {
    class ScaleController;
    class BlinkController;
};  // namespace FileSelectItemSub

class FileSelectItem : public LiveActor {
public:
    FileSelectItem(s32, bool, const FileSelectIconID&, const char*);

    virtual ~FileSelectItem();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();

    bool isNew() const;
    bool isExist() const;
    void format();
    void change(const FileSelectIconID&, bool);
    void forceChange(const FileSelectIconID&, bool);
    void invalidateSelect();
    void validateSelect();
    void appearIndex();
    void disappearIndex();
    void copyIconID(FileSelectIconID*);
    void setSelectDelegator(FileSelectItemDelegatorBase*);
    void onPointing();
    void offPointing();
    void validateRotate();
    void invalidateRotate();       // was fn_8024CFE0
    void turnToFront(s32);
    void requestRotateRefresh();   // was fn_8024D010
    void exeFormat();
    void exeChangeFellow();
    void exeChangeMii();
    void createNew();
    void createFellows();
    void createMii();
    void createNumber();
    void updatePointing();
    void updateRotate();
    void playPointedME();
    void playPointedNotUsingME();
    void appearFellowModel();
    void killAllModels();
    void setPlanetColor();
    void emitOpen();
    void emitVanish();
    void emitCopy();
    void emitCompleteEffect();
    void deleteCompleteEffect();

    void exeExistWait();
    void exeNewWait();

   /* 0x90 */ bool mIsNew;
    /* 0x94 */ PartsModel* mPlanetMapObj;
    /* 0x98 */ FileSelectIconID* mIconID;
    /* 0x9C */ FileSelectModel** mModels;
    /* 0xA0 */ MiiFaceParts* mFaceParts;
   /* 0xA4 */ FileSelectNumber* mNumber;
   /* 0xA8 */ TPos3f mPlanetMtx;
   /* 0xD8 */ TMtx34f mFellowMtx;
   /* 0x108 */ TMtx34f mMiiMtx;
   /* 0x138 */ TVec3f mUnusedVector;
   /* 0x144 */ s32 mFileIndex;
   /* 0x148 */ u8 mIsPointed;
    /* 0x149 */ bool mIsInvalidateSelect;
   /* 0x14A */ u8 mHasInvalidMiiId;
   /* 0x14B */ u8 mIsComplete;
    /* 0x14C */ FileSelectItemSub::ScaleController* mScaleCtrl;
    /* 0x150 */ FileSelectItemSub::BlinkController* mBlinkCtrl;
    /* 0x154 */ FileSelectItemDelegatorBase* mDelegator;
   /* 0x158 */ u8 mNeedsPointerReset;
   /* 0x159 */ u8 mIsDragging;
   /* 0x15A */ u8 mWasDragging;
   /* 0x15B */ u8 mReservedDragFlag;
   /* 0x15C */ TVec2f mPreviousPointerPos;
   /* 0x164 */ f32 mRotationSpeed;
    /* 0x168 */ bool mIsInvalidRotate;
   /* 0x169 */ u8 mUseCopyEffect;
   /* 0x16A */ u8 mReservedEffectFlag0;
   /* 0x16B */ u8 mReservedEffectFlag1;
   /* 0x16C */ s32 mTurnToFrontDuration;
   /* 0x170 */ s32 mTurnToFrontStep;
};

namespace FileSelectItemSub {
    class ScaleController : public NerveExecutor {
    public:
        ScaleController();

        void exeToSmall();
        void exeToBig();
        void exeSmall();
        void exeBig();

        f32 getScale() const {
           /* 0x08 */ return mScale;
        }

        /* 0x08 */ f32 mScale;
    };

    class BlinkController : public NerveExecutor {
    public:
        BlinkController(FileSelectItem*);

        void exeOpen();
        void exeShut();
        void exeSleep();
        void exeBlink();
        void shut();
        void open();
        void sleep();

        /* 0x08 */ FileSelectItem* mItem;
        /* 0x0C */ s32 mBlinkWaitFrames;
        /* 0x10 */ s32 mSleepFrames;
    };
};  // namespace FileSelectItemSub
