#pragma once

#include "Game/Scene/Scene.hpp"

class CameraContext;
class J3DDrawBuffer;
class MultiSceneActor;
class NameObjGroup;

class ModelWipeScene : public Scene {
public:
    ModelWipeScene();

    virtual ~ModelWipeScene();
    virtual void init();
    virtual void update();
    virtual void draw() const;
    virtual void calcAnim();
    virtual void calcViewAndEntry();

    static J3DDrawBuffer* createDrawBuffer();

    void setCurrentActor(MultiSceneActor*);
    void startCloudWipeForward();
    void startGalaxyWipeForward();
    void startSpaceWipeForward();
    void startCloudWipeBackward();
    void startGalaxyWipeBackward();
    void startSpaceWipeBackward();
    void requestEnd();
    bool tryEnd();
    void forceHide();
    bool isHideOrEnd() const;
    bool isHidden() const;
    void exeStart();
    void exeWait();
    void exeEnd();

    /* 0x14 */ J3DDrawBuffer* mDrawBufferOpa;
    /* 0x18 */ J3DDrawBuffer* mDrawBufferXlu;
    /* 0x1C */ CameraContext* mCamera;
    /* 0x20 */ NameObjGroup* mGroup;
    /* 0x24 */ bool mIsForward;
    /* 0x25 */ bool mIsPendingEnd;
    /* 0x28 */ MultiSceneActor* mCurrentActor;
    /* 0x2C */ MultiSceneActor* mCloudActor;
    /* 0x30 */ MultiSceneActor* mGalaxyActor;
    /* 0x34 */ MultiSceneActor* mSpaceActor;
};
