#pragma once

#include "Game/LiveActor/LiveActorGroup.hpp"
#include <JSystem/JKernel/JKRDisposer.hpp>
#include <RVLFaceLib.h>

class MiiDatabase;
class MiiFaceParts;
class MiiFaceRecipe;
struct RFLDrawCoreSetting;

typedef void (MiiFaceParts::*DrawPartsFuncPtr)(const RFLDrawCoreSetting*) const;

class MiiFacePartsHolder : public LiveActorGroup, JKRDisposer {
public:
    MiiFacePartsHolder(int);

    virtual ~MiiFacePartsHolder();
    virtual void init(const JMapInfoIter& rIter);
    virtual void draw() const;
    virtual void calcAnim();
    virtual void calcViewAndEntry();

    void reinitCharModel();
    bool isInitEnd() const;
    bool isError() const;
    static MiiFaceParts* createPartsFromReceipe(const char*, const MiiFaceRecipe&);
    static MiiFaceParts* createPartsFromDefault(const char*, u16);
    void drawEachActor(DrawPartsFuncPtr, const RFLDrawCoreSetting*) const;
    void drawExtra() const;
    void setTevOpa() const;
    void setTevXlu() const;

    /* 0x38 */ u8* mRFLWorkBuffer;
    /* 0x3C */ MiiDatabase* mMiiDatabase;
    /* 0x40 */ RFLErrcode mRFLStatus;
    /* 0x44 */ s32 mLastNANDReason;
};
