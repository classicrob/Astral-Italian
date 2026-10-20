#include "gx_ledger.hpp"
#include <revolution/gx.h>
#include <revolution/vi.h>
#include <revolution/os.h>
#include <petari/platform/gx_sync.hpp>
#include <petari/platform/null_gx_processor.hpp>
#include <petari/host_allocation.hpp>
#include <petari/progress_hook.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <type_traits>

namespace {
using PetariNative::Headless::GxLedger;
namespace Sync = PetariNative::Platform::GXSync;
GxLedger gFifo;
using Processor = PetariNative::Platform::NullGxProcessor;
Processor gProcessor;
// All use of the ledger/processor is serialized by the OS interrupt lock.
unsigned gDeferProcessing = 0;
class InterruptGuard {
public:
    InterruptGuard() : mEnabled(OSDisableInterrupts()) {}
    ~InterruptGuard() { OSRestoreInterrupts(mEnabled); }
private:
    BOOL mEnabled;
};
void pump() {
    PetariNative::HostAllocationScope host;
    gProcessor.publish(gFifo.bytes());
    gProcessor.pump(Sync::breakpointPosition(), [](const Processor::Event& event) {
        if (event.kind == Processor::Kind::Token) Sync::reportToken(event.token, event.position);
        else Sync::reportDrawDone(event.position);
    }, [](auto processed, auto written) { Sync::reportProgress(processed, written); },
       [](auto position) { Sync::reportBreakpointReached(position); });
}
std::uint64_t gCalls = 0, gDisplayListBytes = 0, gLastDone = 0;
GXFifoObj gCpu{}, gGpu{};
f32 gViewport[6] = {0, 0, 640, 480, 0, 1};
f32 gProjection[7]{};
u32 gScissor[4] = {0, 0, 640, 480};
u16 gCopyHeight = 480;
GXAttrType gDescriptors[32]{};
GXVtxAttrFmtList gAttributes[8][32]{};
GXTexRegionCallback gTexCallback = nullptr;
GXTlutRegionCallback gTlutCallback = nullptr;
[[noreturn]] void unsupported(const char* function) {
    OSPanic(__FILE__, __LINE__, "headless GX: %s requires a CPU implementation or render feedback", function);
    std::abort();
}
void emit(const void* data, u32 count) {
    InterruptGuard interrupts;
    try { gFifo.write(data, count); }
    catch (const std::exception& error) { OSPanic(__FILE__, __LINE__, "%s", error.what()); std::abort(); }
    if (!gFifo.recording() && !gDeferProcessing) pump();
}
void word(std::uint64_t value, unsigned count) {
    u8 bytes[8];
    for (unsigned i = 0; i < count; ++i) bytes[count - i - 1] = static_cast<u8>(value >> (8 * i));
    emit(bytes, count);
}
struct Texture {
    void* data;
    u16 width, height;
    u32 format, wrapS, wrapT;
    GXBool mip;
    u32 minFilter, magFilter;
    f32 minLod, maxLod, bias;
    GXBool biasClamp, edge;
    u32 anisotropy, tlut;
};
static_assert(sizeof(Texture) <= sizeof(GXTexObj));
Texture texture(const GXTexObj* object) { Texture value; std::memcpy(&value, object, sizeof(value)); return value; }
void texture(GXTexObj* object, const Texture& value) { std::memset(object, 0, sizeof(*object)); std::memcpy(object, &value, sizeof(value)); }
GXRenderModeObj mode(VITVMode tv, u16 height) {
    GXRenderModeObj result{};
    result.viTVmode = tv; result.fbWidth = result.viWidth = 640;
    result.efbHeight = result.xfbHeight = result.viHeight = height;
    result.xFBmode = VI_XFBMODE_DF;
    for (auto& sample : result.sample_pattern) sample[0] = sample[1] = 6;
    const u8 filter[7] = {8, 8, 10, 12, 10, 8, 8};
    std::memcpy(result.vfilter, filter, 7);
    return result;
}
}
extern "C" {
void petari_gx_pipeline_stage_begin(const char*, const char*) { ++gCalls; }
bool petari_gx_waiting_for_pipeline() { return false; }
void petari_progress_badge(const char*, int, float, float) { ++gCalls; }
void petari_progress_star_get(int, int) { ++gCalls; }
GXRenderModeObj GXNtsc480IntDf = mode(VI_TVMODE_NTSC_INT, 480);
GXRenderModeObj GXNtsc480Int = mode(VI_TVMODE_NTSC_INT, 480);
GXRenderModeObj GXMpal480IntDf = mode(VI_TVMODE_MPAL_INT, 480);
GXRenderModeObj GXPal528IntDf = mode(VI_TVMODE_PAL_INT, 528);
GXRenderModeObj GXEurgb60Hz480IntDf = mode(VI_TVMODE_EURGB60_INT, 480);
GXFifoObj* GXInit(void*, u32) { return &gCpu; }
// No depth buffer headlessly: every depth peek reads the far plane, i.e. nothing in front (sun/lens-flare and
// Star Pointer occlusion tests see an unobstructed view). Listed in HARNESS.md as a known headless difference.
void GXPeekZ(u16, u16, u32* z) { ++gCalls; *z = 0xFFFFFF; }
// No framebuffer headlessly either: colour peeks read transparent black.
void GXPeekARGB(u16, u16, u32* color) { ++gCalls; *color = 0; }
void GXInitFifoBase(GXFifoObj* object, void*, u32) { std::memset(object, 0, sizeof(*object)); }
void GXSetCPUFifo(const GXFifoObj* object) { gCpu = *object; }
void GXSetGPFifo(const GXFifoObj* object) { gGpu = *object; }
GXBool GXGetCPUFifo(GXFifoObj* object) { *object = gCpu; return GX_TRUE; }
GXBool GXGetGPFifo(GXFifoObj* object) { *object = gGpu; return GX_TRUE; }
GXBool GXGetFifoWrap(const GXFifoObj*) { return GX_FALSE; }
GXBool __GXIsGPFifoReady() { return GX_TRUE; }
void GXGetFifoPtrs(const GXFifoObj*, void** read, void** write) {
    InterruptGuard interrupts;
    if (read) *read = Sync::positionToPointer(gProcessor.processed());
    if (write) *write = Sync::positionToPointer(gFifo.bytes());
}
void GXGetGPStatus(GXBool* over, GXBool* under, GXBool* readIdle, GXBool* commandIdle, GXBool* breakpoint) {
    const auto status=Sync::gpStatus();
    if (over) *over = GX_FALSE; if (under) *under = GX_FALSE;
    if (readIdle) *readIdle = status.readIdle; if (commandIdle) *commandIdle = status.commandIdle;
    if (breakpoint) *breakpoint = status.breakpoint;
}
void GXEnableBreakPt(void* pointer) {
    InterruptGuard interrupts;
    Sync::setBreakpoint(Sync::pointerToPosition(pointer));
    pump();
}
void GXDisableBreakPt() { InterruptGuard interrupts; Sync::clearBreakpoint(); pump(); }
GXBreakPtCallback GXSetBreakPtCallback(GXBreakPtCallback cb) { return Sync::setBreakpointCallback(cb); }
GXDrawSyncCallback GXSetDrawSyncCallback(GXDrawSyncCallback cb) { return Sync::setDrawSyncCallback(cb); }
GXDrawDoneCallback GXSetDrawDoneCallback(GXDrawDoneCallback cb) { return Sync::setDrawDoneCallback(cb); }
void GXSetDrawSync(u16 token) {
    InterruptGuard interrupts;
    PetariNative::HostAllocationScope host;
    // Register the command event before exposing its bytes to processing. The
    // interrupt lock also prevents DrawSyncManager arming a stale saved pointer
    // between this command's write and event registration.
    ++gDeferProcessing;
    word(0x61, 1); word(0x48000000U | token, 4);
    --gDeferProcessing;
    if (!gFifo.recording()) {
        gProcessor.publish(gFifo.bytes());
        gProcessor.append(Processor::Kind::Token, token, gFifo.bytes());
        pump();
    }
}
void GXSetDrawDone() {
    InterruptGuard interrupts;
    PetariNative::HostAllocationScope host;
    ++gDeferProcessing;
    word(0x61, 1); word(0x45000002, 4);
    --gDeferProcessing;
    if (!gFifo.recording()) {
        gLastDone = Sync::noteDrawDoneIssued();
        gProcessor.publish(gFifo.bytes());
        gProcessor.append(Processor::Kind::DrawDone, 0, gFifo.bytes());
        pump();
    }
}
void GXDrawDone() { GXSetDrawDone(); Sync::waitDrawDone(gLastDone); }
u16 GXReadDrawSync() { return Sync::lastToken(); }
void GXAbortFrame() {
    InterruptGuard interrupts;
    PetariNative::HostAllocationScope host;
    Sync::abortFrame();
    gProcessor.abort(gFifo.bytes());
    Sync::abortApplied(gProcessor.processed(), gProcessor.written());
}
void GXFlush() { InterruptGuard interrupts; if (!gFifo.recording()) pump(); }
void GXBeginDisplayList(void* buffer, u32 capacity) { InterruptGuard interrupts; gFifo.begin(buffer, capacity); }
u32 GXEndDisplayList() { InterruptGuard interrupts; return gFifo.end(); }
void GXCallDisplayList(const void* buffer, u32 size) {
    if (size && !buffer) unsupported("GXCallDisplayList(null)");
    word(0x40, 1); word(0, 4); word(size, 4); gDisplayListBytes += size;
}
void GXBegin(GXPrimitive primitive, GXVtxFmt format, u16 vertices) { word(static_cast<u8>(primitive) | static_cast<u8>(format), 1); word(vertices, 2); }
void GXEnd() {}
void GXInsertDebugMarker(const char*) {}
void GXSetVtxDesc(GXAttr attr, GXAttrType type) {
    if (static_cast<unsigned>(attr)>=32) unsupported("GXSetVtxDesc(attr)");
    gDescriptors[attr]=type;
}
void GXClearVtxDesc() { for (auto& type:gDescriptors) type=GX_NONE; }
void GXSetVtxDescv(GXVtxDescList* list) {
    for (unsigned i=0;i<32;++i) {if (list[i].attr==GX_VA_NULL) return; GXSetVtxDesc(list[i].attr,list[i].type);}
    unsupported("GXSetVtxDescv(unterminated)");
}
void GXGetVtxDesc(GXAttr attr, GXAttrType* type) {
    if (static_cast<unsigned>(attr)>=32) unsupported("GXGetVtxDesc(attr)");
    *type=gDescriptors[attr];
}
void GXGetVtxDescv(GXVtxDescList* list) {
    unsigned j=0;
    for (unsigned i=0;i<=GX_VA_TEX7;++i) if (gDescriptors[i]!=GX_NONE) list[j++]={static_cast<GXAttr>(i),gDescriptors[i]};
    list[j]={GX_VA_NULL,GX_NONE};
}
void GXSetVtxAttrFmt(GXVtxFmt fmt,GXAttr attr,GXCompCnt count,GXCompType type,u8 frac) {
    if (static_cast<unsigned>(fmt)>=8 || static_cast<unsigned>(attr)>=32) unsupported("GXSetVtxAttrFmt(index)");
    gAttributes[fmt][attr]={attr,count,type,frac};
}
void GXSetVtxAttrFmtv(GXVtxFmt fmt,const GXVtxAttrFmtList* list) {
    for (unsigned i=0;i<32;++i) {if (list[i].attr==GX_VA_NULL) return; GXSetVtxAttrFmt(fmt,list[i].attr,list[i].cnt,list[i].type,list[i].frac);}
    unsupported("GXSetVtxAttrFmtv(unterminated)");
}
void GXGetVtxAttrFmtv(GXVtxFmt fmt,GXVtxAttrFmtList* list) {
    if (static_cast<unsigned>(fmt)>=8) unsupported("GXGetVtxAttrFmtv(format)");
    unsigned j=0;
    for (unsigned i=GX_VA_POS;i<=GX_VA_TEX7;++i) {list[j]=gAttributes[fmt][i];list[j++].attr=static_cast<GXAttr>(i);}
    list[j].attr=GX_VA_NULL;
}
#define VERT1(name, type) void name(type x) { word(static_cast<std::make_unsigned_t<type>>(x), sizeof(type)); }
#define VERT2(name, type) void name(type x, type y) { word(static_cast<std::make_unsigned_t<type>>(x), sizeof(type)); word(static_cast<std::make_unsigned_t<type>>(y), sizeof(type)); }
#define VERT3(name, type) void name(type x, type y, type z) { word(static_cast<std::make_unsigned_t<type>>(x), sizeof(type)); word(static_cast<std::make_unsigned_t<type>>(y), sizeof(type)); word(static_cast<std::make_unsigned_t<type>>(z), sizeof(type)); }
VERT1(GXCmd1u8, u8) VERT1(GXCmd1u16, u16) VERT1(GXCmd1u32, u32)
VERT1(GXPosition1x8, u8) VERT1(GXNormal1x8, u8) VERT1(GXTexCoord1x8, u8)
VERT3(GXPosition3u8, u8) VERT3(GXPosition3s8, s8) VERT3(GXPosition3u16, u16) VERT3(GXPosition3s16, s16)
VERT2(GXPosition2u8, u8) VERT2(GXPosition2s8, s8) VERT2(GXPosition2u16, u16) VERT2(GXPosition2s16, s16)
VERT1(GXColor1u32, u32)
VERT2(GXTexCoord2u8, u8) VERT2(GXTexCoord2u16, u16) VERT2(GXTexCoord2s16, s16)
VERT2(GXTexCoord2s8, s8)
#undef VERT1
#undef VERT2
#undef VERT3
void GXCmd1f32(f32 value) { u32 bits; std::memcpy(&bits, &value, 4); GXCmd1u32(bits); }
void GXPosition3f32(f32 x, f32 y, f32 z) { GXCmd1f32(x); GXCmd1f32(y); GXCmd1f32(z); }
void GXPosition2f32(f32 x, f32 y) { GXCmd1f32(x); GXCmd1f32(y); }
void GXNormal3f32(f32 x, f32 y, f32 z) { GXPosition3f32(x, y, z); }
void GXTexCoord2f32(f32 x, f32 y) { GXPosition2f32(x, y); }
void GXColor4u8(u8 r, u8 g, u8 b, u8 a) { word((u32(r) << 24) | (u32(g) << 16) | (u32(b) << 8) | a, 4); }
void GXInitTexObj(GXTexObj* object, void* data, u16 w, u16 h, GXTexFmt fmt, GXTexWrapMode s, GXTexWrapMode t, GXBool mip) {
    Texture value{}; value.data = data; value.width = w; value.height = h; value.format = fmt;
    value.wrapS = s; value.wrapT = t; value.mip = mip; value.minFilter = GX_LINEAR; value.magFilter = GX_LINEAR;
    texture(object, value);
}
void GXInitTexObjCI(GXTexObj* object, void* data, u16 w, u16 h, GXCITexFmt fmt, GXTexWrapMode s, GXTexWrapMode t, GXBool mip, u32 tlut) {
    GXInitTexObj(object, data, w, h, static_cast<GXTexFmt>(fmt), s, t, mip); GXInitTexObjTlut(object, tlut);
}
void GXInitTexObjTlut(GXTexObj* object, u32 tlut) { auto value = texture(object); value.tlut = tlut; texture(object, value); }
void GXInitTexObjLOD(GXTexObj* object, GXTexFilter min, GXTexFilter mag, f32 lo, f32 hi, f32 bias, GXBool clamp, GXBool edge, GXAnisotropy aniso) {
    auto value = texture(object); value.minFilter = min; value.magFilter = mag; value.minLod = lo; value.maxLod = hi;
    value.bias = bias; value.biasClamp = clamp; value.edge = edge; value.anisotropy = aniso; texture(object, value);
}
u32 GXGetTexObjTlut(const GXTexObj* object) { return texture(object).tlut; }
GXTexFmt GXGetTexObjFmt(const GXTexObj* object) { return static_cast<GXTexFmt>(texture(object).format); }
GXBool GXGetTexObjMipMap(const GXTexObj* object) { return texture(object).mip; }
void GXGetTexObjAll(const GXTexObj* object, void** data, u16* w, u16* h, GXTexFmt* fmt, GXTexWrapMode* s, GXTexWrapMode* t, GXBool* mip) {
    const auto v = texture(object); if (data) *data = v.data; if (w) *w = v.width; if (h) *h = v.height;
    if (fmt) *fmt = static_cast<GXTexFmt>(v.format); if (s) *s = static_cast<GXTexWrapMode>(v.wrapS);
    if (t) *t = static_cast<GXTexWrapMode>(v.wrapT); if (mip) *mip = v.mip;
}
void GXGetTexObjLODAll(const GXTexObj* object, GXTexFilter* min, GXTexFilter* mag, f32* lo, f32* hi, f32* bias, GXBool* clamp, GXBool* edge, GXAnisotropy* aniso) {
    const auto v = texture(object); if (min) *min = static_cast<GXTexFilter>(v.minFilter); if (mag) *mag = static_cast<GXTexFilter>(v.magFilter);
    if (lo) *lo = v.minLod; if (hi) *hi = v.maxLod; if (bias) *bias = v.bias;
    if (clamp) *clamp = v.biasClamp; if (edge) *edge = v.edge; if (aniso) *aniso = static_cast<GXAnisotropy>(v.anisotropy);
}
u32 GXGetTexBufferSize(u16 width, u16 height, u32 format, GXBool mip, u8 maxLod) {
    unsigned tw = 4, th = 4, bytes = 32;
    switch (format) {
    case 0: case 8: case 14: tw = th = 8; break;
    case 1: case 2: case 9: case 0x11: tw = 8; break;
    case 3: case 4: case 5: case 10: case 0x13: break;
    case 6: case 0x16: bytes = 64; break;
    default: unsupported("GXGetTexBufferSize(format)");
    }
    std::uint64_t total = 0; unsigned w = width, h = height;
    for (unsigned level = 0; ; ++level) {
        total += std::uint64_t((w + tw - 1) / tw) * ((h + th - 1) / th) * bytes;
        if (!mip || level >= maxLod || (w <= 1 && h <= 1)) break;
        w = std::max(1U, w / 2); h = std::max(1U, h / 2);
    }
    if (total > UINT32_MAX) unsupported("GXGetTexBufferSize(overflow)");
    return static_cast<u32>(total);
}
GXTexRegionCallback GXSetTexRegionCallback(GXTexRegionCallback callback) { auto old = gTexCallback; gTexCallback = callback; return old; }
GXTlutRegionCallback GXSetTlutRegionCallback(GXTlutRegionCallback callback) { auto old = gTlutCallback; gTlutCallback = callback; return old; }
void GXSetViewport(f32 x, f32 y, f32 w, f32 h, f32 near, f32 far) { const f32 v[6] = {x,y,w,h,near,far}; std::memcpy(gViewport,v,sizeof(v)); }
void GXGetViewportv(f32* view) { std::memcpy(view, gViewport, sizeof(gViewport)); }
void GXSetProjectionv(const f32* value) { std::memcpy(gProjection, value, sizeof(gProjection)); }
void GXGetProjectionv(f32* value) { std::memcpy(value, gProjection, sizeof(gProjection)); }
void GXSetProjection(const f32 matrix[4][4], GXProjectionType type) {
    const unsigned c = type == GX_PERSPECTIVE ? 2 : 3;
    const f32 p[7] = {static_cast<f32>(type), matrix[0][0], matrix[0][c], matrix[1][1], matrix[1][c], matrix[2][2], matrix[2][3]};
    GXSetProjectionv(p);
}
void GXProject(f32 x, f32 y, f32 z, const f32 matrix[3][4], const f32* p, const f32* v, f32* sx, f32* sy, f32* sz) {
    const f32 cx = matrix[0][0]*x + matrix[0][1]*y + matrix[0][2]*z + matrix[0][3];
    const f32 cy = matrix[1][0]*x + matrix[1][1]*y + matrix[1][2]*z + matrix[1][3];
    const f32 cz = matrix[2][0]*x + matrix[2][1]*y + matrix[2][2]*z + matrix[2][3];
    const bool perspective = p[0] == GX_PERSPECTIVE;
    const f32 inv = perspective ? -1.0f/cz : 1.0f;
    *sx = v[0] + v[2] * 0.5f * (1.0f + (p[1]*cx + p[2]*(perspective ? cz : 1.0f))*inv);
    *sy = v[1] + v[3] * 0.5f * (1.0f - (p[3]*cy + p[4]*(perspective ? cz : 1.0f))*inv);
    *sz = v[5] + (p[5]*cz + p[6])*inv*(v[5]-v[4]);
}
void GXSetScissor(u32 x, u32 y, u32 w, u32 h) { const u32 v[4] = {x,y,w,h}; std::memcpy(gScissor,v,sizeof(v)); }
void GXGetScissor(u32* x, u32* y, u32* w, u32* h) { *x=gScissor[0]; *y=gScissor[1]; *w=gScissor[2]; *h=gScissor[3]; }
f32 GXGetYScaleFactor(u16 h, u16 xfb) { if (!h) unsupported("GXGetYScaleFactor(0)"); return static_cast<f32>(xfb)/h; }
u16 GXGetNumXfbLines(u16 h, f32 scale) {
    if (!h || !std::isfinite(scale) || scale < 1.0f/256 || scale > 256) unsupported("GXGetNumXfbLines(scale)");
    const u32 inverse = static_cast<u32>(256.0f/scale);
    return static_cast<u16>(std::min(1024U, 1U + ((u32(h)-1U)*256U)/inverse));
}
void GXSetDispCopySrc(u16, u16, u16, u16 h) { gCopyHeight = h; }
u32 GXSetDispCopyYScale(f32 scale) { return GXGetNumXfbLines(gCopyHeight, scale); }
#include <gx_noops.inc>
}
PetariNative::Headless::GxStats PetariNative::Headless::gxStats() {
    return {gFifo.bytes(),gCalls,gDisplayListBytes};
}
