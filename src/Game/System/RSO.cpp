#include "Game/System/HomeButtonMenuWrapper.hpp"
#include "Game/Util.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <revolution/rso.h>

#ifdef GALAXY_NATIVE
#include "petari/home_menu_hbm.hpp"
#endif

void (*HBMCreateRSO)(const HBMDataInfo*);
void (*HBMInitRSO)(void);
void (*HBMCalcRSO)(const HBMControllerData*);
void (*HBMDrawRSO)(void);
HBMSelectBtnNum (*HBMGetSelectBtnNumRSO)(void);
void (*HBMSetAdjustFlagRSO)(int);
void (*HBMStartBlackOutRSO)(void);

static RSOExportFuncTable exp_tbl[] = {{"HBMCreateRSO", (u32*)&HBMCreateRSO},
                                       {"HBMInitRSO", (u32*)&HBMInitRSO},
                                       {"HBMCalcRSO", (u32*)&HBMCalcRSO},
                                       {"HBMDrawRSO", (u32*)&HBMDrawRSO},
                                       {"HBMGetSelectBtnNumRSO", (u32*)&HBMGetSelectBtnNumRSO},
                                       {"HBMSetAdjustFlagRSO", (u32*)&HBMSetAdjustFlagRSO},
                                       {"HBMStartBlackOutRSO", (u32*)&HBMStartBlackOutRSO}};

typedef void (*ProloguePtr)(BOOL);

void RSO::setupRsoHomeButtonMenu() {
    u32 i;
    RSOObjectHeader* rsoPtr;
    RSOExportFuncTable* pTbl;
    const RSOObjectHeader* symbolTable = reinterpret_cast< const RSOObjectHeader* >(MR::receiveFile("/ModuleData/product.sel"));
    int jumpCodeSize;
    void* bss;
    void* jumps;

    RSOListInit((void*)symbolTable);
    if (symbolTable != nullptr) {
        jumpCodeSize = RSOGetJumpCodeSize(symbolTable);

        if (jumpCodeSize == 0) {
            jumps = nullptr;
        } else {
            jumps = new (MR::getStationedHeapNapa(), 4) char[jumpCodeSize];
            RSOMakeJumpCode(symbolTable, jumps);
        }
        rsoPtr = reinterpret_cast< RSOObjectHeader* >(MR::receiveFile("/ModuleData/HomeButtonMenuWrapperRSO.rso"));
        if (rsoPtr->mBssSize) {
            bss = new (MR::getStationedHeapNapa(), 4) char[rsoPtr->mBssSize];
        }

        RSOLinkList((void*)rsoPtr, bss);

        if (rsoPtr != nullptr) {
            reinterpret_cast< ProloguePtr >(rsoPtr->mProlog)(RSOLinkJump(rsoPtr, symbolTable, jumps));
            for (i = 0; i < ARRAY_SIZE(exp_tbl); i++) {
                pTbl = &exp_tbl[i];
                *(pTbl->symbol_ptr) = (u32)RSOFindExportSymbolAddr(rsoPtr, pTbl->symbol_name);
            }
        }
    }
}

void RSO::HBMCreate(const HBMDataInfo* pHBInfo) {
    (*HBMCreateRSO)(pHBInfo);
}

void RSO::HBMInit() {
    (*HBMInitRSO)();
}

void RSO::HBMCalc(const HBMControllerData* pController) {
    (*HBMCalcRSO)(pController);
}

void RSO::HBMDraw() {
    (*HBMDrawRSO)();
}

HBMSelectBtnNum RSO::HBMGetSelectBtnNum() {
    return (*HBMGetSelectBtnNumRSO)();
}

void RSO::HBMSetAdjustFlag(int flag) {
    (*HBMSetAdjustFlagRSO)(flag);
}

void RSO::HBMStartBlackOut() {
    (*HBMStartBlackOutRSO)();
}

void _unresolved(void) {
}
