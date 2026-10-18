#pragma once
#include "Game/LiveActor/PartsModel.hpp"
class MagmaMeteor;
class MagmaMeteorParts : public PartsModel {
public:
    MagmaMeteorParts(MagmaMeteor*, const char*);
    virtual ~MagmaMeteorParts();
    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();
    virtual void exeFall();
    void setOffset(const TVec3f&);
    const TVec3f& getOffset() const;
    TVec3f mOffset;
};
