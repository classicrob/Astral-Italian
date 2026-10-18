#pragma once

#include "Game/LiveActor/LiveActorGroup.hpp"

class Coin;
class NameObj;

class PurpleCoinHolder : public DeriveActorGroup< Coin > {
public:
    /// @brief Creates a new `PurpleCoinHolder`.
    PurpleCoinHolder();

    virtual void initAfterPlacement();
};

namespace MR {
    void createPurpleCoinHolder();
    void addToPurpleCoinHolder(const NameObj*, Coin*);
};  // namespace MR
