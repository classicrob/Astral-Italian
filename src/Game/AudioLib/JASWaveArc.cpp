#include "JSystem/JAudio2/JASWaveArcLoader.hpp"
#include "JSystem/JAudio2/JASDvdThread.hpp"
#include "JSystem/JAudio2/JASMutex.hpp"
#include "JSystem/JAudio2/JASTaskThread.hpp"
#include "JSystem/JKernel/JKRDvdAramRipper.hpp"
#include <cstring>
#include <stdint.h>













bool JASWaveArc::load(JASHeap* heap) {
    if (mEntryNum < 0) {
        return false;
    }
    JASMutexLock mutexLock(&mMutex);
    if (mStatus != 0) {
        return false;
    }
    if (heap == nullptr) {
        heap = JASWaveArcLoader::getRootHeap();
    }
    if (mHeap.alloc(heap, mFileLength) == false) {
        return false;
    }
    return sendLoadCmd();
}









void JASWaveArc::setFileName(char const* fileName) {
    char* currentDir = JASWaveArcLoader::getCurrentDir();
    size_t length = strlen(currentDir);
    length = length + strlen(fileName);
    char* path = new (JASKernel::getSystemHeap(), -4) char[length + 1];
    strcpy(path, currentDir);
    strcat(path, fileName);
    path[length] = '\0';
    int entryNum = DVDConvertPathToEntrynum(path);
    delete[] path;
    if (entryNum < 0) {
        return;
    }
    setEntryNum(entryNum);
}
