#pragma once

#include "JSystem/JMath/JMATrigonometric.hpp"
#include <cmath>
#ifndef __MWERKS__
#include <cfloat>
#endif
#include <revolution/types.h>

namespace JGeometry {
    template < typename T >
    class TUtil {
    public:
        static f32 invert(f32 value) {
            return 1.0f / value;
        }

#ifdef JGEOMETRY_EXPLICIT_INSTANTIATIONS
        static int epsilonEquals(T a1, T a2, T a3);
#else
        static int epsilonEquals(T a1, T a2, T a3) {
            int ret = 0;

            a1 -= a2;

            if (-a3 <= a1 && a1 <= a3) {
                ret = 1;
            }

            return ret;
        }
#endif

#ifdef JGEOMETRY_EXPLICIT_INSTANTIATIONS
        static T sqrt(T val) NO_INLINE;
#else
        static T sqrt(T val) NO_INLINE {
            if (val <= 0.0f) {
                return val;
            }

            float outVal = __frsqrte(val);
            return (val * (0.5f * outVal * (3.0f - (val * (outVal * outVal)))));
        }
#endif

        static f32 sqrtInline(f32 val) {
            if (val <= 0.0f) {
                return val;
            }

            float outVal = __frsqrte(val);
            return (val * (0.5f * outVal * (3.0f - (val * (outVal * outVal)))));
        }

        static f32 asin(f32 val) NO_INLINE {
            if (val >= 1.0f) {
                return 1.5707964f;
            }

            if (val <= -1.0f) {
                return -1.5707964f;
            }

            if (val < 0.0f) {
                return -JMath::sAsinAcosTable.mTable[(u32)(1023.5f * -val)];
            }

            return JMath::sAsinAcosTable.mTable[(u32)(1023.5f * val)];
        }

        static f32 acos(f32 val) NO_INLINE {
#ifdef JGEOMETRY_ACOS_INLINE_LOOKUP
            if (val >= 1.0f) {
                return 0.0f;
            }
            if (val <= -1.0f) {
                return 3.1415927f;
            }
            if (val < 0.0f) {
                return 1.5707964f + JMath::sAsinAcosTable.mTable[(u32)(1023.5f * -val)];
            }
            return 1.5707964f - JMath::sAsinAcosTable.mTable[(u32)(1023.5f * val)];
#else
            return JMath::sAsinAcosTable.acos_(val);
#endif
        }

        static f32 PI() {
            return 3.1415927f;
        }

        static T clamp(T val, T min, T max) NO_INLINE {
            if (val < min) {
                return min;
            }

            if (val > max) {
                return max;
            }

            return val;
        }

        static f32 inv_sqrt(f32 val) NO_INLINE {
            if (val <= 0.0f) {
                return val;
            }
            f32 outVal = (f32)__frsqrte(val);
            return outVal * 0.5f * (3.0f - outVal * outVal * val);
        }

        static inline f32 epsilon() {
            return 32.0f * FLT_EPSILON;
        }
    };
};  // namespace JGeometry
