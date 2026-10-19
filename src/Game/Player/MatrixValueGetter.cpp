#include "Game/Player/MatrixControl.hpp"
#include "Game/Util/HashUtil.hpp"

MatrixValueGetter::MatrixValueGetter(const char* name, MatrixValueTable* values) : NameObj(name), _C(values) {
    u32 count = 0;
    while (values[count].mName[0] != '\0') {
        count++;
    }
    _10 = new HashSortTable(count);
    for (u32 i = 0; i < count; i++) {
        _10->add(_C[i].mName, i, false);
        _10->sort();
    }
}

bool MatrixValueGetter::getValue(const char* name, f32* value) const {
    uintptr_t index;
    if (!_10->search(name, &index)) {
        return false;
    }
    if (value != nullptr) {
        *value = _C[index].mValue;
    }
    return true;
}

MatrixValueGetter::~MatrixValueGetter() {
}
