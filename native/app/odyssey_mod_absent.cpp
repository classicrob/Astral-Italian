// Built only while the Odyssey movement mod is not linked (GALAXY_ODYSSEY_MOD=OFF). The app's camera test and
// the smoke movement checks observe the mod's state through these C entry points; without the mod there is no
// Cappy and no orbit camera, so the observations are "inactive" and "zero". The real definitions come with the
// mod (src/Game/Camera/OdysseyCamera.cpp, src/Game/Player/MarioCappy.cpp, ported by the core lane).
extern "C" int petari_camera_orbit_active(void) { return 0; }
extern "C" int petari_cappy_state() { return 0; }
extern "C" int petari_cappy_bounces() { return 0; }
extern "C" int petari_cappy_hits() { return 0; }
extern "C" int petari_cappy_collected() { return 0; }
