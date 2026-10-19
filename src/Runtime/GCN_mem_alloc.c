#include <revolution/os.h>

static void InitDefaultHeap(void) {
    void* arenaLo;
    void* arenaHi;
    OSReport("GCN_Mem_Alloc.c : InitDefaultHeap. No Heap Available\n");
    OSReport("Metrowerks CW runtime library initializing default heap\n");
    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();
    arenaLo = OSInitAlloc(arenaLo, arenaHi, 1);
    OSSetArenaLo(arenaLo);
    arenaLo = (void*)(((unsigned long)arenaLo + 31) & ~31UL);
    arenaHi = (void*)((unsigned long)arenaHi & ~31UL);
    OSSetCurrentHeap(OSCreateHeap(arenaLo, arenaHi));
    OSSetArenaLo(arenaHi);
}
void __sys_free(void* ptr) {
    if (__OSCurrHeap == -1) InitDefaultHeap();
    OSFreeToHeap(__OSCurrHeap, ptr);
}
