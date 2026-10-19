#pragma once

#include <revolution/types.h>

class LiveActor;

namespace MR {
    void drawScreenAlphaShadow(s32, bool, bool);
    void drawModelAlphaShadow(const LiveActor*);
    void setupDrawForNW4RLayout(f32);
};

// Merged from Petari (core lane): declarations missing here
#include <JSystem/J2DGraph/J2DOrthoGraph.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/gx.h>
class J3DModelData;
class J3DModel;
class J3DMaterial;
class JUTTexture;

class J2DOrthoGraphSimple : public J2DOrthoGraph {
public:
    J2DOrthoGraphSimple();

    virtual ~J2DOrthoGraphSimple() {
    }

    virtual void setPort();
};

namespace MR {
    void drawInit();
    void reinitGX();
    void resetTextureCacheSize();
    void loadViewMtxFor2DModel();
    void drawInitFor2DModel();
    void setDefaultViewportAndScissor();
    void loadProjectionMtxFor2D();
    void clearZBuffer();
    void clearAlphaBuffer(u8);
    void fillScreenSetup(const GXColor&);
    void fillScreenArea(const TVec2s&, const TVec2s&);
    void fillScreen(const GXColor&);
    void setupShadowVolumeDraw();
    void drawSimpleModel(J3DModelData*);
    void drawModelAlphaShadow(J3DModel*) NO_INLINE;
    void fillSilhouetteColor();
    const JUTTexture* getMarioShadowTex();
    const JUTTexture* getMarioShadowTexForDisplayList();
    const JUTTexture* getMarioShadowTexForLoad();
    const TVec3f& getMarioShadowVec();
    void setMarioShadowTex(const JUTTexture*);
    void setMarioShadowVec(const TVec3f&);
    void loadTexProjectionMtx(u32);
    void nonFilteredCapture(JUTTexture*, s16, s16);
    void simpleDraw(J3DModel*, J3DMaterial*);
    void activateGameSceneDraw3D();
    void deactivateGameSceneDraw3D();
};  // namespace MR

// Merged from Petari (core lane): overloads missing here


namespace MR {
    void clearAlphaBuffer(u8, const TVec2f&, const TVec2f&);
};  // namespace MR
