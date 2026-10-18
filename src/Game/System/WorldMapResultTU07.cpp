#include "Game/System/WorldMapResultLocal.hpp"

// Callees living in other WorldMapResult translation units (unnamed in the target).
extern "C" {
    bool fn_804EC410(void*, s32, s32, s32);
    bool fn_804EC4A0(void*, s32, s32);
    bool fn_804EC500(void*, s32);
    void fn_804EC250(void*);
    void fn_804EA560(void*);
    void fn_804ECD80(void*);
    void fn_804EE380(void*);
    void* fn_804EE150(void*, void*);
    void* fn_804EC230(void*, void*);
    void* fn_804EA540(void*, void*);
    void* fn_804ECD60(void*, void*);
    void* fn_804EE360(void*, void*);
    void fn_804EADC0(void*);
    void fn_804EC3A0(void*, s32);
    void fn_804EE000(void*, s32);
    void fn_804EABA0(void*);
    void fn_804ED9C0(void*);
    void fn_804EE660(void*);
    void fn_804EC2A0(void*);
    void fn_804EAC20(void*);
    void fn_804EDA30(void*);
    void fn_804EE6E0(void*);
    void fn_804EC320(void*);
    bool fn_804EC790(void*, s32, s32);
    bool fn_804EA910(void*, s32);
    void fn_804EDB80(void*);
    void fn_804EE620(void*);
    void fn_804E5780();
    void fn_804E57C0();
    s32 fn_804D72E0();
    void fn_804D7580();
    void fn_804ECBD0(WorldMapResultRoute*, s32, s32);
    void fn_804ECC30(WorldMapResultRoute*, WorldMapResultRoute*);
    f32 fn_8003DBB0(f32, f32, f32);
    void* fn_804F9330(void*, void*);
    void* fn_804FD0D0(void*, void*);
    void* fn_804FA6E0(void*, void*);
    void* fn_804FC9B0(void*, void*);
    void* fn_804FD620(void*, void*);
    void* fn_804FE0C0(void*, void*);
    void* fn_804FBC30(void*, void*);
    void* fn_804FB4D0(void*, void*);
    void* fn_804FB770(void*, void*);
    void* fn_804FA3D0(void*, void*);
    void* fn_804F4C20(void*, void*);
    void* fn_804F56C0(void*, void*);
    void* fn_804F5DC0(void*, void*);
    void* fn_804F90A0(void*, void*);
    void* fn_804F9740(void*, void*);
    void* fn_804F99E0(void*, void*);
    void fn_804F8120(void*);
    void fn_804F82C0(void*, s32, s32);
    void* fn_804E8300(void*);
    void fn_804E86E0(void*, s32);
    void fn_804E87F0(void*);
    void* fn_804E8850(void*);
    bool fn_804E8DC0();
    bool fn_804E8E30();
    u64 fn_80088630();
    bool fn_804F60E0();
    bool fn_804F60F0();
    bool fn_804F6230(void*, void*);
    bool fn_804F6A10(void*, void*);
    bool fn_804F67F0(void*);
    bool fn_804F62C0(void*);
    void fn_804F71E0(void*);
    bool fn_804F6440(void*);
    bool fn_804F6880(void*);
    bool fn_804F69A0(void*, void*);
    bool fn_804F7550(void*);
    s32 fn_804F6B70(void*);
    void fn_804FD4E0(void*, s32);
    void fn_804FD540(void*);
    bool fn_804F6510(void*, s32);
    bool fn_804F6700(void*);
    bool fn_804F6150(void*, void*, s32);
    bool fn_804F63D0(void*, s32);
    bool fn_804F6330(void*, s32);
    bool fn_804F68D0(void*, void*, s32);
    bool fn_804F6760(void*, s32);
    s32 fn_804F65B0(void*, f32);
    bool fn_804FD820(void*);
    bool fn_804FD2D0(void*);
    bool fn_804FD350(void*);

    void fn_804EC600(void*);
    void fn_804EC6E0(void*);
    void fn_804EC7D0(void*, s32, s32, f32);
    void fn_804EC840(void*, s32, s32);
    void fn_804EC870(void*, s32, s32);
    void fn_804EC8A0(void*, s32, s32);
    void fn_804EC8E0(void*);
    void fn_804ED2B0(void*, s32);
    s32 fn_804ED310(void*, s32);
    void fn_804ED450(void*, s32);
    bool fn_804ED4F0(void*, s32, s32);
    bool fn_804ED530(void*, s32);
    void fn_804ED620(void*, s32);
    bool fn_804ED6D0(void*, s32);
    bool fn_804ED7E0(void*, s32);
    void fn_804ED860(void*, s32);
    void fn_804ED8A0(void*, s32);
    void fn_804EDBC0(void*);
    void fn_804EA810(void*);
    void fn_804EA870(void*);
    const char* fn_804EA950(void*, s32);
    void* fn_804EAA50(void*);
    bool fn_804E9C40();
    void fn_804EE7F0(void*);
    void fn_804EE750(void*);
}


namespace GameSequenceFunction {
    // The names in the symbol table are wrong guesses: both really take a route (r3 = route).
    void getGalaxyCometState(const char*);
    void reserveUserName(const wchar_t*);
}

static f32 sCameraAngle = 37.86f;

namespace {
    NO_INLINE TVec3f vecAdd(const TVec3f& rA, const TVec3f& rB) {
        return rA + rB;
    }

    NO_INLINE void vecCopy(TVec3f* pDst, const TVec3f& rSrc) {
        pDst->x = rSrc.x;
        pDst->y = rSrc.y;
        pDst->z = rSrc.z;
    }

    // Stands for the out-of-line TVec3f copy constructor.
    struct VecCopy : public TVec3f {
        VecCopy(const TVec3f&) NO_INLINE;
    };

    VecCopy::VecCopy(const TVec3f& rSrc) {
        x = rSrc.x;
        y = rSrc.y;
        z = rSrc.z;
    }

    NO_INLINE void vecScale(TVec3f* pVec, f32 scale) {
        pVec->scale(scale);
    }

    NO_INLINE void vecSet(TVec3f* pDst, const TVec3f& rSrc) {
        pDst->set(rSrc);
    }

    NO_INLINE void vecCtor(TVec3f* pVec, f32 x, f32 y, f32 z) {
        pVec->x = x;
        pVec->y = y;
        pVec->z = z;
    }
}  // namespace

WorldMapResultBox::WorldMapResultBox() {
    vecCtor(&mMin, 100000.0f, 100000.0f, 100000.0f);
    mMax.x = -100000.0f;
    mMax.y = -100000.0f;
    mMax.z = -100000.0f;
}

void WorldMapResultBox::extend(const TVec3f& rPos) {
    if (rPos.x < mMin.x) {
        mMin.x = rPos.x;
    }

    if (mMax.x < rPos.x) {
        mMax.x = rPos.x;
    }

    if (rPos.y < mMin.y) {
        mMin.y = rPos.y;
    }

    if (mMax.y < rPos.y) {
        mMax.y = rPos.y;
    }

    if (rPos.z < mMin.z) {
        mMin.z = rPos.z;
    }

    if (mMax.z < rPos.z) {
        mMax.z = rPos.z;
    }
}

// NONMATCHING: out-of-line copy ctor call sets r4 before r3 in the target (instruction order only)
void WorldMapResultBox::getCenter(TVec3f* pDst) const {
    VecCopy center(vecAdd(mMax, mMin));
    vecScale(&center, 0.5f);
    vecSet(pDst, center);
    pDst->z -= 2000.0f;
}

// NONMATCHING: f2/f3 register naming of the sin/cos temps; angle needs a non-const static to avoid constant folding
void WorldMapResultBox::getCameraPos(TVec3f* pDst) const {
    f32 dist = calcCameraDistance();
    TVec3f center;
    TVec3f dir;
    f32 sinAngle = JMASinDegree(sCameraAngle);
    f32 cosAngle = JMACosDegree(sCameraAngle);
    dir.x = 0.0f;
    dir.y = sinAngle;
    dir.z = cosAngle;
    getCenter(&center);
    f32 len = 45000.0f;
    if (len >= dist) {
    } else {
        len = dist;
    }
    TVec3f scaled;
    vecCopy(&scaled, dir);
    vecScale(&scaled, len);
    TVec3f pos = vecAdd(center, scaled);
    vecSet(pDst, pos);
}

// NONMATCHING: instruction scheduling / register numbers
f32 WorldMapResultBox::calcCameraDistance() const {
    f32 fov = 0.4537856f;
    f32 tanHalf = JMASinRadian(0.5f * fov) / JMACosRadian(0.5f * fov);
    f32 dist = (0.5f * (f32)MR::getScreenHeight()) / tanHalf;
    f32 width = (dist * (mMax.x - mMin.x)) / (0.75f * 608.0f);
    f32 depth = (dist * (mMax.z - mMin.z)) / (0.75f * (f32)MR::getScreenHeight());
    if (width >= depth) {
    } else {
        width = depth;
    }

    return width;
}

static const char sMapArcFormat[] = "WorldMap%02d.arc";

// NONMATCHING: target loads mWorldId after li r4,64 (scheduling)
JMapInfo* WorldMapResultWorld::createParser(const char* pFile) const {
    char arc[0x40];
    snprintf(arc, 0x40, sMapArcFormat, mWorldId);
    return MR::createCsvParserFromFile(arc, pFile);
}

s32 WorldMapResultWorld::getNodeNum() const {
    return mMapData->mNum;
}

WorldMapResultNode* WorldMapResultWorld::getNode(s32 index) const {
    return &mMapData->mNodes[index];
}

bool WorldMapResultWorld::isNodeValid(s32 index) const {
    return mMapData->isNodeValid(index);
}

bool WorldMapResultMapData::isNodeValid(s32 index) const {
    return mNodes[index].mIsValid != 0;
}

bool WorldMapResultWorld::isNodeHidden(s32 index) const {
    return mMapData->isNodeHidden(index);
}

bool WorldMapResultMapData::isNodeHidden(s32 index) const {
    return mNodes[index].mIsHidden != 0;
}

bool WorldMapResultWorld::isNodeFlag11(s32 index) const {
    return mMapData->mNodes[index].mFlag11 != 0;
}

bool WorldMapResultWorld::isLinked(s32 a, s32 b) const {
    if (fn_804EC410(mLinks, a, b, 1)) {
        return true;
    }

    return fn_804ED4F0(mUnit14, a, b);
}

bool WorldMapResultWorld::isLinked14(s32 a, s32 b) const {
    return fn_804ED4F0(mUnit14, a, b);
}

bool WorldMapResultWorld::isLinkedDirect(s32 a, s32 b) const {
    return fn_804EC410(mLinks, a, b, 1);
}

bool WorldMapResultWorld::isPathReachable(s32 index) const {
    return mPathFinder->isReachable(index);
}

void WorldMapResultWorld::resetLinks() {
    fn_804EC600(mLinks);
}

void WorldMapResultWorld::resetLinks2() {
    fn_804EC6E0(mLinks);
}

bool WorldMapResultWorld::isNodeOpened(s32 index) const {
    const char* pName = fn_804EA950(mWorldData, index);

    if (pName != nullptr) {
        return GameDataFunction::getPowerStarNumOwned(pName) > 0;
    }

    if (fn_804ED6D0(mUnit14, index)) {
        return false;
    }

    return !fn_804ED7E0(mUnit14, index);
}

void WorldMapResultWorld::updatePaths(s32 start) {
    mPathFinder->update(start);
}

s32 WorldMapResultWorld::getPathDistance(s32 index) const {
    return mPathFinder->getDistance(index);
}

s32 WorldMapResultWorld::getPathNode(s32 index, s32 step) const {
    return mPathFinder->getPath(index, step);
}

bool WorldMapResultWorld::setupNode(s32 index, s32 start) {
    s32 target = start;

    if (start < 0) {
        target = index;
    }

    if (fn_804EC500(mLinks, index)) {
        refreshNode(target);
        return true;
    }

    fn_804EE7F0(mUnit18);
    return false;
}

void WorldMapResultWorld::refreshNode(s32 start) {
    mPathFinder->update(start);
    fn_804EE7F0(mUnit18);
    fn_804EA810(mWorldData);
}

void WorldMapResultWorld::refreshNodeFull(s32 start) {
    mPathFinder->update(start);
    fn_804EE750(mUnit18);
    fn_804EE7F0(mUnit18);
    fn_804EA870(mWorldData);
}

bool WorldMapResultWorld::isNodeNotReady(s32 index) const {
    if (hasWorldData(index)) {
        return hasWorldDataAndComet(index);
    }

    return !fn_804ED530(mUnit14, index);
}

bool WorldMapResultWorld::hasWorldData(s32 index) const {
    return fn_804EA950(mWorldData, index) != nullptr;
}

bool WorldMapResultWorld::hasWorldDataAndComet(s32) const {
    if (fn_804EAA50(mWorldData) != nullptr && fn_804E9C40()) {
        return true;
    }

    return false;
}

bool WorldMapResultWorld::isWorldDataType(s32, s32 type) const {
    void* pData = fn_804EAA50(mWorldData);
    if (pData != nullptr && type == *(s32*)((u8*)pData + 0xA4)) {
        return true;
    }

    return false;
}

bool WorldMapResultWorld::isWorldDataType3(s32 index) const {
    return isWorldDataType(index, 3);
}

bool WorldMapResultWorld::isWorldDataType4(s32 index) const {
    return isWorldDataType(index, 4);
}

bool WorldMapResultWorld::isWorldDataType5(s32 index) const {
    return isWorldDataType(index, 5);
}

bool WorldMapResultWorld::callU14_6D0(s32 index) const {
    return fn_804ED6D0(mUnit14, index);
}

void WorldMapResultWorld::callU14_450(s32 index) {
    fn_804ED450(mUnit14, index);
}

void WorldMapResultWorld::callU14_860(s32 index) {
    fn_804ED860(mUnit14, index);
}

void WorldMapResultWorld::callU14_8A0(s32 index) {
    fn_804ED8A0(mUnit14, index);
}

void WorldMapResultWorld::callU14_620(s32 index) {
    fn_804ED620(mUnit14, index);
}

void WorldMapResultWorld::callU14_2B0(s32 index) {
    fn_804ED2B0(mUnit14, index);
}

s32 WorldMapResultWorld::callU14_310(s32 index) {
    return fn_804ED310(mUnit14, index);
}

void WorldMapResultWorld::setLinkProgress(s32 a, s32 b, f32 t) {
    fn_804EC7D0(mLinks, a, b, t);

    if (t >= 1.0f) {
        fn_804EC870(mLinks, a, b);
    } else if (t > 0.0f) {
        fn_804EC840(mLinks, a, b);
    }
}

void WorldMapResultWorld::setLinkProgressFull(s32 a, s32 b) {
    fn_804EC8E0(mLinks);
    fn_804EC8A0(mLinks, a, b);
    fn_804EC7D0(mLinks, a, b, 0.0f);
}

s32 WorldMapResultWorld::getWorldId() const {
    return mWorldId;
}

void WorldMapResultWorld::disappear() {
    fn_804EDBC0(mUnit14);
}

// NONMATCHING: scheduling of the first loads (r5 vs r3)
void WorldMapResultRoute::collect(WorldMapResultWorld* pWorld) {
    mCount = 0;
    WorldMapResultMapData* pMap = pWorld->mMapData;
    void* pLinks = pWorld->mLinks;
    s32 num = pMap->mNum;

    for (s32 i = 0; i < num; i++) {
        for (s32 j = i + 1; j < num; j++) {
            if (fn_804EC410(pLinks, i, j, 1)) {
                if (fn_804EC4A0(pLinks, i, j)) {
                    fn_804ECBD0(this, j, i);
                } else {
                    fn_804ECBD0(this, i, j);
                }
            }
        }
    }
}

// NONMATCHING: callee-saved register assignment and loop-top layout (step check hoisted before the body in the target)
void WorldMapResultWorld::setupRouteSteps(WorldMapResultRoute* pRoute, s32 node) {
    s32 step = 0;

    for (s32 i = 0; i < pRoute->mCount; i++) {
        WorldMapResultEdge* pEdge = &pRoute->mEdges[i];
        s32 from = pEdge->mFrom;
        s32 to = pEdge->mTo;

        if (from == node || to == node) {
            fn_804EC7D0(mLinks, from, to, 0.0f);
            pEdge->mStep = 0;

            if (from == node) {
                pEdge->mReverse = 0;
            } else {
                pEdge->mReverse = 1;
            }
        }
    }

    bool found;

    for (;;) {
        if (step > 100) {
            break;
        }

        found = false;
        step++;

        for (s32 i = 0; i < pRoute->mCount; i++) {
            WorldMapResultEdge* pEdge = &pRoute->mEdges[i];
            s32 from = pEdge->mFrom;
            s32 to = pEdge->mTo;

            if (pEdge->mStep == -1) {
                for (s32 j = 0; j < pRoute->mCount; j++) {
                    WorldMapResultEdge* pOther = &pRoute->mEdges[j];

                    if (i != j && step - 1 == pOther->mStep) {
                        s32 otherFrom = pOther->mFrom;
                        s32 otherTo = pOther->mTo;
                        bool adjacent = false;

                        if (from == otherFrom || from == otherTo || to == otherFrom || to == otherTo) {
                            adjacent = true;
                        }

                        adjacent = adjacent | isLinked14(from, otherFrom);
                        adjacent = adjacent | isLinked14(from, otherTo);
                        adjacent = adjacent | isLinked14(to, otherFrom);

                        if (adjacent | isLinked14(to, otherTo)) {
                            fn_804EC7D0(mLinks, from, to, 0.0f);
                            pEdge->mStep = step;
                            found = true;

                            if (from == otherFrom || from == otherTo) {
                                pEdge->mReverse = 0;
                            } else {
                                pEdge->mReverse = 1;
                            }

                            break;
                        }
                    }
                }
            }
        }

        if (!found) {
            break;
        }
    }
}

void WorldMapResultWorld::refreshRouteComet() {
    WorldMapResultRoute route;
    route.collect(this);
    GameSequenceFunction::getGalaxyCometState(reinterpret_cast< const char* >(&route));
}

void WorldMapResultWorld::fn2ED0() {
    fn_804EE620(mUnit18);
    fn_804E5780();
}

void WorldMapResultWorld::fn2F00() {
    fn_804EE620(mUnit18);
    fn_804E57C0();
}

void WorldMapResultWorld::setupRoute(WorldMapResultRoute* pRoute) {
    s32 node = fn_804D72E0();
    fn_804EC500(mLinks, 0);
    WorldMapResultRoute route;
    GameSequenceFunction::reserveUserName(reinterpret_cast< const wchar_t* >(&route));
    pRoute->collect(this);
    fn_804ECC30(pRoute, &route);
    fn_804D7580();
    mPathFinder->update(node);
    setupRouteSteps(pRoute, node);
    refreshNode(node);
    fn_804EDB80(mUnit14);
}

// NONMATCHING: scheduling of li/stw before the first loads
void WorldMapResultWorld::setupRouteFromNode(WorldMapResultRoute* pRoute, s32 node) {
    fn_804EC500(mLinks, 0);
    WorldMapResultRoute route;
    route.mCount = 0;
    void* pLinks = mLinks;
    s32 num = mMapData->mNum;

    for (s32 i = 0; i < num; i++) {
        for (s32 j = i + 1; j < num; j++) {
            if (fn_804EC410(pLinks, i, j, 1) && !fn_804EC790(pLinks, i, j)) {
                if (fn_804EC4A0(pLinks, i, j)) {
                    fn_804ECBD0(&route, j, i);
                } else {
                    fn_804ECBD0(&route, i, j);
                }
            }
        }
    }

    pRoute->collect(this);
    fn_804ECC30(pRoute, &route);
    fn_804D7580();
    mPathFinder->update(node);
    setupRouteSteps(pRoute, node);
    refreshNode(node);
    fn_804EDB80(mUnit14);
}

// NONMATCHING: callee-saved register assignment (loop counter r28 vs r30)
void WorldMapResultBox::extendByRoute(WorldMapResultWorld* pWorld, WorldMapResultRoute* pRoute) {
    s32 i = 0;
    WorldMapResultMapData* pMap = pWorld->mMapData;

    for (; i < pRoute->mCount; i++) {
        s32 to = pRoute->mEdges[i].mTo;
        extend(pMap->mNodes[pRoute->mEdges[i].mFrom].mPos);
        extend(pMap->mNodes[to].mPos);
    }
}

bool WorldMapResultWorld::updateRouteStep(WorldMapResultRoute* pRoute, s32 step, f32 t) {
    s32 maxStep = 0;

    for (s32 i = 0; i < pRoute->mCount; i++) {
        WorldMapResultEdge* pEdge = &pRoute->mEdges[i];
        s32 from = pEdge->mFrom;
        s32 edgeStep = pEdge->mStep;
        s32 to = pEdge->mTo;

        if (maxStep < edgeStep) {
            maxStep = edgeStep;
        }

        if (edgeStep == step) {
            if (pEdge->mReverse != 0) {
                setLinkProgress(to, from, t);
            } else {
                setLinkProgress(from, to, t);
            }
        }
    }

    if (t >= 1.0f) {
        refreshNodeFull(fn_804D72E0());
    }

    bool done = false;

    if (t >= 1.0f && step >= maxStep) {
        done = true;
    }

    return done;
}

// NONMATCHING: callee-saved register assignment
bool WorldMapResultWorld::isRouteNotHeld(WorldMapResultRoute* pRoute) const {
    void* pData = mWorldData;

    for (s32 i = 0; i < pRoute->mCount; i++) {
        s32 to = pRoute->mEdges[i].mTo;

        if (fn_804EA910(pData, pRoute->mEdges[i].mFrom) || fn_804EA910(pData, to)) {
            return false;
        }
    }

    return true;
}

void WorldMapResultBox::extendByWorld(WorldMapResultWorld* pWorld) {
    WorldMapResultMapData* pMap = pWorld->mMapData;

    for (s32 i = 0; i < pWorld->mMapData->mNum; i++) {
        WorldMapResultNode* pNode = &pMap->mNodes[i];

        if (pNode->mIsValid != 0 && pNode->mIsHidden == 0) {
            extend(pNode->mPos);
        }
    }
}

// NONMATCHING: stack slot order of the three temporaries and one fmuls operand order
void calcRoutePos(TVec3f* pDst, const TVec3f& rFrom, const TVec3f& rTo, s32 step, s32 maxStep) {
    s32 splitStep = (s32)(0.8f * (f32)maxStep);
    f32 rate = MR::getLinerValue(fn_8003DBB0((f32)step, 0.0f, (f32)splitStep), 0.0f, 1.0f - 0.12f, 1.0f);
    rate += MR::getEaseOutValue(fn_8003DBB0((f32)step, (f32)splitStep, (f32)maxStep), 0.0f, 0.12f, 1.0f);
    VecCopy to(rTo);
    vecScale(&to, rate);
    VecCopy from(rFrom);
    vecScale(&from, 1.0f - rate);
    TVec3f pos = vecAdd(from, to);
    vecSet(pDst, pos);
}

void WorldMapResultWorld::setNum(s32 num) {
    fn_804EADC0(mWorldData);
    fn_804EC3A0(mLinks, num);
    fn_804EE000(mUnit14, num);
}

WorldMapResultPathFinder::Entry::Entry() : mDistance(0), mPath(nullptr) {
}

WorldMapResultPathFinder::WorldMapResultPathFinder(WorldMapResultWorld* pWorld) {
    mWorld = pWorld;
    mEntries = nullptr;
    allocate();
    update(0);
}

// NONMATCHING: callee-saved register assignment (target keeps another constant 1 in a register)
void WorldMapResultPathFinder::update(s32 start) {
    s32 num = mWorld->getNodeNum();

    for (s32 i = 0; i < num; i++) {
        Entry* pEntry = &mEntries[i];

        for (s32 j = 0; j < num; j++) {
            pEntry->mDistance = -1;
            pEntry->mPath[j] = -1;
        }
    }

    u8 visited[0x100];

    for (s32 i = 0; i < num; i++) {
        visited[i] = 0;
    }

    visited[start] = 1;
    mEntries[start].mDistance = 0;

    for (s32 i = 0; i < num; i++) {
        Entry* pEntry = &mEntries[i];

        if (mWorld->isLinked(start, i)) {
            pEntry->mDistance = 1;
            visited[i] = 1;
            pEntry->mPath[0] = i;
        }
    }

    s32 depth = 1;

    do {
        bool found = false;

        for (s32 i = 0; i < num; i++) {
            if (mWorld->isNodeValid(i) && visited[i] && i == mEntries[i].mPath[depth - 1]) {
                for (s32 j = 0; j < num; j++) {
                    Entry* pOther = &mEntries[j];

                    if (mWorld->isNodeValid(j) && !visited[j] && mWorld->isLinked(i, j)) {
                        pOther->mDistance = depth + 1;
                        found = true;
                        visited[j] = 1;
                        pOther->mPath[depth] = j;

                        for (s32 k = depth - 1; k >= 0; k--) {
                            pOther->mPath[k] = mEntries[i].mPath[k];
                        }
                    }
                }
            }
        }

        if (!found) {
            break;
        }

        depth++;
    } while (depth < 32);
}

bool WorldMapResultPathFinder::isReachable(s32 index) const {
    return mEntries[index].mDistance >= 0;
}

s32 WorldMapResultPathFinder::getDistance(s32 index) const {
    return mEntries[index].mDistance;
}

s32 WorldMapResultPathFinder::getPath(s32 index, s32 step) const {
    return mEntries[index].mPath[step];
}

void WorldMapResultPathFinder::allocate() {
    s32 num = mWorld->getNodeNum();
    mEntries = new Entry[num];

    for (s32 i = 0; i < num; i++) {
        Entry* pEntry = &mEntries[i];
        pEntry->mPath = new s32[num];

        for (s32 j = 0; j < num; j++) {
            pEntry->mDistance = 1;
            pEntry->mPath[j] = -1;
        }
    }
}

WorldMapResultWorld::WorldMapResultWorld(s32 worldId) {
    mWorldId = worldId;
    mMapData = nullptr;
    mLinks = nullptr;
    mWorldData = nullptr;
    _10 = 0;
    mUnit14 = nullptr;
    mUnit18 = nullptr;
    mPathFinder = nullptr;
}

void WorldMapResultWorld::initWorld(s32 flag) {
    void* p = ::operator new(0xC);

    if (p != nullptr) {
        p = fn_804EE150(p, this);
    }

    mMapData = static_cast< WorldMapResultMapData* >(p);
    p = ::operator new(0xC);

    if (p != nullptr) {
        p = fn_804EC230(p, this);
    }

    mLinks = p;
    fn_804EC250(p);
    p = ::operator new(0xC);

    if (p != nullptr) {
        p = fn_804EA540(p, this);
    }

    mWorldData = p;
    fn_804EA560(p);
    p = ::operator new(0x10);

    if (p != nullptr) {
        p = fn_804ECD60(p, this);
    }

    mUnit14 = p;
    fn_804ECD80(p);
    p = ::operator new(0xC);

    if (p != nullptr) {
        p = fn_804EE360(p, this);
    }

    mUnit18 = p;
    fn_804EE380(p);
    mPathFinder = new WorldMapResultPathFinder(this);

    if (flag != 0) {
        setupNode(0, -1);
    }

    finish();
}

void WorldMapResultWorld::prepare() {
    fn_804EABA0(mWorldData);
    fn_804ED9C0(mUnit14);
    fn_804EE660(mUnit18);
    fn_804EC2A0(mLinks);
}

void WorldMapResultWorld::finish() {
    fn_804EAC20(mWorldData);
    fn_804EDA30(mUnit14);
    fn_804EE6E0(mUnit18);
    fn_804EC320(mLinks);
}

// ---------------------------------------------------------------------------------------------------------------
// WorldMapSequence: "シーケンス管理", the state machine of the world map
// ---------------------------------------------------------------------------------------------------------------

class WorldMapSequence;

#define WMS_NERVE_DECL(name)                                                                                                                         \
    class name : public Nerve {                                                                                                                      \
    public:                                                                                                                                          \
        virtual void execute(Spine*) const;                                                                                                          \
        static name sInstance;                                                                                                                       \
    }

namespace NrvWorldMapSequence {
    WMS_NERVE_DECL(WorldMapSequenceStart);
    WMS_NERVE_DECL(WorldMapSequenceWait);
    WMS_NERVE_DECL(WorldMapSequencePointMove);
    WMS_NERVE_DECL(WorldMapSequencePointWarp);
    WMS_NERVE_DECL(WorldMapSequencePointEvent);
    WMS_NERVE_DECL(WorldMapSequenceTicoRouteCreator);
    WMS_NERVE_DECL(WorldMapSequenceWorldWarp);
    WMS_NERVE_DECL(WorldMapSequenceOpenGalaxy);
    WMS_NERVE_DECL(WorldMapSequenceEnterGalaxy);
    WMS_NERVE_DECL(WorldMapSequenceEnterHome);
    WMS_NERVE_DECL(WorldMapSequenceEnterKoopaCastle);
    WMS_NERVE_DECL(WorldMapSequenceWorldNews);
    WMS_NERVE_DECL(WorldMapSequenceGalaxyMapEnter);
    WMS_NERVE_DECL(WorldMapSequenceGalaxyMap);
    WMS_NERVE_DECL(WorldMapSequenceGalaxyMapExit);
    WMS_NERVE_DECL(WorldMapSequenceGreenStarDemo);
    WMS_NERVE_DECL(WorldMapSequenceNoMove);
};  // namespace NrvWorldMapSequence

/// Stands for the owner's actor: only its virtual slot at vtable offset 0x30 is used.
class SlotActor {
public:
    virtual ~SlotActor();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual void slot9();
    virtual void slot10();
};

class WorldMapSequence : public NerveExecutor {
public:
    WorldMapSequence(void*);

    virtual ~WorldMapSequence();

    void init(bool);
    void exeStart();
    void exeWait();
    void exePointMove();
    void exePointWarp();
    void exeGreenStarDemo();
    void exeNoMove();
    bool isGalaxyMap() const NO_INLINE;
    bool updatePoint(bool) NO_INLINE;
    bool checkWorldNews() NO_INLINE;

    /* 0x08 */ void* mOwner;
    /* 0x0C */ void* mStateWait;
    /* 0x10 */ void* mStatePointEvent;
    /* 0x14 */ void* mStateTicoRoute;
    /* 0x18 */ void* mStateWorldNews;
    /* 0x1C */ void* mStateWorldWarp;
    /* 0x20 */ void* mStateGalaxyMap;
    /* 0x24 */ bool mFlag;
};

WorldMapSequence::WorldMapSequence(void* pOwner) : NerveExecutor("シーケンス管理") {
    mOwner = pOwner;
    mStateWait = nullptr;
    mStatePointEvent = nullptr;
    mStateTicoRoute = nullptr;
    mStateWorldNews = nullptr;
    mFlag = false;
}

void WorldMapSequence::init(bool startAtMap) {
    fn_804F8120(mOwner);
    initNerve(&NrvWorldMapSequence::WorldMapSequenceStart::sInstance, 0x14);
    void* p = ::operator new(0x20);

    if (p != nullptr) {
        p = fn_804FD0D0(p, mOwner);
    }

    mStateWait = p;
    p = ::operator new(0x3C);

    if (p != nullptr) {
        p = fn_804FA6E0(p, mOwner);
    }

    mStatePointEvent = p;
    p = ::operator new(0x24);

    if (p != nullptr) {
        p = fn_804FC9B0(p, mOwner);
    }

    mStateTicoRoute = p;
    p = ::operator new(0x30);

    if (p != nullptr) {
        p = fn_804FD620(p, mOwner);
    }

    mStateWorldNews = p;
    p = ::operator new(0x40);

    if (p != nullptr) {
        p = fn_804FE0C0(p, mOwner);
    }

    mStateWorldWarp = p;
    p = ::operator new(0x18);

    if (p != nullptr) {
        p = fn_804F9330(p, mOwner);
    }

    mStateGalaxyMap = p;
    p = ::operator new(0x2C);

    if (p != nullptr) {
        p = fn_804FBC30(p, mOwner);
    }

    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(p), &NrvWorldMapSequence::WorldMapSequenceStart::sInstance, "Start");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(mStateWait), &NrvWorldMapSequence::WorldMapSequenceWait::sInstance, "Wait");
    p = ::operator new(0x18);

    if (p != nullptr) {
        p = fn_804FB4D0(p, mOwner);
    }

    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(p), &NrvWorldMapSequence::WorldMapSequencePointMove::sInstance, "PointMove");
    p = ::operator new(0x10);

    if (p != nullptr) {
        p = fn_804FB770(p, mOwner);
    }

    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(p), &NrvWorldMapSequence::WorldMapSequencePointWarp::sInstance, "PointWarp");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(mStatePointEvent), &NrvWorldMapSequence::WorldMapSequencePointEvent::sInstance, "PointEvent");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(mStateTicoRoute), &NrvWorldMapSequence::WorldMapSequenceTicoRouteCreator::sInstance, "TicoRouteCreator");
    p = ::operator new(0x1C);

    if (p != nullptr) {
        p = fn_804FA3D0(p, mOwner);
    }

    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(p), &NrvWorldMapSequence::WorldMapSequenceOpenGalaxy::sInstance, "OpenGalaxy");
    p = ::operator new(0x40);

    if (p != nullptr) {
        p = fn_804F4C20(p, mOwner);
    }

    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(p), &NrvWorldMapSequence::WorldMapSequenceEnterGalaxy::sInstance, "EnterGalaxy");
    p = ::operator new(0x18);

    if (p != nullptr) {
        p = fn_804F56C0(p, mOwner);
    }

    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(p), &NrvWorldMapSequence::WorldMapSequenceEnterHome::sInstance, "EnterHome");
    p = ::operator new(0x10);

    if (p != nullptr) {
        p = fn_804F5DC0(p, mOwner);
    }

    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(p), &NrvWorldMapSequence::WorldMapSequenceEnterKoopaCastle::sInstance, "EnterKoopaCastle");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(mStateWorldNews), &NrvWorldMapSequence::WorldMapSequenceWorldNews::sInstance, "WorldNews");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(mStateWorldWarp), &NrvWorldMapSequence::WorldMapSequenceWorldWarp::sInstance, "WorldWarp");
    p = ::operator new(0x14);

    if (p != nullptr) {
        p = fn_804F90A0(p, mOwner);
    }

    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(p), &NrvWorldMapSequence::WorldMapSequenceGalaxyMapEnter::sInstance, "GalaxyMapEnter");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(mStateGalaxyMap), &NrvWorldMapSequence::WorldMapSequenceGalaxyMap::sInstance, "GalaxyMap");
    p = ::operator new(0x14);

    if (p != nullptr) {
        p = fn_804F9740(p, mOwner);
    }

    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(p), &NrvWorldMapSequence::WorldMapSequenceGalaxyMapExit::sInstance, "GalaxyMapExit");
    p = ::operator new(0x20);

    if (p != nullptr) {
        p = fn_804F99E0(p, mOwner);
    }

    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(p), &NrvWorldMapSequence::WorldMapSequenceGreenStarDemo::sInstance, "GreenStarDemo");
    u8* pBase = static_cast< u8* >(fn_804E8300(mOwner));
    fn_804E86E0(mOwner, *reinterpret_cast< s32* >(pBase + 0x3C));
    reinterpret_cast< SlotActor* >(*reinterpret_cast< void** >(pBase + 0x50))->slot10();
    MR::startStarPointerModeSphereSelectorOnReaction(this);
    fn_804F82C0(mOwner, -1, 0);

    if (startAtMap) {
        setNerve(&NrvWorldMapSequence::WorldMapSequenceGalaxyMap::sInstance);
    }

    if (fn_804E8DC0()) {
        setNerve(&NrvWorldMapSequence::WorldMapSequenceGreenStarDemo::sInstance);
    }
}

void WorldMapSequence::exeStart() {
    if (MR::updateActorState(this)) {
        if (fn_804E8E30()) {
            fn_804E87F0(mOwner);
            setNerve(&NrvWorldMapSequence::WorldMapSequenceNoMove::sInstance);
            return;
        }

        if (!checkWorldNews()) {
            setNerve(&NrvWorldMapSequence::WorldMapSequenceWait::sInstance);
        }
    }
}

void WorldMapSequence::exeWait() {
    if (MR::isFirstStep(this)) {
        mFlag = fn_804F60F0();
    }

    MR::updateActorState(this);

    if (!updatePoint(fn_804FD350(mStateWait)) && fn_804FD2D0(mStateWait)) {
        setNerve(&NrvWorldMapSequence::WorldMapSequenceGalaxyMapEnter::sInstance);
    }
}

void WorldMapSequence::exePointMove() {
    if (MR::updateActorState(this)) {
        if (fn_804F6230(mOwner, mStatePointEvent)) {
            setNerve(&NrvWorldMapSequence::WorldMapSequencePointEvent::sInstance);
            return;
        }

        if (fn_804F6A10(mOwner, mStateWorldWarp)) {
            setNerve(&NrvWorldMapSequence::WorldMapSequenceWorldWarp::sInstance);
            return;
        }

        if (fn_804F67F0(mOwner)) {
            setNerve(&NrvWorldMapSequence::WorldMapSequencePointWarp::sInstance);
            return;
        }

        if (fn_804F62C0(mOwner)) {
            fn_804F71E0(mOwner);
            setNerve(&NrvWorldMapSequence::WorldMapSequenceWait::sInstance);
            return;
        }

        if (fn_804F6440(mOwner)) {
            setNerve(&NrvWorldMapSequence::WorldMapSequenceEnterKoopaCastle::sInstance);
            return;
        }

        setNerve(&NrvWorldMapSequence::WorldMapSequenceWait::sInstance);
    }
}

void WorldMapSequence::exePointWarp() {
    if (MR::updateActorState(this)) {
        if (fn_804F6230(mOwner, mStatePointEvent)) {
            setNerve(&NrvWorldMapSequence::WorldMapSequencePointEvent::sInstance);
            return;
        }

        if (fn_804F6880(mOwner)) {
            setNerve(&NrvWorldMapSequence::WorldMapSequencePointMove::sInstance);
            return;
        }

        setNerve(&NrvWorldMapSequence::WorldMapSequenceWait::sInstance);
    }
}

void WorldMapSequence::exeGreenStarDemo() {
    if (MR::isFirstStep(this)) {
        fn_80088630();
    }

    if (MR::updateActorState(this)) {
        setNerve(&NrvWorldMapSequence::WorldMapSequenceWait::sInstance);
    }
}

void WorldMapSequence::exeNoMove() {
    if (MR::isGreaterEqualStep(this, 0x3C)) {
        fn_804E8850(mOwner);
        MR::deactivateDefaultGameLayout();
        setNerve(&NrvWorldMapSequence::WorldMapSequenceGalaxyMapEnter::sInstance);
    }
}

bool WorldMapSequence::isGalaxyMap() const {
    return isNerve(&NrvWorldMapSequence::WorldMapSequenceGalaxyMap::sInstance);
}

bool WorldMapSequence::updatePoint(bool canAct) {
    u8* pBase = static_cast< u8* >(fn_804E8300(mOwner));
    LiveActor* pActor = *reinterpret_cast< LiveActor** >(pBase + 0x50);

    if (fn_804F69A0(mOwner, mStateWorldWarp)) {
        setNerve(&NrvWorldMapSequence::WorldMapSequenceWorldWarp::sInstance);
        return true;
    }

    s32 point = -1;

    if (!fn_804F7550(mOwner)) {
        point = fn_804F6B70(mOwner);
    }

    if (point > -1) {
        fn_804FD4E0(mStateWait, point);
    } else {
        fn_804FD540(mStateWait);
    }

    bool moved = fn_804F6510(mOwner, point);
    bool warped = fn_804F6700(mOwner);

    if (moved) {
        MR::startActionSound(pActor, "PushAButton", -1, -1, -1);
    }

    if (moved || warped) {
        setNerve(&NrvWorldMapSequence::WorldMapSequencePointMove::sInstance);
        return true;
    }

    if (fn_804F7550(mOwner)) {
        return false;
    }

    if (canAct) {
        if (fn_804F6150(mOwner, mStatePointEvent, point)) {
            MR::startActionSound(pActor, "PushAButton", -1, -1, -1);
            setNerve(&NrvWorldMapSequence::WorldMapSequencePointEvent::sInstance);
            return true;
        }

        if (fn_804F63D0(mOwner, point)) {
            MR::startActionSound(pActor, "PushAButton", -1, -1, -1);
            setNerve(&NrvWorldMapSequence::WorldMapSequenceEnterGalaxy::sInstance);
            return true;
        }

        if (fn_804F6330(mOwner, point)) {
            MR::startActionSound(pActor, "PushAButton", -1, -1, -1);
            setNerve(&NrvWorldMapSequence::WorldMapSequenceEnterHome::sInstance);
            return true;
        }

        if (fn_804F68D0(mOwner, mStateTicoRoute, point)) {
            MR::startActionSound(pActor, "PushAButton", -1, -1, -1);
            setNerve(&NrvWorldMapSequence::WorldMapSequenceTicoRouteCreator::sInstance);
            return true;
        }
    }

    if (fn_804F6760(mOwner, point)) {
        setNerve(&NrvWorldMapSequence::WorldMapSequencePointWarp::sInstance);
        MR::startCSSound("CS_CLICK_WMAP", nullptr, 0, 0, 1.0f);
        return true;
    }

    if (!moved && point > 0 && fn_804F60E0()) {
        MR::startActionSound(pActor, "NoMove", -1, -1, -1);
    }

    bool blocked = fn_804F65B0(mOwner, 0.1f) >= 0;

    if (!warped && !mFlag && fn_804F60F0() && !blocked) {
        MR::startActionSound(pActor, "NoMove", -1, -1, -1);
    }

    mFlag = fn_804F60F0();
    return false;
}

bool WorldMapSequence::checkWorldNews() {
    if (fn_804FD820(mStateWorldNews)) {
        setNerve(&NrvWorldMapSequence::WorldMapSequenceWorldNews::sInstance);
        return true;
    }

    return false;
}

WorldMapSequence::~WorldMapSequence() {
}

namespace NrvWorldMapSequence {
    WorldMapSequenceStart WorldMapSequenceStart::sInstance;
    WorldMapSequenceWait WorldMapSequenceWait::sInstance;
    WorldMapSequencePointMove WorldMapSequencePointMove::sInstance;
    WorldMapSequencePointWarp WorldMapSequencePointWarp::sInstance;
    WorldMapSequencePointEvent WorldMapSequencePointEvent::sInstance;
    WorldMapSequenceTicoRouteCreator WorldMapSequenceTicoRouteCreator::sInstance;
    WorldMapSequenceWorldWarp WorldMapSequenceWorldWarp::sInstance;
    WorldMapSequenceOpenGalaxy WorldMapSequenceOpenGalaxy::sInstance;
    WorldMapSequenceEnterGalaxy WorldMapSequenceEnterGalaxy::sInstance;
    WorldMapSequenceEnterHome WorldMapSequenceEnterHome::sInstance;
    WorldMapSequenceEnterKoopaCastle WorldMapSequenceEnterKoopaCastle::sInstance;
    WorldMapSequenceWorldNews WorldMapSequenceWorldNews::sInstance;
    WorldMapSequenceGalaxyMapEnter WorldMapSequenceGalaxyMapEnter::sInstance;
    WorldMapSequenceGalaxyMap WorldMapSequenceGalaxyMap::sInstance;
    WorldMapSequenceGalaxyMapExit WorldMapSequenceGalaxyMapExit::sInstance;
    WorldMapSequenceGreenStarDemo WorldMapSequenceGreenStarDemo::sInstance;
    WorldMapSequenceNoMove WorldMapSequenceNoMove::sInstance;

    void WorldMapSequenceNoMove::execute(Spine* pSpine) const {
        reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor)->exeNoMove();
    }

    void WorldMapSequenceGreenStarDemo::execute(Spine* pSpine) const {
        reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor)->exeGreenStarDemo();
    }

    void WorldMapSequenceGalaxyMapExit::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            if (!pSeq->checkWorldNews()) {
                pSeq->setNerve(&WorldMapSequenceWait::sInstance);
            }
        }
    }

    void WorldMapSequenceGalaxyMap::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceGalaxyMapExit::sInstance);
        }
    }

    void WorldMapSequenceGalaxyMapEnter::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceGalaxyMap::sInstance);
        }
    }

    void WorldMapSequenceWorldNews::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequenceEnterKoopaCastle::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequenceEnterHome::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequenceEnterGalaxy::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequenceOpenGalaxy::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequenceWorldWarp::execute(Spine* pSpine) const {
        MR::updateActorState(reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor));
    }

    void WorldMapSequenceTicoRouteCreator::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequencePointEvent::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequencePointWarp::execute(Spine* pSpine) const {
        reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor)->exePointWarp();
    }

    void WorldMapSequencePointMove::execute(Spine* pSpine) const {
        reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor)->exePointMove();
    }

    void WorldMapSequenceWait::execute(Spine* pSpine) const {
        reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor)->exeWait();
    }

    void WorldMapSequenceStart::execute(Spine* pSpine) const {
        reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor)->exeStart();
    }
};  // namespace NrvWorldMapSequence
