#pragma once

#include "petari/home_menu.hpp"

#include <filesystem>
#include <mutex>
#include <string>

namespace PetariNative::App::DisplaySettings {

using Settings = HomeMenu::DisplayOptions;
Settings clamp(Settings settings);
bool load(const std::filesystem::path& file, Settings* settings, std::string* error);
bool save(const std::filesystem::path& file, Settings settings, std::string* error);

// Only tick calls the window, on the host thread. Menu callbacks use a mailbox.
class Window {
public:
    virtual ~Window() = default;
    virtual bool fullscreen() = 0;
    virtual bool setFullscreen(bool on) = 0;
    virtual bool setSize(int width, int height) = 0;
};

class Controller final : public HomeMenu::DisplayBackend {
public:
    bool configure(const std::filesystem::path& file, std::string* error);
    Settings settings();
    void read(Settings* out, char* message, std::size_t capacity) override;
    void change(int row, int delta) override;
    void tick(Window& window);

private:
    std::mutex mMutex;
    std::filesystem::path mFile;
    Settings mSettings;
    bool mStartupVsync = true;
    bool mStartupUbershaders = true;
    int mPending = -1;
    int mDelta = 1;
    std::string mMessage;
};
Controller& controller();

}  // namespace PetariNative::App::DisplaySettings
