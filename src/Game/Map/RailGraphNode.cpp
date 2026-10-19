#include "Game/Map/RailGraphNode.hpp"

RailGraphNode::RailGraphNode() {
    mPosition.x = 0.0f;
    mPosition.y = 0.0f;
    mPosition.z = 0.0f;
    mFirstEdge = -1;
    mEdgeCount = 0;
}
