#pragma once

#include <JSystem/JGeometry/TVec.hpp>
#include <revolution.h>

class RailGraphIter;
class RailGraphNode;
class RailGraphEdge;

class RailGraph {
public:
    RailGraph();

    s32 addNode(const TVec3f&);
    void connectNodeTwoWay(s32, s32, const RailGraphEdge*);
    RailGraphNode* getNode(s32) const;
    RailGraphEdge* getEdge(s32) const;
    bool isValidEdge(s32) const;
    void connectEdgeToNode(s32, s32);
    RailGraphIter getIterator() const;

    RailGraphNode* mNodes;  // 0x0
    s32 mNodeCount;         // 0x4
    /* 0x08 */ u32 mUsedNodeCount;
    RailGraphEdge* mEdges;  // 0xC
    s32 mEdgeCount;         // 0x10
    /* 0x14 */ s32 mUsedEdgeCount;
};
