#pragma once
class ClipArea;
namespace MR { void registerStarLightArea(ClipArea*); }
class HitSensor;
namespace MR { bool tryRegisterStarLightReceiver(HitSensor*); }
