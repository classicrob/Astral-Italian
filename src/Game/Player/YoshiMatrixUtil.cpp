#define JGEOMETRY_OUTLINE_MATRIX_DIRECTION
#define JGEOMETRY_OUTLINE_MATRIX_ROTATION
#define JGEOMETRY_OUTLINE_MATRIX_POSITION
#define JGEOMETRY_VEC3_ANGLE_NOINLINE
#define JGEOMETRY_VEC3_WMR_OUTLINE
#include "Game/Player/Yoshi.hpp"
#include "Game/Util/MathUtil.hpp"

// NONMATCHING: scalar literal pool relocations differ; instructions and scalar bytes match.
void fn_8040BD00(TPos3f* matrix, const TVec3f& direction, f32 rate, f32 maxAngle) {
    TVec3f front;
    matrix->getZDir(front);
    TVec3f up;
    matrix->getYDir(up);
    TVec3f projected;
    projected.killElement(direction, up);
    if (MR::normalizeOrZero(&projected)) {
        projected.set(front);
    }

    f32 angle = front.angle(projected);
    TVec3f cross;
    cross.cross(front, projected);
    if (cross.dot(up) < 0.0f) {
        angle = -angle;
    }
    angle *= rate;
    if (angle > maxAngle) {
        angle = maxAngle;
    } else if (angle < -maxAngle) {
        angle = -maxAngle;
    }

    TVec3f position;
    TVec3f axis(0.0f, 1.0f, 0.0f);
    TPos3f rotation;
    rotation.makeRotate(axis, angle);
    matrix->getTrans(position);
    matrix->concat(rotation);
    matrix->setTrans(position);
}
