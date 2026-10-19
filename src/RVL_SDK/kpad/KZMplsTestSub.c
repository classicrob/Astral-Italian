#include "revolution/kpad/KMPLSInternal.h"
#include <cmath>

KPADMPLSDirection kmplsIdentity = {
    {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}
};

s32 __KMPLS_f2i(f32 value) {
    if (value < 0.0f) {
        return (s32)(value - 0.5f);
    }
    return (s32)(0.5f + value);
}

f32 __KMPLS_normalize_Fxyz(Vec* value) {
    f32 length = (f32)sqrt(value->z * value->z +
                           (value->x * value->x + value->y * value->y));
    if (length != 0.0f) {
        f32 inverse = 1.0f / length;
        value->x *= inverse;
        value->y *= inverse;
        value->z *= inverse;
    }
    return length;
}

// NONMATCHING: FP register allocation and cross-product spills differ. Ordered nine output expressions, six sqrt inputs, and complete iteration guard agree.
void __KMPLS_normalize_directionXYZ(KPADMPLSDirection* direction, f32 minimum) {
    f32 total;
    do {
        {
            Vec* value = &direction->X;
            f32 length = (f32)sqrt(value->z * value->z +
                                   (value->x * value->x + value->y * value->y));
            f32 inverse = 1.0f / length;
            value->x *= inverse;
            value->y *= inverse;
            value->z *= inverse;
        }
        {
            Vec* value = &direction->Y;
            f32 length = (f32)sqrt(value->z * value->z +
                                   (value->x * value->x + value->y * value->y));
            f32 inverse = 1.0f / length;
            value->x *= inverse;
            value->y *= inverse;
            value->z *= inverse;
        }
        {
            Vec* value = &direction->Z;
            f32 length = (f32)sqrt(value->z * value->z +
                                   (value->x * value->x + value->y * value->y));
            f32 inverse = 1.0f / length;
            value->x *= inverse;
            value->y *= inverse;
            value->z *= inverse;
        }
        {
            Vec crossX, crossY, crossZ;
            f32 lengthX, lengthY, lengthZ;
            crossX.x = direction->Y.y * direction->Z.z - direction->Y.z * direction->Z.y;
            crossX.y = direction->Y.z * direction->Z.x - direction->Y.x * direction->Z.z;
            crossX.z = direction->Y.x * direction->Z.y - direction->Y.y * direction->Z.x;
            crossY.x = direction->Z.y * direction->X.z - direction->Z.z * direction->X.y;
            crossY.y = direction->Z.z * direction->X.x - direction->Z.x * direction->X.z;
            crossY.z = direction->Z.x * direction->X.y - direction->Z.y * direction->X.x;
            crossZ.x = direction->X.y * direction->Y.z - direction->X.z * direction->Y.y;
            crossZ.y = direction->X.z * direction->Y.x - direction->X.x * direction->Y.z;
            crossZ.z = direction->X.x * direction->Y.y - direction->X.y * direction->Y.x;
            lengthX = (f32)sqrt(crossX.z * crossX.z + (crossX.x * crossX.x + crossX.y * crossX.y));
            {
                f32 inverse = 1.0f / lengthX;
                direction->X.x = 0.5f * (direction->X.x + inverse * crossX.x);
                direction->X.y = 0.5f * (direction->X.y + inverse * crossX.y);
                direction->X.z = 0.5f * (direction->X.z + inverse * crossX.z);
            }
            lengthY = (f32)sqrt(crossY.z * crossY.z + (crossY.x * crossY.x + crossY.y * crossY.y));
            {
                f32 inverse = 1.0f / lengthY;
                direction->Y.x = 0.5f * (direction->Y.x + inverse * crossY.x);
                direction->Y.y = 0.5f * (direction->Y.y + inverse * crossY.y);
                direction->Y.z = 0.5f * (direction->Y.z + inverse * crossY.z);
            }
            total = lengthX;
            total += lengthY;
            lengthZ = (f32)sqrt(crossZ.z * crossZ.z + (crossZ.x * crossZ.x + crossZ.y * crossZ.y));
            {
                f32 inverse = 1.0f / lengthZ;
                direction->Z.x = 0.5f * (direction->Z.x + inverse * crossZ.x);
                direction->Z.y = 0.5f * (direction->Z.y + inverse * crossZ.y);
                direction->Z.z = 0.5f * (direction->Z.z + inverse * crossZ.z);
            }
            total += lengthZ;
        }
    } while (total < minimum);
}

// NONMATCHING: FP register allocation and vector stack slots differ. Both zero/nonzero branches retain ordered nine output expressions and sqrt input.
void __KMPLS_make_vec_dir(KPADMPLSDirection* direction, Vec* from, Vec* to) {
    Vec cross;
    f32 xy = from->x * to->y;
    f32 yx = from->y * to->x;
    f32 xz = from->x * to->z;
    f32 zx = from->z * to->x;
    f32 yz = from->y * to->z;
    f32 zy = from->z * to->y;
    f32 length;
    cross.x = yz - zy;
    cross.y = zx - xz;
    cross.z = xy - yx;
    length = (f32)sqrt(cross.z * cross.z + (cross.x * cross.x + cross.y * cross.y));
    if (length == 0.0f) {
        *direction = kmplsIdentity;
    } else {
        Vec fromCross, toCross;
        f32 inverse = 1.0f / length;
        f32 wxwy, wxwz, wywz;
        cross.x *= inverse;
        cross.y *= inverse;
        cross.z *= inverse;
        fromCross.x = from->y * cross.z - from->z * cross.y;
        fromCross.y = from->z * cross.x - from->x * cross.z;
        fromCross.z = from->x * cross.y - from->y * cross.x;
        toCross.x = to->y * cross.z - to->z * cross.y;
        toCross.y = to->z * cross.x - to->x * cross.z;
        toCross.z = to->x * cross.y - to->y * cross.x;
        wxwy = cross.y * cross.x;
        wxwz = cross.z * cross.x;
        wywz = cross.z * cross.y;
        direction->X.y = xy + (wxwy + toCross.y * fromCross.x);
        direction->X.z = xz + (wxwz + toCross.z * fromCross.x);
        direction->Y.x = yx + (wxwy + toCross.x * fromCross.y);
        direction->X.x = to->x * from->x + (cross.x * cross.x + toCross.x * fromCross.x);
        direction->Y.z = yz + (wywz + toCross.z * fromCross.y);
        direction->Z.x = zx + (wxwz + toCross.x * fromCross.z);
        direction->Z.y = zy + (wywz + toCross.y * fromCross.z);
        direction->Y.y = to->y * from->y + (cross.y * cross.y + toCross.y * fromCross.y);
        direction->Z.z = to->z * from->z + (cross.z * cross.z + toCross.z * fromCross.z);
    }
}

void __KMPLS_mult_dir(KPADMPLSDirection* left, KPADMPLSDirection* right,
                      KPADMPLSDirection* result) {
    result->X.x = right->X.z * left->Z.x + (right->X.x * left->X.x + right->X.y * left->Y.x);
    result->X.y = right->X.z * left->Z.y + (right->X.x * left->X.y + right->X.y * left->Y.y);
    result->X.z = right->X.z * left->Z.z + (right->X.x * left->X.z + right->X.y * left->Y.z);
    result->Y.x = right->Y.z * left->Z.x + (right->Y.x * left->X.x + right->Y.y * left->Y.x);
    result->Y.y = right->Y.z * left->Z.y + (right->Y.x * left->X.y + right->Y.y * left->Y.y);
    result->Y.z = right->Y.z * left->Z.z + (right->Y.x * left->X.z + right->Y.y * left->Y.z);
    result->Z.x = right->Z.z * left->Z.x + (right->Z.x * left->X.x + right->Z.y * left->Y.x);
    result->Z.y = right->Z.z * left->Z.y + (right->Z.x * left->X.y + right->Z.y * left->Y.y);
    result->Z.z = right->Z.z * left->Z.z + (right->Z.x * left->X.z + right->Z.y * left->Y.z);
}

void __KMPLS_linear_Direction(KPADMPLSDirection* from, KPADMPLSDirection* to,
                             KPADMPLSDirection* result, f32 amount) {
    result->X.x = from->X.x + amount * (to->X.x - from->X.x);
    result->X.y = from->X.y + amount * (to->X.y - from->X.y);
    result->X.z = from->X.z + amount * (to->X.z - from->X.z);
    result->Y.x = from->Y.x + amount * (to->Y.x - from->Y.x);
    result->Y.y = from->Y.y + amount * (to->Y.y - from->Y.y);
    result->Y.z = from->Y.z + amount * (to->Y.z - from->Y.z);
    result->Z.x = from->Z.x + amount * (to->Z.x - from->Z.x);
    result->Z.y = from->Z.y + amount * (to->Z.y - from->Z.y);
    result->Z.z = from->Z.z + amount * (to->Z.z - from->Z.z);
    __KMPLS_normalize_directionXYZ(result, 2.999f);
}
