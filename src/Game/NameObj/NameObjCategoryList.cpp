#include "Game/NameObj/NameObjCategoryList.hpp"
#include "Game/NameObj/NameObj.hpp"
#include <algorithm>

NameObjCategoryList::NameObjCategoryList(u32 count, const CategoryListInitialTable* pTable, NameObjMethod pMethod, bool allocateByCheck,
                                         const char*) {
    NameObjMethod method;
    method = pMethod;
    mDelegator = new NameObjRealDelegator< NameObjMethod >(method);
    _D = allocateByCheck;
    _C = 0;
    initTable(count, pTable);
}

NameObjCategoryList::NameObjCategoryList(u32 count, const CategoryListInitialTable* pTable, NameObjMethodConst pMethod, bool allocateByCheck,
                                         const char*) {
    NameObjMethodConst method;
    method = pMethod;
    mDelegatorConst = new NameObjRealDelegator< NameObjMethodConst >(method);
    _D = allocateByCheck;
    _C = 0;
    initTable(count, pTable);
}

NameObjCategoryList::~NameObjCategoryList() {
    delete mDelegator;
}

void NameObjCategoryList::execute(int idx) {
    NameObjCategoryList::CategoryInfo* inf = &mInfoTable.mArr[idx];

    if (inf->_8 == 0) {
        return;
    }

    if (inf->mFunc) {
        (*inf->mFunc)();
    }

    NameObj** arr = inf->mList.mArr;

    while (arr != &inf->mList.mArr[inf->_8]) {
        (*mDelegator)(*arr);
        arr++;
    }
}

void NameObjCategoryList::incrementCheck(NameObj* /*unused*/, int index) {
    mInfoTable.mArr[index].mCheck++;
}

void NameObjCategoryList::allocateBuffer() {
    if (_D) {
        for (s32 i = 0; i < mInfoTable.size(); i++) {
            NameObjCategoryList::CategoryInfo* inf = &mInfoTable.mArr[i];
            u32 size = inf->mCheck;
            NameObj** nameObjArr = new NameObj*[size];
            MR::AssignableArray< NameObj* >* arr = &mInfoTable.mArr[i].mList;
            arr->mArr = nameObjArr;
            arr->mMaxSize = size;
        }

        _C = 1;
    }
}

void NameObjCategoryList::add(NameObj* pObj, int idx) {
    NameObjCategoryList::CategoryInfo* inf = &mInfoTable.mArr[idx];
    s32 cnt = inf->_8++;
    inf->mList.mArr[cnt] = pObj;
}
void NameObjCategoryList::remove(NameObj* pObj, int idx) {
    NameObjCategoryList::CategoryInfo* inf = &mInfoTable.mArr[idx];
    NameObj** pFound = std::find(inf->mList.begin(), &inf->mList[inf->_8], pObj);
    inf->mList[pFound - inf->mList.begin()] = inf->mList[inf->_8 - 1];
    inf->_8--;
}

void NameObjCategoryList::registerExecuteBeforeFunction(const MR::FunctorBase& rFunc, int idx) {
    NameObjCategoryList::CategoryInfo* pCategoryInfo = &mInfoTable.mArr[idx];
    pCategoryInfo->mFunc = rFunc.clone(nullptr);
}

bool NameObjCategoryList::isRegisteredExecuteFunction(int idx) const {
    return mInfoTable.mArr[idx].mFunc != nullptr;
}

void NameObjCategoryList::initTable(u32 count, const CategoryListInitialTable* pTable) {
    mInfoTable.mArr = new CategoryInfo[count];
    mInfoTable.mMaxSize = count;

    NameObjCategoryList::CategoryInfo* curInf = &mInfoTable.mArr[0];

    while ((curInf != &mInfoTable.mArr[mInfoTable.mMaxSize])) {
        curInf->mFunc = nullptr;
        curInf++;
    }

    const CategoryListInitialTable* ent = &pTable[0];

    while (ent->mIndex != -1) {
        if (!_D) {
            u32 size = ent->mCount;
            NameObj** arr = new NameObj*[size];
            NameObjCategoryList::CategoryInfo* inf = &mInfoTable.mArr[ent->mIndex];
            inf->mList.mArr = arr;
            inf->mList.mMaxSize = size;
            _C = 1;
        }

        mInfoTable.mArr[ent->mIndex].mCheck = 0;

        ent++;
    }
}

NameObjCategoryList::CategoryInfo::CategoryInfo() {
    _8 = 0;
}

NameObjCategoryList::CategoryInfo::~CategoryInfo() {
}
