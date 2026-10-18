#pragma once
#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry/TVec.hpp>
class StarPieceFlowerController : public NameObj {
public:
    StarPieceFlowerController(const char*);
    virtual ~StarPieceFlowerController();
    virtual void init(const JMapInfoIter&);
    virtual void movement();
    TVec3f mRotation;
};
namespace MR { void getStarPieceFlowerRotation(TVec3f*); }
