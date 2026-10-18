#pragma once

#include "Game/System/NerveExecutor.hpp"
#include "Game/Screen/StarPointerInput.hpp"

class LayoutActor;

class StarPointerMovePosition {
public:
    StarPointerMovePosition();
    void reset();

    TVec2f mPosition;
    TVec2f mOffset;
    const char* mName;
    StarPointerMovePosition* mLeft;
    StarPointerMovePosition* mRight;
    StarPointerMovePosition* mUp;
    StarPointerMovePosition* mDown;
};

class StarPointerMovePositionHolder : public NerveExecutor, public StarPointerInput {
public:
    explicit StarPointerMovePositionHolder(s32);
    virtual ~StarPointerMovePositionHolder();

    virtual const TVec2f& getScreenPosition() const;
    virtual const TVec2f& getScreenVelocity() const;
    virtual const TVec3f& getWorldVelocity() const;
    virtual f32 getScreenSpeed() const;
    virtual f32 getViewDistanceZ() const;
    virtual s32 getPastPointCount() const;
    virtual s32 getNextPastPointCount() const;
    virtual const TVec2f& getPastPosition(s32) const;
    virtual const TVec2f& getNextPastPosition(s32) const;
    virtual const TVec3f& getWorldPosition() const;
    virtual bool isInScreen() const;
    virtual bool isOutScreenLong() const;

    void beginRegistration(LayoutActor*, bool);
    void resetSelection();
    void update(const TVec2f&, bool);
    void requestMovement();
    bool isActive() const;
    bool selectPosition(StarPointerMovePosition*);
    void updateStickRepeat();
    bool tryMove(StarPointerMovePosition*);
    void exeWait();
    void exeActive();
    void exeRelease();
    StarPointerMovePosition* addPosition(const char*, const TVec2f*, const TVec2f&);
    void addMovePosition(const char*, const TVec2f*, const TVec2f&);
    StarPointerMovePosition* getPositionByName(const char*);
    void setPosition(const char*, const TVec2f*);
    void setDefaultLeft(const char*);
    void setDefaultRight(const char*);
    void setDefaultUp(const char*);
    void setDefaultDown(const char*);
    void connectRight(const char*, const char*);
    void connectDown(const char*, const char*);
    const char* getPositionName(s32) const;

    LayoutActor* mLayout;
    s32 mPadChannel;
    bool mMovementRequested;
    bool mRegistered;
    bool mUseSubPad;
    StarPointerMovePosition mRoot;
    StarPointerMovePosition* mSelected;
    StarPointerMovePosition* mPositions;
    s32 mPositionCount;
    TVec2f mScreenPosition;
    TVec2f mSelectionPhysicalPosition;
    bool mSelectionPhysicalInScreen;
    TVec2f mPhysicalPosition;
    bool mPhysicalInScreen;
    bool mStickUp;
    bool mStickDown;
    bool mStickLeft;
    bool mStickRight;
    s32 mStickUpFrames;
    s32 mStickDownFrames;
    s32 mStickLeftFrames;
    s32 mStickRightFrames;
};
