#pragma once

#include "Game/AreaObj/ImageEffectArea.hpp"

namespace MR {
    void createPostFog();
};  // namespace MR

class PostFogArea : public ImageEffectArea {
public:
    PostFogArea(const char*);

    virtual void init(const JMapInfoIter&);
    virtual const char* getManagerName() const;

    s32 getMinRange() const;
    s32 getMaxRange() const;
    u8 getAlpha() const;
    u8 getRed() const;
    u8 getGreen() const;
    u8 getBlue() const;
    s32 getGradType() const;
};
