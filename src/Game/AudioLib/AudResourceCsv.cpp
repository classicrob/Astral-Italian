#include "Game/Util/JMapInfo.hpp"
const char* JMapInfoIter::getValueString(const char* key) const {
    const char* value = 0;
    getValue(key, &value);
    return value;
}
s32 JMapInfoIter::getValueS32(const char* key) const {
    s32 value = 0;
    getValue(key, &value);
    return value;
}
u32 JMapInfoIter::getValueU32(const char* key) const {
    u32 value = 0;
    getValue(key, &value);
    return value;
}
bool JMapInfoIter::getValueBool(const char* key) const {
    bool value = false;
    getValue(key, &value);
    return value;
}
f32 JMapInfoIter::getValueFloat(const char* key) const {
    f32 value = 0.0f;
    getValue(key, &value);
    return value;
}
u16 JMapInfoIter::getValueU16(const char* key) const {
    s32 value = 0;
    getValue(key, &value);
    return (u16)value;
}
