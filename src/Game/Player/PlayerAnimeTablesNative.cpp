#ifdef GALAXY_NATIVE
#include "Game/Animation/XanimeResource.hpp"

// SB4E01 Mario, Mario-ride and Yoshi animation tables (.data outside every split), decoded from the
// original DOL into host-width pointers and scalars by tools/port/gen_player_anime_tables.py.
// Values are shared with Wii through MarioAnimatorTables.inc and YoshiAnimationTables.inc.

// Shared SB4E01 Mario/Mario-ride values, also compiled by the Wii MarioAnimator unit.
#include "../src/Game/Player/MarioAnimatorTables.inc"

#include "../src/Game/Player/YoshiAnimationTables.inc"

#endif
