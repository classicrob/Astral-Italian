#include "../app/title_observation.hpp"
#include <stdexcept>
using O = PetariNative::App::TitleObservation;
void check(bool ok) { if (!ok) throw std::runtime_error("title observation boundary"); }
int main() {
    O cold;
    check(cold.tick(0, false) == O::Step::Running);
    check(cold.tick(899.999, false) == O::Step::Running);
    check(cold.tick(900, false) == O::Step::TimedOut);
    check(cold.tick(901, true) == O::Step::Running); // stop/quit is idempotent
    O warm;
    check(warm.tick(10, true) == O::Step::Running);
    check(warm.tick(39.999, true) == O::Step::Running);
    check(warm.tick(40, true) == O::Step::Complete);
    check(warm.completionSeconds() == 10);
    O tooLate;
    check(tooLate.tick(880, true) == O::Step::Running);
    check(tooLate.tick(900, true) == O::Step::TimedOut); // postroll cannot extend the 15m cap
    O window(315);
    check(window.tick(10, true) == O::Step::Running);
    check(window.tick(40, true) == O::Step::Running);
    check(window.tick(315, true) == O::Step::Complete);
}
