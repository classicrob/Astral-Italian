#include "Game/Map/RailGraph.hpp"
#include "Game/Map/RailGraphEdge.hpp"
#include "Game/Map/RailGraphIter.hpp"
#include "Game/Map/RailGraphNode.hpp"

RailGraph::RailGraph() {
    mNodes = 0;
    mNodeCount = 0;
    mUsedNodeCount = 0;
    mEdges = 0;
    mEdgeCount = 0;
    mUsedEdgeCount = 0;

    mNodes = new RailGraphNode[0x100];
    mNodeCount = 0x100;
    mEdges = new RailGraphEdge[0x100];
    mEdgeCount = 0x200;
}

s32 RailGraph::addNode(const TVec3f& rVec) {
    RailGraphNode node;
    node.mPosition.x = rVec.x;
    node.mPosition.y = rVec.y;
    node.mPosition.z = rVec.z;
    s32 cnt = mUsedNodeCount++;

    RailGraphNode* nodes = &mNodes[cnt];
    nodes->mPosition = node.mPosition;
    nodes->mFirstEdge = node.mFirstEdge;
    nodes->mEdgeCount = node.mEdgeCount;
    return mUsedNodeCount - 1;
}

void RailGraph::connectNodeTwoWay(s32 a1, s32 a2, const RailGraphEdge* pEdge) {
    RailGraphEdge edge;

    if (pEdge) {
        edge = *pEdge;
    }

    edge.clearConnectInfo();
    edge.mNode0 = a1;
    edge.mNode1 = a2;
    edge.mDistance = PSVECDistance((const Vec*)&mNodes[a1].mPosition, (const Vec*)&mNodes[a2].mPosition);
    s32 cnt = mUsedEdgeCount;
    mUsedEdgeCount = cnt + 1;
    mEdges[cnt] = edge;
    s32 edgeIdx = mUsedEdgeCount - 1;
    connectEdgeToNode(a1, edgeIdx);
    connectEdgeToNode(a2, edgeIdx);
}

RailGraphNode* RailGraph::getNode(s32 idx) const {
    return &mNodes[idx];
}

RailGraphEdge* RailGraph::getEdge(s32 idx) const {
    return &mEdges[idx];
}

bool RailGraph::isValidEdge(s32 edgeIdx) const {
    bool ret = false;

    if (edgeIdx >= 0 && edgeIdx < mUsedEdgeCount) {
        ret = true;
    }

    return ret;
}

void RailGraph::connectEdgeToNode(s32 a1, s32 a2) {
    RailGraphNode* node = &mNodes[a1];
    s32 val = node->mFirstEdge;

    if (val == -1) {
        node->mFirstEdge = a2;
    } else {
        s32 next_edge = mEdges[val].getNextEdge(a1);

        while (next_edge != -1) {
            val = next_edge;
            next_edge = mEdges[next_edge].getNextEdge(a1);
        }

        mEdges[val].setNextEdge(a2, a1);
    }

    mNodes[a1].mEdgeCount++;
}

RailGraphIter RailGraph::getIterator() const {
    return RailGraphIter(this);
}
