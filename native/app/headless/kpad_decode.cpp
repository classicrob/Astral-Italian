#include "platform.hpp"
#include <cstring>

namespace PetariNative::Headless {
namespace {
u32 u32at(const Sample& bytes, unsigned offset) {
    u32 value = 0;
    for (unsigned i = 0; i < 4; ++i) value = (value << 8) | bytes[offset + i];
    return value;
}
f32 f32at(const Sample& bytes, unsigned offset) {
    const auto bits = u32at(bytes, offset);
    if ((bits & 0x7f800000) == 0x7f800000) throw ReplayError("nonfinite KPAD sample");
    f32 value; std::memcpy(&value, &bits, 4); return value;
}
Vec vector(const Sample& bytes, unsigned offset) { return {f32at(bytes, offset), f32at(bytes, offset+4), f32at(bytes, offset+8)}; }
Vec2 vector2(const Sample& bytes, unsigned offset) { return {f32at(bytes, offset), f32at(bytes, offset+4)}; }
}
KPADStatus decodeSample(const Sample& bytes) {
    KPADStatus out{};
    out.hold=u32at(bytes,0); out.trig=u32at(bytes,4); out.release=u32at(bytes,8);
    out.acc=vector(bytes,12); out.acc_value=f32at(bytes,24); out.acc_speed=f32at(bytes,28);
    out.pos=vector2(bytes,32); out.vec=vector2(bytes,40); out.speed=f32at(bytes,48);
    out.horizon=vector2(bytes,52); out.hori_vec=vector2(bytes,60); out.hori_speed=f32at(bytes,68);
    out.dist=f32at(bytes,72); out.dist_vec=f32at(bytes,76); out.dist_speed=f32at(bytes,80);
    out.acc_vertical=vector2(bytes,84); out.dev_type=bytes[92];
    std::memcpy(&out.wpad_err,&bytes[93],1); std::memcpy(&out.dpd_valid_fg,&bytes[94],1); out.data_format=bytes[95];
    if (out.dev_type == 1 || out.dev_type == 6) {
        auto& ex=out.ex_status.fs; ex.stick=vector2(bytes,96); ex.acc=vector(bytes,104);
        ex.acc_value=f32at(bytes,116); ex.acc_speed=f32at(bytes,120);
    } else if (out.dev_type == 2 || out.dev_type == 7) {
        auto& ex=out.ex_status.cl; ex.hold=u32at(bytes,96); ex.trig=u32at(bytes,100); ex.release=u32at(bytes,104);
        ex.lstick=vector2(bytes,108); ex.rstick=vector2(bytes,116); ex.ltrigger=f32at(bytes,124); ex.rtrigger=f32at(bytes,128);
    } else if (out.dev_type != 0 && out.dev_type != 5 && out.dev_type != 252 && out.dev_type != 253 && out.dev_type != 255) {
        throw ReplayError("unsupported KPAD extension union");
    }
    out.mpls.angularVelocity=vector(bytes,176); out.mpls.angle=vector(bytes,188);
    out.mpls.direction.X=vector(bytes,200); out.mpls.direction.Y=vector(bytes,212); out.mpls.direction.Z=vector(bytes,224);
    out.unk_EC=u32at(bytes,236);
    return out;
}
}
