#pragma once

#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

class LiveActor;

namespace MR {
    void syncJointAnimation(LiveActor*, const LiveActor*);

    void syncMaterialAnimation(LiveActor*, const LiveActor*);
};  // namespace MR

// Merged from Petari (core lane): declarations missing here
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>
#include <JSystem/JGeometry/TBox.hpp>
#include <revolution.h>
class J3DModel;
class J3DMaterial;
class JUTTexture;
class ResourceHolder;
struct ResTIMG;
class XanimePlayer;
class XanimeResourceTable;
class J3DModelData;
class J3DShape;

namespace MR {
    ResTIMG* getTexture(ResourceHolder*, const char*);
    XanimePlayer* newXanimePlayer(const ResourceHolder*, const char*, const ResourceHolder*, J3DMdlFlag, XanimeResourceTable*);
    XanimeResourceTable* newXanimeResourceTable(ResourceHolder*);
    void updateModelManager(LiveActor*);
    void calcAnimModelManager(LiveActor*);
    void updateModelAnimPlayer(LiveActor*);
    void invalidateMtxCalc(J3DModelData*);
    void invalidateJointCallback(J3DModelData*);
    J3DModel* getJ3DModel(const LiveActor*);
    void calcJ3DModel(LiveActor*);
    J3DModelData* getJ3DModelData(const LiveActor*);
    bool findBckNameStringInResource(const char**, const ResourceHolder*, const char*);
    u16 getMaterialNo(J3DModelData*, const char*);
    J3DMaterial* getMaterial(J3DModelData*, const char*);
    s32 getMaterialNum(J3DModel*);
    const char* getMaterialName(const J3DModelData*, int);
    void updateModelDiffDL(LiveActor*);
    bool isEnvelope(J3DMaterial*);
    void hideMaterial(J3DModel*, const char*);
    void showMaterial(const J3DModel*, const char*);
    void calcFogStartEnd(TVec3f, f32, f32*, f32*);
    bool isUseTex(J3DMaterial*, u16);
    bool isNormalTexMtx(J3DMaterial*);
    bool isUseTexMtx(J3DMaterial*, u32);
    bool isExistEffectTexMtx(J3DModelData*);
    bool isExistEffectTexMtx(LiveActor*);
    bool isExistProjMapTexMtx(J3DModelData*);
    bool isExistProjMapTexMtx(LiveActor*);
#ifdef MODELUTIL_OUTLINE_TEXTURE_COUNT
    u16 getTextureNum(const LiveActor*) NO_INLINE;
#else
    u16 getTextureNum(const LiveActor*);
#endif
    u16 getMaterialNum(const LiveActor*);
    bool isUseTexMtxEnvMap(J3DMaterial*);
    bool isUseTexMtxProjMap(J3DMaterial*);
    void setShapeVcdVatCmdSelf(J3DShape*);
    void initEnvelopeAndEnvMapOrProjMapModelData(J3DModelData*);
    ResTIMG* getResTIMG(const LiveActor*, int);
    JUTTexture* getTextue(ResourceHolder*, const char*);
    const char* getTextureName(const LiveActor*, int);
    s32 fn_80043AB0(const LiveActor*, const char*);
    int findPosNrmMtxIndexFromModel(J3DModelData*, u16);
    int findPosNrmMtxIndexFromShape(J3DShape*, u16);
    void copyJointAnimation(J3DModel*, J3DModel*);
    void getLightNum(J3DMaterial*, s32*, s32*, s32*, s32*);
    void downFracVtx(J3DModelData*);
    void calcModelBoundingBox(TBox3f*, const LiveActor*);
    void calcModelBoundingRadius(f32*, const LiveActor*);
    bool isExistCollisionResource(const LiveActor*, const char*);
    bool isExistModel(const char*);
    bool isExistSubModel(const char*, const char*);
    J3DModel* newJ3DModel(const ResourceHolder*, const char*, J3DMdlFlag);
    bool isUseFur(const J3DModelData*);
};  // namespace MR

// Merged from Petari (core lane): overloads missing here


namespace MR {
    s16 getBckFrameMax(const LiveActor*, const char*);
    s16 getBrkFrameMax(const LiveActor*, const char*);
    s16 getBvaFrameMax(const LiveActor*, const char*);
    bool isBckPlaying(XanimePlayer*, const char*);
    u16 getMaterialNo(J3DModel*, const char*);
    J3DMaterial* getMaterial(J3DModelData*, int);
    J3DMaterial* getMaterial(J3DModel*, int);
    J3DMaterial* getMaterial(const LiveActor*, int);
    void hideMaterial(const LiveActor*, const char*);
    void showMaterial(const LiveActor*, const char*);
    ResTIMG* getResTIMG(const J3DModelData*, int);
    void copyJointAnimation(LiveActor*, const LiveActor*);
    void syncMaterialAnimation(J3DModel*, J3DModel*);
    s32 getLightNum(J3DMaterial*);
    J3DModelData* getJ3DModelData(const char*);
};  // namespace MR
