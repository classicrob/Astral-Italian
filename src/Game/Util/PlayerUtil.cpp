#include "Game/Util/PlayerUtil.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/Util.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/PlayerEvent.hpp"
#include "Game/Player/RushEndInfo.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Animation/BckCtrl.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/MapObj/SwingActionWatcher.hpp"
#include "Game/Player/Yoshi.hpp"
#include "JSystem/JAudio2/JAISound.hpp"
#include "JSystem/JGeometry/TUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

#if defined(GALAXY_NATIVE) || defined(__MWERKS__)

// SB4E01 playerGhost tables, decoded into host-width pointers and scalars.
XanimeBckTable4 playerGhostQuadAnimes[] = {
    {{"\x8A\xEE\x96\x7B"}, {{"WalkSoft", 0.0f}, {"Walk", 0.0f}, {"Run", 0.0f}, {"Wait", 1.0f}}},
    {{"\x90\x85\x89\x6A\x8A\xEE\x96\x7B"}, {{"SwimWait", 1.0f}, {"SwimFlutterSurface", 0.0f}, {"SwimFlutter", 0.0f}, {"SwimDrift", 0.0f}}},
    {{"\x83\x65\x83\x6A\x83\x58\x8A\xEE\x96\x7B"}, {{"WalkSoft", 0.0f}, {"Walk", 0.0f}, {"TennisRun", 0.0f}, {"TennisWait", 1.0f}}},
    {{"\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B\x90\x4B"}, {{"SlideHipForWard", 0.300000012f}, {"SlideHipBackward", 0.200000003f}, {"SlideHipLeft", 0.25f}, {"SlideHipRight", 0.25f}}},
    {{"\x8D\xE2\x82\xB7\x82\xD7\x82\xE8\x89\xBA\x8C\xFC\x82\xAB\x82\xA0\x82\xA8\x82\xDE\x82\xAF"}, {{"SlideHipForWard", 0.300000012f}, {"SlideHipBackward", 0.200000003f}, {"SlideHipLeft", 0.25f}, {"SlideHipRight", 0.25f}}},
    {{"\x83\x5E\x83\x7D\x83\x52\x83\x8D\x88\xDA\x93\xAE"}, {{"BallIdle", 1.0f}, {"BallWalkSoft", 0.0f}, {"BallWalk", 0.0f}, {"BallRun", 0.0f}}},
    {{""}, {{"", 0.0f}, {nullptr, 0.0f}, {nullptr, 0.0f}, {nullptr, 0.0f}}},
};

XanimeBckTable3 playerGhostTripleAnimes[] = {
    {{"\x8D\xE2\x8D\xB6\x89\x45\x83\x45\x83\x47\x83\x43\x83\x67"}, {{"Wait", 1.0f}, {"WaitSlopeL", 0.0f}, {"WaitSlopeR", 0.0f}}},
    {{"\x8D\xE2\x91\x4F\x8C\xE3\x83\x45\x83\x47\x83\x43\x83\x67"}, {{"Wait", 1.0f}, {"WaitSlopeD", 0.0f}, {"WaitSlopeU", 0.0f}}},
    {{"\x83\x60\x83\x85\x81\x5B\x83\x75\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B"}, {{"TubeSlide", 1.0f}, {"TubeSlideLeft", 0.0f}, {"TubeSlideRight", 0.0f}}},
    {{"\x83\x4F\x83\x89\x83\x43\x83\x6F\x81\x5B\x83\x68\x8A\x8A\x8B\xF3\x57\x61\x69\x74"}, {{"GliBirdGlideWait", 1.0f}, {"GliBirdGlideTurnL", 0.0f}, {"GliBirdGlideTurnR", 0.0f}}},
    {{"\x83\x4F\x83\x89\x83\x43\x83\x6F\x81\x5B\x83\x68\x8A\x8A\x8B\xF3\x44\x6F\x77\x6E"}, {{"GliBirdGlideDown", 1.0f}, {"GliBirdGlideTurnL", 0.0f}, {"GliBirdGlideTurnR", 0.0f}}},
    {{"\x83\x4F\x83\x89\x83\x43\x83\x6F\x81\x5B\x83\x68\x8A\x8A\x8B\xF3\x46\x61\x73\x74"}, {{"GliBirdGlideDownFast", 1.0f}, {"GliBirdGlideTurnL", 0.0f}, {"GliBirdGlideTurnR", 0.0f}}},
    {{"\x83\x4F\x83\x89\x83\x43\x83\x6F\x81\x5B\x83\x68\x8A\x8A\x8B\xF3\x55\x70"}, {{"GliBirdGlideUp", 1.0f}, {"GliBirdGlideTurnL", 0.0f}, {"GliBirdGlideTurnR", 0.0f}}},
    {{""}, {{"", 0.0f}, {nullptr, 0.0f}, {nullptr, 0.0f}}},
};

XanimeBckTable2 playerGhostDoubleAnimes[] = {
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x8A\xEE\x96\x7B"}, {{"SquatWait", 1.0f}, {"SquatWalk", 0.0f}}},
    {{"\x82\xBB\x82\xCC\x8F\xEA\x91\xAB\x93\xA5\x82\xDD"}, {{"Run", 0.75f}, {"Wait", 0.25f}}},
    {{"\x83\x5E\x83\x7D\x83\x52\x83\x8D\x82\xB5\x82\xE1\x82\xAA\x82\xDD"}, {{"BallSquat", 1.0f}, {"BallWalk", 0.0f}}},
    {{""}, {{"", 0.0f}, {nullptr, 0.0f}}},
};

XanimeBckTable1 playerGhostSingleAnimes[] = {
    {{"\x83\x57\x83\x83\x83\x93\x83\x76"}, "Jump", 0, 0},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x42"}, "Jump2", 0, 0},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x43"}, "JumpRoll", 0, 0},
    {{"\x83\x5E\x81\x5B\x83\x93\x83\x57\x83\x83\x83\x93\x83\x76"}, "JumpTurn", 0, 0},
    {{"\x8B\xF3\x92\x86\x88\xEA\x89\xF1\x93\x5D"}, "AirControl", 0, 0},
    {{"\x83\x74\x83\x8A\x81\x5B\x83\x57\x83\x83\x83\x93\x83\x76"}, "Rise", 0, 0},
    {{"\x83\x77\x83\x8A\x83\x52\x83\x76\x83\x5E\x81\x5B\x83\x57\x83\x83\x83\x93\x83\x76"}, "FlickAir", 0, 0},
    {{"\x83\x5F\x83\x62\x83\x56\x83\x85\x83\x57\x83\x83\x83\x93\x83\x76"}, "Rolling", 0, 0},
    {{"\x94\xF2\x82\xD1\x82\xB7\x82\xB3\x82\xE8\x83\x57\x83\x83\x83\x93\x83\x76"}, "Bounce", 0, 0},
    {{"\x83\x56\x83\x87\x81\x5B\x83\x67\x83\x57\x83\x83\x83\x93\x83\x76"}, "GravityChange", 0, 0},
    {{"\x83\x58\x83\x4A\x83\x43\x83\x89\x83\x75\x83\x57\x83\x83\x83\x93\x83\x76"}, "JumpTwin", 0, 0},
    {{"\x83\x6E\x83\x60\x83\x7B\x81\x5B\x83\x8B\x83\x57\x83\x83\x83\x93\x83\x76"}, "BeeBallHit", 0, 0},
    {{"\x83\x58\x83\x50\x83\x4C\x83\x88"}, "Bury", 0, 0},
    {{"\x83\x58\x83\x50\x83\x4C\x83\x88\x92\x45\x8F\x6F"}, "Bury", 0, 0},
    {{"\x96\x84\x82\xDC\x82\xE8"}, "BuryStandWait", 0, 0},
    {{"\x96\x84\x82\xDC\x82\xE8\x92\x45\x8F\x6F"}, "BuryStandEnd", 0, 0},
    {{"\x8B\x74\x92\x85\x92\x6E"}, "CannonFlyLand", 0, 0},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x83\x57\x83\x83\x83\x93\x83\x76\x41"}, "HopperWaitA", 0, 0},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x83\x57\x83\x83\x83\x93\x83\x76\x42"}, "HopperWaitB", 0, 0},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x82\xD3\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76\x41"}, "HopperWaitA", 0, 0},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x82\xD3\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76\x42"}, "HopperWaitB", 0, 0},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x88\xDA\x93\xAE\x41"}, "HopperRunA", 0, 0},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x88\xDA\x93\xAE\x42"}, "HopperRunB", 0, 0},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x83\x6E\x83\x43\x83\x57\x83\x83\x83\x93\x83\x76\x41"}, "HopperJumpA", 0, 0},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x83\x6E\x83\x43\x83\x57\x83\x83\x83\x93\x83\x76\x42"}, "HopperJumpB", 0, 0},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x95\xC7\x83\x57\x83\x83\x83\x93\x83\x76"}, "HopperWallJump", 0, 0},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x8A\x4A\x8E\x6E"}, "HopperMarioHipDropStart", 0, 0},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76"}, "HopperMarioHipDrop", 0, 0},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x82\xD3\x82\xDD\x31"}, "Jump", 0, 0},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x82\xD3\x82\xDD\x32"}, "JumpPress2nd", 0, 0},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x82\xD3\x82\xDD\x33"}, "JumpPress3rd", 0, 0},
    {{"\x88\xF8\x82\xAB\x96\xDF\x82\xB5"}, "PullBack", 0, 0},
    {{"\x88\xF8\x82\xAB\x96\xDF\x82\xB5\x92\x85\x92\x6E"}, "PullBackLand", 0, 0},
    {{"\x83\x7C\x83\x62\x83\x68\x83\x8F\x81\x5B\x83\x76\x8A\x4A\x8E\x6E"}, "WarpPodStart", 0, 0},
    {{"\x83\x7C\x83\x62\x83\x68\x83\x8F\x81\x5B\x83\x76\x8F\x49\x97\xB9"}, "WarpPodEnd", 0, 0},
    {{"\x95\x9D\x82\xC6\x82\xD1"}, "JumpBroad", 0, 0},
    {{"\x92\x85\x92\x6E"}, "Land", 0, 0},
    {{"\x92\x85\x92\x6E\x42"}, "Jump2Land", 0, 0},
    {{"\x92\x85\x92\x6E\x43"}, "JumpRollLand", 0, 0},
    {{"\x92\x85\x92\x6E\x83\x5E\x81\x5B\x83\x93"}, "JumpTurnLand", 0, 0},
    {{"\x92\x85\x92\x6E\x95\x9D\x82\xC6\x82\xD1"}, "JumpBroadLand", 0, 0},
    {{"\x83\x6E\x81\x5B\x83\x68\x92\x85\x92\x6E"}, "LandStiffen", 0, 0},
    {{"\x82\xB7\x82\xD7\x82\xE8\x92\x85\x92\x6E"}, "LandSlope", 0, 0},
    {{"\x83\x56\x83\x87\x81\x5B\x83\x67\x92\x85\x92\x6E"}, "Land", 0, 0},
    {{"\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x8A\x4A\x8E\x6E"}, "HipDropStart", 0, 0},
    {{"\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76"}, "HipDrop", 0, 0},
    {{"\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x92\x85\x92\x6E"}, "HipDropLand", 0, 0},
    {{"\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x8A\x8A\x82\xE8"}, "LandRotation", 0, 0},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x8F\x87\x8A\x8A\x82\xE8"}, "LandRotation", 0, 0},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x8B\x74\x8A\x8A\x82\xE8"}, "Fall", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x8A\x4A\x8E\x6E"}, "HipDropHomingStart", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76"}, "HipDropHoming", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x92\x85\x92\x6E"}, "HipDropHomingLand", 0, 0},
    {{"\x83\x58\x83\x8A\x83\x62\x83\x76\x83\x41\x83\x62\x83\x76"}, "SlipUp", 0, 0},
    {{"\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x83\x58\x83\x8A\x83\x62\x83\x76\x83\x41\x83\x62\x83\x76\x8F\x80\x94\xF5"}, "HangSlipUpStart", 0, 0},
    {{"\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x83\x58\x83\x8A\x83\x62\x83\x76\x83\x41\x83\x62\x83\x76"}, "HangSlipUp", 0, 0},
    {{"\x83\x6E\x83\x60\x94\xF2\x8D\x73\x92\x86"}, "BeeFly", 0, 0},
    {{"\x83\x6E\x83\x60\x94\xF2\x8D\x73\x92\x86\x96\xB3\x93\xFC\x97\xCD"}, "BeeFlyWait", 0, 0},
    {{"\x83\x6E\x83\x60\x95\xC7\x82\xAD\x82\xC1\x82\xC2\x82\xAB"}, "BeeLand", 0, 0},
    {{"\x83\x6E\x83\x60\x95\xC7\x82\xAD\x82\xC1\x82\xC2\x82\xAB\x92\x86"}, "BeeWait", 0, 0},
    {{"\x83\x6E\x83\x60\x89\xD4\x88\xDA\x93\xAE"}, "BeeCreepWallWalk", 0, 0},
    {{"\x83\x6E\x83\x60\x83\x57\x83\x83\x83\x93\x83\x76"}, "BeeJump", 0, 0},
    {{"\x83\x6E\x83\x60\x95\xC7\x83\x57\x83\x83\x83\x93\x83\x76"}, "BeeWallJump", 0, 0},
    {{"\x83\x6E\x83\x60\x99\xB3\x99\xB4\x83\x45\x83\x47\x83\x43\x83\x67"}, "BeeCreepWait", 0, 0},
    {{"\x83\x6E\x83\x60\x99\xB3\x99\xB4\x91\x4F\x90\x69"}, "BeeCreepWalk", 0, 0},
    {{"\x83\x6E\x83\x60\x99\xB3\x99\xB4\x92\x85\x92\x6E"}, "BeeCreepLand", 0, 0},
    {{"\x83\x6E\x83\x60\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x8A\x4A\x8E\x6E"}, "BeeHipDropStart", 0, 0},
    {{"\x83\x6E\x83\x60\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76"}, "BeeHipDrop", 0, 0},
    {{"\x83\x6E\x83\x60\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x92\x85\x92\x6E"}, "BeeHipDropLand", 0, 0},
    {{"\x83\x6E\x83\x60\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x95\xC7\x92\x85\x92\x6E"}, "BeeHipDropLand", 0, 0},
    {{"\x93\x44\x92\xE1\x91\xAC\x95\xE0\x8D\x73"}, "WalkBury", 0, 0},
    {{"\x93\x44\x8D\x82\x91\xAC\x95\xE0\x8D\x73"}, "WalkSticky", 0, 0},
    {{"\x97\x8E\x89\xBA"}, "Fall", 0, 0},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x94\xF2\x8D\x73\x8A\x4A\x8E\x6E"}, "FooStart", 0, 0},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x94\xF2\x8D\x73"}, "FooFly", 0, 0},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x94\xF2\x8D\x73\x8D\xC4\x8A\x4A"}, "FooFlyStart", 0, 0},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x90\xC3\x8E\x7E"}, "FooWait", 0, 0},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x89\xF0\x8F\x9C"}, "FooEnd", 0, 0},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x92\x85\x92\x6E"}, "LandSlope", 0, 0},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x83\x58\x83\x73\x83\x93"}, "FooSpin", 0, 0},
    {{"\x95\xC7\x83\x57\x83\x83\x83\x93\x83\x76"}, "WallJump", 0, 0},
    {{"\x95\xC7\x82\xB7\x82\xD7\x82\xE8"}, "WallSlide", 0, 0},
    {{"\x95\xC7\x82\xAD\x82\xC1\x82\xC2\x82\xAB"}, "WallKeep", 0, 0},
    {{"\x95\xC7\x89\x9F\x82\xB5"}, "Push", 0, 0},
    {{"\x95\xC7\x8D\xB6\x95\xE0\x82\xAB"}, "WallWalkL", 0, 0},
    {{"\x95\xC7\x89\x45\x95\xE0\x82\xAB"}, "WallWalkR", 0, 0},
    {{"\x95\xC7\x83\x45\x83\x47\x83\x43\x83\x67"}, "WallWait", 0, 0},
    {{"\x91\x4F\x95\xC7\x83\x45\x83\x47\x83\x43\x83\x67"}, "Push", 0, 0},
    {{"\x95\xC7\x83\x71\x83\x62\x83\x67"}, "WallHit", 0, 0},
    {{"\x95\xC7\x83\x71\x83\x62\x83\x67\x92\x85\x92\x6E"}, "WallHitLand", 0, 0},
    {{"\x95\xC7\x82\xCD\x82\xB6\x82\xAB"}, "WallHit", 0, 0},
    {{"\x8A\x52\x82\xD3\x82\xF1\x82\xCE\x82\xE8"}, "Stagger", 0, 0},
    {{"\x83\x4A\x83\x8A\x83\x4A\x83\x8A\x8C\xC0\x8A\x45"}, "WaitHold", 0, 0},
    {{"\x8D\xE2\x8D\xB6\x83\x45\x83\x47\x83\x43\x83\x67"}, "WaitSlopeL", 0, 0},
    {{"\x8D\xE2\x89\x45\x83\x45\x83\x47\x83\x43\x83\x67"}, "WaitSlopeR", 0, 0},
    {{"\x8D\xE2\x91\x4F\x83\x45\x83\x47\x83\x43\x83\x67"}, "WaitSlopeD", 0, 0},
    {{"\x8D\xE2\x8C\xE3\x83\x45\x83\x47\x83\x43\x83\x67"}, "WaitSlopeU", 0, 0},
    {{"\x83\x89\x83\x93"}, "Run", 0, 0},
    {{"\x95\xE0\x8D\x73"}, "Walk", 0, 0},
    {{"\x93\xDD\x8D\x73"}, "WalkSoft", 0, 0},
    {{"\x83\x81\x83\x5E\x83\x8B\x83\x5F\x83\x62\x83\x56\x83\x85"}, "RunDash", 0, 0},
    {{"\x96\x84\x82\xDC\x82\xE8\x95\xE0\x8D\x73"}, "WalkBury", 0, 0},
    {{"\x96\x84\x82\xDC\x82\xE8\x83\x57\x83\x83\x83\x93\x83\x76\x41"}, "WalkBuryJumpLow", 0, 0},
    {{"\x96\x84\x82\xDC\x82\xE8\x83\x57\x83\x83\x83\x93\x83\x76\x42"}, "WalkBuryJumpLow2", 0, 0},
    {{"\x96\x84\x82\xDC\x82\xE8\x92\x45\x8F\x6F\x83\x57\x83\x83\x83\x93\x83\x76"}, "WalkBuryJumpHi", 0, 0},
    {{"\x82\xC2\x82\xD4\x82\xEA"}, "Press", 0, 0},
    {{"\x82\xC2\x82\xD4\x82\xEA\x95\x9C\x8B\x41"}, "PressRecover", 0, 0},
    {{"\x82\xC2\x82\xD4\x82\xEA\x89\xF0\x8F\x9C"}, "Jump", 0, 0},
    {{"\x83\x75\x83\x8C\x81\x5B\x83\x4C"}, "Brake", 0, 0},
    {{"\x83\x75\x83\x8C\x81\x5B\x83\x4C\x8A\x8A\x82\xE8\x8F\xB0"}, "Run", 0, 0},
    {{"\x83\x5E\x81\x5B\x83\x93\x83\x75\x83\x8C\x81\x5B\x83\x4C"}, "Turn", 0, 0},
    {{"\x83\x5E\x81\x5B\x83\x93\x83\x75\x83\x8C\x81\x5B\x83\x4C\x8A\x8A\x82\xE8\x8F\xB0"}, "Run", 0, 0},
    {{"\x95\xE0\x8D\x73\x90\xA7\x93\xAE\x83\x75\x83\x8C\x81\x5B\x83\x4C"}, "RunEnd", 0, 0},
    {{"\x95\x58\x8F\xE3\x8A\xB5\x90\xAB\x91\x96\x8D\x73"}, "Brake", 0, 0},
    {{"\x95\x58\x8F\xE3\x97\xCD\x8D\x73\x8D\xB6"}, "SkateL", 0, 0},
    {{"\x95\x58\x8F\xE3\x97\xCD\x8D\x73\x89\x45"}, "SkateR", 0, 0},
    {{"\x95\x58\x8F\xE3\x90\xD8\x91\xD6\x8D\xB6"}, "SkateSwitchL", 0, 0},
    {{"\x95\x58\x8F\xE3\x90\xD8\x91\xD6\x89\x45"}, "SkateSwitchR", 0, 0},
    {{"\x95\x58\x8F\xE3\x8C\xE3\x8D\x73\x8D\xB6"}, "SkateBackL", 0, 0},
    {{"\x95\x58\x8F\xE3\x8C\xE3\x8D\x73\x89\x45"}, "SkateBackR", 0, 0},
    {{"\x95\x58\x8F\xE3\x83\x5E\x81\x5B\x83\x93"}, "SkateTurn", 0, 0},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD"}, "SquatWait", 0, 0},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x8A\x4A\x8E\x6E"}, "SquatStart", 0, 0},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x8F\x49\x97\xB9"}, "SquatEnd", 0, 0},
    {{"\x8D\xE2\x82\xB7\x82\xD7\x82\xE8\x8F\xE3\x8C\xFC\x82\xAB\x82\xA4\x82\xC2\x82\xD4\x82\xB9"}, "SlideStmach", 0, 0},
    {{"\x8D\xE2\x82\xB7\x82\xD7\x82\xE8\x8F\xE3\x8C\xFC\x82\xAB\x8F\x49\x97\xB9"}, "SlideStmachEnd", 0, 0},
    {{"\x8D\xE2\x82\xB7\x82\xD7\x82\xE8\x89\xBA\x8C\xFC\x82\xAB\x8F\x49\x97\xB9"}, "SlideHipEnd", 0, 0},
    {{"\x93\xC1\x8E\xEA\x83\x45\x83\x47\x83\x43\x83\x67\x31\x41"}, "Sleep", 0, 0},
    {{"\x93\xC1\x8E\xEA\x83\x45\x83\x47\x83\x43\x83\x67\x31\x42"}, "SleepLie", 0, 0},
    {{"\x90\xED\x93\xAC\x83\x45\x83\x47\x83\x43\x83\x67"}, "BattleWait", 0, 0},
    {{"\x8B\xF3\x93\x5D"}, "Run", 0, 0},
    {{"\x82\xD0\x82\xEB\x82\xA2"}, "CarryStart", 0, 0},
    {{"\x82\xD0\x82\xEB\x82\xA2\x8B\xF3\x92\x86"}, "CarryAirStart", 0, 0},
    {{"\x82\xD0\x82\xEB\x82\xA2\x83\x4E\x83\x43\x83\x62\x83\x4E"}, "CarryStartShort", 0, 0},
    {{"\x82\xD0\x82\xEB\x82\xA2\x83\x45\x83\x47\x83\x43\x83\x67"}, "CarryWait", 0, 0},
    {{"\x83\x4A\x83\x75\x94\xB2\x82\xAB"}, "PullOut", 0, 0},
    {{"\x83\x4A\x83\x75\x83\x45\x83\x47\x83\x43\x83\x67"}, "PullOutWait", 0, 0},
    {{"\x83\x68\x83\x8A\x83\x8B\x82\xD0\x82\xEB\x82\xA2"}, "CarryDrillStart", 0, 0},
    {{"\x83\x68\x83\x8A\x83\x8B\x82\xD0\x82\xEB\x82\xA2\x83\x4E\x83\x43\x83\x62\x83\x4E"}, "CarryDrillShortStart", 0, 0},
    {{"\x83\x68\x83\x8A\x83\x8B\x83\x45\x83\x47\x83\x43\x83\x67"}, "CarryDrillWait", 0, 0},
    {{"\x83\x6E\x83\x93\x83\x7D\x81\x5B\x93\x8A\x82\xB0\x89\xF1\x93\x5D\x92\x86"}, "Swing", 0, 0},
    {{"\x83\x6E\x83\x93\x83\x7D\x81\x5B\x93\x8A\x82\xB0\x83\x8A\x83\x8A\x81\x5B\x83\x58"}, "SwingThrow", 0, 0},
    {{"\x83\x52\x83\x43\x83\x93\x83\x51\x83\x62\x83\x67"}, "CoinGet", 0, 0},
    {{"\x83\x70\x83\x93\x83\x60"}, "Spin2nd", 0, 0},
    {{"\x8B\xF3\x83\x70\x83\x93\x83\x60"}, "Spin2nd", 0, 0},
    {{"\x8D\xB6\x83\x70\x83\x93\x83\x60"}, "Spin2nd", 0, 0},
    {{"\x8D\xB6\x8B\xF3\x83\x70\x83\x93\x83\x60"}, "Spin2nd", 0, 0},
    {{"\x83\x89\x83\x93\x83\x6A\x83\x93\x83\x4F\x83\x4C\x83\x62\x83\x4E"}, "Kick", 0, 0},
    {{"\x83\x41\x83\x62\x83\x70\x81\x5B\x83\x70\x83\x93\x83\x60"}, "Jump", 0, 0},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x83\x41\x83\x62\x83\x70\x81\x5B"}, "SquatEnd", 0, 0},
    {{"\x83\x54\x83\x7D\x81\x5B\x83\x5C\x83\x8B\x83\x67"}, "SpinLow", 0, 0},
    {{"\x93\x8A\x82\xB0"}, "Throw", 0, 0},
    {{"\x83\x74\x83\x40\x83\x43\x83\x41\x93\x8A\x82\xB0"}, "Throw", 0, 0},
    {{"\x97\xBC\x8E\xE8\x93\x8A\x82\xB0"}, "ThrowBoth", 0, 0},
    {{"\x83\x74\x83\x40\x83\x43\x83\x41\x83\x58\x83\x73\x83\x93"}, "FireSpin", 0, 0},
    {{"\x83\x74\x83\x40\x83\x43\x83\x41\x83\x58\x83\x73\x83\x93\x8B\xF3\x92\x86"}, "FireSpin", 0, 0},
    {{"\x8A\x52\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x8A\x4A\x8E\x6E"}, "HangStartUnder", 0, 0},
    {{"\x8A\x52\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x92\x86"}, "HangWait", 0, 0},
    {{"\x8A\x52\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x8F\x49\x97\xB9"}, "HangUp", 0, 0},
    {{"\x8A\x52\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x8F\x49\x97\xB9\x8D\xE2"}, "HangUp", 0, 0},
    {{"\x8A\x52\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x8D\x7E\x82\xE8"}, "HangStart", 0, 0},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76"}, "JumpBack", 0, 0},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76\x92\x85\x92\x6E"}, "JumpBackLand", 0, 0},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x95\xE0\x82\xAB"}, "SquatWalk", 0, 0},
    {{"\x82\xAA\x82\xF1\x82\xCE\x82\xE8\x91\x96\x82\xE8"}, "RunSlope", 0, 0},
    {{"\x95\xA0\x82\xCE\x82\xA2\x83\x57\x83\x83\x83\x93\x83\x76"}, "SlideStomachRecover", 0, 0},
    {{"\x90\x4B\x8A\x8A\x82\xE8\x83\x57\x83\x83\x83\x93\x83\x76"}, "SlideHipRecover", 0, 0},
    {{"\x8C\xFC\x82\xA9\x82\xA2\x95\x97\x91\x96\x82\xE8"}, "RunSlope", 0, 0},
    {{"\x8C\xFC\x82\xA9\x82\xA2\x95\x97\x82\xD3\x82\xF1\x82\xCE\x82\xE8"}, "Stagger", 0, 0},
    {{"\x82\xBB\x82\xCC\x8F\xEA\x91\xAB\x93\xA5\x82\xDD\x8F\xE3\x94\xBC\x90\x67"}, "Walk", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x45\x83\x47\x83\x43\x83\x67"}, "SwimWait", 0, 0},
    {{"\x90\x85\x89\x6A\x88\xEA\x91\x7E\x82\xAB"}, "SwimBreast", 0, 0},
    {{"\x90\x85\x8F\xE3\x88\xEA\x91\x7E\x82\xAB"}, "SwimBreastSurface", 0, 0},
    {{"\x90\x85\x89\x6A\x90\xF6\x82\xE8"}, "SwimDive", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x57\x83\x83\x83\x93\x83\x76\x83\x5F\x83\x43\x83\x75"}, "LandWater", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x57\x83\x83\x83\x93\x83\x76\x83\x5F\x83\x43\x83\x75\x89\xF1\x93\x5D"}, "LandWaterDive", 0, 0},
    {{"\x90\x85\x89\x6A\x8F\xE3\x8F\xB8\x8C\xC4\x8B\x7A"}, "SwimRise", 0, 0},
    {{"\x90\x85\x89\x6A\x90\x85\x96\xCA\x8F\x89\x8A\xFA\x88\xDA\x93\xAE"}, "SwimStartSurface", 0, 0},
    {{"\x90\x85\x8F\xE3\x83\x58\x83\x73\x83\x93"}, "SwimSpinSurface", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x58\x83\x73\x83\x93"}, "SwimSpin", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x58\x83\x73\x83\x93\x88\xDA\x93\xAE"}, "SwimSpinAttack", 0, 0},
    {{"\x90\x85\x8F\xE3\x83\x58\x83\x73\x83\x93\x88\xDA\x93\xAE"}, "SwimSpinAttackSurface", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x58\x83\x73\x83\x93\x83\x57\x83\x83\x83\x93\x83\x76"}, "SwimJump", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x5F\x83\x81\x81\x5B\x83\x57"}, "SwimDamageSmall", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x5F\x83\x81\x81\x5B\x83\x57\x8B\x54"}, "SwimFlutterBoardDamageSmall", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x86"}, "SwimDamageMiddle", 0, 0},
    {{"\x90\x85\x8F\xE3\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x86"}, "SwimDamageMiddleSurface", 0, 0},
    {{"\x90\x85\x8F\xE3\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x85\x90\x85"}, "SwimDamageMiddleSurfaceLand", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x57\x83\x46\x83\x62\x83\x67"}, "SwimFlutterboard", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x57\x83\x46\x83\x62\x83\x67\x8F\x49\x97\xB9"}, "SwimDamageMiddle", 0, 0},
    {{"\x90\x85\x89\x6A\x8B\x54\x93\x8A\x82\xB0"}, "SwimFlutterboardThrow", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x57\x83\x46\x83\x62\x83\x67\x8A\x4A\x8E\x6E"}, "SwimFlutterboardStart", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x57\x83\x46\x83\x62\x83\x67\x95\xC7\x83\x5E\x81\x5B\x83\x93"}, "SwimFlutterboardTurn", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x5F\x83\x45\x83\x93"}, "SwimDie", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x5E\x81\x5B\x83\x93\x8D\xB6"}, "SwimTurnL", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x5E\x81\x5B\x83\x93\x89\x45"}, "SwimTurnR", 0, 0},
    {{"\x90\x85\x89\x6A\x95\xC7\x83\x71\x83\x62\x83\x67"}, "SwimWallHit", 0, 0},
    {{"\x90\x85\x89\x6A\x83\x5E\x81\x5B\x83\x93\x89\xBA"}, "SwimTurnForward", 0, 0},
    {{"\x90\x85\x89\x6A\x97\xA4\x82\xA4\x82\xBF\x82\xA0\x82\xB0"}, "SlideStmachEnd", 0, 0},
    {{"\x94\xF2\x82\xD1\x8D\x9E\x82\xDD\x8F\x80\x94\xF5"}, "DiveWait", 0, 0},
    {{"\x94\xF2\x82\xD1\x8D\x9E\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76"}, "Dive", 0, 0},
    {{"\x8C\xE3\x95\xFB\x94\xF2\x82\xD1\x8D\x9E\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76"}, "DiveBack", 0, 0},
    {{"\x94\xF2\x82\xD1\x8D\x9E\x82\xDD\x8E\xB8\x94\x73\x92\x85\x92\x6E"}, "Land", 0, 0},
    {{"\x94\xF2\x82\xD1\x8D\x9E\x82\xDD\x8E\xB8\x94\x73\x89\xF1\x93\x5D\x92\x85\x92\x6E"}, "LandRotation", 0, 0},
    {{"\x83\x8A\x83\x93\x83\x4F\x83\x5F\x83\x62\x83\x56\x83\x85"}, "SwimDashRing", 0, 0},
    {{"\x83\x8A\x83\x93\x83\x4F\x83\x5F\x83\x62\x83\x56\x83\x85\x8F\x80\x94\xF5"}, "SwimDashRingStart", 0, 0},
    {{"\x83\x4A\x83\x81\x8E\x9D\x82\xBF\x83\x8A\x83\x93\x83\x4F\x8F\x80\x94\xF5"}, "SwimFlutterBoardDashRingStart", 0, 0},
    {{"\x83\x4A\x83\x81\x8E\x9D\x82\xBF\x83\x8A\x83\x93\x83\x4F"}, "SwimFlutterBoardDashRing", 0, 0},
    {{"\x99\xB3\x99\xB4\x91\x4F\x90\x69"}, "SwimGetUp", 0, 0},
    {{"\x91\x4F\x95\xFB\x8F\xAC\x83\x5F\x83\x81\x81\x5B\x83\x57"}, "DamageSmallFront", 0, 0},
    {{"\x8C\xE3\x95\xFB\x8F\xAC\x83\x5F\x83\x81\x81\x5B\x83\x57"}, "DamageSmallBack", 0, 0},
    {{"\x8B\xAD\x90\xA7\x83\x57\x83\x83\x83\x93\x83\x76"}, "Rise", 0, 0},
    {{"\x92\x86\x83\x5F\x83\x81\x81\x5B\x83\x57"}, "DamageMiddleFront", 0, 0},
    {{"\x92\x86\x83\x5F\x83\x81\x81\x5B\x83\x57\x8B\xF3\x92\x86"}, "DamageMiddleFrontAir", 0, 0},
    {{"\x92\x86\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x85\x92\x6E"}, "DamageMiddleFrontLand", 0, 0},
    {{"\x92\x86\x8C\xE3\x83\x5F\x83\x81\x81\x5B\x83\x57"}, "DamageMiddleBack", 0, 0},
    {{"\x92\x86\x8C\xE3\x83\x5F\x83\x81\x81\x5B\x83\x57\x8B\xF3\x92\x86"}, "DamageMiddleBackAir", 0, 0},
    {{"\x92\x86\x8C\xE3\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x85\x92\x6E"}, "DamageMiddleBackLand", 0, 0},
    {{"\x89\x8A\x83\x5F\x83\x81\x81\x5B\x83\x57"}, "DamageFire", 0, 0},
    {{"\x83\x6D\x81\x5B\x83\x5F\x83\x81\x81\x5B\x83\x57"}, "DamageBit", 0, 0},
    {{"\x89\x8A\x82\xCC\x83\x89\x83\x93\x83\x69\x81\x5B"}, "FireRun", 0, 0},
    {{"\x83\x74\x83\x40\x83\x43\x83\x41\x83\x89\x83\x93\x91\x4F\x92\x9B"}, "FireRunStart", 0, 0},
    {{"\x93\x64\x8B\x43\x83\x5F\x83\x81\x81\x5B\x83\x57"}, "DamageElectric", 0, 0},
    {{"\x93\x64\x8B\x43\x83\x5F\x83\x81\x81\x5B\x83\x57\x8F\x49\x97\xB9"}, "DamageElectricEnd", 0, 0},
    {{"\x83\x74\x83\x40\x83\x43\x83\x41\x83\x5F\x83\x93\x83\x58"}, "FireRun", 0, 0},
    {{"\x83\x5F\x83\x81\x81\x5B\x83\x57"}, "DamageStart", 0, 0},
    {{"\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x85\x92\x6E"}, "Land", 0, 0},
    {{"\x83\x5F\x83\x81\x81\x5B\x83\x57\x83\x45\x83\x47\x83\x43\x83\x67"}, "DamageWait", 0, 0},
    {{"\x83\x6D\x81\x5B\x83\x7D\x83\x8B\x83\x45\x83\x47\x83\x43\x83\x67"}, "Wait", 0, 0},
    {{"\x95\x58\x8C\x8B"}, "Freeze", 0, 0},
    {{"\x95\x58\x8C\x8B\x89\xF0\x8F\x9C"}, "IceFlick", 0, 0},
    {{"\x82\xB5\x82\xD1\x82\xEA"}, "DamageNumb", 0, 0},
    {{"\x82\xB5\x82\xD1\x82\xEA\x89\xF1\x95\x9C"}, "DamageNumbEnd", 0, 0},
    {{"\x82\xCD\x82\xCB\x82\xC6\x82\xCE\x82\xB3\x82\xEA"}, "DamageFlick", 0, 0},
    {{"\x82\xCD\x82\xCB\x82\xC6\x82\xCE\x82\xB3\x82\xEA\x8F\x49\x97\xB9"}, "DamageFlickEnd", 0, 0},
    {{"\x8C\xE3\x93\x5D\x82\xD3\x82\xC1\x82\xC6\x82\xD1"}, "DamageWeakFront", 0, 0},
    {{"\x8C\xE3\x93\x5D\x82\xD3\x82\xC1\x82\xC6\x82\xD1\x8B\xF3\x92\x86"}, "DamageWeakFrontAir", 0, 0},
    {{"\x8C\xE3\x93\x5D\x82\xD3\x82\xC1\x82\xC6\x82\xD1\x92\x85\x92\x6E"}, "DamageWeakFrontLand", 0, 0},
    {{"\x90\x81\x82\xAB\x94\xF2\x82\xD1"}, "Blow", 0, 0},
    {{"\x8D\xC0\x82\xE8\x83\x5F\x83\x45\x83\x93"}, "DieSit", 0, 0},
    {{"\x8A\xB4\x93\x64\x83\x5F\x83\x45\x83\x93"}, "DieElectric", 0, 0},
    {{"\x89\x8A\x83\x5F\x83\x45\x83\x93"}, "DieFire", 0, 0},
    {{"\x8B\xC2\x8C\xFC\x82\xAF\x83\x5F\x83\x45\x83\x93"}, "DieOver", 0, 0},
    {{"\x98\xEB\x82\xB9\x83\x5F\x83\x45\x83\x93"}, "DieUnder", 0, 0},
    {{"\x93\xDE\x97\x8E\x83\x5F\x83\x45\x83\x93"}, "DieFall", 0, 0},
    {{"\x83\x75\x83\x89\x83\x62\x83\x4E\x83\x7A\x81\x5B\x83\x8B\x97\x8E\x89\xBA"}, "DieBlackHole", 0, 0},
    {{"\x82\xC2\x82\xD4\x82\xEA\x83\x5F\x83\x45\x83\x93"}, "DieSit", 0, 0},
    {{"\x83\x51\x81\x5B\x83\x80\x83\x49\x81\x5B\x83\x6F\x81\x5B"}, "DieOver", 0, 0},
    {{"\x83\x8C\x81\x5B\x83\x58\x95\x89\x82\xAF"}, "DieEvent", 0, 0},
    {{"\x90\x85\x92\x86\x83\x8C\x81\x5B\x83\x58\x95\x89\x82\xAF"}, "DieSwimEvent", 0, 0},
    {{"\x96\x84\x82\xDC\x82\xE8\x83\x5F\x83\x45\x83\x93"}, "DieBury", 0, 0},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x56\x83\x87\x83\x62\x83\x67\x8D\xB6"}, "TennisShotL", 0, 0},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x56\x83\x87\x83\x62\x83\x67\x89\x45"}, "TennisShotR", 0, 0},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x56\x83\x87\x83\x62\x83\x67\x92\x86"}, "TennisShotM", 0, 0},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x56\x83\x87\x83\x62\x83\x67\x8B\xF3"}, "TennisShotAir", 0, 0},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x45\x83\x47\x83\x43\x83\x67"}, "TennisWait", 0, 0},
    {{"\x83\x47\x83\x8C\x83\x81\x83\x93\x83\x67\x83\x51\x83\x62\x83\x67"}, "ElementGet", 0, 0},
    {{"\x83\x47\x83\x8C\x83\x81\x83\x93\x83\x67\x83\x51\x83\x62\x83\x67\x90\xDA\x92\x6E\x92\x86"}, "ElementGetGround", 0, 0},
    {{"\x8B\xF3\x92\x86\x82\xD0\x82\xCB\x82\xE8"}, "Spin", 0, 0},
    {{"\x92\x6E\x8F\xE3\x82\xD0\x82\xCB\x82\xE8"}, "SpinGround", 0, 0},
    {{"\x83\x41\x83\x43\x83\x58\x82\xD0\x82\xCB\x82\xE8"}, "IceSpin", 0, 0},
    {{"\x83\x41\x83\x43\x83\x58\x82\xD0\x82\xCB\x82\xE8\x88\xDA\x93\xAE"}, "IceSkateSpin", 0, 0},
    {{"\x83\x41\x83\x43\x83\x58\x82\xD0\x82\xCB\x82\xE8\x90\xC3\x8E\x7E"}, "IceSpin", 0, 0},
    {{"\x83\x6E\x83\x60\x83\x58\x83\x73\x83\x93\x8B\xF3\x92\x86"}, "BeeSpin", 0, 0},
    {{"\x83\x6E\x83\x60\x83\x58\x83\x73\x83\x93"}, "BeeSpinGround", 0, 0},
    {{"\x89\x5F\x83\x58\x83\x73\x83\x93\x8B\xF3\x92\x86"}, "CloudSpin", 0, 0},
    {{"\x89\x5F\x83\x58\x83\x73\x83\x93"}, "CloudSpinGround", 0, 0},
    {{"\x83\x41\x83\x43\x83\x58\x82\xD0\x82\xCB\x82\xE8\x8B\xF3\x92\x86"}, "IceSpinAir", 0, 0},
    {{"\x83\x58\x83\x50\x81\x5B\x83\x67\x83\x41\x83\x4E\x83\x5A\x83\x8B\x83\x57\x83\x83\x83\x93\x83\x76"}, "IceJump", 0, 0},
    {{"\x83\x58\x83\x50\x81\x5B\x83\x67\x83\x57\x83\x83\x83\x93\x83\x76\x32"}, "IceJump2", 0, 0},
    {{"\x83\x58\x83\x50\x81\x5B\x83\x67\x83\x57\x83\x83\x83\x93\x83\x76\x33"}, "IceJump3", 0, 0},
    {{"\x83\x58\x83\x50\x81\x5B\x83\x67\x92\x85\x92\x6E"}, "IceJumpLand", 0, 0},
    {{"\x83\x58\x83\x50\x81\x5B\x83\x67\x90\xC3\x8E\x7E\x92\x85\x92\x6E"}, "IceJumpStopLand", 0, 0},
    {{"\x83\x60\x83\x85\x81\x5B\x83\x75\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B\x95\xC7\x83\x71\x83\x62\x83\x67"}, "TubeSlideWallHit", 0, 0},
    {{"\x83\x60\x83\x85\x81\x5B\x83\x75\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B\x92\x85\x92\x6E"}, "TubeSlideLand", 0, 0},
    {{"\x83\x60\x83\x85\x81\x5B\x83\x75\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B\x83\x57\x83\x83\x83\x93\x83\x76"}, "TubeSlideJump", 0, 0},
    {{"\x83\x60\x83\x85\x81\x5B\x83\x75\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B\x83\x5F\x83\x81\x81\x5B\x83\x57"}, "TubeSlideDamage", 0, 0},
    {{"\x83\x60\x83\x85\x81\x5B\x83\x75\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x85\x92\x6E"}, "TubeSlideDamageLand", 0, 0},
    {{"\x8C\xA9\x82\xE9"}, "Watch", 0, 0},
    {{"\x83\x8C\x81\x5B\x83\x58\x8C\xA9\x82\xE9"}, "Watch", 0, 0},
    {{"\x93\x79\x8A\xC7\x83\x4E\x83\x43\x83\x62\x83\x4E\x83\x57\x83\x83\x83\x93\x83\x76"}, "EarthenPipeJumpQuick", 0, 0},
    {{"\x83\x53\x83\x8D\x8A\xE2\x89\xF0\x8F\x9C\x83\x57\x83\x83\x83\x93\x83\x76"}, "RollingRockEnd", 0, 0},
    {{"\x83\x53\x83\x8D\x8A\xE2\x8F\xD5\x93\xCB\x89\xF0\x8F\x9C\x83\x57\x83\x83\x83\x93\x83\x76"}, "RollingRockHitEnd", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x97\xA3\x92\x45\x83\x57\x83\x83\x83\x93\x83\x76"}, "YoshiGetOff", 0, 0},
    {{"\x90\x48\x82\xD7\x82\xE9"}, "Wait", 0, 0},
    {{"\x8A\x9A\x82\xDE"}, "Wait", 0, 0},
    {{"\x88\xF9\x82\xDD\x89\xBA\x82\xB7"}, "Wait", 0, 0},
    {{"\x83\x74\x83\x62\x83\x4E\x90\xDA\x8B\xDF"}, "Wait", 0, 0},
    {{"\x83\x74\x83\x62\x83\x4E\x82\xD4\x82\xE7\x89\xBA\x82\xAA\x82\xE8"}, "Wait", 0, 0},
    {{"\x83\x74\x83\x62\x83\x4E\x83\x57\x83\x83\x83\x93\x83\x76"}, "Wait", 0, 0},
    {{"\x93\xA5\x82\xF1\x92\xA3\x82\xE8"}, "Wait", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x88\xF8\x82\xC1\x92\xA3\x82\xE8"}, "Wait", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x88\xF8\x82\xC1\x92\xA3\x82\xE8\x95\xE0\x82\xAB"}, "Wait", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x88\xF8\x82\xC1\x92\xA3\x82\xE8\x8F\x49\x97\xB9"}, "Wait", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x88\xF8\x82\xC1\x92\xA3\x82\xE8\x90\x48\x82\xD7\x8F\x49\x97\xB9"}, "Wait", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x8F\xE6\x8E\xD4"}, "Throw", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x83\x58\x83\x73\x83\x93\x8F\xE6\x8E\xD4"}, "Throw", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x83\x56\x83\x87\x83\x62\x83\x67"}, "Wait", 0, 0},
    {{"\x83\x6F\x83\x8B\x81\x5B\x83\x93\x96\x63\x82\xE7\x82\xDE"}, "Wait", 0, 0},
    {{"\x83\x6F\x83\x8B\x81\x5B\x83\x93\x91\xD2\x8B\x40"}, "Wait", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x83\x5F\x83\x62\x83\x56\x83\x85"}, "RunDash", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x83\x89\x83\x93"}, "Run", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x95\xE0\x8D\x73"}, "Walk", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x93\xDD\x8D\x73"}, "WalkSoft", 0, 0},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x83\x57\x83\x83\x83\x93\x83\x76"}, "Jump", 0, 0},
    {{"\x8D\x9C\x83\x6F\x83\x89\x83\x6F\x83\x89\x8A\x4A\x8E\x6E"}, "BoneBreakStart", 0, 0},
    {{"\x8D\x9C\x83\x6F\x83\x89\x83\x6F\x83\x89\x8C\x70\x91\xB1"}, "BoneBreakWait", 0, 0},
    {{"\x8D\x9C\x83\x6F\x83\x89\x83\x6F\x83\x89\x8F\x49\x97\xB9"}, "BoneBreakEnd", 0, 0},
    {{"\x83\x58\x83\x65\x81\x5B\x83\x57\x83\x43\x83\x93\x41"}, "StageStartGround", 0, 0},
    {{"\x83\x58\x83\x65\x81\x5B\x83\x57\x83\x43\x83\x93\x42"}, "LandScenarioStart", 0, 0},
    {{"\x83\x45\x83\x48\x81\x5B\x83\x4E\x83\x43\x83\x93"}, "GoThrough", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x31\x5D"}, "DemoGetPower", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x89\xEF\x98\x62\x31\x5D"}, "DemoGetPower", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x32\x5D"}, "DemoGetPower", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x89\xEF\x98\x62\x32\x5D"}, "DemoGetPower", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x33\x5D"}, "DemoGetPower", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x89\xEF\x98\x62\x33\x5D"}, "DemoGetPower", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x34\x5D"}, "DemoGetPower", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x35\x5D"}, "DemoGetPower", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x89\xEF\x98\x62\x34\x5D"}, "DemoGetPower", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x36\x5D"}, "DemoGetPower", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x37\x5D"}, "DemoGetPower", 0, 0},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x38\x5D"}, "DemoGetPower", 0, 0},
    {{"\x83\x8C\x81\x5B\x83\x58\x8A\x4A\x8E\x6E"}, "RaceStart", 0, 0},
    {{"\x83\x8C\x81\x5B\x83\x58\x83\x4E\x83\x89\x83\x45\x83\x60\x83\x93\x83\x4F\x8A\x4A\x8E\x6E"}, "RaceStartCrouch", 0, 0},
    {{"\x83\x53\x81\x5B\x83\x58\x83\x67\x83\x8C\x81\x5B\x83\x58\x8A\x4A\x8E\x6E"}, "RaceStartGhost", 0, 0},
    {{"\x83\x53\x81\x5B\x83\x58\x83\x67\x8F\x9F\x97\x98"}, "WinGhost", 0, 0},
    {{"\x83\x53\x81\x5B\x83\x58\x83\x67\x8F\x6F\x8C\xBB"}, "AppearGhost", 0, 0},
    {{""}, "", 0, 0},
};

XanimeGroupInfo playerGhostAnimeGroups[] = {
    {{"\x8A\xEE\x96\x7B"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x8A\xEE\x96\x7B"}, 1.0f, 2, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x8A\xEE\x96\x7B"}, 1.0f, 30, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x65\x83\x6A\x83\x58\x8A\xEE\x96\x7B"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xE2\x8D\xB6\x89\x45\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xE2\x91\x4F\x8C\xE3\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xE2\x8D\xB6\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xE2\x89\x45\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xE2\x91\x4F\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xE2\x8C\xE3\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x89\x83\x93"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xE0\x8D\x73"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x93\xDD\x8D\x73"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x81\x83\x5E\x83\x8B\x83\x5F\x83\x62\x83\x56\x83\x85"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x5F\x83\x62\x83\x56\x83\x85\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x94\xF2\x82\xD1\x82\xB7\x82\xB3\x82\xE8\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x56\x83\x87\x81\x5B\x83\x67\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x96\x84\x82\xDC\x82\xE8\x95\xE0\x8D\x73"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x96\x84\x82\xDC\x82\xE8\x83\x57\x83\x83\x83\x93\x83\x76\x41"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x96\x84\x82\xDC\x82\xE8\x83\x57\x83\x83\x83\x93\x83\x76\x42"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x96\x84\x82\xDC\x82\xE8\x92\x45\x8F\x6F\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8B\x74\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x75\x83\x8C\x81\x5B\x83\x4C"}, 1.0f, 4, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x75\x83\x8C\x81\x5B\x83\x4C\x8A\x8A\x82\xE8\x8F\xB0"}, 4.0f, 4, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x5E\x81\x5B\x83\x93\x83\x75\x83\x8C\x81\x5B\x83\x4C"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x5E\x81\x5B\x83\x93\x83\x75\x83\x8C\x81\x5B\x83\x4C\x8A\x8A\x82\xE8\x8F\xB0"}, 3.0f, 4, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xE0\x8D\x73\x90\xA7\x93\xAE\x83\x75\x83\x8C\x81\x5B\x83\x4C"}, 1.0f, 4, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\x58\x8F\xE3\x8A\xB5\x90\xAB\x91\x96\x8D\x73"}, 1.0f, 16, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\x58\x8F\xE3\x97\xCD\x8D\x73\x8D\xB6"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\x58\x8F\xE3\x97\xCD\x8D\x73\x89\x45"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\x58\x8F\xE3\x90\xD8\x91\xD6\x8D\xB6"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\x58\x8F\xE3\x90\xD8\x91\xD6\x89\x45"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\x58\x8F\xE3\x8C\xE3\x8D\x73\x8D\xB6"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\x58\x8F\xE3\x8C\xE3\x8D\x73\x89\x45"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\x58\x8F\xE3\x83\x5E\x81\x5B\x83\x93"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x93\xC1\x8E\xEA\x83\x45\x83\x47\x83\x43\x83\x67\x31\x41"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x93\xC1\x8E\xEA\x83\x45\x83\x47\x83\x43\x83\x67\x31\x42"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\xED\x93\xAC\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 30, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x99\xB3\x99\xB4\x91\x4F\x90\x69"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x42"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x43"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x5E\x81\x5B\x83\x93\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x83\x8A\x81\x5B\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x82\xD3\x82\xDD\x31"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x82\xD3\x82\xDD\x32"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x82\xD3\x82\xDD\x33"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x88\xF8\x82\xAB\x96\xDF\x82\xB5"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x88\xF8\x82\xAB\x96\xDF\x82\xB5\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7C\x83\x62\x83\x68\x83\x8F\x81\x5B\x83\x76\x8A\x4A\x8E\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7C\x83\x62\x83\x68\x83\x8F\x81\x5B\x83\x76\x8F\x49\x97\xB9"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x4A\x83\x43\x83\x89\x83\x75\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x83\x7B\x81\x5B\x83\x8B\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8B\xF3\x92\x86\x88\xEA\x89\xF1\x93\x5D"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x50\x83\x4C\x83\x88"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x50\x83\x4C\x83\x88\x92\x45\x8F\x6F"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x96\x84\x82\xDC\x82\xE8"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x96\x84\x82\xDC\x82\xE8\x92\x45\x8F\x6F"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x83\x57\x83\x83\x83\x93\x83\x76\x41"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x83\x57\x83\x83\x83\x93\x83\x76\x42"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x82\xD3\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76\x41"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x82\xD3\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76\x42"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x88\xDA\x93\xAE\x41"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x88\xDA\x93\xAE\x42"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x83\x6E\x83\x43\x83\x57\x83\x83\x83\x93\x83\x76\x41"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x83\x6E\x83\x43\x83\x57\x83\x83\x83\x93\x83\x76\x42"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x95\xC7\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x8A\x4A\x8E\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\x9D\x82\xC6\x82\xD1"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x8A\x4A\x8E\x6E"}, 1.0f, 2, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x8A\x8A\x82\xE8"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x8F\x87\x8A\x8A\x82\xE8"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76\x8B\x74\x8A\x8A\x82\xE8"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x8A\x83\x62\x83\x76\x83\x41\x83\x62\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x83\x58\x83\x8A\x83\x62\x83\x76\x83\x41\x83\x62\x83\x76\x8F\x80\x94\xF5"}, 1.0f, 4, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x83\x58\x83\x8A\x83\x62\x83\x76\x83\x41\x83\x62\x83\x76"}, 1.0f, 2, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x8A\x4A\x8E\x6E"}, 1.0f, 2, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x97\x8E\x89\xBA"}, 1.0f, 15, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x92\x85\x92\x6E"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x92\x85\x92\x6E\x42"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x81\x5B\x83\x68\x92\x85\x92\x6E"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xB7\x82\xD7\x82\xE8\x92\x85\x92\x6E"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x56\x83\x87\x81\x5B\x83\x67\x92\x85\x92\x6E"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x94\xF2\x8D\x73\x92\x86"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x94\xF2\x8D\x73\x92\x86\x96\xB3\x93\xFC\x97\xCD"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x95\xC7\x82\xAD\x82\xC1\x82\xC2\x82\xAB"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x95\xC7\x82\xAD\x82\xC1\x82\xC2\x82\xAB\x92\x86"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x95\xC7\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x99\xB3\x99\xB4\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x99\xB3\x99\xB4\x91\x4F\x90\x69"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x99\xB3\x99\xB4\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x89\xD4\x88\xDA\x93\xAE"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x8A\x4A\x8E\x6E"}, 1.0f, 2, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x95\xC7\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x94\xF2\x8D\x73\x8A\x4A\x8E\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x94\xF2\x8D\x73"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x94\xF2\x8D\x73\x8D\xC4\x8A\x4A"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x90\xC3\x8E\x7E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x89\xF0\x8F\x9C"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x83\x58\x83\x73\x83\x93"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x93\x44\x92\xE1\x91\xAC\x95\xE0\x8D\x73"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x93\x44\x8D\x82\x91\xAC\x95\xE0\x8D\x73"}, 2.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xC7\x82\xAD\x82\xC1\x82\xC2\x82\xAB"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xC7\x82\xB7\x82\xD7\x82\xE8"}, 1.0f, 10, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xC7\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xC7\x89\x9F\x82\xB5"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xC7\x8D\xB6\x95\xE0\x82\xAB"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xC7\x89\x45\x95\xE0\x82\xAB"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xC7\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x91\x4F\x95\xC7\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xC7\x83\x71\x83\x62\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xC7\x83\x71\x83\x62\x83\x67\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xC7\x82\xCD\x82\xB6\x82\xAB"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x5F\x83\x81\x81\x5B\x83\x57"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x5F\x83\x81\x81\x5B\x83\x57\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6D\x81\x5B\x83\x7D\x83\x8B\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\x58\x8C\x8B"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\x58\x8C\x8B\x89\xF0\x8F\x9C"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xB5\x82\xD1\x82\xEA"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xB5\x82\xD1\x82\xEA\x89\xF1\x95\x9C"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x52\x83\x43\x83\x93\x83\x51\x83\x62\x83\x67"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x70\x83\x93\x83\x60"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8B\xF3\x83\x70\x83\x93\x83\x60"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xB6\x83\x70\x83\x93\x83\x60"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xB6\x8B\xF3\x83\x70\x83\x93\x83\x60"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x89\x83\x93\x83\x6A\x83\x93\x83\x4F\x83\x4C\x83\x62\x83\x4E"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x41\x83\x62\x83\x70\x81\x5B\x83\x70\x83\x93\x83\x60"}, 1.0f, 3, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x83\x41\x83\x62\x83\x70\x81\x5B"}, 1.0f, 3, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x54\x83\x7D\x81\x5B\x83\x5C\x83\x8B\x83\x67"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x51\x81\x5B\x83\x80\x83\x49\x81\x5B\x83\x6F\x81\x5B"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xC2\x82\xD4\x82\xEA"}, 0.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xC2\x82\xD4\x82\xEA\x95\x9C\x8B\x41"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xC2\x82\xD4\x82\xEA\x89\xF0\x8F\x9C"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8B\xF3\x93\x5D"}, 4.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xE2\x82\xB7\x82\xD7\x82\xE8\x8F\xE3\x8C\xFC\x82\xAB\x82\xA4\x82\xC2\x82\xD4\x82\xB9"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xE2\x82\xB7\x82\xD7\x82\xE8\x89\xBA\x8C\xFC\x82\xAB\x82\xA0\x82\xA8\x82\xDE\x82\xAF"}, 2.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8C\xFC\x82\xA9\x82\xA2\x95\x97\x91\x96\x82\xE8"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8C\xFC\x82\xA9\x82\xA2\x95\x97\x82\xD3\x82\xF1\x82\xCE\x82\xE8"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x60\x83\x85\x81\x5B\x83\x75\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B"}, 1.0f, 15, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x60\x83\x85\x81\x5B\x83\x75\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B\x95\xC7\x83\x71\x83\x62\x83\x67"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x60\x83\x85\x81\x5B\x83\x75\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x60\x83\x85\x81\x5B\x83\x75\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x60\x83\x85\x81\x5B\x83\x75\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B\x83\x5F\x83\x81\x81\x5B\x83\x57"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x60\x83\x85\x81\x5B\x83\x75\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x85\x92\x6E"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x4F\x83\x89\x83\x43\x83\x6F\x81\x5B\x83\x68\x8A\x8A\x8B\xF3\x57\x61\x69\x74"}, 1.0f, 20, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x4F\x83\x89\x83\x43\x83\x6F\x81\x5B\x83\x68\x8A\x8A\x8B\xF3\x44\x6F\x77\x6E"}, 1.0f, 20, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x4F\x83\x89\x83\x43\x83\x6F\x81\x5B\x83\x68\x8A\x8A\x8B\xF3\x46\x61\x73\x74"}, 1.0f, 20, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x4F\x83\x89\x83\x43\x83\x6F\x81\x5B\x83\x68\x8A\x8A\x8B\xF3\x55\x70"}, 1.0f, 10, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x5E\x83\x7D\x83\x52\x83\x8D\x88\xDA\x93\xAE"}, 1.0f, 15, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x5E\x83\x7D\x83\x52\x83\x8D\x82\xB5\x82\xE1\x82\xAA\x82\xDD"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x89\x83\x43\x83\x5F\x81\x5B\x90\x4B"}, 2.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xBB\x82\xCC\x8F\xEA\x91\xAB\x93\xA5\x82\xDD"}, 1.0f, 2, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xBB\x82\xCC\x8F\xEA\x91\xAB\x93\xA5\x82\xDD\x8F\xE3\x94\xBC\x90\x67"}, 1.0f, 4, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x88\xEA\x91\x7E\x82\xAB"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x8F\xE3\x88\xEA\x91\x7E\x82\xAB"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x90\xF6\x82\xE8"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x57\x83\x83\x83\x93\x83\x76\x83\x5F\x83\x43\x83\x75"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x57\x83\x83\x83\x93\x83\x76\x83\x5F\x83\x43\x83\x75\x89\xF1\x93\x5D"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x8F\xE3\x8F\xB8\x8C\xC4\x8B\x7A"}, 1.0f, 16, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x90\x85\x96\xCA\x8F\x89\x8A\xFA\x88\xDA\x93\xAE"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x8F\xE3\x83\x58\x83\x73\x83\x93"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x58\x83\x73\x83\x93"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x58\x83\x73\x83\x93\x88\xDA\x93\xAE"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x8F\xE3\x83\x58\x83\x73\x83\x93\x88\xDA\x93\xAE"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x5F\x83\x81\x81\x5B\x83\x57"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x5F\x83\x81\x81\x5B\x83\x57\x8B\x54"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x86"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x8F\xE3\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x86"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x8F\xE3\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x85\x90\x85"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x5F\x83\x45\x83\x93"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x5E\x81\x5B\x83\x93\x8D\xB6"}, 1.0f, 20, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x5E\x81\x5B\x83\x93\x89\x45"}, 1.0f, 20, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x5E\x81\x5B\x83\x93\x89\xBA"}, 1.0f, 20, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x57\x83\x46\x83\x62\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x57\x83\x46\x83\x62\x83\x67\x8A\x4A\x8E\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x57\x83\x46\x83\x62\x83\x67\x8F\x49\x97\xB9"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x8B\x54\x93\x8A\x82\xB0"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x95\xC7\x83\x71\x83\x62\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x57\x83\x46\x83\x62\x83\x67\x95\xC7\x83\x5E\x81\x5B\x83\x93"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x97\xA4\x82\xA4\x82\xBF\x82\xA0\x82\xB0"}, 1.0f, 16, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x94\xF2\x82\xD1\x8D\x9E\x82\xDD\x8F\x80\x94\xF5"}, 1.0f, 20, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x94\xF2\x82\xD1\x8D\x9E\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8C\xE3\x95\xFB\x94\xF2\x82\xD1\x8D\x9E\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x94\xF2\x82\xD1\x8D\x9E\x82\xDD\x8E\xB8\x94\x73\x92\x85\x92\x6E"}, 1.0f, 15, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x94\xF2\x82\xD1\x8D\x9E\x82\xDD\x8E\xB8\x94\x73\x89\xF1\x93\x5D\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x89\x6A\x83\x58\x83\x73\x83\x93\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x8A\x83\x93\x83\x4F\x83\x5F\x83\x62\x83\x56\x83\x85"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x8A\x83\x93\x83\x4F\x83\x5F\x83\x62\x83\x56\x83\x85\x8F\x80\x94\xF5"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x4A\x83\x81\x8E\x9D\x82\xBF\x83\x8A\x83\x93\x83\x4F\x8F\x80\x94\xF5"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x4A\x83\x81\x8E\x9D\x82\xBF\x83\x8A\x83\x93\x83\x4F"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x92\x85\x92\x6E\x43"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x92\x85\x92\x6E\x83\x5E\x81\x5B\x83\x93"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x92\x85\x92\x6E\x95\x9D\x82\xC6\x82\xD1"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x8A\x4A\x8E\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x8F\x49\x97\xB9"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xE2\x82\xB7\x82\xD7\x82\xE8\x8F\xE3\x8C\xFC\x82\xAB\x8F\x49\x97\xB9"}, 1.0f, 4, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xE2\x82\xB7\x82\xD7\x82\xE8\x89\xBA\x8C\xFC\x82\xAB\x8F\x49\x97\xB9"}, 1.0f, 4, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xD0\x82\xEB\x82\xA2"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xD0\x82\xEB\x82\xA2\x8B\xF3\x92\x86"}, 1.0f, 3, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xD0\x82\xEB\x82\xA2\x83\x4E\x83\x43\x83\x62\x83\x4E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xD0\x82\xEB\x82\xA2\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x4A\x83\x75\x94\xB2\x82\xAB"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x4A\x83\x75\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x68\x83\x8A\x83\x8B\x82\xD0\x82\xEB\x82\xA2"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x68\x83\x8A\x83\x8B\x82\xD0\x82\xEB\x82\xA2\x83\x4E\x83\x43\x83\x62\x83\x4E"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x68\x83\x8A\x83\x8B\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x93\x83\x7D\x81\x5B\x93\x8A\x82\xB0\x89\xF1\x93\x5D\x92\x86"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x93\x83\x7D\x81\x5B\x93\x8A\x82\xB0\x83\x8A\x83\x8A\x81\x5B\x83\x58"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x93\x8A\x82\xB0"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x83\x40\x83\x43\x83\x41\x93\x8A\x82\xB0"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x97\xBC\x8E\xE8\x93\x8A\x82\xB0"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x83\x40\x83\x43\x83\x41\x83\x58\x83\x73\x83\x93"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x83\x40\x83\x43\x83\x41\x83\x58\x83\x73\x83\x93\x8B\xF3\x92\x86"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8A\x52\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x8A\x4A\x8E\x6E"}, 1.0f, 5, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8A\x52\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x92\x86"}, 1.0f, 5, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8A\x52\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x8F\x49\x97\xB9"}, 1.0f, 4, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8A\x52\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x8F\x49\x97\xB9\x8D\xE2"}, 1.0f, 4, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8A\x52\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x8D\x7E\x82\xE8"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8A\x52\x82\xD3\x82\xF1\x82\xCE\x82\xE8"}, 1.0f, 2, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x95\xE0\x82\xAB"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xAA\x82\xF1\x82\xCE\x82\xE8\x91\x96\x82\xE8"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x95\xA0\x82\xCE\x82\xA2\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x4B\x8A\x8A\x82\xE8\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x91\x4F\x95\xFB\x8F\xAC\x83\x5F\x83\x81\x81\x5B\x83\x57"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8C\xE3\x95\xFB\x8F\xAC\x83\x5F\x83\x81\x81\x5B\x83\x57"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8B\xAD\x90\xA7\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x77\x83\x8A\x83\x52\x83\x76\x83\x5E\x81\x5B\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x92\x86\x83\x5F\x83\x81\x81\x5B\x83\x57"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x92\x86\x83\x5F\x83\x81\x81\x5B\x83\x57\x8B\xF3\x92\x86"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x92\x86\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x92\x86\x8C\xE3\x83\x5F\x83\x81\x81\x5B\x83\x57"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x92\x86\x8C\xE3\x83\x5F\x83\x81\x81\x5B\x83\x57\x8B\xF3\x92\x86"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x92\x86\x8C\xE3\x83\x5F\x83\x81\x81\x5B\x83\x57\x92\x85\x92\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6D\x81\x5B\x83\x5F\x83\x81\x81\x5B\x83\x57"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x89\x8A\x83\x5F\x83\x81\x81\x5B\x83\x57"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x89\x8A\x82\xCC\x83\x89\x83\x93\x83\x69\x81\x5B"}, 8.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x83\x40\x83\x43\x83\x41\x83\x89\x83\x93\x91\x4F\x92\x9B"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x93\x64\x8B\x43\x83\x5F\x83\x81\x81\x5B\x83\x57"}, 1.0f, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x93\x64\x8B\x43\x83\x5F\x83\x81\x81\x5B\x83\x57\x8F\x49\x97\xB9"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x83\x40\x83\x43\x83\x41\x83\x5F\x83\x93\x83\x58"}, 6.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8C\xE3\x93\x5D\x82\xD3\x82\xC1\x82\xC6\x82\xD1"}, 1.0f, 2, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8C\xE3\x93\x5D\x82\xD3\x82\xC1\x82\xC6\x82\xD1\x8B\xF3\x92\x86"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8C\xE3\x93\x5D\x82\xD3\x82\xC1\x82\xC6\x82\xD1\x92\x85\x92\x6E"}, 1.0f, 2, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x81\x82\xAB\x94\xF2\x82\xD1"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\xC0\x82\xE8\x83\x5F\x83\x45\x83\x93"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8A\xB4\x93\x64\x83\x5F\x83\x45\x83\x93"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x89\x8A\x83\x5F\x83\x45\x83\x93"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8B\xC2\x8C\xFC\x82\xAF\x83\x5F\x83\x45\x83\x93"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x98\xEB\x82\xB9\x83\x5F\x83\x45\x83\x93"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x93\xDE\x97\x8E\x83\x5F\x83\x45\x83\x93"}, 1.0f, 20, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x75\x83\x89\x83\x62\x83\x4E\x83\x7A\x81\x5B\x83\x8B\x97\x8E\x89\xBA"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x8C\x81\x5B\x83\x58\x95\x89\x82\xAF"}, 1.0f, 15, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x85\x92\x86\x83\x8C\x81\x5B\x83\x58\x95\x89\x82\xAF"}, 1.0f, 15, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x96\x84\x82\xDC\x82\xE8\x83\x5F\x83\x45\x83\x93"}, 1.0f, 30, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xC2\x82\xD4\x82\xEA\x83\x5F\x83\x45\x83\x93"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xCD\x82\xCB\x82\xC6\x82\xCE\x82\xB3\x82\xEA"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x82\xCD\x82\xCB\x82\xC6\x82\xCE\x82\xB3\x82\xEA\x8F\x49\x97\xB9"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x56\x83\x87\x83\x62\x83\x67\x8D\xB6"}, 1.25f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x56\x83\x87\x83\x62\x83\x67\x89\x45"}, 1.25f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x56\x83\x87\x83\x62\x83\x67\x92\x86"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x56\x83\x87\x83\x62\x83\x67\x8B\xF3"}, 1.25f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x45\x83\x47\x83\x43\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x47\x83\x8C\x83\x81\x83\x93\x83\x67\x83\x51\x83\x62\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x47\x83\x8C\x83\x81\x83\x93\x83\x67\x83\x51\x83\x62\x83\x67\x90\xDA\x92\x6E\x92\x86"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8B\xF3\x92\x86\x82\xD0\x82\xCB\x82\xE8"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x92\x6E\x8F\xE3\x82\xD0\x82\xCB\x82\xE8"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x83\x58\x83\x73\x83\x93"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6E\x83\x60\x83\x58\x83\x73\x83\x93\x8B\xF3\x92\x86"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x89\x5F\x83\x58\x83\x73\x83\x93\x8B\xF3\x92\x86"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x89\x5F\x83\x58\x83\x73\x83\x93"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x41\x83\x43\x83\x58\x82\xD0\x82\xCB\x82\xE8"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x41\x83\x43\x83\x58\x82\xD0\x82\xCB\x82\xE8\x88\xDA\x93\xAE"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x41\x83\x43\x83\x58\x82\xD0\x82\xCB\x82\xE8\x90\xC3\x8E\x7E"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x41\x83\x43\x83\x58\x82\xD0\x82\xCB\x82\xE8\x8B\xF3\x92\x86"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x50\x81\x5B\x83\x67\x83\x41\x83\x4E\x83\x5A\x83\x8B\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x50\x81\x5B\x83\x67\x83\x57\x83\x83\x83\x93\x83\x76\x32"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x50\x81\x5B\x83\x67\x83\x57\x83\x83\x83\x93\x83\x76\x33"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x50\x81\x5B\x83\x67\x92\x85\x92\x6E"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x50\x81\x5B\x83\x67\x90\xC3\x8E\x7E\x92\x85\x92\x6E"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8C\xA9\x82\xE9"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x8C\x81\x5B\x83\x58\x8C\xA9\x82\xE9"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x93\x79\x8A\xC7\x83\x4E\x83\x43\x83\x62\x83\x4E\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x4A\x83\x8A\x83\x4A\x83\x8A\x8C\xC0\x8A\x45"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x97\xA3\x92\x45\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x90\x48\x82\xD7\x82\xE9"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8A\x9A\x82\xDE"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x88\xF9\x82\xDD\x89\xBA\x82\xB7"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x83\x62\x83\x4E\x90\xDA\x8B\xDF"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x83\x62\x83\x4E\x82\xD4\x82\xE7\x89\xBA\x82\xAA\x82\xE8"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x74\x83\x62\x83\x4E\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x93\xA5\x82\xF1\x92\xA3\x82\xE8"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x88\xF8\x82\xC1\x92\xA3\x82\xE8"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x88\xF8\x82\xC1\x92\xA3\x82\xE8\x95\xE0\x82\xAB"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x88\xF8\x82\xC1\x92\xA3\x82\xE8\x8F\x49\x97\xB9"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x88\xF8\x82\xC1\x92\xA3\x82\xE8\x90\x48\x82\xD7\x8F\x49\x97\xB9"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x8F\xE6\x8E\xD4"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x83\x58\x83\x73\x83\x93\x8F\xE6\x8E\xD4"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x83\x56\x83\x87\x83\x62\x83\x67"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6F\x83\x8B\x81\x5B\x83\x93\x96\x63\x82\xE7\x82\xDE"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x6F\x83\x8B\x81\x5B\x83\x93\x91\xD2\x8B\x40"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x83\x5F\x83\x62\x83\x56\x83\x85"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x83\x89\x83\x93"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x95\xE0\x8D\x73"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x93\xDD\x8D\x73"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x88\x83\x62\x83\x56\x81\x5B\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\x9C\x83\x6F\x83\x89\x83\x6F\x83\x89\x8A\x4A\x8E\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\x9C\x83\x6F\x83\x89\x83\x6F\x83\x89\x8C\x70\x91\xB1"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x8D\x9C\x83\x6F\x83\x89\x83\x6F\x83\x89\x8F\x49\x97\xB9"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x65\x81\x5B\x83\x57\x83\x43\x83\x93\x41"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x65\x81\x5B\x83\x57\x83\x43\x83\x93\x42"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x45\x83\x48\x81\x5B\x83\x4E\x83\x43\x83\x93"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x31\x5D"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x89\xEF\x98\x62\x31\x5D"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x32\x5D"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x89\xEF\x98\x62\x32\x5D"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x33\x5D"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x89\xEF\x98\x62\x33\x5D"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x34\x5D"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x35\x5D"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x89\xEF\x98\x62\x34\x5D"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x36\x5D"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x37\x5D"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x58\x83\x73\x83\x93\x83\x51\x83\x62\x83\x67\x5B\x83\x66\x83\x82\x38\x5D"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x8C\x81\x5B\x83\x58\x8A\x4A\x8E\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x8C\x81\x5B\x83\x58\x83\x4E\x83\x89\x83\x45\x83\x60\x83\x93\x83\x4F\x8A\x4A\x8E\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x53\x81\x5B\x83\x58\x83\x67\x83\x8C\x81\x5B\x83\x58\x8A\x4A\x8E\x6E"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x53\x81\x5B\x83\x58\x83\x67\x8F\x9F\x97\x98"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x53\x81\x5B\x83\x58\x83\x67\x8F\x6F\x8C\xBB"}, 1.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x53\x83\x8D\x8A\xE2\x89\xF0\x8F\x9C\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{"\x83\x53\x83\x8D\x8A\xE2\x8F\xD5\x93\xCB\x89\xF0\x8F\x9C\x83\x57\x83\x83\x83\x93\x83\x76"}, 1.0f, 6, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
    {{""}, 0.0f, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, {nullptr, nullptr, nullptr, nullptr}, {0.0f, 0.0f, 0.0f, 0.0f}, 0, nullptr},
};

XanimeOfsInfo playerGhostAnimeOffsets[] = {
    {{"\x83\x5F\x83\x81\x81\x5B\x83\x57"}, 0.0f, 10.0f, 10.0f, 1},
    {{"\x8D\xE2\x82\xB7\x82\xD7\x82\xE8\x89\xBA\x8C\xFC\x82\xAB\x82\xA0\x82\xA8\x82\xDE\x82\xAF"}, 0.0f, 2.0f, 2.0f, 1},
    {{"\x8D\xE2\x82\xB7\x82\xD7\x82\xE8\x8F\xE3\x8C\xFC\x82\xAB\x82\xA4\x82\xC2\x82\xD4\x82\xB9"}, 0.0f, 2.0f, 2.0f, 1},
    {{"\x83\x5E\x81\x5B\x83\x93\x83\x75\x83\x8C\x81\x5B\x83\x4C"}, 0.0f, 20.0f, 20.0f, 1},
    {{"\x83\x57\x83\x83\x83\x93\x83\x76"}, 0.0f, 24.0f, 24.0f, 1},
    {{"\x95\xC7\x83\x71\x83\x62\x83\x67"}, 0.0f, 25.0f, 25.0f, 1},
    {{"\x95\xC7\x83\x71\x83\x62\x83\x67\x92\x85\x92\x6E"}, 0.0f, 67.0f, 0.0f, 0},
    {{"\x83\x4A\x83\x75\x94\xB2\x82\xAB"}, 0.0f, 49.0f, 49.0f, 1},
    {{"\x83\x68\x83\x8A\x83\x8B\x82\xD0\x82\xEB\x82\xA2"}, -1.0f, -1.0f, -1.0f, 1},
    {{"\x83\x68\x83\x8A\x83\x8B\x82\xD0\x82\xEB\x82\xA2\x83\x4E\x83\x43\x83\x62\x83\x4E"}, -1.0f, -1.0f, -1.0f, 1},
    {{"\x97\x8E\x89\xBA"}, 0.0f, 29.0f, 29.0f, 1},
    {{"\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x8A\x4A\x8E\x6E"}, 0.0f, 13.0f, 13.0f, 1},
    {{"\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76"}, 0.0f, 9.0f, 9.0f, 1},
    {{"\x83\x58\x83\x73\x83\x93\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76"}, 0.0f, 19.0f, 19.0f, 1},
    {{"\x83\x6E\x83\x60\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x95\xC7\x92\x85\x92\x6E"}, 0.0f, 20.0f, 20.0f, 0},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x82\xD3\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76\x41"}, 10.0f, 50.0f, 0.0f, 1},
    {{"\x83\x7A\x83\x62\x83\x70\x81\x5B\x82\xD3\x82\xDD\x83\x57\x83\x83\x83\x93\x83\x76\x42"}, 10.0f, 50.0f, 0.0f, 1},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x56\x83\x87\x83\x62\x83\x67\x8D\xB6"}, 4.0f, 26.0f, 26.0f, 0},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x56\x83\x87\x83\x62\x83\x67\x89\x45"}, 4.0f, 26.0f, 26.0f, 0},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x56\x83\x87\x83\x62\x83\x67\x92\x86"}, 3.0f, 38.0f, 38.0f, 0},
    {{"\x83\x65\x83\x6A\x83\x58\x83\x56\x83\x87\x83\x62\x83\x67\x8B\xF3"}, 5.0f, 26.0f, 26.0f, 0},
    {{"\x93\x79\x8A\xC7\x83\x4E\x83\x43\x83\x62\x83\x4E\x83\x57\x83\x83\x83\x93\x83\x76"}, -1.0f, -1.0f, -1.0f, 1},
    {{"\x82\xBB\x82\xCC\x8F\xEA\x91\xAB\x93\xA5\x82\xDD"}, 0.0f, 17.0f, 8.0f, 0},
    {{"\x83\x75\x83\x8C\x81\x5B\x83\x4C"}, 0.0f, 2.0f, 0.0f, 1},
    {{"\x83\x56\x83\x87\x81\x5B\x83\x67\x92\x85\x92\x6E"}, 0.0f, 5.0f, 0.0f, 0},
    {{"\x83\x41\x83\x62\x83\x70\x81\x5B\x83\x70\x83\x93\x83\x60"}, 22.0f, 23.0f, 0.0f, 1},
    {{"\x82\xB5\x82\xE1\x82\xAA\x82\xDD\x83\x41\x83\x62\x83\x70\x81\x5B"}, 0.0f, 6.0f, 0.0f, 0},
    {{"\x83\x58\x83\x50\x83\x4C\x83\x88"}, 0.0f, 59.0f, 30.0f, 2},
    {{"\x83\x58\x83\x50\x83\x4C\x83\x88\x92\x45\x8F\x6F"}, 60.0f, 120.0f, 0.0f, 0},
    {{"\x93\xC1\x8E\xEA\x83\x45\x83\x47\x83\x43\x83\x67\x31\x41"}, 0.0f, 494.0f, 375.0f, 2},
    {{"\x93\xC1\x8E\xEA\x83\x45\x83\x47\x83\x43\x83\x67\x31\x42"}, 0.0f, 254.0f, 135.0f, 2},
    {{"\x83\x41\x83\x43\x83\x58\x82\xD0\x82\xCB\x82\xE8"}, 0.0f, 35.0f, 35.0f, 0},
    {{"\x83\x74\x83\x40\x83\x43\x83\x41\x83\x58\x83\x73\x83\x93\x8B\xF3\x92\x86"}, 0.0f, 30.0f, 30.0f, 0},
    {{"\x8A\x52\x82\xC2\x82\xA9\x82\xDC\x82\xE8\x8F\x49\x97\xB9\x8D\xE2"}, 0.0f, 27.0f, 27.0f, 0},
    {{"\x91\x4F\x95\xC7\x83\x45\x83\x47\x83\x43\x83\x67"}, 57.0f, 59.0f, 57.0f, 1},
    {{"\x83\x74\x81\x5B\x83\x74\x83\x40\x83\x43\x83\x5E\x81\x5B\x90\xC3\x8E\x7E"}, 0.0f, 159.0f, 40.0f, 2},
    {{"\x82\xD0\x82\xEB\x82\xA2"}, -1.0f, -1.0f, -1.0f, 1},
    {{"\x82\xD0\x82\xEB\x82\xA2\x83\x4E\x83\x43\x83\x62\x83\x4E"}, 0.0f, 26.0f, 0.0f, 1},
    {{"\x8D\x9C\x83\x6F\x83\x89\x83\x6F\x83\x89\x8C\x70\x91\xB1"}, 0.0f, 14.0f, 0.0f, 1},
    {{""}, 0.0f, 0.0f, 0.0f, 0},
};

XanimeSwapTable playerGhostLuigiSwap[] = {
    {"Run", "LuigiRun"},
    {"Jump", "LuigiJump"},
    {"JumpRoll", "LuigiJumpRoll"},
    {"JumpBack", "LuigiJumpBack"},
    {"RunEnd", "LuigiRunEnd"},
    {"Spin", "LuigiSpin"},
    {"SpinGround", "LuigiSpinGround"},
    {"SpaceFlyShort", "LuigiSpaceFlyShort"},
    {"Wait", "LuigiWait"},
    {"WaitSlopeL", "LuigiWaitSlopeL"},
    {"WaitSlopeR", "LuigiWaitSlopeR"},
    {"WaitSlopeU", "LuigiWaitSlopeU"},
    {"WaitSlopeD", "LuigiWaitSlopeD"},
    {"", nullptr},
};

#endif

extern XanimeGroupInfo playerGhostAnimeGroups[];
extern XanimeOfsInfo playerGhostAnimeOffsets[];
extern XanimeBckTable1 playerGhostSingleAnimes[];
extern XanimeBckTable2 playerGhostDoubleAnimes[];
extern XanimeBckTable3 playerGhostTripleAnimes[];
extern XanimeBckTable4 playerGhostQuadAnimes[];
extern XanimeSwapTable playerGhostLuigiSwap[];

namespace MR {
    bool isOnGroundPlayer() {
        return MarioAccess::isOnGround(0);
    }

    Triangle* getPlayerGroundingPolygon() {
        return MarioAccess::getGroundingPolygon(0);
    }

    void forceKillPlayerByAbyss() {
        return MarioAccess::forceKill(0, 0);
    }


    void forceKillPlayerByGroundRace() {
        return MarioAccess::forceKill(5, 0);
    }

    void forceGameOverPlayerBySink() {
        MarioAccess::forceGameOverSink();
    }


    bool isPlayerDead() {
        return !MarioAccess::getPlayerActor()->isEnableNerveChange();
    }

    bool isPlayerRefuseTalk() {
        return MarioAccess::getPlayerActor()->isRefuseTalk();
    }

    bool isPlayerTeresaDisappear() {
        return MarioAccess::isTeresaDisappear();
    }

    bool isPlayerInAreaObj(const char* pName) {
        return isInAreaObj(pName, *getPlayerPos());
    }

    TVec3f* getPlayerPos() {
        if (!MarioAccess::isVisible() && isCurrentRushActor("岩マリオ")) {
            return &getCurrentRushActor()->mPosition;
        }

        return &MarioAccess::getPlayerActor()->mPosition;
    }

    TVec3f* getPlayerCenterPos() {
        return &MarioAccess::getPlayerActor()->_240;
    }

    void getPlayerTakePos(TVec3f* pPos) {
        return MarioAccess::getTakePos(pPos);
    }

    void setPlayerPos(const TVec3f& rPos) {
        MarioAccess::setTrans(rPos, 0);
    }

    void setPlayerUpperRotateY(f32 angle) {
        MarioAccess::getPlayerActor()->setUpperRotateY(angle);
    }

    TVec3f* getPlayerRotate() {
        return &MarioAccess::getPlayerActor()->mRotation;
    }

    TVec3f* getPlayerShadowRotate() {
        return &MarioAccess::getPlayerActor()->_A18;
    }

    TVec3f* getPlayerVelocity() {
        return MarioAccess::getVelocity();
    }

    TVec3f* getPlayerLastMove() {
        return MarioAccess::getLastMove();
    }

    void setPlayerJumpVec(const TVec3f& rJumpVec) {
        MarioAccess::setJumpVec(rJumpVec);
    }

    f32 getPlayerHitRadius() {
        return MarioAccess::getPlayerActor()->mPlayerMode == (s16)6 ? 100.0f : 60.0f;
    }

    // Yoshi-eat item launch velocity (MarioAccess::calcYoshiEatItemVelocity -> fn_8041BE00: velocity * 2 - gravity * 40); caller ActorItemGenerator
    void calcYoshiEatItemVelocity(TVec3f* pOut) {
        MarioAccess::calcYoshiEatItemVelocity(pOut);
    }

    // original name unknown; named by behaviour: returns the cached ceiling distance, or a fixed value during confront-death or disabled nerve changes.
    f32 getPlayerCeilingDistance() {
        return MarioAccess::getCeilingDistance();
    }

    void clearPlayerMovementFlag20000000() {
        MarioAccess::fn_803B72A0();
    }


    const TVec3f* getPlayerGravity() {
        return &MarioAccess::getPlayerActor()->getGravityVec();
    }

    void calcPlayerSpinPullVelocity(TVec3f* pVelocity, const TVec3f& rPos) {
        MarioAccess::calcSpinPullVelocity(pVelocity, rPos);
    }


    bool checkPlayerSwingTrigger() {
        return MarioAccess::getPlayerActor()->_1E1;
    }

    f32 calcDistanceToPlayer(const TVec3f& rPos) {
        return getPlayerPos()->distance(rPos);
    }

    void getPlayerUpVec(TVec3f* pVec) {
        MarioAccess::getPlayerActor()->getUpVec(pVec);
    }

    void getPlayerFrontVec(TVec3f* pVec) {
        MarioAccess::getPlayerActor()->getFrontVec(pVec);
    }

    void getPlayerSideVec(TVec3f* pVec) {
        MarioAccess::getPlayerActor()->getSideVec(pVec);
    }

    void getPlayerThrowVec(TVec3f* pVec) {
        MarioAccess::getThrowVec(pVec);
    }

    void getPlayerGroundPos(TVec3f* pPos) {
        MarioAccess::getPlayerActor()->getGroundPos(pPos);
    }

    const TVec3f* getPlayerGroundNormal() {
        return getPlayerGroundingPolygon()->getFaceNormal();
    }

    void setPlayerFrontTargetVec(const TVec3f& rTarget, s32 param2) {
        MarioAccess::setFrontVecTarget(rTarget, param2);
    }

    void setPlayerFrontVec(const TVec3f& rVec, s32 param2) {
        MarioAccess::setFrontVecKeepUp(rVec, param2);
    }

    void setPlayerSwingInhibitTimer(u16 timer) {
        MarioAccess::getPlayerActor()->_EF6 = timer;
    }

    void setPlayerSwingPermission(bool permission) {
        MarioAccess::getPlayerActor()->_EEB = permission;
    }

    void setPlayerStateWait() {
        MarioAccess::setStateWait();
    }

    void startBckPlayer(const char* pName, const char* pParam2) {
        if (pParam2 != nullptr) {
            MarioAccess::changeAnimationE(pName, pParam2);
        } else {
            MarioAccess::changeAnimationE(pName, pName);
        }
    }

    void startBckPlayer(const char* pName, const BckCtrlData& rCtrlData) {
        MarioAccess::changeAnimationE(pName, rCtrlData);
    }

    bool startPlayerBvaWithCtrl(const char* pName, const BckCtrlData& rCtrlData) {
        if (!startBvaIfExist(MarioAccess::getPlayerActor(), pName)) {
            return false;
        }

        BckCtrlFunction::reflectBckCtrlDataToFrameCtrl(rCtrlData, getBvaCtrl(MarioAccess::getPlayerActor()));
        return true;
    }

    void startPlayerNormalBva() {
        startBva(MarioAccess::getPlayerActor(), "Normal");
    }

    bool isBckStoppedPlayer() {
        return MR::isActionEnd(MarioAccess::getPlayerOrYoshiActor());
    }

    bool isBckOneTimeAndStoppedPlayer() {
        return MR::isBckOneTimeAndStopped(MarioAccess::getPlayerOrYoshiActor());
    }


    s16 getBckFrameMaxPlayer(const char* pName) {
        return MR::getBckFrameMax(MarioAccess::getPlayerActor(), pName);
    }

    void startBckPlayerJ(const char* pName) {
        MarioAccess::changeAnimationJ(pName);
    }

    void becomeContinuousBckPlayer() {
        MarioAccess::keepCurrentAnimation();
    }

    void progressPlayerBckOnPause() {
        MarioAccess::progressAnimation();
    }

    const char* getPlayerCurrentBckName() {
        return MarioAccess::getCurrentBckName();
    }

    void setBckBlendWeight(f32 param1, f32 param2) {
        const f32 blendWeight[4] = {param1, param2, 0.0f, 0.0f};

        MarioAccess::setAnimationBlendWeight(blendWeight);
    }

    void setBckBlendWeight(f32 param1, f32 param2, f32 param3) {
        const f32 blendWeight[4] = {param1, param2, param3, 0.0f};

        MarioAccess::setAnimationBlendWeight(blendWeight);
    }

    void setBckBlendWeight(f32 param1, f32 param2, f32 param3, f32 param4) {
        const f32 blendWeight[] = {param1, param2, param3, param4};

        MarioAccess::setAnimationBlendWeight(blendWeight);
    }

    void setBckRatePlayer(f32 rate) {
        MR::setBckRate(MarioAccess::getPlayerActor(), rate);
    }

    XanimeResourceTable* getPlayerXanimeResource() {
        return MarioAccess::getPlayerActor()->getResourceTable();
    }

    XanimeResourceTable* createPlayerXanimeResource(bool isLuigi) {
        XanimeSwapTable* swap = nullptr;
        if (isLuigi) swap = playerGhostLuigiSwap;
        return new XanimeResourceTable(MR::getResourceHolder(MarioAccess::getPlayerActor()), playerGhostAnimeGroups,
                                      playerGhostAnimeOffsets, playerGhostSingleAnimes, playerGhostDoubleAnimes,
                                      playerGhostTripleAnimes, playerGhostQuadAnimes, swap);
    }

    void jumpPlayer(const TVec3f& rParam1) {
        MarioAccess::freeJump(rParam1, 0);
    }

    void forceJumpPlayer(const TVec3f& rParam1) {
        MarioAccess::forceJump(rParam1, 0);
    }

    void forceFlyPlayer(const TVec3f& rParam1, const TVec3f& rParam2, s32 param3) {
        MarioAccess::forceFly(rParam1, rParam2, param3);
    }

    void tornadoJumpPlayer() {
        MarioAccess::tornadoJump();
    }

    void miniTornadoJumpPlayer() {
        MarioAccess::tornadoJumpMini();
    }

    void clearPlayerStateFlags00300000() {
        MarioAccess::fn_803B51B0();
    }

    void becomePlayerNormalJumpStatus() {
        MarioAccess::becomeNormalJumpStatus();
    }

    bool isOnPlayer(const HitSensor* pSensor) {
        if (MarioAccess::isInRush()) {
            if (isBindedGround(getCurrentRushActor())) {
                return getGroundSensor(getCurrentRushActor()) == pSensor;
            }

            return false;
        }

        Triangle* polygon = MarioAccess::getGroundingPolygon(0);
        if (polygon != nullptr) {
            return polygon->mSensor == pSensor;
        }

        return false;
    }

    bool isActorOnPlayer(const LiveActor* pActor) {
        if (MarioAccess::isInRush()) {
            if (isBindedGround(getCurrentRushActor())) {
                return getGroundActor(getCurrentRushActor()) == pActor;
            }

            return false;
        }

        return MarioAccess::isOnActor(pActor);
    }

    bool isOnPlayerShadow(const LiveActor* pActor) {
        Triangle* polygon = MarioAccess::getShadowingPolygon();

        if (polygon != nullptr) {
            return polygon->mSensor->mHost == pActor;
        }

        return false;
    }


    void setPlayerPos(const char* pParam1) {
        TPos3f mtx;
        mtx.identity();

        MR::findNamePos(pParam1, mtx.toMtxPtr());
        MarioAccess::setBaseMtx(mtx.toMtxPtr());

        TVec3f trans;
        MR::extractMtxTrans(mtx.toMtxPtr(), &trans);

        setPlayerPos(trans);
    }

    void setPlayerPosAndWait(const TVec3f& rPos, bool resetBeforePosition) {
        if (resetBeforePosition) MarioAccess::prepareForPositionReset();
        MarioAccess::setTrans(rPos, 0);
        MarioAccess::getPlayerActor()->resetCondition();
        MarioAccess::setStateWait();
        startBckPlayer("Wait", static_cast<const char*>(nullptr));
    }

    void setPlayerPosAndWait(const char* pName, bool resetBeforePosition) {
        if (resetBeforePosition) MarioAccess::prepareForPositionReset();
        setPlayerPos(pName);
        MarioAccess::getPlayerActor()->resetCondition();
        MarioAccess::setStateWait();
        startBckPlayer("Wait", static_cast<const char*>(nullptr));
    }

    void setPlayerLinkPosAndWait(const NameObj* pParam1, const char* pParam2, bool resetBeforePosition) {
        if (resetBeforePosition) MarioAccess::prepareForPositionReset();
        TPos3f mtx;
        mtx.identity();

        MR::findLinkNamePos(pParam1, pParam2, mtx.toMtxPtr());
        MarioAccess::setBaseMtx(mtx.toMtxPtr());
        MarioAccess::getPlayerActor()->resetCondition();
        MarioAccess::setStateWait();
        startBckPlayer("Wait", static_cast<const char*>(nullptr));
    }

    void setPlayerPosOnGround(const char* pParam1) {
        TPos3f mtx;
        mtx.identity();

        MR::findNamePosOnGround(pParam1, mtx.toMtxPtr());
        MarioAccess::setBaseMtx(mtx.toMtxPtr());
    }

    void setPlayerPosOnGroundAndWait(const char* pParam1, bool resetBeforePosition) {
        if (resetBeforePosition) MarioAccess::prepareForPositionReset();
        TPos3f mtx;
        mtx.identity();

        MR::findNamePosOnGround(pParam1, mtx.toMtxPtr());
        MarioAccess::setBaseMtx(mtx.toMtxPtr());
        MarioAccess::getPlayerActor()->resetCondition();
        MarioAccess::setStateWait();
        MR::startBckPlayer("Wait",static_cast<const char*>(nullptr));
    }

    bool isPlayerHipDropFalling() {
        return MarioAccess::isHipDropFalling();
    }

    bool isPlayerHipDropLand() {
        return MarioAccess::isHipDropLand();
    }

    s32 getPlayerLife() {
        return MarioAccess::getLife();
    }

    bool isPlayerLifeOne() {
        return MarioAccess::getLife() == 1;
    }

    // original name unknown; named by behaviour: decrements swim life, otherwise requests a life decrement with timer 20.
    void decrementPlayerSwimLife() {
        MarioAccess::decrementLife();
    }

    void incPlayerLife(u32 add) {
        MarioAccess::incLife(add);
    }

    void incPlayerOxygen(u32 add) {
        MarioAccess::incOxygen(add);
    }

    bool isPlayerConfrontDeath() {
        return MarioAccess::isConfrontDeath();
    }

    void getStarPiecePlayer() {
        MarioAccess::addStarPiece();
    }

    void notifyDirectGetStarPiecePlayer() {
        MarioAccess::getStarPieceDirect();
    }

    void scatterStarPiecePlayer(u32 num) {
        MarioAccess::scatterStarPiece(num);
    }

    void extendPlayerBeeAirWalk(s32 param1) {
        MarioAccess::extendBeeAirWalk(param1);
    }

    bool isPlayerSwingAction() {
        return MarioAccess::isSwingAction();
    }

    // original name unknown; named by behaviour: tests jumping and Mario jump action 5.
    bool isPlayerInJumpAction() {
        return MarioAccess::isInJumpAction();
    }

    // Separate SMG2 entrypoint that forwards to the same swing-start predicate.
    bool isPlayerSwingActionStartNow() {
        return isPlayerSwingActionStart();
    }

    bool isPlayerPointedBy2POnTriggerButton() {
        return MR::isStarPointerPointing2POnTriggerButton(MarioAccess::getPlayerActor(), "弱", true, false);
    }

    bool isPlayerSquat() {
        return MarioAccess::isSquat();
    }

    bool isPlayerInRush() {
        return MarioAccess::isInRush();
    }

    bool isPlayerNeedBrakingCamera() {
        return MarioAccess::isNeedBrakingCamera();
    }

    // original name unknown; named by behaviour: sets the rush silhouette override and clears its hidden-frame timer.
    void enablePlayerSilhouetteDuringRush() {
        MarioAccess::enableSilhouetteDuringRush();
    }

    // original name unknown; named by behaviour: sets the countdown that suppresses silhouette visibility.
    void setPlayerSilhouetteHiddenFrames(s32 pParam1) {
        MarioAccess::setSilhouetteHiddenFrames(pParam1);
    }

    // original name unknown; named by behaviour: clears the actor footprint emitter if present.
    void clearPlayerFootprints() {
        MarioAccess::clearFootprints();
    }

    // original name unknown; named by behaviour: sets the flag that skips reflection drawing and view updates.
    void disablePlayerReflection() {
        MarioAccess::disableReflection();
    }

    // original name unknown; named by behaviour: clears the flag that skips reflection drawing and view updates.
    void enablePlayerReflection() {
        MarioAccess::enableReflection();
    }

    bool isPlayerFlying() {
        return MarioAccess::isFlying();
    }


    bool isPlayerElementModeTornado() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 9;
    }

    bool isPlayerElementModeInvincible() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 1;
    }

    bool isPlayerElementModeBee() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 4;
    }

    bool isPlayerElementModeHopper() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 5;
    }

    bool isPlayerElementModeTeresa() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 6;
    }

    bool isPlayerElementModeIce() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 3;
    }

    bool isPlayerElementModeNormal() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 0;
    }



    void curePlayerElementMode() {
        MarioAccess::changeItemStatus(8);
    }

    void forceCurePlayerElementMode() {
        MarioAccess::changeItemStatus(11);
    }

    bool isPlayerParalyzing() {
        return MarioAccess::isParalyzing();
    }

    bool isPlayerDamaging() {
        return MarioAccess::getPlayerActor()->isDamaging();
    }

    // original name unknown; named by behaviour: tests Mario flag 0x80000000 or fire states 8 through 10.
    bool isPlayerFireDamageActive() {
        return MarioAccess::getPlayerActor()->isFireDamageActive();
    }

    bool isPlayerStaggering() {
        return MarioAccess::getPlayerActor()->isStaggering();
    }

    bool isPlayerSwimming() {
        return MarioAccess::isSwimming();
    }

    bool isPlayerSleeping() {
        return MarioAccess::getPlayerActor()->isSleeping();
    }

    bool isPlayerJumpRising() {
        return MarioAccess::getPlayerActor()->isJumpRising();
    }



    void endBindAndPlayerWait(LiveActor* pActor) {
        TVec3f velocity(0.0f, 0.0f, 0.0f);
        RushEndInfo info(pActor, 0, velocity, false, 0);

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerJump(LiveActor* pActor, const TVec3f& rParam2, u32 param3) {
        RushEndInfo info = RushEndInfo(pActor, 2, rParam2, true, param3);

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerForceJump(LiveActor* pActor, const TVec3f& rParam2, u32 param3) {
        RushEndInfo info = RushEndInfo(pActor, 2, rParam2, true, param3);
        info._20 |= 0x40000000;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerWeakGravityJump(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerForceWeakGravityJump(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0x40000000;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerForceWeakGravityJumpInputOff(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0xC0000000;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerWeakGravityLimitJump(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0x800000;

        MarioAccess::endRush(&info);
    }

    void endBindAndSpinDriverJump(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0xC0000000;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerDamage(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 1;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerFlip(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 6;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerJumpWithRollLanding(LiveActor* pActor, const TVec3f& rParam2, u32 param3) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, param3);
        info._20 |= 0x400000;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerDamageMsg(LiveActor* pActor, u32 param2) {
        endBindAndPlayerDamageMsg(pActor, param2, TVec3f(0.0f, 0.0f, 0.0f));
    }

    void endBindAndPlayerDamageMsg(LiveActor* pActor, u32 param2, const TVec3f& rParam3) {
        switch (param2) {
        case 0x65:
        case 0x66:
        case 0x67:
            endBindAndPlayerFireDamage(pActor);
            break;
        case 0x6A:
            endBindAndPlayerAcidDamage(pActor);
            break;
        case 0x6B:
            endBindAndPlayerFreezeDamage(pActor);
            break;
        case 0x68:
        case 0x69:
            endBindAndPlayerElectricDamage(pActor);
            break;
        case 0x64:
            endBindAndPlayerDamage(pActor, rParam3);
            break;
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5F:
        case 0x60:
            endBindAndPlayerWeakGravityLimitJump(pActor, rParam3);
            break;
        case 0x5E:
            endBindAndPlayerFlip(pActor, rParam3);
            break;
        default:
            endBindAndPlayerDamage(pActor, rParam3);
            break;
        }
    }

    void endBindAndPlayerAcidDamage(LiveActor* pActor) {
        RushEndInfo info = RushEndInfo(pActor, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 4;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerFreezeDamage(LiveActor* pActor) {
        RushEndInfo info = RushEndInfo(pActor, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 3;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerFireDamage(LiveActor* pActor) {
        RushEndInfo info = RushEndInfo(pActor, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
        info._20 |= 0x40000000;
        info.mFlags.mDamageType = 2;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerElectricDamage(LiveActor* pActor) {
        RushEndInfo info = RushEndInfo(pActor, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 5;

        MarioAccess::endRush(&info);
    }

    LiveActor* getCurrentRushActor() {
        if (isPlayerInRush() == false) {
            return nullptr;
        }

        return MarioAccess::getPlayerActor()->_924->mHost;
    }

    HitSensor* getCurrentRushSensor() {
        if (isPlayerInRush() == false) {
            return nullptr;
        }

        return MarioAccess::getPlayerActor()->_924;
    }

    void tryPlayerCoinPull() {
        MarioAccess::tryCoinPull();
    }

    void tryPlayerPullActor(HitSensor* pSensor) {
        MarioAccess::getPlayerActor()->tryTornadoPull(pSensor);
    }

    void tryPlayerDropTakingActor() {
        MarioAccess::dropTakingActor();
    }

    void tryPlayerKillTakingActor() {
        MarioAccess::killTakingActor();
    }

    bool isPlayerCarrySensorType(u32 type) {
        if (MarioAccess::getTakingSensor() == nullptr) return false;
        if (type == 0) return true;
        return MarioAccess::getTakingSensor()->isType(type);
    }

    bool isPlayerLifeDecreasedThisFrame() {
        return MarioAccess::getPlayerActor()->mLifeDecreasedThisFrame;
    }

    bool isPlayerCarryAny() {
        return MarioAccess::getPlayerActor()->_468 != 0;
    }

    LiveActor* getPlayerCarrySensorActor() {
        HitSensor* pSensor = MarioAccess::getTakingSensor();
        if (pSensor == nullptr) {
            return nullptr;
        }

        return getSensorHost(pSensor);
    }

    void startSoundPlayer(const char* pName, s32 param1, s32 param2) {
        MR::startSound(MarioAccess::getPlayerActor(), pName, param1, param2);
    }

    void startSoundPlayer(JAISoundID id, s32 param2, s32 param3) {
        MR::startSound(MarioAccess::getPlayerActor(), id, param2, param3);
    }

    JAISoundHandle* startSoundPlayerById(s32 id, s32 pitch, s32 velocity) {
        return MR::startSound(MarioAccess::getPlayerActor(), JAISoundID(id), pitch, velocity);
    }

    void startLevelSoundPlayer(const char* pName, s32 param1) {
        MR::startLevelSound(MarioAccess::getPlayerActor(), pName, param1);
    }

    JAISoundHandle* startLevelSoundPlayerById(s32 id, s32 pitch) {
        return MR::startLevelSound(MarioAccess::getPlayerActor(), JAISoundID(id), pitch, -1, -1);
    }

    void stopSoundPlayer(const char* pName, u32 param1) {
        MR::stopSound(MarioAccess::getPlayerActor(), pName, param1);
    }

    void startSoundPlayerJ(const char* pName) {
        MarioAccess::getPlayerActor()->playSound(pName, -1);
    }

    void showPlayer() {
        MarioAccess::show();
    }

    void hidePlayer() {
        MarioAccess::hide(true);
    }

    void hidePlayerDeleteEffect() {
        MarioAccess::hide(true);
    }

    void showPlayerJoint(const char* pName) {
        MR::showJoint(MarioAccess::getPlayerActor(), pName);
    }

    void hidePlayerJoint(const char* pName) {
        MR::hideJoint(MarioAccess::getPlayerActor(), pName);
    }

    bool isPlayerVisible() {
        return MarioAccess::isVisible();
    }

    void setPlayerSpot(f32 param1, u32 param2) {
        MarioAccess::setSpot(param1, param2);
    }

    void startPlayerDownWipe() {
        MarioAccess::startDownWipe();
    }

    void setCameraTargetToPlayer(CameraTargetArg* pTarget) {
        MarioActor* player = MarioAccess::getPlayerActor();
        pTarget->mTargetObj = nullptr;
        pTarget->mTargetMtx = nullptr;
        pTarget->mLiveActor = nullptr;
        pTarget->mMarioActor = player;
    }

    bool isPlayerDisableFpView() {
        return MarioAccess::isDisableFpView();
    }

    bool isFpViewChangingFailure() {
        return MarioAccess::isFpViewChangingFailure();
    }

    void stopPlayerFpView() {
        return MarioAccess::stopFpView();
    }



    void startPlayerTalk(const LiveActor* pActor) {
        MarioAccess::startTalk(pActor);
    }

    void endPlayerTalk() {
        MarioAccess::endTalk();
    }

    void preparePlayerForPositionReset() {
        MarioAccess::prepareForPositionReset();
    }


    bool isExistMario() {
        return MR::isExistSceneObj(SceneObj_MarioHolder);
    }

    void startPlayerEvent(const char* pName) {
        EventSequencer* eventSequencer;

        eventSequencer = MR::getSceneObj< EventSequencer >(SceneObj_EventSequencer);
        eventSequencer->startEvent(pName);

        requestMovementOn(eventSequencer);
        requestMovementOnPlayer();
    }

    void offPlayerControl() {
        MarioAccess::offControl();
        offSupportTicoControl();
    }

    void onPlayerControl(bool param1) {
        MarioAccess::onControl(param1);
        onSupportTicoControl();
    }

    bool isOffPlayerControl() {
        return MarioAccess::isOffControl();
    }

    void unlockPlayerAnimation() {
        MarioAccess::getPlayerActor()->setB90(false);
    }

    void resetPlayerStatus() {
        MarioAccess::getPlayerActor()->resetCondition();
    }

    void resetPlayerEffect() {
        MR::forceDeleteEffectAll(MarioAccess::getPlayerActor());
        resetChasingStarPiece();
    }

    void setPlayerBaseMtx(MtxPtr pMtx) {
        MarioAccess::setBaseMtx(pMtx);
    }

    // NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
    void setPlayerQuatAndPos(TQuat4f* pQuat, TVec3f* pPos) {
        TPos3f mtx;
        f32 y2 = 2.0f * pQuat->y;
        f32 x2 = 2.0f * pQuat->x;
        f32 z2 = 2.0f * pQuat->z;
        f32 yy = y2 * pQuat->y;
        f32 xx = x2 * pQuat->x;
        f32 w2 = 2.0f * pQuat->w;
        mtx.setTrans(*pPos);
        f32 zz = z2 * pQuat->z;
        mtx.mMtx[1][1] = 1.0f - xx - zz;
        mtx.mMtx[0][0] = 1.0f - yy - zz;
        f32 xy = x2 * pQuat->y;
        f32 xz = x2 * pQuat->z;
        f32 wz = w2 * pQuat->z;
        mtx.mMtx[2][2] = 1.0f - xx - yy;
        f32 wy = w2 * pQuat->y;
        mtx.mMtx[0][1] = xy - wz;
        mtx.mMtx[0][2] = xz + wy;
        mtx.mMtx[1][0] = xy + wz;
        mtx.mMtx[2][0] = xz - wy;
        f32 yz = y2 * pQuat->z;
        f32 wx = w2 * pQuat->x;
        mtx.mMtx[1][2] = yz - wx;
        mtx.mMtx[2][1] = yz + wx;
        MarioAccess::setBaseMtx(mtx.toMtxPtr());
    }

    MtxPtr getPlayerBaseMtx() {
        return MarioAccess::getBaseMtx();
    }

    void getPlayerQuatAndPos(TQuat4f* pQuat, TVec3f* pPos) {
        TPos3f mtx;
        mtx.set(MarioAccess::getBaseMtx());
        mtx.getQuat(*pQuat);
        pPos->set(*getPlayerPos());
    }

    LiveActor* getPlayerDemoActor() {
        return MarioAccess::getPlayerActor();
    }

    void requestMovementOnPlayer() {
        MarioAccess::getPlayerActor()->requestMovementOn();
    }

    void requestPlayerMovementOnDuringDemo() {
        MarioAccess::getPlayerActor()->requestHairAndHatMovementOn();
    }

    void killPlayerFireBalls() {
        MarioAccess::killAllFireBall();
    }





    void fn_800515E0() {
        MarioAccess::tryPressAgainstCeiling(30);
    }

    void calcPlayerJointMtx(TPos3f* pMtx, const char* pName) {
        pMtx->set(MarioAccess::getJointMtx(pName));
    }

    void concatWithPlayerJointMtx(TPos3f* pMtx, const char* pName) {
        MarioAccess::concatWithJointMtx(pMtx, pName);
    }


    void pushPlayer(const TVec3f& rAccel) {
        MarioAccess::addVelocity(rAccel);
    }

    void pushPlayerFromArea(const TVec3f& rAccel) {
        MarioAccess::addVelocityFromArea(rAccel);
    }

    void noticePlayerDashChance() {
        MarioAccess::noticeDashChance();
    }

    bool isPlayerInWaterMode() {
        return MarioAccess::isInWaterMode();
    }

    bool isPlayerOnWaterSurface() {
        return MarioAccess::isOnWaterSurface();
    }

    bool isPlayerHidden() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 6;
    }

    void calcPlayerWorldPadDir(TVec3f* pDir, f32 param2, f32 param3) {
        MarioAccess::calcWorldPadDir(pDir, param2, param3);
    }

    void calcPlayerKillDiffVec(TVec3f* pDst, const TVec3f& rA, const TVec3f& rB, const TVec3f& rUp) {
        pDst->set(rA - rB);
        vecKillElement(*pDst, rUp, pDst);
        normalizeOrZero(pDst);
    }

    void calcPlayerKillVecOnPlane(TVec3f* pDst, const TVec3f& rVec, const TVec3f& rA, const TVec3f& rB, const TVec3f& rNormal) {
        TVec3f tmp;
        tmp.set(rA - rB);
        vecKillElement(tmp, rNormal, &tmp);
        normalizeOrZero(&tmp);
        vecKillElement(rVec, tmp, pDst);
    }

    // Keep the normal displacement; clamp the planar radius to scale - 200.
    // NONMATCHING: register/stack or instruction scheduling (FP order/grouping proven by default rdclass a).
    void calcPlayerScaledVec(TVec3f* pDst, const TVec3f& rA, const TVec3f& rB, const TVec3f& rC, f32 scale) {
        TVec3f tmp = rA - rB;
        f32 normalDistance = vecKillElement(tmp, rC, &tmp);
        tmp.setLength(scale - 200.0f);

        TVec3f add(rC);
        add.scale(normalDistance);
        tmp += add;

        TVec3f result(rB);
        result += tmp;
        pDst->set(result);
    }

    JUTTexture* getFullScreenBlurTexture() {
        return MarioAccess::getPlayerActor()->_B7C;
    }

    // Mario's shadow texture (CollisionShadow+0x308; DisplayListMaker draws it).
    const JUTTexture* getPlayerShadowTexture() {
        return MarioAccess::getShadowTexture();
    }

    u16 getPlayerMovementTimer() {
        return MarioAccess::getPlayerActor()->_378;
    }

    void forceKillPlayer() {
        if (MarioAccess::isInWaterMode()) {
            MarioAccess::forceKill(1, 0);
        } else {
            MarioAccess::forceKill(5, 0);
        }
    }

    f32 getBckFrameMaxPlayer() {
        return MarioAccess::getAnimationFrameMax();
    }

    bool isPlayerSkating() {
        return MarioAccess::isSkating();
    }

    bool isPlayerPowerupTransforming() {
        return MarioAccess::isPowerupTransforming();
    }

    // original name unknown; named by behaviour: compares the selected Mario model with MarioAllModel::getJ3DModel().
    bool isPlayerDefaultModel() {
        return MarioAccess::isDefaultModel();
    }

    bool isPlayerRecovering() {
        return MarioAccess::isRecovering();
    }

    bool isPlayerHanging() {
        return MarioAccess::isHanging();
    }

    bool isPlayerBury() {
        return MarioAccess::isBury();
    }

    bool fn_80050020() {
        return MarioAccess::isRushingWithRestrictedYoshiJointControl();
    }

    void changePlayerModeBee() {
        MarioAccess::changeItemStatus(2);
    }

    void changePlayerModeHopper() {
        MarioAccess::changeItemStatus(1);
    }

    void changePlayerModeTeresa() {
        MarioAccess::changeItemStatus(3);
    }

    void changePlayerModeIce() {
        MarioAccess::changeItemStatus(4);
    }

    void changePlayerModeFire() {
        MarioAccess::changeItemStatus(5);
    }

    void changePlayerModeYoshi() {
        MarioAccess::changePlayerModeYoshi();
    }

    void changePlayerModeCloud() {
        MarioAccess::changePlayerModeCloud();
    }

    void changePlayerModeRock() {
        MarioAccess::changePlayerModeRock();
    }

    void changePlayerModeFoo() {
        MarioAccess::changeItemStatus(6);
    }

    void changePlayerModeInvincible() {
        MarioAccess::changeItemStatus(7);
    }

    void changePlayerItemLifeUp() {
        MarioAccess::changeItemStatus(10);
    }

    void hidePlayerTryDeleteEffect() {
        MarioAccess::hide(false);
    }

    void emitEffectPlayer(const char* pName) {
        MarioAccess::getPlayerActor()->playEffect(pName);
    }

    void deleteEffectPlayer(const char* pName) {
        MarioAccess::getPlayerActor()->stopEffect(pName);
    }

    bool isCurrentRushActor(const char* pName) {
        LiveActor* pRushActor = getCurrentRushActor();
        if (pRushActor != nullptr) {
            return isEqualString(pRushActor->getName(), pName);
        } else {
            return false;
        }
    }

    bool isCurrentRushRockMario() {
        return isCurrentRushActor("岩マリオ");
    }

    bool isCurrentRushTamakoro() {
        return isCurrentRushActor("たまころ");
    }

    // NONMATCHING: literal pool sharing (target lis/addi per literal, ours shares one base)
    bool isCurrentRushSpinDriver() {
        bool result = false;
        if (isCurrentRushActor("スーパースピンドライバー") || isCurrentRushActor("スピンドライバ") || isCurrentRushActor("ピンクスーパースピンドライバー")) {
            result = true;
        }

        return result;
    }

    bool isCurrentRushItemDrill() {
        return isCurrentRushActor("アイテムドリル");
    }

    HitSensor* getPlayerBodySensor() {
        return MarioAccess::getPlayerActor()->getSensor("Body");
    }

    void copyPlayerTR(LiveActor* pActor) {
        setPosition(pActor, *getPlayerPos());
        setRotation(pActor, MarioAccess::getPlayerActor()->mRotation);
    }

    void changePlayerAnimationAndBva(const char* pName) {
        MarioAccess::changeAnimationE(pName, pName);
        startBvaIfExist(MarioAccess::getPlayerActor(), pName);
    }

    void startDemoBckPlayer(const char* pName, const BckCtrlData& rCtrlData) {
        MarioAccess::changeAnimationE(pName, rCtrlData);
        startPlayerBvaWithCtrl(pName, rCtrlData);
    }

    void startLevelSoundPlayer(const char* pName, s32 pitch, s32 volume) {
        startActionSound(MarioAccess::getPlayerActor(), pName, pitch, volume, -1);
    }

    void startLevelSoundPlayer(JAISoundID id, s32 param2) {
        MR::startLevelSound(MarioAccess::getPlayerActor(), id, param2, -1, -1);
    }

    void startActionSoundPlayer(const char* pName, s32 param2, s32 param3) {
        startActionSound(MarioAccess::getPlayerActor()->getYoshi(), pName, param2, param3, -1);
    }

    // SMG2 mPlayerMode ids (Fire 2, Yoshi 0xA, Cloud 0xC, Rock 0xD); PlayerMode enum in MarioActor.hpp is SMG1-shaped
    bool isPlayerElementModeFire() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 2;
    }

    bool isPlayerElementModeYoshi() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 10;
    }

    bool isPlayerElementModeCloud() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 12;
    }

    bool isPlayerElementModeRock() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 13;
    }

};  // namespace MR

namespace MR {
    void startPlayerBlink() {
        MarioAccess::getPlayerActor()->fn_803D9710();
    }
    void stopPlayerBlink() {
        MarioAccess::getPlayerActor()->fn_803D9730();
    }
}
