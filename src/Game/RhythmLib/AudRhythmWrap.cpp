#include "Game/RhythmLib/AudRhythmWrap.hpp"
#include "Game/Util.hpp"
#include "Game/RhythmLib/AudRhythmMeSystem.hpp"

void AudRhythmWrap::rhythmProc() {
    AudRhythmMeSystem::getInstance()->rhythmProc();
}
