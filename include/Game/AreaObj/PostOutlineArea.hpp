#pragma once

#include "Game/AreaObj/ImageEffectArea.hpp"

namespace MR {
    void createPostOutline();
};  // namespace MR

class PostOutlineArea : public ImageEffectArea {
public:
    PostOutlineArea(const char*);

    virtual void init(const JMapInfoIter&);
    virtual const char* getManagerName() const;

    s32 getIntensity() const;
};
