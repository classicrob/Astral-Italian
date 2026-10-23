#include "display_settings.hpp"

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <unistd.h>

namespace PetariNative::App::DisplaySettings {
namespace {
bool fail(std::string* error, const char* text) {
    if (error) *error = text;
    return false;
}
}

Settings clamp(Settings settings) {
    settings.size = std::clamp(settings.size, 0, HomeMenu::kDisplaySizeCount - 1);
    settings.restartNeeded = false;
    settings.busy = false;
    return settings;
}

bool load(const std::filesystem::path& file, Settings* settings, std::string* error) {
    *settings = {};
    if (error) error->clear();
    std::error_code ec;
    if (!std::filesystem::exists(file, ec) && !ec) return true;
    std::ifstream input(file);
    if (!input) return fail(error, "Could not read display.txt. Check that your user folder is readable.");
    Settings parsed;
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty() || line[0] == '#') continue;
        const auto equals = line.find('=');
        if (equals == std::string::npos) return fail(error, "Invalid display.txt. Remove it to restore the default display settings.");
        const std::string key = line.substr(0, equals);
        if (key != "fullscreen" && key != "size" && key != "vsync") continue;
        const char* begin = line.data() + equals + 1;
        const char* end = line.data() + line.size();
        if (end > begin && end[-1] == '\r') --end;
        int value = 0;
        const auto result = std::from_chars(begin, end, value);
        if (result.ec != std::errc{} || result.ptr != end ||
            (key != "size" && value != 0 && value != 1)) {
            return fail(error, "Invalid display.txt. Remove it to restore the default display settings.");
        }
        if (key == "fullscreen") parsed.fullscreen = value != 0;
        if (key == "size") parsed.size = value;
        if (key == "vsync") parsed.vsync = value != 0;
    }
    if (input.bad()) return fail(error, "Could not read display.txt. Check that your user folder is readable.");
    *settings = clamp(parsed);
    return true;
}

bool save(const std::filesystem::path& file, Settings settings, std::string* error) {
    if (error) error->clear();
    settings = clamp(settings);
    std::error_code ec;
    if (!file.parent_path().empty()) std::filesystem::create_directories(file.parent_path(), ec);
    if (ec) return fail(error, "Could not save display settings. Check that your user folder is writable, then change the option again.");
    // Unique sibling, then rename: readers see the complete old or new file.
    std::string temporary = file.string() + ".tmp-XXXXXX";
    const int fd = mkstemp(temporary.data());
    if (fd < 0) return fail(error, "Could not save display settings. Check that your user folder is writable, then change the option again.");
    FILE* output = fdopen(fd, "w");
    bool ok = output != nullptr;
    if (output) {
        ok = std::fprintf(output, "# Display settings; size: 0=1280x720, 1=1600x900, 2=1920x1080, 3=2560x1440\nfullscreen=%d\nsize=%d\nvsync=%d\n",
                          settings.fullscreen, settings.size, settings.vsync) > 0;
        if (std::fflush(output) != 0 || fsync(fd) != 0) ok = false;
        if (std::fclose(output) != 0) ok = false;
    } else {
        close(fd);
    }
    if (ok) {
        std::filesystem::rename(temporary, file, ec);
        ok = !ec;
    }
    if (!ok) {
        std::filesystem::remove(temporary, ec);
        return fail(error, "Could not save display settings. Check that your user folder is writable, then change the option again.");
    }
    return true;
}

bool Controller::configure(const std::filesystem::path& file, std::string* error) {
    std::lock_guard lock(mMutex);
    mFile = file;
    const bool ok = load(file, &mSettings, error);
    mStartupVsync = mSettings.vsync;
    mPending = -1;
    mMessage = ok ? "" : (error ? *error : "Could not read display.txt. Remove it to restore defaults.");
    return ok;
}

Settings Controller::settings() {
    std::lock_guard lock(mMutex);
    return mSettings;
}

void Controller::read(Settings* out, char* message, std::size_t capacity) {
    std::lock_guard lock(mMutex);
    *out = mSettings;
    if (capacity) std::snprintf(message, capacity, "%s", mSettings.busy ? "Applying display settings..." : mMessage.c_str());
}

void Controller::change(int row, int delta) {
    std::lock_guard lock(mMutex);
    if (mSettings.busy || row < 0 || row > 2 || (row == 1 && mSettings.fullscreen)) return;
    mPending = row;
    mDelta = delta;
    mSettings.busy = true;
}

void Controller::tick(Window& window) {
    Settings next;
    int row, delta;
    {
        std::lock_guard lock(mMutex);
        next = mSettings;
        row = mPending;
        delta = mDelta;
        mPending = -1;
    }
    const bool actual = window.fullscreen();
    if (row < 0 && actual == next.fullscreen) return;
    std::string message;
    bool ok = true;
    if (row == 0) {
        ok = window.setFullscreen(!actual);
    } else if (row == 1 && !actual) {
        const int size = (next.size + (delta < 0 ? -1 : 1) + HomeMenu::kDisplaySizeCount) % HomeMenu::kDisplaySizeCount;
        const auto preset = HomeMenu::kDisplaySizes[size];
        ok = window.setSize(preset.width, preset.height);
        if (ok) next.size = size;
    } else if (row == 2) {
        next.vsync = !next.vsync;
    }
    next.fullscreen = window.fullscreen();
    next.busy = false;
    next.restartNeeded = next.vsync != mStartupVsync;
    if (!ok) message = "Could not change the window. Try again after the fullscreen animation finishes.";
    else if (!save(mFile, next, &message)) {
        std::fprintf(stderr, "petari: %s\n", message.c_str());
    }
    std::lock_guard lock(mMutex);
    // A new request can arrive while an external fullscreen change is saved.
    next.busy = mPending >= 0;
    mSettings = next;
    mMessage = next.busy ? "Applying display settings..." : message;
}

Controller& controller() {
    static Controller instance;
    return instance;
}
}  // namespace PetariNative::App::DisplaySettings
