#include <revolution/mtx.h>
#include <petari/ppc_math.hpp>
#include <cmath>
#include <cstring>

namespace PPC = PetariNative::PPC;

namespace {
double refineReciprocalLength(double squareLength) {
    const double estimate = PPC::frsqrte(squareLength);
    const double squareEstimate = PPC::multiplySingle(estimate, estimate);
    const double halfEstimate = PPC::multiplySingle(estimate, 0.5);
    const double correction = PPC::negativeMultiplySubtractSingle(squareEstimate, squareLength, 3.0);
    return PPC::multiplySingle(correction, halfEstimate);
}

double vectorSquareLength(const Vec* v) {
    const PPC::Paired xy = PPC::loadPair(&v->x);
    const PPC::Paired squares = PPC::multiply(xy, xy);
    const PPC::Paired z = PPC::splat(PPC::loadSingle(v->z));
    const PPC::Paired xz = PPC::multiplyAdd(z, z, squares);
    return PPC::sum0(xz, squares, squares).first;
}
}

extern const Vec gZeroVec = {0.0f, 0.0f, 0.0f};

void PSVECAdd(const Vec* a, const Vec* b, Vec* out) {
    PPC::storePair(&out->x, PPC::add(PPC::loadPair(&a->x), PPC::loadPair(&b->x)));
    PPC::storePair(&out->z, PPC::add(PPC::loadPair(&a->z, true), PPC::loadPair(&b->z, true)), true);
}

void PSVECSubtract(const Vec* a, const Vec* b, Vec* out) {
    PPC::storePair(&out->x, PPC::subtract(PPC::loadPair(&a->x), PPC::loadPair(&b->x)));
    PPC::storePair(&out->z, PPC::subtract(PPC::loadPair(&a->z, true), PPC::loadPair(&b->z, true)), true);
}

void PSVECScale(const Vec* v, Vec* out, f32 scale) {
    const PPC::Paired xy = PPC::loadPair(&v->x);
    const PPC::Paired z = PPC::loadPair(&v->z, true);
    const PPC::Paired scalar = PPC::splat(PPC::loadSingle(scale));
    PPC::storePair(&out->x, PPC::multiplyScalar0(xy, scalar));
    PPC::storePair(&out->z, PPC::multiplyScalar0(z, scalar), true);
}

f32 PSVECDotProduct(const Vec* a, const Vec* b) {
    const PPC::Paired yz = PPC::multiply(PPC::loadPair(&a->y), PPC::loadPair(&b->y));
    const PPC::Paired xy = PPC::multiplyAdd(PPC::loadPair(&a->x), PPC::loadPair(&b->x), yz);
    return PPC::forceSingle(PPC::sum0(xy, yz, yz).first);
}

f32 PSVECMag(const Vec* v) {
    const double squareLength = vectorSquareLength(v);
    if (squareLength == 0.0) return PPC::forceSingle(squareLength);
    return PPC::forceSingle(PPC::multiplySingle(squareLength, refineReciprocalLength(squareLength)));
}

void PSVECNormalize(const Vec* v, Vec* out) {
    const PPC::Paired xy = PPC::loadPair(&v->x);
    const PPC::Paired z = PPC::loadPair(&v->z, true);
    const PPC::Paired squares = PPC::multiply(xy, xy);
    const PPC::Paired xz = PPC::multiplyAdd(z, z, squares);
    const double squareLength = PPC::sum0(xz, z, squares).first;
    const PPC::Paired inverse = PPC::splat(refineReciprocalLength(squareLength));
    PPC::storePair(&out->x, PPC::multiplyScalar0(xy, inverse));
    PPC::storePair(&out->z, PPC::multiplyScalar0(z, inverse), true);
}

void PSVECCrossProduct(const Vec* a, const Vec* b, Vec* out) {
    const PPC::Paired bxy = PPC::loadPair(&b->x);
    const PPC::Paired az = PPC::splat(PPC::loadSingle(a->z));
    const PPC::Paired axy = PPC::loadPair(&a->x);
    const PPC::Paired byx = PPC::merge10(bxy, bxy);
    const PPC::Paired bz = PPC::splat(PPC::loadSingle(b->z));
    const PPC::Paired azb = PPC::multiply(bxy, az);
    const PPC::Paired axb = PPC::multiplyScalar0(bxy, axy);
    const PPC::Paired xz = PPC::multiplySubtract(axy, bz, azb);
    const PPC::Paired xy = PPC::multiplySubtract(axy, byx, axb);
    PPC::storePair(&out->x, PPC::merge11(xz, xz), true);
    PPC::storePair(&out->y, PPC::negate(PPC::merge01(xz, xy)));
}

f32 PSVECDistance(const Vec* a, const Vec* b) {
    const PPC::Paired yz = PPC::subtract(PPC::loadPair(&a->y), PPC::loadPair(&b->y));
    const PPC::Paired xy = PPC::subtract(PPC::loadPair(&a->x), PPC::loadPair(&b->x));
    const PPC::Paired squares = PPC::multiply(yz, yz);
    const PPC::Paired sum = PPC::multiplyAdd(xy, xy, squares);
    const double squareLength = PPC::sum0(sum, squares, squares).first;
    if (squareLength == 0.0) return PPC::forceSingle(squareLength);
    return PPC::forceSingle(PPC::multiplySingle(squareLength, refineReciprocalLength(squareLength)));
}

void PSMTXIdentity(Mtx out) {
    std::memset(out, 0, sizeof(Mtx));
    for (int i = 0; i < 3; ++i) out[i][i] = 1.0f;
}

void C_MTXIdentity(Mtx out) { PSMTXIdentity(out); }
void PSMTXCopy(const Mtx src, Mtx out) { std::memmove(out, src, sizeof(Mtx)); }
void C_MTXCopy(const Mtx src, Mtx out) { PSMTXCopy(src, out); }

void PSMTX44Identity(Mtx44 out) {
    std::memset(out, 0, sizeof(Mtx44));
    for (int i = 0; i < 4; ++i) out[i][i] = 1.0f;
}

void PSMTX44Copy(const Mtx44 src, Mtx44 out) { std::memmove(out, src, sizeof(Mtx44)); }

void PSMTXConcat(const Mtx a, const Mtx b, Mtx out) {
    Mtx result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 4; ++col) {
            result[row][col] = a[row][0] * b[0][col] + a[row][1] * b[1][col] + a[row][2] * b[2][col];
            if (col == 3) result[row][col] += a[row][3];
        }
    }
    PSMTXCopy(result, out);
}

void PSMTXMultVecSR(const Mtx m, const Vec* v, Vec* out) {
    *out = {m[0][0] * v->x + m[0][1] * v->y + m[0][2] * v->z,
            m[1][0] * v->x + m[1][1] * v->y + m[1][2] * v->z,
            m[2][0] * v->x + m[2][1] * v->y + m[2][2] * v->z};
}

void PSMTXMultVec(const Mtx m, const Vec* v, Vec* out) {
    Vec result;
    PSMTXMultVecSR(m, v, &result);
    *out = {result.x + m[0][3], result.y + m[1][3], result.z + m[2][3]};
}

void PSMTXMultVecArraySR(const Mtx m, const Vec* src, Vec* out, u32 count) {
    for (u32 i = 0; i < count; ++i) PSMTXMultVecSR(m, src + i, out + i);
}

u32 PSMTXInverse(const Mtx m, Mtx out) {
    // Register pairs preserve the bound Gekko cofactor and reciprocal sequence.
    PPC::Paired f[14]{};
    f[0] = PPC::loadPair(&m[0][0], true);
    f[1] = PPC::loadPair(&m[0][1]);
    f[2] = PPC::loadPair(&m[1][0], true);
    f[6] = PPC::merge10(f[1], f[0]);
    f[3] = PPC::loadPair(&m[1][1]);
    f[4] = PPC::loadPair(&m[2][0], true);
    f[7] = PPC::merge10(f[3], f[2]);
    f[5] = PPC::loadPair(&m[2][1]);
    f[11] = PPC::multiply(f[3], f[6]);
    f[8] = PPC::merge10(f[5], f[4]);
    f[13] = PPC::multiply(f[5], f[7]);
    f[11] = PPC::multiplySubtract(f[1], f[7], f[11]);
    f[12] = PPC::multiply(f[1], f[8]);
    f[13] = PPC::multiplySubtract(f[3], f[8], f[13]);
    f[10] = PPC::multiply(f[3], f[4]);
    f[12] = PPC::multiplySubtract(f[5], f[6], f[12]);
    f[7] = PPC::multiply(f[0], f[13]);
    f[9] = PPC::multiply(f[0], f[5]);
    f[8] = PPC::multiply(f[1], f[2]);
    f[7] = PPC::multiplyAdd(f[2], f[12], f[7]);
    f[6] = PPC::subtract(f[6], f[6]);
    f[10] = PPC::multiplySubtract(f[2], f[5], f[10]);
    f[7] = PPC::multiplyAdd(f[4], f[11], f[7]);
    f[9] = PPC::multiplySubtract(f[1], f[4], f[9]);
    f[8] = PPC::multiplySubtract(f[0], f[3], f[8]);
    if (f[7].first == f[6].first) return 0;
    f[0] = PPC::splat(PPC::fres(f[7].first));
    f[6] = PPC::add(f[0], f[0]);
    f[5] = PPC::multiply(f[7], f[0]);
    f[0] = PPC::negativeMultiplySubtract(f[0], f[5], f[6]);
    f[1] = PPC::splat(PPC::loadSingle(*&m[0][3]));
    f[13] = PPC::multiplyScalar0(f[13], f[0]);
    f[2] = PPC::splat(PPC::loadSingle(*&m[1][3]));
    f[12] = PPC::multiplyScalar0(f[12], f[0]);
    f[3] = PPC::splat(PPC::loadSingle(*&m[2][3]));
    f[11] = PPC::multiplyScalar0(f[11], f[0]);
    f[5] = PPC::merge00(f[13], f[12]);
    f[4] = PPC::merge11(f[13], f[12]);
    f[6] = PPC::multiply(f[13], f[1]);
    PPC::storePair(&out[0][0], f[5]);
    PPC::storePair(&out[1][0], f[4]);
    f[10] = PPC::multiplyScalar0(f[10], f[0]);
    f[9] = PPC::multiplyScalar0(f[9], f[0]);
    f[6] = PPC::multiplyAdd(f[12], f[2], f[6]);
    PPC::storePair(&out[2][0], f[10], true);
    f[8] = PPC::multiplyScalar0(f[8], f[0]);
    f[6] = PPC::negativeMultiplyAdd(f[11], f[3], f[6]);
    PPC::storePair(&out[2][1], f[9], true);
    f[7] = PPC::multiply(f[10], f[1]);
    f[5] = PPC::merge00(f[11], f[6]);
    PPC::storePair(&out[2][2], f[8], true);
    f[7] = PPC::multiplyAdd(f[9], f[2], f[7]);
    f[4] = PPC::merge11(f[11], f[6]);
    PPC::storePair(&out[0][2], f[5]);
    f[7] = PPC::negativeMultiplyAdd(f[8], f[3], f[7]);
    PPC::storePair(&out[1][2], f[4]);
    PPC::storePair(&out[2][3], f[7], true);
    return 1;
}

extern "C" u32 PSMTXInvXpose(const Mtx m, Mtx out) {
    // Register pairs preserve the bound Gekko cofactor and reciprocal sequence.
    PPC::Paired f[14]{};
    f[0] = PPC::loadPair(&m[0][0], true);
    f[1] = PPC::loadPair(&m[0][1]);
    f[2] = PPC::loadPair(&m[1][0], true);
    f[6] = PPC::merge10(f[1], f[0]);
    f[3] = PPC::loadPair(&m[1][1]);
    f[4] = PPC::loadPair(&m[2][0], true);
    f[7] = PPC::merge10(f[3], f[2]);
    f[5] = PPC::loadPair(&m[2][1]);
    f[11] = PPC::multiply(f[3], f[6]);
    f[8] = PPC::merge10(f[5], f[4]);
    f[13] = PPC::multiply(f[5], f[7]);
    f[11] = PPC::multiplySubtract(f[1], f[7], f[11]);
    f[12] = PPC::multiply(f[1], f[8]);
    f[13] = PPC::multiplySubtract(f[3], f[8], f[13]);
    f[10] = PPC::multiply(f[3], f[4]);
    f[12] = PPC::multiplySubtract(f[5], f[6], f[12]);
    f[7] = PPC::multiply(f[0], f[13]);
    f[9] = PPC::multiply(f[0], f[5]);
    f[8] = PPC::multiply(f[1], f[2]);
    f[7] = PPC::multiplyAdd(f[2], f[12], f[7]);
    f[6] = PPC::subtract(f[6], f[6]);
    f[10] = PPC::multiplySubtract(f[2], f[5], f[10]);
    f[7] = PPC::multiplyAdd(f[4], f[11], f[7]);
    f[9] = PPC::multiplySubtract(f[1], f[4], f[9]);
    f[8] = PPC::multiplySubtract(f[0], f[3], f[8]);
    if (f[7].first == f[6].first) return 0;
    f[0] = PPC::splat(PPC::fres(f[7].first));
    PPC::storePair(&out[0][3], f[6], true);
    f[4] = PPC::add(f[0], f[0]);
    f[5] = PPC::multiply(f[7], f[0]);
    PPC::storePair(&out[1][3], f[6], true);
    f[0] = PPC::negativeMultiplySubtract(f[0], f[5], f[4]);
    PPC::storePair(&out[2][3], f[6], true);
    f[13] = PPC::multiplyScalar0(f[13], f[0]);
    f[12] = PPC::multiplyScalar0(f[12], f[0]);
    PPC::storePair(&out[0][0], f[13]);
    f[11] = PPC::multiplyScalar0(f[11], f[0]);
    PPC::storePair(&out[1][0], f[12]);
    f[10] = PPC::multiplyScalar0(f[10], f[0]);
    PPC::storePair(&out[2][0], f[11]);
    f[9] = PPC::multiplyScalar0(f[9], f[0]);
    PPC::storePair(&out[0][2], f[10], true);
    f[8] = PPC::multiplyScalar0(f[8], f[0]);
    PPC::storePair(&out[1][2], f[9], true);
    PPC::storePair(&out[2][2], f[8], true);
    return 1;
}

void PSMTXTrans(Mtx out, f32 x, f32 y, f32 z) {
    PSMTXIdentity(out);
    out[0][3] = x; out[1][3] = y; out[2][3] = z;
}

void PSMTXTransApply(const Mtx src, Mtx out, f32 x, f32 y, f32 z) {
    PSMTXCopy(src, out);
    out[0][3] += x; out[1][3] += y; out[2][3] += z;
}

void PSMTXScale(Mtx out, f32 x, f32 y, f32 z) {
    std::memset(out, 0, sizeof(Mtx));
    out[0][0] = x; out[1][1] = y; out[2][2] = z;
}

void PSMTXScaleApply(const Mtx src, Mtx out, f32 x, f32 y, f32 z) {
    const f32 scale[3] = {x, y, z};
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 4; ++col) out[row][col] = src[row][col] * scale[row];
}

void PSMTXRotTrig(Mtx out, char axis, f32 sine, f32 cosine) {
    axis |= 0x20;
    if (axis != 'x' && axis != 'y' && axis != 'z') return;
    PSMTXIdentity(out);
    const int first = axis == 'x' ? 1 : axis == 'y' ? 2 : 0;
    const int second = (first + 1) % 3;
    out[first][first] = out[second][second] = cosine;
    out[first][second] = -sine;
    out[second][first] = sine;
}

void PSMTXRotRad(Mtx out, char axis, f32 radians) {
    PSMTXRotTrig(out, axis, std::sin(radians), std::cos(radians));
}

namespace {
// Gekko rotation kernel; the caller supplies its trigonometric values.
void makeAxisRotation(Mtx out, const Vec* axis, f32 sine, f32 cosine) {
    PPC::Paired f[13]{};
    f[1] = PPC::splat(PPC::loadSingle(sine));
    f[2] = PPC::splat(PPC::loadSingle(cosine));
    f[3] = PPC::loadPair(&axis->x);
    f[11] = PPC::splat(PPC::loadSingle(PPC::forceSingle(f[2].first)));
    f[10] = PPC::splat(0.5);
    f[12] = PPC::splat(PPC::loadSingle(PPC::forceSingle(f[1].first)));
    f[4] = PPC::multiply(f[3], f[3]);
    f[2] = PPC::splat(PPC::loadSingle(*&axis->z));
    f[8] = PPC::splat(PPC::addSingle(f[10].first, f[10].first));
    f[9] = PPC::splat(3.0);
    f[1] = PPC::splat(PPC::subtractSingle(f[10].first, f[10].first));
    f[5] = PPC::multiplyAdd(f[2], f[2], f[4]);
    f[0] = PPC::splat(PPC::subtractSingle(f[8].first, f[11].first));
    f[11] = PPC::merge00(f[11], f[11]);
    f[6] = PPC::sum0(f[5], f[2], f[4]);
    f[7].first = PPC::frsqrte(f[6].first);
    f[4] = PPC::splat(PPC::multiplySingle(f[7].first, f[7].first));
    f[5] = PPC::splat(PPC::multiplySingle(f[7].first, f[10].first));
    f[4] = PPC::splat(PPC::negativeMultiplySubtractSingle(f[4].first, f[6].first, f[9].first));
    f[7] = PPC::splat(PPC::multiplySingle(f[4].first, f[5].first));
    f[3] = PPC::multiplyScalar0(f[3], f[7]);
    f[2] = PPC::multiplyScalar0(f[2], f[7]);
    f[6] = PPC::multiplyScalar0(f[3], f[0]);
    f[7] = PPC::multiplyScalar0(f[2], f[0]);
    f[10] = PPC::multiplyScalar0(f[3], f[12]);
    f[5] = PPC::multiplyScalar1(f[6], f[3]);
    f[4] = PPC::multiplyScalar0(f[6], f[3]);
    f[6] = PPC::multiplyScalar0(f[6], f[2]);
    f[0] = PPC::splat(PPC::negativeMultiplySubtractSingle(f[2].first, f[12].first, f[5].first));
    f[3] = PPC::negate(f[10]);
    f[8] = PPC::splat(PPC::multiplyAddSingle(f[2].first, f[12].first, f[5].first));
    f[4] = PPC::sum0(f[4], f[0], f[11]);
    f[0] = PPC::sum0(f[3], f[1], f[6]);
    f[7] = PPC::multiplyScalar0(f[7], f[2]);
    PPC::storePair(&out[0][0], f[4]);
    f[9] = PPC::sum0(f[6], f[1], f[10]);
    f[3] = PPC::sum0(f[6], f[6], f[3]);
    PPC::storePair(&out[1][2], f[0]);
    f[5] = PPC::sum1(f[11], f[8], f[5]);
    f[7] = PPC::sum0(f[7], f[1], f[11]);
    PPC::storePair(&out[0][2], f[9]);
    f[6] = PPC::sum1(f[10], f[3], f[6]);
    PPC::storePair(&out[1][0], f[5]);
    PPC::storePair(&out[2][0], f[6]);
    PPC::storePair(&out[2][2], f[7]);
}
}

void PSMTXRotAxisRad(Mtx out, const Vec* axis, f32 radians) {
    makeAxisRotation(out, axis, std::sin(radians), std::cos(radians));
}

void PSQUATMultiply(const Quaternion* a, const Quaternion* b, Quaternion* out) {
    *out = {a->w*b->x + a->x*b->w + a->y*b->z - a->z*b->y,
            a->w*b->y - a->x*b->z + a->y*b->w + a->z*b->x,
            a->w*b->z + a->x*b->y - a->y*b->x + a->z*b->w,
            a->w*b->w - a->x*b->x - a->y*b->y - a->z*b->z};
}

f32 PSQUATDotProduct(const Quaternion* a, const Quaternion* b) {
    return a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w;
}

void PSMTXQuat(Mtx out, const Quaternion* q) {
    PPC::Paired f[14]{};
    f[4] = PPC::loadPair(&q->x);
    f[5] = PPC::loadPair(&q->z);
    f[6] = PPC::multiply(f[4], f[4]);
    f[1] = PPC::splat(1.0);
    f[9] = PPC::merge10(f[4], f[4]);
    f[0] = PPC::splat(PPC::subtractSingle(f[1].first, f[1].first));
    f[8] = PPC::multiplyAdd(f[5], f[5], f[6]);
    f[10] = PPC::multiplyScalar1(f[5], f[5]);
    PPC::storePair(&out[0][3], f[0], true);
    f[2] = PPC::splat(PPC::addSingle(f[1].first, f[1].first));
    f[3] = PPC::sum0(f[8], f[8], f[8]);
    PPC::storePair(&out[2][3], f[0], true);
    f[7] = PPC::multiply(f[5], f[5]);
    f[12] = PPC::multiplyAdd(f[4], f[9], f[10]);
    f[13] = PPC::splat(PPC::fres(f[3].first));
    f[3] = PPC::negativeMultiplySubtract(f[3], f[13], f[2]);
    f[11] = PPC::multiplyScalar1(f[9], f[5]);
    f[10] = PPC::multiplySubtract(f[4], f[9], f[10]);
    f[3] = PPC::multiply(f[13], f[3]);
    f[9] = PPC::multiplyAddScalar0(f[4], f[5], f[11]);
    f[8] = PPC::sum1(f[7], f[8], f[6]);
    f[3] = PPC::splat(PPC::multiplySingle(f[3].first, f[2].first));
    f[11] = PPC::negativeMultiplySubtract(f[11], f[2], f[9]);
    f[6] = PPC::sum0(f[6], f[6], f[6]);
    f[9] = PPC::multiply(f[9], f[3]);
    f[11] = PPC::multiply(f[11], f[3]);
    f[8] = PPC::negativeMultiplySubtract(f[8], f[3], f[1]);
    PPC::storePair(&out[0][2], f[9], true);
    f[12] = PPC::multiply(f[12], f[3]);
    f[10] = PPC::multiply(f[10], f[3]);
    f[7] = PPC::merge10(f[11], f[0]);
    f[5] = PPC::merge00(f[12], f[8]);
    f[4] = PPC::merge10(f[8], f[10]);
    PPC::storePair(&out[1][2], f[7]);
    f[13] = PPC::merge01(f[11], f[9]);
    f[6] = PPC::negativeMultiplySubtract(f[6], f[3], f[1]);
    PPC::storePair(&out[1][0], f[5]);
    PPC::storePair(&out[2][2], f[6], true);
    PPC::storePair(&out[0][0], f[4]);
    PPC::storePair(&out[2][0], f[13]);
}
