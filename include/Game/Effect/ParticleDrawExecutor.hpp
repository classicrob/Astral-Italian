#pragma once

#include <JSystem/JGeometry/TMatrix.hpp>

class EffectSystem;
class NameObjAdaptor;

class ParticleDrawExecutor {
public:
    ParticleDrawExecutor(const EffectSystem*, bool);

    static void initDraw();
    void draw3D() const;
    void draw2D() const;
    void drawIndirect() const;
    void drawAfterIndirect() const;
    void drawFor2DModel() const;
    void drawForBloomEffect() const;
    void drawAfterImageEffect() const;
    void drawForWorldMap() const;
    void drawWithViewMtx3D(const TPos3f&) const;
    void drawWithViewMtxAfterIndirect(const TPos3f&) const;
    void drawWithViewMtxForBloomEffect(const TPos3f&) const;
    void drawWithViewMtxAfterImageEffect(const TPos3f&) const;
    void drawWithViewMtxForWorldMap(const TPos3f&) const;
    void initDrawAdaptor();

    /* 0x00 */ const EffectSystem* mHost;
    /* 0x04 */ NameObjAdaptor* mDraw3DAdaptor;
    /* 0x08 */ NameObjAdaptor* mDraw2DAdaptor;
    /* 0x0C */ NameObjAdaptor* mDrawIndirectAdaptor;
    /* 0x10 */ NameObjAdaptor* mDrawAfterIndirectAdaptor;
    /* 0x14 */ NameObjAdaptor* mDraw2DModelAdaptor;
    /* 0x18 */ NameObjAdaptor* mDrawBloomAdaptor;
    /* 0x1C */ NameObjAdaptor* mDrawAfterImageEffectAdaptor;
    /* 0x20 */ NameObjAdaptor* mDrawWorldMapAdaptor;
    /* 0x24 */ bool mIsDrawEnabled;
    /* 0x25 */ bool _21;
};
