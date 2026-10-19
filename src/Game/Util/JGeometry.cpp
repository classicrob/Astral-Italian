#define JGEOMETRY_EXPLICIT_INSTANTIATIONS
#include "JSystem/JGeometry/TMatrix.hpp"

// Canonical geometry methods shared by game translation units.
namespace JGeometry {
    template <> void TMatrix34< SMatrix34C< f32 > >::identity() NO_INLINE;
    template <> void TMatrix34< SMatrix34C< f32 > >::mult(const TVec3f& rSrc, TVec3f& rDest) const NO_INLINE;
    template <> void TMatrix34< SMatrix34C< f32 > >::multTranspose(const TVec3f& rSrc, TVec3f& rDest) const NO_INLINE;
    template <> bool TMatrix34< SMatrix34C< f32 > >::invert(const TMatrix34< SMatrix34C< f32 > >& rSrc) NO_INLINE;
    template <> void TMatrix34< SMatrix34C< f32 > >::concat(const SMatrix34C< f32 >& rSrcA, const SMatrix34C< f32 >& rSrcB) NO_INLINE;
    template <> void SMatrix34C< f32 >::set(f32 xx, f32 xy, f32 xz, f32 tx, f32 yx, f32 yy, f32 yz, f32 ty, f32 zx, f32 zy, f32 zz, f32 tz) NO_INLINE;
    template <> void TMatrix34< SMatrix34C< f32 > >::concat(const SMatrix34C< f32 >& rSrc) NO_INLINE;
    template <> void TRotation3< TMtx34f >::getQuat(TQuat4f& rDest) const NO_INLINE;
    template <> f32 TUtil< f32 >::sqrt(f32 val) NO_INLINE;
    template <> template <> void TVec4< f32 >::set< f32 >(f32 _x, f32 _y, f32 _z, f32 _w) NO_INLINE;
    template <> void TQuat4< f32 >::setEuler(f32 _x, f32 _y, f32 _z) NO_INLINE;
    template <> void TQuat4< f32 >::setRotate(const TVec3f& rA, const TVec3f& rB) NO_INLINE;
    template <> void TQuat4< f32 >::slerp(const TQuat4f& rOther, f32 ratio) NO_INLINE;
    template <> void TQuat4< f32 >::normalize(const TQuat4f& rSrc) NO_INLINE;
    template <> void TQuat4< f32 >::getEuler(TVec3f& rDest) const NO_INLINE;
    template <> TQuat4f& TQuat4< f32 >::operator=(const TQuat4f& rSrc) NO_INLINE;

    template <> TPosition3< TMtx34f >::TPosition3() {}

    TVec3< f32 >::TVec3(f32 val) {
        set(val, val, val);
    }

    // NONMATCHING: f0/f1 allocation is swapped; all stores and constants agree.
    template <> void TMatrix34< SMatrix34C< f32 > >::identity() {
        f32 one = 1.0f;
        this->mMtx[1][0] = this->mMtx[2][0] = this->mMtx[0][1] = this->mMtx[2][1] = this->mMtx[0][2] = this->mMtx[1][2] = this->mMtx[0][3] =
            this->mMtx[1][3] = this->mMtx[2][3] = 0.0f;
        this->mMtx[0][0] = this->mMtx[1][1] = this->mMtx[2][2] = one;
    }

    template <> void TMatrix34< SMatrix34C< f32 > >::mult(const TVec3f& rSrc, TVec3f& rDest) const {
        rDest.set< f32 >((*this)(0, 3) + (rSrc.z * (*this)(0, 2) + (rSrc.x * (*this)(0, 0) + rSrc.y * (*this)(0, 1))),
                         (*this)(1, 3) + (rSrc.z * (*this)(1, 2) + (rSrc.x * (*this)(1, 0) + rSrc.y * (*this)(1, 1))),
                         (*this)(2, 3) + (rSrc.z * (*this)(2, 2) + (rSrc.x * (*this)(2, 0) + rSrc.y * (*this)(2, 1))));
    }

    template < typename T >
    void TVec3< f32 >::set(T _x, T _y, T _z) {
        x = _x;
        y = _y;
        z = _z;
    }

    template <> void TMatrix34< SMatrix34C< f32 > >::multTranspose(const TVec3f& rSrc, TVec3f& rDest) const {
        TVec3f translated;
        translated.x = rSrc.x - (*this)(0, 3);
        translated.y = rSrc.y - (*this)(1, 3);
        translated.z = rSrc.z - (*this)(2, 3);

        rDest.set< f32 >(translated.z * (*this)(2, 0) + (translated.x * (*this)(0, 0) + translated.y * (*this)(1, 0)),
                         translated.z * (*this)(2, 1) + (translated.x * (*this)(0, 1) + translated.y * (*this)(1, 1)),
                         translated.z * (*this)(2, 2) + (translated.x * (*this)(0, 2) + translated.y * (*this)(1, 2)));
    }

    template <> bool TMatrix34< SMatrix34C< f32 > >::invert(const TMatrix34< SMatrix34C< f32 > >& rSrc) {
        f32 determinant = rSrc.determinant33();

        if (determinant <= TUtil< f32 >::epsilon() && determinant >= -TUtil< f32 >::epsilon()) {
            return false;
        }

        SMatrix33R< f32 > inverse;
        f32 scale = TUtil< f32 >::invert(determinant);

        TVec3f translation;
        translation.x = rSrc(0, 3);
        translation.x = -translation.x;

        inverse.set(scale * (rSrc(1, 1) * rSrc(2, 2) - rSrc(1, 2) * rSrc(2, 1)), scale * -(rSrc(0, 1) * rSrc(2, 2) - rSrc(0, 2) * rSrc(2, 1)),
                    scale * (rSrc(0, 1) * rSrc(1, 2) - rSrc(0, 2) * rSrc(1, 1)), scale * -(rSrc(1, 0) * rSrc(2, 2) - rSrc(1, 2) * rSrc(2, 0)),
                    scale * (rSrc(0, 0) * rSrc(2, 2) - rSrc(0, 2) * rSrc(2, 0)), scale * -(rSrc(0, 0) * rSrc(1, 2) - rSrc(0, 2) * rSrc(1, 0)),
                    scale * (rSrc(1, 0) * rSrc(2, 1) - rSrc(1, 1) * rSrc(2, 0)), scale * -(rSrc(0, 0) * rSrc(2, 1) - rSrc(0, 1) * rSrc(2, 0)),
                    scale * (rSrc(0, 0) * rSrc(1, 1) - rSrc(0, 1) * rSrc(1, 0)));

        translation.y = rSrc(1, 3);
        translation.y = -translation.y;
        translation.z = rSrc(2, 3);
        translation.z = -translation.z;

        translation.set< f32 >(translation.x * inverse.mMtx[0][0] + translation.y * inverse.mMtx[1][0] + translation.z * inverse.mMtx[2][0],
                               translation.x * inverse.mMtx[0][1] + translation.y * inverse.mMtx[1][1] + translation.z * inverse.mMtx[2][1],
                               translation.x * inverse.mMtx[0][2] + translation.y * inverse.mMtx[1][2] + translation.z * inverse.mMtx[2][2]);
        this->mMtx[0][0] = inverse.get(0, 0);
        this->mMtx[1][0] = inverse.get(1, 0);
        this->mMtx[2][0] = inverse.get(2, 0);
        this->mMtx[0][1] = inverse.get(0, 1);
        this->mMtx[1][1] = inverse.get(1, 1);
        this->mMtx[2][1] = inverse.get(2, 1);
        this->mMtx[0][2] = inverse.get(0, 2);
        this->mMtx[1][2] = inverse.get(1, 2);
        this->mMtx[2][2] = inverse.get(2, 2);
        this->mMtx[0][3] = translation.x;
        this->mMtx[1][3] = translation.y;
        this->mMtx[2][3] = translation.z;
        return true;
    }

    template < typename T >
    int TUtil< T >::epsilonEquals(T a1, T a2, T a3) {
        int ret = 0;

        a1 -= a2;

        if (-a3 <= a1 && a1 <= a3) {
            ret = 1;
        }

        return ret;
    }

    template int TUtil< f32 >::epsilonEquals(f32, f32, f32);

    template <> void TMatrix34< SMatrix34C< f32 > >::concat(const SMatrix34C< f32 >& rSrcA, const SMatrix34C< f32 >& rSrcB) {
        this->set(rSrcA(0, 2) * rSrcB(2, 0) + (rSrcA(0, 0) * rSrcB(0, 0) + rSrcA(0, 1) * rSrcB(1, 0)),
                  rSrcA(0, 2) * rSrcB(2, 1) + (rSrcA(0, 0) * rSrcB(0, 1) + rSrcA(0, 1) * rSrcB(1, 1)),
                  rSrcA(0, 2) * rSrcB(2, 2) + (rSrcA(0, 0) * rSrcB(0, 2) + rSrcA(0, 1) * rSrcB(1, 2)),
                  rSrcA(0, 3) + (rSrcA(0, 2) * rSrcB(2, 3) + (rSrcA(0, 0) * rSrcB(0, 3) + rSrcA(0, 1) * rSrcB(1, 3))),
                  rSrcA(1, 2) * rSrcB(2, 0) + (rSrcA(1, 0) * rSrcB(0, 0) + rSrcA(1, 1) * rSrcB(1, 0)),
                  rSrcA(1, 2) * rSrcB(2, 1) + (rSrcA(1, 0) * rSrcB(0, 1) + rSrcA(1, 1) * rSrcB(1, 1)),
                  rSrcA(1, 2) * rSrcB(2, 2) + (rSrcA(1, 0) * rSrcB(0, 2) + rSrcA(1, 1) * rSrcB(1, 2)),
                  rSrcA(1, 3) + (rSrcA(1, 2) * rSrcB(2, 3) + (rSrcA(1, 0) * rSrcB(0, 3) + rSrcA(1, 1) * rSrcB(1, 3))),
                  rSrcA(2, 2) * rSrcB(2, 0) + (rSrcA(2, 0) * rSrcB(0, 0) + rSrcA(2, 1) * rSrcB(1, 0)),
                  rSrcA(2, 2) * rSrcB(2, 1) + (rSrcA(2, 0) * rSrcB(0, 1) + rSrcA(2, 1) * rSrcB(1, 1)),
                  rSrcA(2, 2) * rSrcB(2, 2) + (rSrcA(2, 0) * rSrcB(0, 2) + rSrcA(2, 1) * rSrcB(1, 2)),
                  rSrcA(2, 3) + (rSrcA(2, 2) * rSrcB(2, 3) + (rSrcA(2, 0) * rSrcB(0, 3) + rSrcA(2, 1) * rSrcB(1, 3))));
    }

    template <> void SMatrix34C< f32 >::set(f32 xx, f32 xy, f32 xz, f32 tx, f32 yx, f32 yy, f32 yz, f32 ty, f32 zx, f32 zy, f32 zz, f32 tz) {
        mMtx[0][0] = xx;
        mMtx[0][1] = xy;
        mMtx[0][2] = xz;
        mMtx[0][3] = tx;
        mMtx[1][0] = yx;
        mMtx[1][1] = yy;
        mMtx[1][2] = yz;
        mMtx[1][3] = ty;
        mMtx[2][0] = zx;
        mMtx[2][1] = zy;
        mMtx[2][2] = zz;
        mMtx[2][3] = tz;
    }

    template <> void TMatrix34< SMatrix34C< f32 > >::concat(const SMatrix34C< f32 >& rSrc) {
        this->set((*this)(0, 2) * rSrc(2, 0) + ((*this)(0, 0) * rSrc(0, 0) + (*this)(0, 1) * rSrc(1, 0)),
                  (*this)(0, 2) * rSrc(2, 1) + ((*this)(0, 0) * rSrc(0, 1) + (*this)(0, 1) * rSrc(1, 1)),
                  (*this)(0, 2) * rSrc(2, 2) + ((*this)(0, 0) * rSrc(0, 2) + (*this)(0, 1) * rSrc(1, 2)),
                  (*this)(0, 3) + ((*this)(0, 2) * rSrc(2, 3) + ((*this)(0, 0) * rSrc(0, 3) + (*this)(0, 1) * rSrc(1, 3))),
                  (*this)(1, 2) * rSrc(2, 0) + ((*this)(1, 0) * rSrc(0, 0) + (*this)(1, 1) * rSrc(1, 0)),
                  (*this)(1, 2) * rSrc(2, 1) + ((*this)(1, 0) * rSrc(0, 1) + (*this)(1, 1) * rSrc(1, 1)),
                  (*this)(1, 2) * rSrc(2, 2) + ((*this)(1, 0) * rSrc(0, 2) + (*this)(1, 1) * rSrc(1, 2)),
                  (*this)(1, 3) + ((*this)(1, 2) * rSrc(2, 3) + ((*this)(1, 0) * rSrc(0, 3) + (*this)(1, 1) * rSrc(1, 3))),
                  (*this)(2, 2) * rSrc(2, 0) + ((*this)(2, 0) * rSrc(0, 0) + (*this)(2, 1) * rSrc(1, 0)),
                  (*this)(2, 2) * rSrc(2, 1) + ((*this)(2, 0) * rSrc(0, 1) + (*this)(2, 1) * rSrc(1, 1)),
                  (*this)(2, 2) * rSrc(2, 2) + ((*this)(2, 0) * rSrc(0, 2) + (*this)(2, 1) * rSrc(1, 2)),
                  (*this)(2, 3) + ((*this)(2, 2) * rSrc(2, 3) + ((*this)(2, 0) * rSrc(0, 3) + (*this)(2, 1) * rSrc(1, 3))));
    }

    template <> void TRotation3< TMtx34f >::getQuat(TQuat4f& rDest) const {
        if ((*this)(0, 0) + (*this)(1, 1) + (*this)(2, 2) >= 0.0f) {
            f32 root = (*this)(0, 0) + (*this)(1, 1) + (*this)(2, 2);
            root += 1.0f;
            root = TUtil< f32 >::sqrt(root);
            f32 scale = 0.5f / root;
            rDest.w = 0.5f * root;
            rDest.x = scale * ((*this)(2, 1) - (*this)(1, 2));
            rDest.y = scale * ((*this)(0, 2) - (*this)(2, 0));
            rDest.z = scale * ((*this)(1, 0) - (*this)(0, 1));
        } else {
            f32 maximum = JGeometry::max(((*this)(0, 0) >= (*this)(1, 1) ? (*this)(0, 0) : (*this)(1, 1)), (*this)(2, 2));

            if (maximum == (*this)(0, 0)) {
                f32 root = TUtil< f32 >::sqrt(1.0f + ((*this)(0, 0) - ((*this)(1, 1) + (*this)(2, 2))));
                f32 scale = 0.5f / root;
                rDest.x = 0.5f * root;
                rDest.y = scale * ((*this)(0, 1) + (*this)(1, 0));
                rDest.z = scale * ((*this)(2, 0) + (*this)(0, 2));
                rDest.w = scale * ((*this)(2, 1) - (*this)(1, 2));
            } else if (maximum == (*this)(1, 1)) {
                f32 root = TUtil< f32 >::sqrt(1.0f + ((*this)(1, 1) - ((*this)(2, 2) + (*this)(0, 0))));
                f32 scale = 0.5f / root;
                rDest.y = 0.5f * root;
                rDest.z = scale * ((*this)(1, 2) + (*this)(2, 1));
                rDest.x = scale * ((*this)(0, 1) + (*this)(1, 0));
                rDest.w = scale * ((*this)(0, 2) - (*this)(2, 0));
            } else {
                f32 root = TUtil< f32 >::sqrt(1.0f + ((*this)(2, 2) - ((*this)(0, 0) + (*this)(1, 1))));
                f32 scale = 0.5f / root;
                rDest.z = 0.5f * root;
                rDest.x = scale * ((*this)(2, 0) + (*this)(0, 2));
                rDest.y = scale * ((*this)(1, 2) + (*this)(2, 1));
                rDest.w = scale * ((*this)(1, 0) - (*this)(0, 1));
            }
        }
    }

    template <> f32 TUtil< f32 >::sqrt(f32 val) {
        if (val <= 0.0f) {
            return val;
        }

        float outVal = __frsqrte(val);
        return (val * (0.5f * outVal * (3.0f - (val * (outVal * outVal)))));
    }

    template <> template <> void TVec4< f32 >::set< f32 >(f32 _x, f32 _y, f32 _z, f32 _w) {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    template <> void TQuat4< f32 >::setEuler(f32 _x, f32 _y, f32 _z) {
        f32 cx = cos(0.5f * _x);
        f32 cy = cos(0.5f * _y);
        f32 cz = cos(0.5f * _z);
        f32 sx = sin(0.5f * _x);
        f32 sy = sin(0.5f * _y);
        f32 sz = sin(0.5f * _z);
        f32 cycz = cy * cz;
        f32 sycz = sy * cz;
        f32 cysz = cy * sz;
        f32 sysz = sy * sz;
        this->x = cycz * sx - sysz * cx;
        this->y = sycz * cx + cysz * sx;
        this->z = cysz * cx - sycz * sx;
        this->w = cycz * cx + sysz * sx;
    }

    template <> void TQuat4< f32 >::setRotate(const TVec3f& rA, const TVec3f& rB) {
        TVec3< f32 > dir = rA.cross(rB);
        f32 crossPart = dir.length();

        if (crossPart <= TUtil< f32 >::epsilon()) {
            this->template set< f32 >(0.0f, 0.0f, 0.0f, 1.0f);
        } else {
            f32 dotPart = JMathInlineVEC::PSVECDotProduct(&rA, &rB);
            f32 halfAngle = 0.5f * JMAATan2(crossPart, dotPart);
            f32 scale = static_cast< f32 >(sin(halfAngle)) / crossPart;
            this->x = dir.x * scale;
            this->y = dir.y * scale;
            this->z = dir.z * scale;
            this->w = cos(halfAngle);
        }
    }

    template <> void TQuat4< f32 >::slerp(const TQuat4f& rOther, f32 ratio) {
        TQuat4< f32 > a;
        TQuat4< f32 > b;
        a.normalize(*this);
        b.normalize(rOther);
        f32 product = a.dot(b);
        bool reverse;

        if (product < 0.0f) {
            product = -product;
            reverse = true;
        } else {
            reverse = false;
        }

        f32 weight;

        if (1.0f - product <= TUtil< f32 >::epsilon()) {
            weight = 1.0f - ratio;
        } else {
            f32 angle = JMath::sAsinAcosTable.acos_(product);
            f32 sinAngle = sin(angle);
            weight = (f32)sin((1.0f - ratio) * angle) / sinAngle;
            ratio = (f32)sin(ratio * angle) / sinAngle;
        }

        if (reverse) {
            ratio = -ratio;
        }

        this->template set< f32 >(weight * a.x + ratio * b.x, weight * a.y + ratio * b.y, weight * a.z + ratio * b.z, weight * a.w + ratio * b.w);
    }

    template <> void TQuat4< f32 >::normalize(const TQuat4f& rSrc) {
        f32 length = rSrc.squared();

        if (length <= TUtil< f32 >::epsilon()) {
            this->template set< f32 >(0.0f, 0.0f, 0.0f, 1.0f);
        } else {
            f32 inverse;

            if (length <= 0.0f) {
                inverse = length;
            } else {
                f32 estimate = __frsqrte(length);
                inverse = (0.5f * estimate) * (3.0f - length * (estimate * estimate));
            }

            this->x = rSrc.x * inverse;
            this->y = rSrc.y * inverse;
            this->z = rSrc.z * inverse;
            this->w = rSrc.w * inverse;
        }
    }

    template <> void TQuat4< f32 >::getEuler(TVec3f& rDest) const {
        f32 m20 = 2.0f * (this->x * this->z - this->w * this->y);

        if (m20 - 1.0f >= -TUtil< f32 >::epsilon()) {
            f32 numerator = 2.0f * (this->x * this->y - this->w * this->z);
            rDest.x = JMAATan2(-numerator, 1.0f - 2.0f * (this->x * this->x + this->z * this->z));
            rDest.y = -JMath::TAngleConstant_< f32 >::RADIAN_DEG090();
            rDest.z = 0.0f;
        } else if (1.0f + m20 <= TUtil< f32 >::epsilon()) {
            f32 numerator = 2.0f * (this->x * this->y - this->w * this->z);
            rDest.x = JMAATan2(numerator, 1.0f - 2.0f * (this->x * this->x + this->z * this->z));
            rDest.y = JMath::TAngleConstant_< f32 >::RADIAN_DEG090();
            rDest.z = 0.0f;
        } else {
            f32 m10 = 2.0f * (this->x * this->y + this->w * this->z);
            f32 numerator = 2.0f * (this->y * this->z + this->w * this->x);
            f32 denominator = 1.0f - 2.0f * (this->x * this->x + this->y * this->y);
            f32 m00 = 1.0f - 2.0f * (this->y * this->y + this->z * this->z);
            rDest.x = JMAATan2(numerator, denominator);
            rDest.y = JMath::sAsinAcosTable.asin_(-m20);
            rDest.z = JMAATan2(m10, m00);
        }
    }

    template <> TQuat4f& TQuat4< f32 >::operator=(const TQuat4f& rSrc) {
        this->x = rSrc.x;
        this->y = rSrc.y;
        this->z = rSrc.z;
        this->w = rSrc.w;
        return *this;
    }

}
