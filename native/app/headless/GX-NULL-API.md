# Null GX generated API inventory

Preview generated from stable `ht4/overlay` headers. The build regenerates its definitions from the active hooked headers. Manual CPU/state/FIFO implementations live in `gx_null.cpp`. This list is not a compile or runtime validation result.

| Signature | Behavior |
| --- | --- |
| `void GXSetNumIndStages(u8)` | Discard rendering state; count the call |
| `void GXSetIndTexOrder(GXIndTexStageID,  GXTexCoordID,  GXTexMapID)` | Discard rendering state; count the call |
| `void GXSetIndTexMtx(GXIndTexMtxID,  const f32[2][3],  s8)` | Discard rendering state; count the call |
| `void GXSetTevIndWarp(GXTevStageID,  GXIndTexStageID,  GXBool,  GXBool,  GXIndTexMtxID)` | Discard rendering state; count the call |
| `void GXSetTevIndirect(GXTevStageID,  GXIndTexStageID,  GXIndTexFormat,  GXIndTexBiasSel,  GXIndTexMtxID,  GXIndTexWrap,  GXIndTexWrap,  GXBool,  GXBool,  GXIndTexAlphaSel)` | Discard rendering state; count the call |
| `void GXSetIndTexCoordScale(GXIndTexStageID,  GXIndTexScale,  GXIndTexScale)` | Discard rendering state; count the call |
| `void GXSetTevDirect(GXTevStageID)` | Discard rendering state; count the call |
| `void __GXSetIndirectMask(u32)` | Discard rendering state; count the call |
| `void __GXFlushTextureState()` | Discard rendering state; count the call |
| `void GXPokeAlphaMode(GXCompare,  u8)` | Discard rendering state; count the call |
| `void GXPokeAlphaRead(GXAlphaReadMode)` | Discard rendering state; count the call |
| `void GXPokeAlphaUpdate(GXBool)` | Discard rendering state; count the call |
| `void GXPokeBlendMode(GXBlendMode,  GXBlendFactor,  GXBlendFactor,  GXLogicOp)` | Discard rendering state; count the call |
| `void GXPokeColorUpdate(GXBool)` | Discard rendering state; count the call |
| `void GXPokeDstAlpha(GXBool,  u8)` | Discard rendering state; count the call |
| `void GXPokeDither(GXBool)` | Discard rendering state; count the call |
| `void GXPokeZMode(GXBool,  GXCompare,  GXBool)` | Discard rendering state; count the call |
| `void GXPeekARGB(u16 x,  u16 y,  u32* color)` | Named OSPanic + abort; output/query not implemented |
| `void GXPeekZ(u16 x,  u16 y,  u32* z)` | Named OSPanic + abort; output/query not implemented |
| `void GXSetCullMode(GXCullMode)` | Discard rendering state; count the call |
| `void GXSetCoPlanar(GXBool)` | Discard rendering state; count the call |
| `void __GXFifoInit()` | Discard rendering state; count the call |
| `void GXSetTexCopySrc(u16,  u16,  u16,  u16)` | Discard rendering state; count the call |
| `void GXSetDispCopyDst(u16,  u16)` | Discard rendering state; count the call |
| `void GXSetTexCopyDst(u16,  u16,  GXTexFmt,  GXBool)` | Discard rendering state; count the call |
| `void GXSetDispCopyFrame2Field(GXCopyMode)` | Discard rendering state; count the call |
| `void GXSetCopyClamp(GXFBClamp)` | Discard rendering state; count the call |
| `void GXSetCopyClear(GXColor,  u32)` | Discard rendering state; count the call |
| `void GXSetCopyFilter(GXBool,  const u8[12][2],  GXBool,  const u8[7])` | Discard rendering state; count the call |
| `void GXSetDispCopyGamma(GXGamma)` | Discard rendering state; count the call |
| `void GXCopyDisp(void*,  GXBool)` | Discard rendering state; count the call |
| `void GXCopyTex(void*,  GXBool)` | Discard rendering state; count the call |
| `void GXClearBoundingBox()` | Discard rendering state; count the call |
| `void GXSetArray(GXAttr,  const void *,  u32,  u8,  bool)` | Discard rendering state; count the call |
| `void GXSetTexCoordGen2(GXTexCoordID,  GXTexGenType,  GXTexGenSrc,  u32,  GXBool,  u32)` | Discard rendering state; count the call |
| `void GXSetNumTexGens(u8)` | Discard rendering state; count the call |
| `void GXInvalidateVtxCache()` | Discard rendering state; count the call |
| `void GXSetLineWidth(u8,  GXTexOffset)` | Discard rendering state; count the call |
| `void GXSetPointSize(u8,  GXTexOffset)` | Discard rendering state; count the call |
| `void GXEnableTexOffsets(GXTexCoordID,  GXBool,  GXBool)` | Discard rendering state; count the call |
| `void __GXSetSUTexRegs()` | Discard rendering state; count the call |
| `void __GXSetVCD()` | Discard rendering state; count the call |
| `void __GXSetVAT()` | Discard rendering state; count the call |
| `void __GXCalculateVLim()` | Discard rendering state; count the call |
| `void __GXSetGenMode()` | Discard rendering state; count the call |
| `void __GXUpdateBPMask()` | Discard rendering state; count the call |
| `void __GXSetProjection()` | Discard rendering state; count the call |
| `void __GXSetViewport()` | Discard rendering state; count the call |
| `void __GXInitGX()` | Discard rendering state; count the call |
| `void GXInitLightAttn(GXLightObj*,  f32,  f32,  f32,  f32,  f32,  f32)` | Discard rendering state; count the call |
| `void GXInitLightPos(GXLightObj*,  f32,  f32,  f32)` | Discard rendering state; count the call |
| `void GXInitLightColor(GXLightObj*,  GXColor)` | Discard rendering state; count the call |
| `void GXLoadLightObjImm(const GXLightObj*,  GXLightID)` | Discard rendering state; count the call |
| `void GXInitLightDistAttn(GXLightObj*,  f32,  f32,  GXDistAttnFn)` | Discard rendering state; count the call |
| `void GXInitLightDir(GXLightObj*,  f32,  f32,  f32)` | Discard rendering state; count the call |
| `void GXInitLightSpot(GXLightObj*,  f32,  GXSpotFn)` | Discard rendering state; count the call |
| `void GXSetNumChans(u8)` | Discard rendering state; count the call |
| `void GXInitSpecularDir(GXLightObj*,  f32,  f32,  f32)` | Discard rendering state; count the call |
| `void GXInitSpecularDirHA(GXLightObj*,  f32,  f32,  f32,  f32,  f32,  f32)` | Discard rendering state; count the call |
| `void GXSetChanAmbColor(GXChannelID,  GXColor)` | Discard rendering state; count the call |
| `void GXSetChanMatColor(GXChannelID,  GXColor)` | Discard rendering state; count the call |
| `void GXSetChanCtrl(GXChannelID,  GXBool,  GXColorSrc,  GXColorSrc,  u32,  GXDiffuseFn,  GXAttnFn)` | Discard rendering state; count the call |
| `void GXPixModeSync()` | Discard rendering state; count the call |
| `void GXSetMisc(GXMiscToken,  u32)` | Discard rendering state; count the call |
| `void __GXPEInit()` | Discard rendering state; count the call |
| `void GXSetGPMetric(GXPerf0,  GXPerf1)` | Discard rendering state; count the call |
| `void GXClearGPMetric()` | Discard rendering state; count the call |
| `void GXReadXfRasMetric(u32*,  u32*,  u32*,  u32*)` | Named OSPanic + abort; output/query not implemented |
| `void GXSetFog(GXFogType,  f32,  f32,  f32,  f32,  GXColor)` | Discard rendering state; count the call |
| `void GXSetFogRangeAdj(GXBool,  u16,  const GXFogAdjTable *)` | Discard rendering state; count the call |
| `void GXSetBlendMode(GXBlendMode,  GXBlendFactor,  GXBlendFactor,  GXLogicOp)` | Discard rendering state; count the call |
| `void GXSetColorUpdate(GXBool)` | Discard rendering state; count the call |
| `void GXSetAlphaUpdate(GXBool)` | Discard rendering state; count the call |
| `void GXSetZMode(GXBool,  GXCompare,  GXBool)` | Discard rendering state; count the call |
| `void GXSetZCompLoc(GXBool)` | Discard rendering state; count the call |
| `void GXSetPixelFmt(GXPixelFmt,  GXZFmt16)` | Discard rendering state; count the call |
| `void GXSetDither(GXBool)` | Discard rendering state; count the call |
| `void GXSetDstAlpha(GXBool,  u8)` | Discard rendering state; count the call |
| `void GXSetFieldMask(GXBool,  GXBool)` | Discard rendering state; count the call |
| `void GXSetFieldMode(GXBool,  GXBool)` | Discard rendering state; count the call |
| `void GXSetTevOp(GXTevStageID,  GXTevMode)` | Discard rendering state; count the call |
| `void GXSetTevColorIn(GXTevStageID,  GXTevColorArg,  GXTevColorArg,  GXTevColorArg,  GXTevColorArg)` | Discard rendering state; count the call |
| `void GXSetTevAlphaIn(GXTevStageID,  GXTevAlphaArg,  GXTevAlphaArg,  GXTevAlphaArg,  GXTevAlphaArg)` | Discard rendering state; count the call |
| `void GXSetTevColorOp(GXTevStageID,  GXTevOp,  GXTevBias,  GXTevScale,  GXBool,  GXTevRegID)` | Discard rendering state; count the call |
| `void GXSetTevAlphaOp(GXTevStageID,  GXTevOp,  GXTevBias,  GXTevScale,  GXBool,  GXTevRegID)` | Discard rendering state; count the call |
| `void GXSetTevColor(GXTevRegID,  GXColor)` | Discard rendering state; count the call |
| `void GXSetTevColorS10(GXTevRegID,  GXColorS10)` | Discard rendering state; count the call |
| `void GXSetAlphaCompare(GXCompare,  u8,  GXAlphaOp,  GXCompare,  u8)` | Discard rendering state; count the call |
| `void GXSetTevOrder(GXTevStageID,  GXTexCoordID,  GXTexMapID,  GXChannelID)` | Discard rendering state; count the call |
| `void GXSetZTexture(GXZTexOp,  GXTexFmt,  u32)` | Discard rendering state; count the call |
| `void GXSetNumTevStages(u8)` | Discard rendering state; count the call |
| `void GXSetTevKColor(GXTevKColorID,  GXColor)` | Discard rendering state; count the call |
| `void GXSetTevKColorSel(GXTevStageID,  GXTevKColorSel)` | Discard rendering state; count the call |
| `void GXSetTevKAlphaSel(GXTevStageID,  GXTevKAlphaSel)` | Discard rendering state; count the call |
| `void GXSetTevSwapModeTable(GXTevSwapSel,  GXTevColorChan,  GXTevColorChan,  GXTevColorChan,  GXTevColorChan)` | Discard rendering state; count the call |
| `void GXSetTevSwapMode(GXTevStageID,  GXTevSwapSel,  GXTevSwapSel)` | Discard rendering state; count the call |
| `void GXLoadTexObj(const GXTexObj *,  GXTexMapID)` | Discard rendering state; count the call |
| `void GXInitTlutObj(GXTlutObj *,  void *,  GXTlutFmt,  u16)` | Discard rendering state; count the call |
| `void GXLoadTlut(const GXTlutObj *,  u32)` | Discard rendering state; count the call |
| `void GXInvalidateTexAll()` | Discard rendering state; count the call |
| `void GXTexModeSync()` | Discard rendering state; count the call |
| `void GXInitTexCacheRegion(GXTexRegion *,  GXBool,  u32,  GXTexCacheSize,  u32,  GXTexCacheSize)` | Discard rendering state; count the call |
| `void GXInitTlutRegion(GXTlutRegion *,  u32,  GXTlutSize)` | Discard rendering state; count the call |
| `void GXSetTexCoordBias(GXTexCoordID coord,  GXBool sBias,  GXBool tBias)` | Discard rendering state; count the call |
| `void GXSetTexCoordScaleManually(GXTexCoordID coord,  GXBool enable,  u16 ss,  u16 ts)` | Discard rendering state; count the call |
| `void GXSetTexCoordCylWrap(GXTexCoordID coord,  GXBool wrapS,  GXBool wrapT)` | Discard rendering state; count the call |
| `void GXLoadPosMtxImm(const f32 mtx[3][4],  u32 id)` | Discard rendering state; count the call |
| `void GXLoadPosMtxIndx(u16 mtx_indx,  u32 id)` | Discard rendering state; count the call |
| `void GXLoadNrmMtxImm(const f32 mtx[3][4],  u32 id)` | Discard rendering state; count the call |
| `void GXLoadNrmMtxImm3x3(const f32 mtx[3][3],  u32 id)` | Discard rendering state; count the call |
| `void GXLoadNrmMtxIndx3x3(u16 mtx_indx,  u32 id)` | Discard rendering state; count the call |
| `void GXSetCurrentMtx(u32 id)` | Discard rendering state; count the call |
| `void GXLoadTexMtxImm(const f32 mtx[][4],  u32 id,  GXTexMtxType type)` | Discard rendering state; count the call |
| `void GXLoadTexMtxIndx(u16 mtx_indx,  u32 id,  GXTexMtxType type)` | Discard rendering state; count the call |
| `void GXSetZScaleOffset(f32,  f32)` | Discard rendering state; count the call |
| `void GXSetScissorBoxOffset(s32,  s32)` | Discard rendering state; count the call |
| `void GXSetClipMode(GXClipMode)` | Discard rendering state; count the call |
| `void __GXSetMatrixIndex(GXAttr)` | Discard rendering state; count the call |
| `void __GXAbort()` | Discard rendering state; count the call |
| `void __GXSetTmemConfig(u32)` | Discard rendering state; count the call |
