#pragma once
#include "Game/NameObj/NameObj.hpp"
class ExterminationWoodBoxFlagHolder : public NameObj {
public:
    ExterminationWoodBoxFlagHolder(const char*);
    virtual ~ExterminationWoodBoxFlagHolder();
    virtual void init(const JMapInfoIter&);
    bool mFlag;
};
namespace MR { void setExterminationWoodBoxFlag(); void clearExterminationWoodBoxFlag(); bool getExterminationWoodBoxFlagHolder(); }
