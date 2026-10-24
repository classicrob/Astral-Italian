#include "revolution/kpad/KMPLSInternal.h"
#include <cmath>

KMPLSWork* kmplsWork;

static s32 s_rateRange[3][3] = {
    {1600, 1600, 1600}, {1600, 1600, 1600}, {1600, 1600, 1600}
};
static s32 s_rawRange[3][3] = {
    {0x7FFFFF, 0x7FFFFF, 0x7FFFFF},
    {0x7FFFFF, 0x7FFFFF, 0x7FFFFF},
    {0x7FFFFF, 0x7FFFFF, 0x7FFFFF}
};

// NONMATCHING: Null-check and channel-address scheduling differ; both result stores retain separate return arms.
void wpad_callback_func(s32 chan, s32 result) {
    if (kmplsWork) {
        KMPLSState* state = &kmplsWork->channels[chan];
        if (result == 0) {
            state->calibrationResult = 1;
            return;
        }
        state->calibrationResult = -1;
        return;
    }
}

static void invalidate_calibration_history(s32 chan) {
    if (kmplsWork) {
        KMPLSState* state = &kmplsWork->channels[chan];
        u32 index = (state->historyHead + 255) & 255;
        state->history[2][index] = 0x80000000;
        state->history[1][index] = 0x80000000;
        state->history[0][index] = 0x80000000;
        state->historyTrust = 0.0f;
    }
}

// NONMATCHING: Register allocation, null-check placement, and scheduling around the same calibration command/history invalidation.
void work_calibration(s32 chan) {
    if (kmplsWork) {
        KMPLSState* state = &kmplsWork->channels[chan];
        if (state->calibrationResult == 0) {
            invalidate_calibration_history(chan);
        } else if (state->calibrationResult < 0) {
            state->calibrationResult = 0;
            WPADiControlMpls(chan, 0x80, wpad_callback_func);
        } else if (state->historyTrust == 1.0f) {
            if (state->calibrationWindow <= 0.01f) {
                state->calibrating = 0;
            } else {
                state->calibrationResult = 0;
                WPADiControlMpls(chan, 0x80, wpad_callback_func);
                state->calibrationWindow -= 0.01f;
                invalidate_calibration_history(chan);
            }
        }
    }
}

BOOL KMPLSIsInit(void) {
    return kmplsWork != NULL;
}

// NONMATCHING: Width/height loads move before the sensor-bar call; the minimum test materializes CR.LT through mfcr/srwi. Ordered FP and unordered branch behavior are retained.
void dpd_revise_scale(s32 chan) {
    f32 x = 0.0f;
    if (kmplsWork) {
        KMPLSState* state = &kmplsWork->channels[chan];
        f32 y;
        f32 width;
        f32 height;
        f32 diagonal;
        if (state->aimingMode) {
            y = WPADGetSensorBarPosition() == 1 ? -0.2f : 0.2f;
        } else {
            y = -state->dpdYOffset;
        }
        width = 1.0f;
        height = 0.75f;
        diagonal = (f32)sqrt(width * width + height * height);
        if (x < 0.0f) width += x;
        else width -= x;
        if (y < 0.0f) height += y;
        else height -= y;
        width = width < height ? width : height;
        kmplsWork->channels[chan].dpdReviseScale = diagonal / width;
    }
}

// NONMATCHING: Literal-pool relocation residue; the ordered instruction body is otherwise retained.
f32 move_mpls_orient(KMPLSState* state, Vec* direction, Vec* previous, Vec* change) {
    Vec difference;
    Vec motion;
    f32 length;
    f32 dot;
    difference.x = direction->x - previous->x;
    difference.y = direction->y - previous->y;
    difference.z = direction->z - previous->z;
    length = (f32)sqrt(difference.z * difference.z +
                        (difference.x * difference.x + difference.y * difference.y));
    motion.x = direction->x - change->x;
    motion.y = direction->y - change->y;
    motion.z = direction->z - change->z;
    dot = motion.z * difference.z + (motion.x * difference.x + motion.y * difference.y);
    if (dot >= 0.0f) {
        return 0.0f;
    }
    dot = direction->z * motion.z + (direction->x * motion.x + direction->y * motion.y);
    motion.x -= dot * direction->x;
    motion.y -= dot * direction->y;
    motion.z -= dot * direction->z;
    dot = (f32)sqrt(motion.z * motion.z + (motion.x * motion.x + motion.y * motion.y));
    if (dot < 0.001f) {
        return 0.0f;
    }
    dot = (length * state->orientationCorrectionRate) / dot;
    direction->x -= dot * motion.x;
    direction->y -= dot * motion.y;
    direction->z -= dot * motion.z;
    return dot;
}

// NONMATCHING: Temporary matrices/vector copies, stack slots, and FP registers differ; aiming/height refresh, valid-report gate, ordered math-call inputs, and max-error branches are retained.
f32 revise_dir_dpd(s32 chan, KPADMPLSDirection* direction, const KPADStatus* context) {
    KMPLSState* state;
    f32 scale = 0.383864f;
    KPADMPLSDirection horizon;
    KPADMPLSDirection rotation;
    KPADMPLSDirection intermediate;
    KPADMPLSDirection previous;
    Vec from;
    Vec desired;
    f32 length;
    f32 errorX, errorY, errorZ;
    if (!kmplsWork) return 0.0f;
    state = &kmplsWork->channels[chan];
    if (kmplsWork->channels[chan].aimingMode != (u8)KPADIsEnableAimingMode(chan) ||
        kmplsWork->channels[chan].dpdYOffset != KPADGetSensorHeight(chan)) {
        kmplsWork->channels[chan].aimingMode = (u8)KPADIsEnableAimingMode(chan);
        kmplsWork->channels[chan].dpdYOffset = KPADGetSensorHeight(chan);
        dpd_revise_scale(chan);
    }
    scale /= state->dpdReviseScale;
    if ((u8)context->dpd_valid_fg != 2) return 0.0f;
    horizon.X.x = context->horizon.x;
    horizon.X.y = context->horizon.y;
    horizon.X.z = 0.0f;
    horizon.Y.x = -context->horizon.y;
    horizon.Y.y = context->horizon.x;
    horizon.Y.z = 0.0f;
    horizon.Z = kmplsIdentity.Z;
    from = kmplsIdentity.Z;
    desired = kmplsIdentity.Z;
    desired.x = (-scale) * context->pos.x;
    __KMPLS_normalize_Fxyz(&desired);
    __KMPLS_make_vec_dir(&rotation, &from, &desired);
    __KMPLS_mult_dir(&rotation, &horizon, &intermediate);
    from = intermediate.Z;
    desired = from;
    length = (f32)sqrt(direction->Z.x * direction->Z.x + direction->Z.z * direction->Z.z);
    desired.x *= length;
    desired.z *= length;
    desired.y = direction->Z.y;
    __KMPLS_make_vec_dir(&rotation, &from, &desired);
    __KMPLS_mult_dir(&rotation, &intermediate, &horizon);
    previous = *direction;
    __KMPLS_linear_Direction(direction, &horizon, direction, state->dpdCorrectionRate);
    previous.X.x -= direction->X.x;
    previous.X.y -= direction->X.y;
    previous.X.z -= direction->X.z;
    errorX = previous.X.z * previous.X.z + (previous.X.x * previous.X.x + previous.X.y * previous.X.y);
    previous.Y.x -= direction->Y.x;
    previous.Y.y -= direction->Y.y;
    previous.Y.z -= direction->Y.z;
    errorY = previous.Y.z * previous.Y.z + (previous.Y.x * previous.Y.x + previous.Y.y * previous.Y.y);
    previous.Z.x -= direction->Z.x;
    previous.Z.y -= direction->Z.y;
    previous.Z.z -= direction->Z.z;
    errorZ = previous.Z.z * previous.Z.z + (previous.Z.x * previous.Z.x + previous.Z.y * previous.Z.y);
    if (errorX > errorY) {
        if (errorX > errorZ) return (f32)sqrt(errorX);
        return (f32)sqrt(errorZ);
    }
    if (errorY > errorZ) return (f32)sqrt(errorY);
    return (f32)sqrt(errorZ);
}

// NONMATCHING: Shorter FPR save set and fewer temporary spills; all nine ordered direction-update expressions and normalization argument agree.
void calc_mpls_dir(KMPLSState* state) {
    KPADMPLSDirection original = state->sample.direction;
    f32 x = state->rotationStep * state->sample.angularVelocity.x;
    f32 y = state->rotationStep * state->sample.angularVelocity.y;
    f32 z;
    f32 degreesX = state->degreesPerSample * x;
    f32 degreesY = state->degreesPerSample * y;
    f32 degreesZ;
    f32 stepX;
    f32 stepY;
    f32 stepZ;
    degreesX *= degreesX;
    degreesY *= degreesY;
    stepX = 1.0f + 0.0001f * degreesX;
    stepY = 1.0f + 0.0001f * degreesY;
    stepX *= state->radiansPerSample * x;
    stepY *= state->radiansPerSample * y;
    state->sample.direction.Z.x = (original.Z.x - stepX * original.Y.x) + stepY * original.X.x;
    state->sample.direction.Z.y = (original.Z.y - stepX * original.Y.y) + stepY * original.X.y;
    state->sample.direction.Z.z = (original.Z.z - stepX * original.Y.z) + stepY * original.X.z;
    z = state->rotationStep * state->sample.angularVelocity.z;
    degreesZ = state->degreesPerSample * z;
    degreesZ *= degreesZ;
    stepZ = 1.0f + 0.0001f * degreesZ;
    stepZ *= state->radiansPerSample * z;
    state->sample.direction.X.x = (original.X.x - stepY * original.Z.x) + stepZ * original.Y.x;
    state->sample.direction.X.y = (original.X.y - stepY * original.Z.y) + stepZ * original.Y.y;
    state->sample.direction.X.z = (original.X.z - stepY * original.Z.z) + stepZ * original.Y.z;
    state->sample.direction.Y.x = (original.Y.x + stepX * original.Z.x) - stepZ * original.X.x;
    state->sample.direction.Y.y = (original.Y.y + stepX * original.Z.y) - stepZ * original.X.y;
    state->sample.direction.Y.z = (original.Y.z + stepX * original.Z.z) - stepZ * original.X.z;
    __KMPLS_normalize_directionXYZ(&state->sample.direction, 2.999f);
}

// NONMATCHING: Temporary vector/matrix stack slots and FP register lifetimes differ; all correction calls, ordered FP expressions, and cutoff branches are retained.
f32 revise_dir_acc(s32 chan, KPADMPLSDirection* direction, const Vec* acceleration) {
    KMPLSState* state;
    Vec normalized;
    Vec current;
    Vec desired;
    KPADMPLSDirection rotation;
    KPADMPLSDirection result;
    f32 lengthSquared;
    f32 length;
    f32 weight;
    f32 inverse;
    if (!kmplsWork) return 0.0f;
    state = &kmplsWork->channels[chan];
    lengthSquared = acceleration->z * acceleration->z +
                     (acceleration->x * acceleration->x + acceleration->y * acceleration->y);
    if (lengthSquared == 0.0f) return 0.0f;
    length = (f32)sqrt(lengthSquared);
    if (length < 1.0f) {
        f32 low = 1.0f - state->accelMagnitudeTolerance;
        if (length <= low) return 0.0f;
        weight = (1.0f / state->accelMagnitudeTolerance) * (length - low);
    } else {
        f32 high = 1.0f + state->accelMagnitudeTolerance;
        if (length >= high) return 0.0f;
        weight = (-1.0f / state->accelMagnitudeTolerance) * (length - high);
    }
    weight *= weight;
    inverse = 1.0f / length;
    normalized.x = inverse * acceleration->x;
    normalized.y = inverse * acceleration->y;
    normalized.z = inverse * acceleration->z;
    current.x = normalized.z * direction->Z.x +
                (normalized.x * direction->X.x + normalized.y * direction->Y.x);
    current.y = normalized.z * direction->Z.y +
                (normalized.x * direction->X.y + normalized.y * direction->Y.y);
    current.z = normalized.z * direction->Z.z +
                (normalized.x * direction->X.z + normalized.y * direction->Y.z);
    weight *= state->accelCorrectionRate;
    desired.x = current.x + (-current.x) * weight;
    desired.y = current.y + (-1.0f - current.y) * weight;
    desired.z = current.z + (-current.z) * weight;
    if (__KMPLS_normalize_Fxyz(&desired) == 0.0f) return 0.0f;
    __KMPLS_make_vec_dir(&rotation, &current, &desired);
    __KMPLS_mult_dir(&rotation, direction, &result);
    *direction = result;
    __KMPLS_normalize_directionXYZ(direction, 2.999f);
    current.x -= desired.x;
    current.y -= desired.y;
    current.z -= desired.z;
    return (f32)sqrt(current.z * current.z + (current.x * current.x + current.y * current.y));
}

// NONMATCHING: Integer/FP register allocation, stack-slot choices, and equivalent signed/unsigned ring comparisons differ. Target history traversal, limits, invalid-sample handling, ordered filtering/integration, and callback behavior are retained.
void read_mpls(s32 chan, s32 ringEnd, s32 count) {
    KMPLSState* state;
    s32 threshold[3];
    s32 historyWindow;
    u8 adjustZero;
    f32 window;
    f32 speed;
    s32 limits[3];
    s32 minimum[3];
    if (count <= 0 || !kmplsWork) return;
    state = &kmplsWork->channels[chan];
    if (state->calibrating) {
        adjustZero = 1;
        window = state->calibrationWindow;
        historyWindow = (s32)(1500.0f * window);
        speed = 0.9f;
    } else {
        adjustZero = state->zeroAdjustEnabled;
        window = state->defaultCalibrationWindow;
        historyWindow = state->historyWindow;
        speed = state->zeroAdjustSpeed;
    }
    limits[0] = s_rawRange[state->reportMode][0];
    limits[1] = s_rawRange[state->reportMode][1];
    limits[2] = s_rawRange[state->reportMode][2];
    minimum[0] = -limits[0];
    minimum[1] = -limits[1];
    minimum[2] = -limits[2];
    threshold[0] = __KMPLS_f2i(window / state->rateScale.x);
    threshold[1] = __KMPLS_f2i(window / state->rateScale.y);
    threshold[2] = __KMPLS_f2i(window / state->rateScale.z);
    if (threshold[0] == 0) ++threshold[0];
    if (threshold[1] == 0) ++threshold[1];
    if (threshold[2] == 0) ++threshold[2];
    state->unstableCount[2] = 0;
    state->unstableCount[1] = 0;
    state->unstableCount[0] = 0;
    state->zeroTrust = 1.0f;
    state->historyTrust = 1.0f;
    do {
        s32 index = ringEnd - count - 1;
        const KPADUnifiedWpadStatus* report;
        Vec previous;
        s32 stable[3] = {1, 1, 1};
        if (index < 0) index += state->externalLength + 16;
        if (index >= 16) report = state->externalRing + (index - 16);
        else report = state->internalRing + index;
        previous = state->previousRate;
        state->previousRate = state->sample.angularVelocity;
        if (report->u.core.err) {
            state->rateDifference.x *= kmplsWork->invalidSampleDecay;
            state->sample.angularVelocity.x += state->rateDifference.x;
            state->rateDifference.y *= kmplsWork->invalidSampleDecay;
            state->sample.angularVelocity.y += state->rateDifference.y;
            state->rateDifference.z *= kmplsWork->invalidSampleDecay;
            state->sample.angularVelocity.z += state->rateDifference.z;
            state->zeroTrust = 0.0f;
            state->historyTrust = 0.0f;
        } else {
            state->historyHead = (state->historyHead + 1) & 255;
            {
                s32 raw;
                s32 sum;
                s32 difference;
                if (state->reportMode < 2) raw = get_mpls_data_x(state, &report->u.mp);
                else raw = report->u.fs.fsAccZ;
                state->history[0][state->historyHead] = raw;
                sum = raw;
                difference = raw - __KMPLS_f2i(state->zeroCenter.x);
                if (difference <= minimum[0] || difference >= limits[0]) {
                    state->rateDifference.x *= kmplsWork->invalidSampleDecay;
                    state->sample.angularVelocity.x += state->rateDifference.x;
                    state->unstableCount[0]++;
                    state->zeroTrust = 0.0f;
                } else {
                    s32 low = raw - threshold[0];
                    s32 high = raw + threshold[0];
                    u32 index = (state->historyHead - 1) & 255;
                    u32 stop = (state->historyHead - historyWindow) & 255;
                    f32 confidence;
                    f32 average;
                    state->sample.angularVelocity.x = (f32)raw;
                    do {
                        s32 value = state->history[0][index];
                        if (value < low || value > high) break;
                        sum += value;
                        index = (index - 1) & 255;
                        stable[0]++;
                    } while (index != stop);
                    confidence = (f32)(stable[0] - 1) / (f32)(historyWindow - 1);
                    average = (f32)sum / (f32)stable[0];
                    confidence *= confidence;
                    confidence *= confidence;
                    confidence *= confidence;
                    confidence *= confidence;
                    confidence *= confidence;
                    state->sample.angularVelocity.x += confidence * (average - state->sample.angularVelocity.x);
                    if (adjustZero) {
                        confidence *= speed;
                        state->zeroCenter.x += confidence * (state->sample.angularVelocity.x - state->zeroCenter.x);
                    }
                    state->sample.angularVelocity.x = state->rateScale.x * (state->sample.angularVelocity.x - state->zeroCenter.x);
                    if (state->zeroPlayEnabled) {
                        f32 value = state->sample.angularVelocity.x;
                        if (value >= -state->zeroPlayRadius && value <= state->zeroPlayRadius) {
                            f32 trust;
                            if (value < 0.0f) value = -value;
                            trust = 1.0f - value / state->zeroPlayRadius;
                            if (trust < state->zeroTrust) state->zeroTrust = trust;
                            state->sample.angularVelocity.x = 0.0f;
                        } else {
                            state->zeroTrust = 0.0f;
                        }
                    }
                    if (state->reportMode == 1) {
                        state->rateDifference.x = 0.5f * (state->sample.angularVelocity.x - previous.x);
                    } else {
                        state->rateDifference.x = state->sample.angularVelocity.x - state->previousRate.x;
                    }
                }
            }
            {
                s32 raw;
                s32 sum;
                s32 difference;
                if (state->reportMode < 2) raw = get_mpls_data_y(state, &report->u.mp);
                else raw = -report->u.fs.fsAccX;
                state->history[1][state->historyHead] = raw;
                sum = raw;
                difference = raw - __KMPLS_f2i(state->zeroCenter.y);
                if (difference <= minimum[1] || difference >= limits[1]) {
                    state->rateDifference.y *= kmplsWork->invalidSampleDecay;
                    state->sample.angularVelocity.y += state->rateDifference.y;
                    state->unstableCount[1]++;
                    state->zeroTrust = 0.0f;
                } else {
                    s32 low = raw - threshold[1];
                    s32 high = raw + threshold[1];
                    u32 index = (state->historyHead - 1) & 255;
                    u32 stop = (state->historyHead - historyWindow) & 255;
                    f32 confidence;
                    f32 average;
                    state->sample.angularVelocity.y = (f32)raw;
                    do {
                        s32 value = state->history[1][index];
                        if (value < low || value > high) break;
                        sum += value;
                        index = (index - 1) & 255;
                        stable[1]++;
                    } while (index != stop);
                    confidence = (f32)(stable[1] - 1) / (f32)(historyWindow - 1);
                    average = (f32)sum / (f32)stable[1];
                    confidence *= confidence;
                    confidence *= confidence;
                    confidence *= confidence;
                    confidence *= confidence;
                    confidence *= confidence;
                    state->sample.angularVelocity.y += confidence * (average - state->sample.angularVelocity.y);
                    if (adjustZero) {
                        confidence *= speed;
                        state->zeroCenter.y += confidence * (state->sample.angularVelocity.y - state->zeroCenter.y);
                    }
                    state->sample.angularVelocity.y = state->rateScale.y * (state->sample.angularVelocity.y - state->zeroCenter.y);
                    if (state->zeroPlayEnabled) {
                        f32 value = state->sample.angularVelocity.y;
                        if (value >= -state->zeroPlayRadius && value <= state->zeroPlayRadius) {
                            f32 trust;
                            if (value < 0.0f) value = -value;
                            trust = 1.0f - value / state->zeroPlayRadius;
                            if (trust < state->zeroTrust) state->zeroTrust = trust;
                            state->sample.angularVelocity.y = 0.0f;
                        } else {
                            state->zeroTrust = 0.0f;
                        }
                    }
                    if (state->reportMode == 1) {
                        state->rateDifference.y = 0.5f * (state->sample.angularVelocity.y - previous.y);
                    } else {
                        state->rateDifference.y = state->sample.angularVelocity.y - state->previousRate.y;
                    }
                }
            }
            {
                s32 raw;
                s32 sum;
                s32 difference;
                if (state->reportMode < 2) raw = get_mpls_data_z(state, &report->u.mp);
                else raw = -report->u.fs.fsAccY;
                state->history[2][state->historyHead] = raw;
                sum = raw;
                difference = raw - __KMPLS_f2i(state->zeroCenter.z);
                if (difference <= minimum[2] || difference >= limits[2]) {
                    state->rateDifference.z *= kmplsWork->invalidSampleDecay;
                    state->sample.angularVelocity.z += state->rateDifference.z;
                    state->unstableCount[2]++;
                    state->zeroTrust = 0.0f;
                } else {
                    s32 low = raw - threshold[2];
                    s32 high = raw + threshold[2];
                    u32 index = (state->historyHead - 1) & 255;
                    u32 stop = (state->historyHead - historyWindow) & 255;
                    f32 confidence;
                    f32 average;
                    state->sample.angularVelocity.z = (f32)raw;
                    do {
                        s32 value = state->history[2][index];
                        if (value < low || value > high) break;
                        sum += value;
                        index = (index - 1) & 255;
                        stable[2]++;
                    } while (index != stop);
                    confidence = (f32)(stable[2] - 1) / (f32)(historyWindow - 1);
                    average = (f32)sum / (f32)stable[2];
                    confidence *= confidence;
                    confidence *= confidence;
                    confidence *= confidence;
                    confidence *= confidence;
                    confidence *= confidence;
                    state->sample.angularVelocity.z += confidence * (average - state->sample.angularVelocity.z);
                    if (adjustZero) {
                        confidence *= speed;
                        state->zeroCenter.z += confidence * (state->sample.angularVelocity.z - state->zeroCenter.z);
                    }
                    state->sample.angularVelocity.z = state->rateScale.z * (state->sample.angularVelocity.z - state->zeroCenter.z);
                    if (state->zeroPlayEnabled) {
                        f32 value = state->sample.angularVelocity.z;
                        if (value >= -state->zeroPlayRadius && value <= state->zeroPlayRadius) {
                            f32 trust;
                            if (value < 0.0f) value = -value;
                            trust = 1.0f - value / state->zeroPlayRadius;
                            if (trust < state->zeroTrust) state->zeroTrust = trust;
                            state->sample.angularVelocity.z = 0.0f;
                        } else {
                            state->zeroTrust = 0.0f;
                        }
                    }
                    if (state->reportMode == 1) {
                        state->rateDifference.z = 0.5f * (state->sample.angularVelocity.z - previous.z);
                    } else {
                        state->rateDifference.z = state->sample.angularVelocity.z - state->previousRate.z;
                    }
                }
            }
            {
                s32 minimum;
                f32 trust;
                if (stable[0] < stable[1]) {
                    minimum = stable[0] < stable[2] ? stable[0] : stable[2];
                } else {
                    minimum = stable[1] < stable[2] ? stable[1] : stable[2];
                }
                trust = (f32)(minimum - 1) / (f32)(historyWindow - 1);
                if (trust < state->historyTrust) state->historyTrust = trust;
            }
        }
        state->sample.angle.x += state->samplePeriod * state->sample.angularVelocity.x;
        state->sample.angle.y += state->samplePeriod * state->sample.angularVelocity.y;
        state->sample.angle.z += state->samplePeriod * state->sample.angularVelocity.z;
        calc_mpls_dir(state);
        if (kmplsWork->samplingCallback) {
            KPADMPLSStatus sample = state->sample;
            kmplsWork->samplingCallback(chan, &sample);
        }
    } while (--count);
    kmplsWork->samplingCallback = NULL;
}

// NONMATCHING: Register/stack allocation and scheduling of the same six zero-scale tests, double conversion coefficients, and calibration/history stores.
void set_calibration_data(s32 chan) {
    if (kmplsWork) {
        KMPLSState* state = &kmplsWork->channels[chan];
        f64 lowRate;
        f64 highRate;
        if (state->reportMode >= 2) {
            state->rateConversion[0][0] = 0.0;
            return;
        }
        WPADiGetMplsCalibration(chan, &state->calibration[0], &state->calibration[1]);
        if (state->calibration[0].axis[0].scale == 0.0f ||
            state->calibration[0].axis[1].scale == 0.0f ||
            state->calibration[0].axis[2].scale == 0.0f ||
            state->calibration[1].axis[0].scale == 0.0f ||
            state->calibration[1].axis[1].scale == 0.0f ||
            state->calibration[1].axis[2].scale == 0.0f) {
            state->rateConversion[0][0] = 0.0;
            return;
        }
        lowRate = 5242.879375 * state->calibration[0].referenceRate;
        state->rateConversion[0][0] = lowRate / state->calibration[0].axis[0].scale;
        state->rateConversion[0][1] = lowRate / state->calibration[0].axis[1].scale;
        state->rateConversion[0][2] = lowRate / state->calibration[0].axis[2].scale;
        highRate = 5242.879375 * state->calibration[1].referenceRate;
        state->rateConversion[1][0] = highRate / state->calibration[1].axis[0].scale;
        state->rateConversion[1][1] = highRate / state->calibration[1].axis[1].scale;
        state->rateConversion[1][2] = highRate / state->calibration[1].axis[2].scale;
        state->calibrationResult = 1;
        if (kmplsWork) {
            kmplsWork->channels[chan].calibrating = 1;
            kmplsWork->channels[chan].calibrationWindow = 0.03f;
            invalidate_calibration_history(chan);
        }
    }
}

// NONMATCHING: Initial status-byte load is scheduled before stack allocation; remaining instructions agree modulo pool relocations.
s32 get_mpls_data_x(KMPLSState* state, const WPADMPStatus* report) {
    f32 zero;
    f64 conversion;
    f64 value;
    f64 speed;
    f64 output;
    if (report->status & 4) {
        zero = state->calibration[1].axis[0].zero;
        conversion = state->rateConversion[1][0];
    } else {
        zero = state->calibration[0].axis[0].zero;
        conversion = state->rateConversion[0][0];
    }
    value = report->rotation[0];
    value -= zero;
    value = conversion * value;
    value -= state->filteredRaw[0];
    speed = 50.0 * value;
    if (speed < 0.0) speed = -speed;
    if (speed < 0.1) speed = 0.1;
    else if (speed > 1.0) speed = 1.0;
    value *= speed;
    state->filteredRaw[0] += value;
    output = state->filteredRaw[0] * state->rawScale.x;
    if (output < 0.0) return (s32)(output - 0.5);
    return (s32)(0.5 + output);
}

// NONMATCHING: Initial status-byte load is scheduled before stack allocation; remaining instructions agree modulo pool relocations.
s32 get_mpls_data_y(KMPLSState* state, const WPADMPStatus* report) {
    f32 zero;
    f64 conversion;
    f64 value;
    f64 speed;
    f64 output;
    if (report->status & 8) {
        zero = state->calibration[1].axis[1].zero;
        conversion = state->rateConversion[1][1];
    } else {
        zero = state->calibration[0].axis[1].zero;
        conversion = state->rateConversion[0][1];
    }
    value = report->rotation[1];
    value -= zero;
    value = conversion * value;
    value -= state->filteredRaw[1];
    speed = 50.0 * value;
    if (speed < 0.0) speed = -speed;
    if (speed < 0.1) speed = 0.1;
    else if (speed > 1.0) speed = 1.0;
    value *= speed;
    state->filteredRaw[1] += value;
    output = state->filteredRaw[1] * state->rawScale.y;
    if (output < 0.0) return (s32)(output - 0.5);
    return (s32)(0.5 + output);
}

// NONMATCHING: Initial status-byte load is scheduled before stack allocation; remaining instructions agree modulo pool relocations.
s32 get_mpls_data_z(KMPLSState* state, const WPADMPStatus* report) {
    f32 zero;
    f64 conversion;
    f64 value;
    f64 speed;
    f64 output;
    if (report->status & 2) {
        zero = state->calibration[1].axis[2].zero;
        conversion = state->rateConversion[1][2];
    } else {
        zero = state->calibration[0].axis[2].zero;
        conversion = state->rateConversion[0][2];
    }
    value = report->rotation[2];
    value -= zero;
    value = conversion * value;
    value -= state->filteredRaw[2];
    speed = 50.0 * value;
    if (speed < 0.0) speed = -speed;
    if (speed < 0.1) speed = 0.1;
    else if (speed > 1.0) speed = 1.0;
    value *= speed;
    state->filteredRaw[2] += value;
    output = state->filteredRaw[2] * state->rawScale.z;
    if (output < 0.0) return (s32)(output - 0.5);
    return (s32)(0.5 + output);
}

// NONMATCHING: Channel-address arithmetic is scheduled after the null guard; the same three ring fields are written.
void KMPLSSetKpadRingBuffer(s32 chan, const KPADUnifiedWpadStatus* internalRing,
                          const KPADUnifiedWpadStatus* externalRing, u32 externalLength) {
    if (kmplsWork) {
        KMPLSState* state = &kmplsWork->channels[chan];
        state->internalRing = internalRing;
        state->externalRing = externalRing;
        state->externalLength = externalLength;
    }
}

// NONMATCHING: Initial null-check/address scheduling and literal references differ.
void KMPLSRead(s32 chan, KPADMPLSStatus* result, s32 ringEnd, s32 count,
               const KPADStatus* context, f64 samplePeriod) {
    KMPLSState* state;
    WPADDeviceType device;
    s32 mode = 2;
    if (!kmplsWork) return;
    state = &kmplsWork->channels[chan];
    if (WPADProbe(chan, &device) != -1 &&
        ((u32)(device - 5) <= 2 || device == 250)) {
        u8 status = WPADiGetMplsStatus(chan);
        if (status == 4) mode = 0;
        else if (status != 0) {
            mode = device != 5;
            state->reportMode = mode;
        }
    }
    if (kmplsWork) {
        KMPLSState* current = &kmplsWork->channels[chan];
        if (current->reportMode != mode) current->rateConversion[0][0] = 0.0;
        current->reportMode = mode;
        current->rateScale.x = (f32)(1.0 / ((360.0 / s_rateRange[current->reportMode][0]) * s_rawRange[current->reportMode][0]));
        current->rateScale.y = (f32)(1.0 / ((360.0 / s_rateRange[current->reportMode][1]) * s_rawRange[current->reportMode][1]));
        current->rateScale.z = (f32)(1.0 / ((360.0 / s_rateRange[current->reportMode][2]) * s_rawRange[current->reportMode][2]));
    }
    if (count > 0 && context && mode != 2) {
        state->samplePeriod = (f32)samplePeriod;
        state->degreesPerSample = (f32)(360.0 * samplePeriod);
        state->radiansPerSample = (f32)(6.283185307179586 * samplePeriod);
        if (state->rateConversion[0][0] == 0.0) set_calibration_data(chan);
        state->previousDirection = state->sample.direction;
        read_mpls(chan, ringEnd, count);
        if (state->calibrating) work_calibration(chan);
        if (state->orientationCorrection) {
            f32 trust;
            if (!kmplsWork) {
                trust = 0.0f;
            } else {
                KMPLSState* current = &kmplsWork->channels[chan];
                f32 value;
                trust = move_mpls_orient(current, &current->sample.direction.X,
                                         &current->previousDirection.X, &current->directionChange.X);
                value = move_mpls_orient(current, &current->sample.direction.Y,
                                         &current->previousDirection.Y, &current->directionChange.Y);
                if (value > trust) trust = value;
                value = move_mpls_orient(current, &current->sample.direction.Z,
                                         &current->previousDirection.Z, &current->directionChange.Z);
                if (value > trust) trust = value;
                __KMPLS_normalize_directionXYZ(&current->sample.direction, 2.999f);
            }
            state->orientationTrust = trust;
        } else {
            state->orientationTrust = 0.0f;
        }
        if (state->accelCorrection) {
            state->accelTrust = revise_dir_acc(chan, &state->sample.direction, &context->acc);
        } else {
            state->accelTrust = 0.0f;
        }
        if (state->dpdCorrection) {
            state->dpdTrust = revise_dir_dpd(chan, &state->sample.direction, context);
        } else {
            state->dpdTrust = 0.0f;
        }
    }
    result->angularVelocity = state->sample.angularVelocity;
    result->angle = state->sample.angle;
    result->direction = state->sample.direction;
}

void KMPLSSetSamplingCallback(KMPLSSamplingCallback callback) {
    if (kmplsWork) {
        kmplsWork->samplingCallback = callback;
    }
}
