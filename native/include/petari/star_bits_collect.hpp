#pragma once
// CollectStarBits policy shared by the native hooks and focused tests.
// No score is awarded here: StarPiece::goToPlayer owns the vanilla lifecycle.
#include <algorithm>
#include <vector>

class StarPiece;

namespace PetariNative::StarBits {

struct Context {
    bool demo = false;
    bool talking = false;
    bool pointerValid = true;
    bool pointer1PDisabled = false;
    int starBits = 0;
};

inline bool canCollect(const Context& context) {
    return !context.demo && !context.talking && context.pointerValid &&
           !context.pointer1PDisabled && context.starBits < 10000;
}

// The same four nerves that execute tryGotJudge. In particular FallAfterReflect,
// Hop (which already awards on its first step), gifts and thrown ammo are absent.
enum class Motion { Floating, Fall, FollowPlayer, RailMove, Other };
struct PieceState {
    bool alive = true;
    bool movementEnabled = true;
    bool collectionEnabled = true;
    bool hasPointerTarget = true;
    int pointerDelay = -1;
    Motion motion = Motion::Other;
};

inline bool eligible(const PieceState& piece) {
    return piece.alive && piece.movementEnabled && piece.collectionEnabled &&
           piece.hasPointerTarget && piece.pointerDelay < 0 && piece.motion != Motion::Other;
}

class CollectionWindow {
public:
    bool beginFrame(bool enabled, bool pressed, bool allowed) {
        if (!enabled || !allowed) {
            mFrames = 0;  // blocked presses never wait for a later valid frame
            return false;
        }
        if (pressed) mFrames = 4;
        if (mFrames == 0) return false;
        --mFrames;
        return true;
    }
    void reset() { mFrames = 0; }
private:
    int mFrames = 0;
};

// Native sidecar owned logically by the scene director, not the pool group.
// A replacement scene clears old actors before any new registration. An old
// director's late destructor cannot clear its replacement's registry.
// The hooks wrap allocating operations in HostAllocationScope.
template <class Piece> class SceneRegistry {
public:
    void beginScene(const void* owner) {
        mPieces.clear();
        mOwner = owner;
    }
    bool endScene(const void* owner) {
        if (owner != mOwner) return false;
        mPieces.clear();
        mOwner = nullptr;
        return true;
    }
    void add(const void* owner, Piece* piece) {
        if (owner && owner == mOwner && piece &&
            std::find(mPieces.begin(), mPieces.end(), piece) == mPieces.end())
            mPieces.push_back(piece);
    }
    template <class Visit> void forEach(const void* owner, Visit visit) const {
        if (owner == mOwner)
            for (Piece* piece : mPieces) visit(piece);
    }
private:
    const void* mOwner = nullptr;
    std::vector<Piece*> mPieces;
};

// Only called by GALAXY_NATIVE blocks in the two game translation units.
void registerPiece(StarPiece* piece);
bool collectPiece(StarPiece* piece);

}  // namespace PetariNative::StarBits
