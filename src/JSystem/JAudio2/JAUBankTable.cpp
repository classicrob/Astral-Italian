#include "JSystem/JAudio2/JAUBankTable.hpp"

void JAUBankTableDictionary::appendBankTable(JSULink< JAUBankTable >* bankTableList) {
    append(bankTableList);
}


// NONMATCHING data: the canonical key function owns this vtable here; retail linked its weak copy with JAUSectionHeap.
JASBank* JAUBankTable::getBank(u32 bank) const {
    return mBankPtrTable.get(bank);
}
