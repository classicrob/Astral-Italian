#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class FixedPosition;
class MiiFaceRecipe;
struct RFLCharModel;
struct RFLDrawCoreSetting;

class MiiFaceParts : public LiveActor {
public:
    MiiFaceParts(const char* pName, const MiiFaceRecipe& rRecipe);

    virtual ~MiiFaceParts();
    virtual void init(const JMapInfoIter& rIter);
    virtual void calcAnim();
    virtual void calcViewAndEntry();

    void drawOpa(const RFLDrawCoreSetting* pSetting) const;
    void drawXlu(const RFLDrawCoreSetting* pSetting) const;
    void initFixedPosition(MtxPtr, const TVec3f&, const TVec3f&);
    void changeExpressionNormal();
    void changeExpressionBlink();
    void changeFaceModel(const MiiFaceRecipe& rRecipe);
    void initFaceModel();
    bool tryChangeExpression(u32 expression);
    /* 0x90 */ RFLCharModel* mCharModel;
    /* 0x94 */ void* mModelBuffer;
    /* 0x98 */ MiiFaceRecipe* mRecipe;
    /* 0x9C */ TPos3f mModelMtx;
    /* 0xCC */ MtxPtr mFollowMtx;
    /* 0xD0 */ FixedPosition* mFixedPosition;
    /* 0xD4 */ bool mNeedsModelReinit;
    /* 0xD5 */ bool mIsModelUninitialized;
};
