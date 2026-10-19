#pragma once

#include "Game/MapObj/MapObjActor.hpp"

class ActorCameraInfo;

class AnmModelObj : public MapObjActor {
public:
    /// @brief Creates a new `AnmModelObj`.
    /// @param pName A pointer to the null-terminated name of the object.
    AnmModelObj(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void control() {
    }
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo&) {
    }
    virtual void initCaseNoUseSwitchA(const MapObjActorInitInfo&) {
    }
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&) {
    }
    virtual void initCaseNoUseSwitchB(const MapObjActorInitInfo&) {
    }
    virtual bool isOnStartAnmTrigger() const = 0;
    virtual bool isDone() const;
    virtual bool isKilledAtMoveDone() const {
        return false;
    }
    virtual bool isInvalidateCollision() const;
    virtual bool isRepeat() const {
        return false;
    }
    virtual void onStartMove() {
    }
    virtual void onLoopMove() {
    }
    virtual void onStopMove() {
    }
    virtual void onDone() {
    }

    void exeWait();
    void exeMove();
    void exeDone();

    /* 0xC8 */ TVec3f mJointPos;
};

class AnmModelSwitchMove : public AnmModelObj {
public:
    /// @brief Creates a new `AnmModelSwitchMove`.
    /// @param pName A pointer to the null-terminated name of the object.
    AnmModelSwitchMove(const char* pName);
    virtual ~AnmModelSwitchMove();

    virtual void init(const JMapInfoIter&);
    virtual bool isOnStartAnmTrigger() const;
};

class AnmModelSwitchMoveValidateCollision : public AnmModelSwitchMove {
public:
    AnmModelSwitchMoveValidateCollision(const char* pName) : AnmModelSwitchMove(pName) {
    }

    virtual void init(const JMapInfoIter&);
    virtual void onStopMove();
};

class AnmModelSwitchMoveEndKill : public AnmModelSwitchMove {
public:
    /// @brief Creates a new `AnmModelSwitchMoveEndKill`.
    /// @param pName A pointer to the null-terminated name of the object.
    AnmModelSwitchMoveEndKill(const char* pName);

    virtual void kill();
    virtual bool isKilledAtMoveDone() const;
};

class AnmModelSwitchMoveEndKillAnyAnim : public AnmModelSwitchMoveEndKill {
public:
    AnmModelSwitchMoveEndKillAnyAnim(const char* pName) : AnmModelSwitchMoveEndKill(pName) {
    }

    virtual void init(const JMapInfoIter&);
    virtual bool isDone() const;
};

class AnmModelSwitchSwapVisibility : public AnmModelObj {
public:
    AnmModelSwitchSwapVisibility(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual bool isOnStartAnmTrigger() const;
    virtual bool isDone() const;
};

class AnmModelSwitchSyncBrk : public AnmModelObj {
public:
    AnmModelSwitchSyncBrk(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual bool isOnStartAnmTrigger() const;
    virtual bool isDone() const;
    virtual void onLoopMove();
    virtual void onDone();
    void checkForReverse();
};

class AnmModelSwitchMoveInvalidateCollision : public AnmModelSwitchMove {
public:
    AnmModelSwitchMoveInvalidateCollision(const char* name) : AnmModelSwitchMove(name) {}
    virtual ~AnmModelSwitchMoveInvalidateCollision();
    virtual bool isInvalidateCollision() const;
};
