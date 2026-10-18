#include "JSystem/JParticle/JPAResourceManager.hpp"

JPAResource* JPAResourceManager::getResource(u16 idx) const {
    return mpResArr[idx];
}
