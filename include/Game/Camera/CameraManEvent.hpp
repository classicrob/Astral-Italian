#pragma once

#include "Game/Camera/CameraMan.hpp"
#include "Game/Camera/CameraTargetArg.hpp"

class Camera;
class CameraHolder;
class CameraParamChunk;
class CameraParamChunkEvent;
class CameraParamChunkHolder;
class CameraParamChunkID;

#define NR_FIFO_ITEMS 2

class CameraManEvent : public CameraMan {
public:
    enum EPriority {

    };

    class ChunkFIFOItem {
    public:
        ChunkFIFOItem();

        /* 0xBC */ CameraParamChunkEvent* mChunk;
        /* 0x04 */ CameraTargetArg mTargetArg;
        /* 0x14 */ u32 mFrame;
    };

    class ChunkFIFOItemPair {
    public:
        inline ChunkFIFOItemPair() {
        }

        /* 0x00 */ ChunkFIFOItem mFirst;
        /* 0x18 */ ChunkFIFOItem mSecond;
    };

    CameraManEvent(CameraHolder*, CameraParamChunkHolder*, const char*);
    virtual ~CameraManEvent();

    virtual void init(const JMapInfoIter&);

    virtual void calc();
    virtual void notifyActivate();
    virtual void notifyDeactivate();
    virtual bool isInterpolationOff() const;
    virtual bool isCollisionOff() const;
    virtual bool isZeroFrameMoveOff() const;
    virtual bool isCorrectingErpPositionOff() const;
    virtual bool isUnknownOff() const;
    virtual bool isEnableToRoundLeft() const;
    virtual bool isEnableToRoundRight() const;
    virtual bool isEnableToReset() const;

    void start(const CameraParamChunkID&, const CameraTargetArg&, s32);
    void end(const CameraParamChunkID&, s32);
    bool isEventActive(const CameraParamChunkID&) const;
    bool isActive() const;
    bool doesNextChunkHaveInterpolation() const;
    u32 getAnimCameraFrame(const CameraParamChunkID&) const;
    void pauseOnAnimCamera(const CameraParamChunkID&);
    void pauseOffAnimCamera(const CameraParamChunkID&);
    void setAnimCameraFrame(const CameraParamChunkID&, f32);
    void updateChunkFIFO();
    void applyChunk();
    void checkReset(ChunkFIFOItem*);
    void changeCamera();
    void setExtraParam();
    void setVPanParam();
    void resetCameraIfRequested();
    void setSafePose();
    CameraParamChunkEvent* findChunk(const CameraParamChunkID&) const NO_INLINE;
    void requestChunk(CameraParamChunkEvent*, EPriority, const CameraTargetArg&, s32);
    bool cleanChunkFIFO(CameraParamChunk*);
    bool isChunkFIFOEmpty() const;
    void sendStartInterpolateFrame(CameraParamChunkEvent*, s32);
    s32 getInterpolateFrame(CameraParamChunkEvent*, s32) const;
    void sendFinishInterpolateFrame(CameraParamChunkEvent*, s32);
    s32 searchPriority(CameraParamChunk*) const;
    bool isInFIFO(CameraParamChunk*) const;
    bool isAnimCameraEnd(const CameraParamChunkID&) const;

    /* 0x50 */ CameraHolder* mHolder;
    /* 0x54 */ CameraParamChunkHolder* mChunkHolder;
    /* 0x58 */ Camera* mCamera;
    /* 0x5C */ ChunkFIFOItemPair mItems[NR_FIFO_ITEMS];
    /* 0xBC */ CameraParamChunkEvent* mChunk;
    /* 0xC0 */ u32 _B8;
    /* 0xC4 */ bool mRequestReset;
};
