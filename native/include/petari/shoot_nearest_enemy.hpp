#pragma once
// ShootEnemy's selection and one-press transaction. No game dependencies;
// StarPieceShooter supplies the real sensor negotiation, LOS and shooting path.
#include <cmath>
#include <limits>

namespace PetariNative::ShootNearestEnemy {
constexpr float kRange = 3000.0f;  // StarPieceShooter's ordinary aim-distance clamp
struct Vec3 { float x, y, z; };
inline bool finite(Vec3 v) { return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z); }
inline double distanceSquared(Vec3 a, Vec3 b) {
    const double x = double(a.x) - b.x, y = double(a.y) - b.y, z = double(a.z) - b.z;
    return x * x + y * y + z * z;
}
struct ActorState { bool alive, unclipped, shown; };
inline bool eligible(ActorState a) { return a.alive && a.unclipped && a.shown; }
struct Candidate {
    ActorState actor;
    bool validSensor, enemy;
    Vec3 position;
    float radius;
};
// Keep the original shoot() float segment calculations finite as well.
inline bool safeSegment(Vec3 from, Vec3 to) {
    return finite(from) && finite(to) &&
           distanceSquared(from, to) <= double(std::numeric_limits<float>::max()) / 4.0;
}
inline bool unobstructed(Vec3 from, Vec3 to, bool hit, Vec3 hitPosition) {
    return safeSegment(from, to) && (!hit || (finite(hitPosition) &&
           distanceSquared(from, hitPosition) >= distanceSquared(from, to)));
}
class Selection {
public:
    explicit Selection(Vec3 player) : mPlayer(player) {}
    template<class Visible, class Accept>
    bool consider(const Candidate& c, Visible visible, Accept accept) {
        if (!finite(mPlayer) || !eligible(c.actor) || !c.validSensor || !c.enemy ||
            !finite(c.position) || !std::isfinite(c.radius) || c.radius < 0.0f)
            return false;
        const double distance = distanceSquared(mPlayer, c.position);
        if (distance > double(kRange) * kRange || (mFound && distance >= mDistance))
            return false;  // inclusive range; first actor/sensor wins exact ties
        if (!visible() || !accept()) return false;
        mDistance = distance;
        mFound = true;
        return true;
    }
private:
    Vec3 mPlayer;
    double mDistance = 0.0;
    bool mFound = false;
};
inline bool normalInputBusy(bool shotThisFrame, int chargeStep, bool heldB) {
    // -1 is also the normal no-ammo/reset state, not an active charge.
    return shotThisFrame || chargeStep > 0 || heldB;
}
struct Request {
    bool enabled = false, pressed = false;
    int port = 0;
    bool pointerUsable = false, canShoot = false, busy = false;
    int ammo = 0;
};
template<class Take>
bool takePress(int port, Take take) { return port == 0 && take(); }
enum class Result { Ignored, Blocked, NoAmmo, NoTarget, Failed, Fired };
template<class Find, class Shoot, class Empty, class Feedback>
Result run(const Request& r, Find find, Shoot shoot, Empty empty, Feedback feedback) {
    if (!r.enabled || !r.pressed || r.port != 0) return Result::Ignored;
    if (!r.pointerUsable || !r.canShoot || r.busy) return Result::Blocked;
    if (r.ammo <= 0) { empty(); return Result::NoAmmo; }
    auto* target = find();
    if (target == nullptr) return Result::NoTarget;
    if (!shoot(target)) return Result::Failed;
    feedback();
    return Result::Fired;
}
}  // namespace PetariNative::ShootNearestEnemy
