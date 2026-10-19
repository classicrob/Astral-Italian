#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

MarioNameMatrix::MarioNameMatrix(const char* name, u32 rows, const char* const* rowNames, u32 columns, const char* const* columnNames, const char* const* columnKeys) : NameObj(name) {
    mRowCount = rows;
    mRows = rowNames;
    mColumnCount = columns;
    mColumns = columnNames;
    mColumnKeys = columnKeys;
    u32 size = rows * columns;
    mValues = new u8[size];
    MR::zeroMemory(mValues, size);
    mRowHash = new HashSortTable(rows);
    for (u32 i = 0; i < rows; ++i) mRowHash->add(rowNames[i], i, false);
    mRowHash->sort();
    mColumnHash = new HashSortTable(columns);
    for (u32 i = 0; i < columns; ++i) mColumnHash->add(columnNames[i], i, false);
    mColumnHash->sort();
    if (!mColumnKeys) mColumnKeys = mColumns;
}

MarioNameMatrix::~MarioNameMatrix() {}

bool MarioNameMatrix::setValue(const char* row, const char* column, u8 value) {
    s32 index = findIndex(row, column);
    if (index < 0) return false;
    mValues[index] = value;
    return true;
}

u8 MarioNameMatrix::getValue(const char* row, const char* column) const {
    return mValues[findIndex(row, column)];
}

bool MarioNameMatrix::getValueOrNone(const char* row, const char* column, u8* value) const {
    s32 index = findIndex(row, column);
    if (index < 0) return false;
    *value = mValues[index];
    return true;
}

void MarioNameMatrix::load(const char* archive, const char* file) {
    JMapInfo* csv = MR::createCsvParserFromFile(archive, file);
    if (!csv) return;
    s32 count = MR::getCsvDataElementNum(csv);
    for (s32 row = 0; row < count; ++row) {
        const char* name;
        MR::getCsvDataStr(&name, csv, "ColumnKey", row);
        for (u32 column = 0; column < mColumnCount; ++column) {
            u8 value;
            MR::getCsvDataU8(&value, csv, mColumnKeys[column], row);
            setValue(name, mColumns[column], value);
        }
    }
}

s32 MarioNameMatrix::findIndex(const char* row, const char* column) const {
    s32 rowIndex = findRow(row);
    s32 columnIndex = findColumn(column);
    if (rowIndex < 0 || columnIndex < 0) return -1;
    return columnIndex + rowIndex * mColumnCount;
}

s32 MarioNameMatrix::findRow(const char* name) const {
    uintptr_t index;
    if (mRowHash->search(name, &index)) return static_cast<s32>(index);
    return -1;
}

s32 MarioNameMatrix::findColumn(const char* name) const {
    uintptr_t index;
    if (mColumnHash->search(name, &index)) return static_cast<s32>(index);
    return -1;
}
