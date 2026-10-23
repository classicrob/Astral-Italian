#pragma once
// Feeds SDL3 events to the native input layer. Header-only: it reads SDL's
// event structures and calls no SDL functions, so it needs SDL3's headers
// but adds no link dependency (target petari_input_sdl3).
//
// The application still tells the input layer where the game image is drawn
// (PetariNative::Input::setViewport) whenever the window or its letterboxing
// changes; SDL does not know the image rectangle.

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_mouse.h>

#include "petari/input.hpp"

namespace PetariNative::Input::SDL3 {

// SDL's standard gamepad button, or false for buttons with no binding name
// (touchpad, MISC3..6).
inline bool padButtonOf(Uint8 sdl, PadButton* button) {
    switch (sdl) {
    case SDL_GAMEPAD_BUTTON_SOUTH: *button = PadButton::South; return true;
    case SDL_GAMEPAD_BUTTON_EAST: *button = PadButton::East; return true;
    case SDL_GAMEPAD_BUTTON_WEST: *button = PadButton::West; return true;
    case SDL_GAMEPAD_BUTTON_NORTH: *button = PadButton::North; return true;
    case SDL_GAMEPAD_BUTTON_BACK: *button = PadButton::Back; return true;
    case SDL_GAMEPAD_BUTTON_GUIDE: *button = PadButton::Guide; return true;
    case SDL_GAMEPAD_BUTTON_START: *button = PadButton::Start; return true;
    case SDL_GAMEPAD_BUTTON_LEFT_STICK: *button = PadButton::LeftStick; return true;
    case SDL_GAMEPAD_BUTTON_RIGHT_STICK: *button = PadButton::RightStick; return true;
    case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: *button = PadButton::LeftShoulder; return true;
    case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: *button = PadButton::RightShoulder; return true;
    case SDL_GAMEPAD_BUTTON_DPAD_UP: *button = PadButton::DpadUp; return true;
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN: *button = PadButton::DpadDown; return true;
    case SDL_GAMEPAD_BUTTON_DPAD_LEFT: *button = PadButton::DpadLeft; return true;
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: *button = PadButton::DpadRight; return true;
    case SDL_GAMEPAD_BUTTON_MISC1: *button = PadButton::Misc1; return true;  // Pro Capture, Xbox Share
    case SDL_GAMEPAD_BUTTON_MISC2: *button = PadButton::Misc2; return true;  // Switch 2 C
    case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1: *button = PadButton::RightPaddle1; return true;  // GR
    case SDL_GAMEPAD_BUTTON_LEFT_PADDLE1: *button = PadButton::LeftPaddle1; return true;    // GL
    case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2: *button = PadButton::RightPaddle2; return true;
    case SDL_GAMEPAD_BUTTON_LEFT_PADDLE2: *button = PadButton::LeftPaddle2; return true;
    default: return false;
    }
}

inline bool padAxisOf(Uint8 sdl, PadAxis* axis) {
    switch (sdl) {
    case SDL_GAMEPAD_AXIS_LEFTX: *axis = PadAxis::LeftX; return true;
    case SDL_GAMEPAD_AXIS_LEFTY: *axis = PadAxis::LeftY; return true;
    case SDL_GAMEPAD_AXIS_RIGHTX: *axis = PadAxis::RightX; return true;
    case SDL_GAMEPAD_AXIS_RIGHTY: *axis = PadAxis::RightY; return true;
    case SDL_GAMEPAD_AXIS_LEFT_TRIGGER: *axis = PadAxis::LeftTrigger; return true;
    case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: *axis = PadAxis::RightTrigger; return true;
    default: return false;
    }
}

// Returns true for events the input layer consumed.
inline bool handleEvent(const SDL_Event& event) {
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        keyEvent(static_cast<KeyCode>(event.key.scancode), event.key.down, event.key.repeat);
        return true;
    case SDL_EVENT_MOUSE_MOTION:
        mouseMoved(event.motion.x, event.motion.y);
        return true;
    case SDL_EVENT_MOUSE_WHEEL: {
        const float flip = event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0f : 1.0f;
        mouseWheel(event.wheel.x * flip, event.wheel.y * flip);
        menuWheel(event.wheel.y);
        return true;
    }
    case SDL_EVENT_PINCH_UPDATE:
        pinch(event.pinch.scale);
        return true;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        MouseButton button;
        switch (event.button.button) {
        case SDL_BUTTON_LEFT: button = MouseButton::Left; break;
        case SDL_BUTTON_MIDDLE: button = MouseButton::Middle; break;
        case SDL_BUTTON_RIGHT: button = MouseButton::Right; break;
        case SDL_BUTTON_X1: button = MouseButton::X1; break;
        case SDL_BUTTON_X2: button = MouseButton::X2; break;
        default: return false;
        }
        mouseMoved(event.button.x, event.button.y);
        mouseButtonEvent(button, event.button.down);
        return true;
    }
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        PadButton button;
        if (!padButtonOf(event.gbutton.button, &button)) {
            return false;
        }
        padButtonEvent(button, event.gbutton.down);
        return true;
    }
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        PadAxis axis;
        if (!padAxisOf(event.gaxis.axis, &axis)) {
            return false;
        }
        // Sint16: sticks -32768..32767, triggers 0..32767.
        const float value = event.gaxis.value < 0 ? event.gaxis.value / 32768.0f : event.gaxis.value / 32767.0f;
        padAxisEvent(axis, value);
        return true;
    }
    case SDL_EVENT_GAMEPAD_REMOVED:
        padDisconnected();
        return true;
    case SDL_EVENT_GAMEPAD_SENSOR_UPDATE: {
        // The app enables the sensors and reports the pad's labels (padConnected),
        // which need SDL calls.
        if (event.gsensor.sensor != SDL_SENSOR_GYRO && event.gsensor.sensor != SDL_SENSOR_ACCEL) return false;
        const Uint64 ns = event.gsensor.sensor_timestamp != 0 ? event.gsensor.sensor_timestamp : event.gsensor.timestamp;
        padSensorEvent(event.gsensor.sensor == SDL_SENSOR_GYRO ? PadSensor::Gyro : PadSensor::Accel, event.gsensor.data,
                       static_cast<double>(ns) * 1e-9);
        return true;
    }
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        mouseLeft();
        return true;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        focusChanged(true);
        return true;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        focusChanged(false);
        return true;
    default:
        return false;
    }
}

}  // namespace PetariNative::Input::SDL3
