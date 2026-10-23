#include <petari/home_menu.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifdef PETARI_HOME_MENU_TEST_IMGUI
#include <imgui.h>
#endif

namespace HM = PetariNative::HomeMenu;
namespace {
int checks = 0;
void check(bool ok, const char* label) {
    ++checks;
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", label); std::exit(1); }
}
void press(HM::Menu& menu, std::uint32_t key) {
    HM::FrameInput input; input.hold = input.trigger = key; menu.update(input);
}
class FakeDisplay final : public HM::DisplayBackend {
public:
    HM::DisplayOptions options;
    int changes = 0;
    const char* message = "";
    void read(HM::DisplayOptions* out, char* text, std::size_t capacity) override {
        *out = options; std::snprintf(text, capacity, "%s", message);
    }
    void change(int row, int delta) override {
        ++changes;
        if (row == 0) options.fullscreen = !options.fullscreen;
        if (row == 1) options.size = (options.size + delta + HM::kDisplaySizeCount) % HM::kDisplaySizeCount;
        if (row == 2) { options.vsync = !options.vsync; options.restartNeeded = options.vsync; }
        if (row == 3) { options.ubershaders = !options.ubershaders; options.ubershadersRestartNeeded = !options.ubershaders; }
    }
};
void openDisplay(HM::Menu& menu) {
    menu.open(); for (int i = 0; i < 10; ++i) menu.update({});
    auto view = menu.view();
    check(view.itemCount == 7 && std::strcmp(view.items[4].label, "Display") == 0, "main list exposes Display");
    check(std::strstr(view.message, "F1 or `") && std::strstr(view.message, "Controls"), "menu/reference discoverability");
    for (int i = 0; i < 4; ++i) press(menu, HM::Button::Down);
    press(menu, HM::Button::A);
    check(menu.phase() == HM::Phase::Display && menu.focus() == 0, "keyboard/controller opens Display");
}
void click(HM::Menu& menu, int row) {
    const auto rect = menu.view().items[row].rect;
    HM::FrameInput input;
    input.pointerValid = true; input.pointerX = (rect.x0 + rect.x1) / 2; input.pointerY = (rect.y0 + rect.y1) / 2;
    input.trigger = input.hold = HM::Button::B;
    menu.update(input);
}
}
int main() {
    HM::Menu menu; menu.create({}); menu.setFooterKeys("Space", "Escape", "F1 / `", "Left/Right");
    FakeDisplay backend; menu.setDisplayBackend(&backend);
    openDisplay(menu);
    auto view = menu.view();
    check(view.itemCount == 5 && std::strstr(view.items[1].label, "1280 x 720"), "four options and Back");
    check(std::strstr(view.message, "quitting and reopening") && std::strstr(view.footer, "change"), "restart and navigation hints");
    check(std::strcmp(view.items[3].label, "Ubershaders: On") == 0 && std::strstr(view.message, "Ubershaders"),
          "default-on ubershaders row explains relaunch");
    click(menu, 3);
    check(!backend.options.ubershaders && std::strstr(menu.view().items[3].label, "Off (restart needed)"), "mouse toggles ubershaders off");
    press(menu, HM::Button::Left);
    check(backend.options.ubershaders && std::strcmp(menu.view().items[3].label, "Ubershaders: On") == 0,
          "left toggles the saved preference back on");
    backend.changes = 0; click(menu, 0);
    check(backend.options.fullscreen && !menu.view().items[1].enabled, "fullscreen toggle disables size row");
    press(menu, HM::Button::Down); press(menu, HM::Button::Right); press(menu, HM::Button::A); click(menu, 1);
    check(backend.changes == 1, "disabled size ignores keyboard/controller/mouse");
    click(menu, 0); click(menu, 1);
    check(backend.options.size == 1 && std::strstr(menu.view().items[1].label, "1600 x 900"), "mouse activates size");
    press(menu, HM::Button::Left);
    check(backend.options.size == 0, "left changes preset");
    press(menu, HM::Button::Left);
    check(backend.options.size == 3, "left wraps");
    press(menu, HM::Button::Right);
    check(backend.options.size == 0, "right wraps");
    click(menu, 2);
    check(std::strstr(menu.view().items[2].label, "restart needed"), "vsync restart note");
    backend.options.busy = true; menu.update({});
    const int changes = backend.changes; click(menu, 0); press(menu, HM::Button::Right);
    check(backend.changes == changes && !menu.view().items[0].enabled, "pending host request disables edits");
    backend.options.busy = false; backend.message = "Could not save display settings. Check your user folder."; menu.update({});
    check(std::strstr(menu.view().message, "Could not save"), "backend failures visible");
#ifdef PETARI_HOME_MENU_TEST_IMGUI
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr; ImGui::GetIO().LogFilename = nullptr;
    unsigned char* pixels; int width, height; ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    for (bool wide : {false, true}) {
        menu.setWidescreen(wide);
        const float w = wide ? 854.0f : 640.0f;
        auto& io = ImGui::GetIO(); io.DisplaySize = ImVec2(w, 480); io.DeltaTime = 1.0f / 60;
        HM::publish(menu.view()); ImGui::NewFrame(); HM::drawImGuiOverlay(0, 0, w, 480); ImGui::Render();
        const auto* data = ImGui::GetDrawData();
        check(data && data->TotalVtxCount > 100, "Display renders offscreen at 480 lines in both aspects");
        for (const auto* list : data->CmdLists) for (const auto& vertex : list->VtxBuffer) {
            check(vertex.pos.x >= -1 && vertex.pos.x <= w + 1 && vertex.pos.y >= -1 && vertex.pos.y <= 481,
                  "Display draw geometry stays in image");
        }
    }
    ImGui::DestroyContext();
#endif
    press(menu, HM::Button::Plus);
    check(menu.phase() == HM::Phase::List && menu.focus() == 4, "back returns to Display entry");
    press(menu, HM::Button::A); click(menu, HM::kDisplayOptionCount);
    check(menu.phase() == HM::Phase::List, "mouse Back");
    press(menu, HM::Button::A); press(menu, HM::Button::Home);
    for (int i = 0; i < 10; ++i) menu.update({});
    check(menu.selection() == HM::Selection::Resume, "Home closes from Display");
    openDisplay(menu); menu.startBlackOut();
    for (int i = 0; i < 30; ++i) menu.update({});
    check(menu.selection() == HM::Selection::Restart, "reset handles Display");
    menu.setDisplayBackend(nullptr); openDisplay(menu);
    check(!menu.view().items[0].enabled && std::strstr(menu.view().message, "unavailable"), "missing backend explicit");
    press(menu, HM::Button::A);
    std::printf("display menu tests passed (%d checks)\n", checks);
}
