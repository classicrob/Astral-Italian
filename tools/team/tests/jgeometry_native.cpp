#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <limits>
#include "JSystem/JGeometry/TVec.hpp"
// Canonical zero-vector data fixture; no platform SDK function stubs.
const Vec gZeroVec = {0,0,0};
void secondTranslationUnit();
using namespace JGeometry;
static void check(const float* a,float x,float y,float z) { assert(a[0]==x && a[1]==y && a[2]==z); }
static float randomFloat() {
    static uint32_t state = 0x12345678;
    state = state * 1664525U + 1013904223U;
    return float(int32_t(state) / 65536) / 2048.0f;
}
static bool same(float a, float b) { return std::memcmp(&a, &b, sizeof a)==0; }
int main() {
    secondTranslationUnit();
    static_assert(sizeof(TVec3<float>)==12,"layout");
    static_assert(sizeof(TVec3<s16>)==6 && alignof(TVec3<s16>)==2,"short layout");
    float a[3]={2,3,5},b[3]={7,11,13},c[3];
    negateInternal(a,c);check(c,-2,-3,-5);
    negateInternal(a,a);check(a,-2,-3,-5);negateInternal(a,a);
    for(int alias=0;alias<3;++alias) {
        float aa[3]={2,3,5},bb[3]={7,11,13};float* out=alias==0?c:alias==1?aa:bb;
        subInternal(aa,bb,out);check(out,-5,-8,8); // Legacy helper reverses only Z.
        float dd[3]={2,3,5},ee[3]={7,11,13};out=alias==0?c:alias==1?dd:ee;
        mulInternal(dd,ee,out);check(out,14,33,65);
    }
    subInternal(a,a,a);check(a,0,0,0);
    float d[3]={2,3,5};mulInternal(d,d,d);check(d,4,9,25);
    Vec raw={2,3,5};TVec3<float> v(raw),copy(v),w(7.f,11.f,13.f),out;
    check(&copy.x,2,3,5);out=w;check(&out.x,7,11,13);out=out;check(&out.x,7,11,13);
    assert(v.dot(w)==112.f && v.dot(v)==38.f);
    assert(v.squared(w)==153.f && v.squared(v)==0.f);
    out.setPSZeroVec();check(&out.x,0,0,0);
    out.sub(v,w);check(&out.x,-5,-8,-8); // Normal subtraction retains normal Z.
    out=v;out.sub(out,w);check(&out.x,-5,-8,-8);
    out=v;out.negate();check(&out.x,-2,-3,-5);
    out.negate(v);check(&out.x,-2,-3,-5);
    Mtx matrix={{1,2,3,4},{5,6,7,8},{9,10,11,12}};
    out.setTrans(matrix);check(&out.x,4,8,12);
    for(int i=0;i<10000;++i) {
        TVec3<float> lhs(randomFloat(),randomFloat(),randomFloat());
        TVec3<float> rhs(randomFloat(),randomFloat(),randomFloat());
        float dot=std::fma(lhs.x,rhs.x,lhs.y*rhs.y)+lhs.z*rhs.z;
        float dx=lhs.x-rhs.x,dy=lhs.y-rhs.y,dz=lhs.z-rhs.z;
        float distance=std::fma(dx,dx,dy*dy)+dz*dz;
        assert(same(lhs.dot(rhs),dot) && same(lhs.squared(rhs),distance));
    }
    float negzero[3]={0.0f,-0.0f,0.0f};negateInternal(negzero,negzero);
    assert(std::signbit(negzero[0]) && !std::signbit(negzero[1]) && std::signbit(negzero[2]));
    TVec3<float> nan(std::numeric_limits<float>::quiet_NaN(),1.f,2.f);
    assert(std::isnan(nan.dot(v)) && std::isnan(nan.squared(v)));
    TVec2<float> p(2.f,3.f);
    const TVec2<float> q(7.f,11.f);
    TVec2<float> r=p.addInline(q);assert(r.x==9 && r.y==14);
    r=p.addOperatorInline(q);assert(r.x==9 && r.y==14);
    const TVec2<float>& lifetime=p.subInline(q);assert(lifetime.x==-5 && lifetime.y==-8);
    r=p.subOperatorInline(q);assert(r.x==-5 && r.y==-8);
    r=p.scaleInline(4);assert(r.x==8 && r.y==12 && p.x==2 && p.y==3);
    assert(p.squared(q)==89 && p.squared(p)==0);
    struct ShortStorage { s16 pad; TVec3<s16> value; } storage;
    static_assert(offsetof(ShortStorage, value)==2, "exercise two-byte alignment");
    TVec3<s8> small(1, -2, 127);assert(small.x==1 && small.y==-2 && small.z==127);
    TVec3<s16> s(-32768,32767,-3);storage.value=s;
    assert(storage.value.x==-32768 && storage.value.y==32767 && storage.value.z==-3);
    storage.value=storage.value;assert(storage.value.z==-3);
    std::puts("JGeometry native: all checks passed (10000 numerical cases)");
}
