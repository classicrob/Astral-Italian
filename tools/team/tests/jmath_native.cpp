#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include "JSystem/JMath/JMath.hpp"

static bool same(float a, float b) {
    return std::memcmp(&a, &b, sizeof a) == 0;
}
static void vec(const Vec& a, const Vec& b) {
    assert(same(a.x,b.x) && same(a.y,b.y) && same(a.z,b.z));
}
static float randomFloat() {
    static uint32_t state=0x12345678;
    state=state*1664525U+1013904223U;
    return (float(int32_t(state) / 65536) / 2048.0f);
}
int main() {
    typedef void (*Copy)(void*,const void*);
    const Copy copies[]={JMath::gekko_ps_copy3,JMath::gekko_ps_copy6,JMath::gekko_ps_copy12,JMath::gekko_ps_copy16};
    const int counts[]={3,6,12,16};
    for (int c=0;c<4;++c) for(int shift=-3;shift<=3;++shift) {
        unsigned char a[128],b[128];
        for (unsigned int i=0;i<sizeof a;++i) a[i]=b[i]=static_cast<unsigned char>(i*37);
        std::memmove(b+32+shift,b+32,counts[c]*sizeof(float));
        copies[c](a+32+shift,a+32);
        assert(std::memcmp(a,b,sizeof a)==0);
    }
    assert(JMAFastSqrt(9.0f)==3.0f);
    assert(JMAFastSqrt(-9.0f)==-9.0f);
    assert(same(JMAFastSqrt(-0.0f),-0.0f));
    assert(std::isnan(JMAFastSqrt(std::numeric_limits<float>::infinity())));
    assert(std::isnan(JMAFastSqrt(std::numeric_limits<float>::quiet_NaN())));
    const float tiny=std::numeric_limits<float>::denorm_min();
    assert(JMAFastSqrt(tiny)>0.0f);

    typedef void (*Binary)(const Vec*,const Vec*,Vec*);
    const Binary binary[]={JMathInlineVEC::PSVECAdd,JMathInlineVEC::PSVECSubtract,JMathInlineVEC::PSVECMultiply};
    for (int n=0;n<10000;++n) {
        Vec a={randomFloat(),randomFloat(),randomFloat()},b={randomFloat(),randomFloat(),randomFloat()};
        for (int op=0;op<3;++op) {
            Vec expected;
            if(op==0) expected={a.x+b.x,a.y+b.y,a.z+b.z};
            if(op==1) expected={a.x-b.x,a.y-b.y,a.z-b.z};
            if(op==2) expected={a.x*b.x,a.y*b.y,a.z*b.z};
            Vec result,ac=a,bc=b;
            binary[op](&a,&b,&result);vec(result,expected);
            binary[op](&ac,&b,&ac);vec(ac,expected);
            binary[op](&a,&bc,&bc);vec(bc,expected);
        }
        Vec copied=a;JMathInlineVEC::PSVECCopy(&copied,&copied);vec(copied,a);
        Vec negated=a;JMathInlineVEC::PSVECNegate(&negated,&negated);vec(negated,Vec{-a.x,-a.y,-a.z});
        float dot=std::fma(a.x,b.x,a.y*b.y)+a.z*b.z;
        assert(same(JMathInlineVEC::PSVECDotProduct(&a,&b),dot));
        assert(same(JMathInlineVEC::PSVECSquareMag(&a),std::fma(a.z,a.z,a.x*a.x)+a.y*a.y));
        float dx=a.x-b.x,dy=a.y-b.y,dz=a.z-b.z;
        assert(same(JMathInlineVEC::PSVECSquareDistance(&a,&b),std::fma(dx,dx,dy*dy)+dz*dz));
        float t=randomFloat();
        Vec expected={std::fma(a.x,t,b.x),std::fma(a.y,t,b.y),std::fma(a.z,t,b.z)},ac=a,bc=b;
        JMAVECScaleAdd(&ac,&b,&ac,t);vec(ac,expected);
        JMAVECScaleAdd(&a,&bc,&bc,t);vec(bc,expected);
        expected={std::fma(b.x-a.x,t,a.x),std::fma(b.y-a.y,t,a.y),std::fma(b.z-a.z,t,a.z)};
        ac=a;bc=b;JMAVECLerp(&ac,&b,&ac,t);vec(ac,expected);
        JMAVECLerp(&a,&bc,&bc,t);vec(bc,expected);

        // Independent Hermite basis evaluated in double precision.
        float t0=-2.0f,t1=3.0f,v0=a.x,v1=b.x,m0=a.y,m1=b.y;
        float pos=-2.0f+5.0f*(float(n%101)/100.0f);
        double u=(double(pos)-t0)/(t1-t0),u2=u*u,u3=u2*u;
        double value=(2*u3-3*u2+1)*v0+(u3-2*u2+u)*(t1-t0)*m0+(-2*u3+3*u2)*v1+(u3-u2)*(t1-t0)*m1;
        float result=JMAHermiteInterpolation(pos,t0,v0,m0,t1,v1,m1);
        assert(std::fabs(result-value)<0.0001*(1+std::fabs(v0)+std::fabs(v1)+std::fabs(m0)+std::fabs(m1)));
        assert(JMAHermiteInterpolation(t0,t0,v0,m0,t1,v1,m1)==v0);
        assert(JMAHermiteInterpolation(t1,t0,v0,m0,t1,v1,m1)==v1);
    }
    assert(std::isnan(JMAHermiteInterpolation(1,1,0,0,1,1,0)));
    const Quaternion p={0,0,0,1},sameQ={0,0,0,1},opposite={0,0,0,-1};
    Quaternion out;
    JMAQuatLerp(&p,&sameQ,0.5f,&out);assert(out.w==1.0f);
    JMAQuatLerp(&p,&opposite,0.5f,&out);assert(out.w==1.0f);
    Quaternion q={1,0,0,0};JMAQuatLerp(&p,&q,0.5f,&q);assert(q.x==0.5f&&q.w==0.5f);
    // A nontrivial negative dot must interpolate toward the negated endpoint.
    const Quaternion from={0.25f,-0.5f,0.0f,0.75f},to={-0.5f,0.25f,0.5f,-0.75f};
    Quaternion expected={0.25f-0.3f*(0.25f-0.5f),-0.5f-0.3f*(-0.5f+0.25f),
                         -0.3f*0.5f,0.75f-0.3f*(0.75f-0.75f)};
    Quaternion a=from,b=to;
    JMAQuatLerp(&a,&to,0.3f,&a);assert(std::memcmp(&a,&expected,sizeof a)==0);
    JMAQuatLerp(&from,&b,0.3f,&b);assert(std::memcmp(&b,&expected,sizeof b)==0);
    const float e=0.0005f;
    const float startSlope=(JMAHermiteInterpolation(e,0,2,3,2,-1,-4)-JMAHermiteInterpolation(-e,0,2,3,2,-1,-4))/(2*e);
    const float endSlope=(JMAHermiteInterpolation(2+e,0,2,3,2,-1,-4)-JMAHermiteInterpolation(2-e,0,2,3,2,-1,-4))/(2*e);
    assert(std::fabs(startSlope-3.0f)<0.002f && std::fabs(endSlope+4.0f)<0.002f);
    // Shortest-path lerp deliberately does not normalize its result.
    assert(q.x*q.x+q.w*q.w==0.5f);
    std::puts("JMath native: 10000 randomized vector/Hermite cases, copy overlap, aliases, and edge cases passed");
}
