#include <petari/pipeline_startup.hpp>
#include <petari/home_menu.hpp>
#include <string>
#include <imgui.h>
#include <cmath>
#include <cstdio>
#include <limits>
#include <initializer_list>
#include <stdexcept>

static void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
int main() try {
    using namespace PetariNative;
    check(!HomeMenu::drawPipelinePreparation({}, 1280, 720), "missing ImGui context cannot draw");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr; io.LogFilename = nullptr; io.DeltaTime = 1.0f / 60;
    unsigned char* pixels; int atlasWidth, atlasHeight;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &atlasWidth, &atlasHeight);
    const ImVec2 sizes[] = {{1280, 720}, {640, 480}, {1024, 768}, {375, 812}};
    for (const auto size : sizes) {
        io.DisplaySize = size;
        for (double remaining : {-1.0, 22.0, 215.0, std::numeric_limits<double>::quiet_NaN()}) {
            ImGui::NewFrame();
            check(HomeMenu::drawPipelinePreparation({1234, 8713, remaining, true}, size.x, size.y), "startup overlay missing");
            ImGui::Render();
            const auto* data = ImGui::GetDrawData();
            check(data && data->TotalVtxCount > 500, "progress text/bar produced insufficient geometry");
            for (int list = 0; list < data->CmdListsCount; ++list) {
                for (const auto& vertex : data->CmdLists[list]->VtxBuffer) {
                    check(std::isfinite(vertex.pos.x) && std::isfinite(vertex.pos.y), "invalid startup vertex");
                    check(vertex.pos.x >= -1 && vertex.pos.y >= -1 && vertex.pos.x <= size.x + 1 && vertex.pos.y <= size.y + 1,
                          "startup overlay exceeds resized window");
                }
            }
        }
    }
    // The title's "Preparing graphics N%" pill: in the image's bottom-right corner (right half, bottom 10%), clear of the
    // centred prompt/copyright, readable at 480 lines.
    for (float fraction : {0.0f, 0.37f, 1.0f}) {
        const float w = 854.0f, h = 480.0f;
        io.DisplaySize = ImVec2(1280.0f, 800.0f);
        ImGui::NewFrame();
        HomeMenu::drawPreparingGraphics(fraction, 0.0f, 0.0f, w, h);
        ImGui::Render();
        float top = 1e9f, bottom = -1e9f, left = 1e9f, right = -1e9f, textTop = 1e9f, textBottom = -1e9f;
        int vertices = 0;
        const auto* data = ImGui::GetDrawData();
        for (int list = 0; list < data->CmdListsCount; ++list) {
            for (const auto& v : data->CmdLists[list]->VtxBuffer) {
                ++vertices;
                top = std::fmin(top, v.pos.y);
                bottom = std::fmax(bottom, v.pos.y);
                left = std::fmin(left, v.pos.x);
                right = std::fmax(right, v.pos.x);
                const ImVec4 c = ImGui::ColorConvertU32ToFloat4(v.col);
                if (c.x > 0.9f && c.y > 0.9f && c.z > 0.97f) { textTop = std::fmin(textTop, v.pos.y); textBottom = std::fmax(textBottom, v.pos.y); }
            }
        }
        check(vertices > 100, "preparing pill drawn");
        check(top >= 0.90f * h && bottom <= h + 0.5f && left >= 0.70f * w && right <= w + 0.5f,
              "preparing pill in the bottom-right corner of the image");
        check(textBottom - textTop >= 6.0f, ("preparing text readable at 480 lines: " + std::to_string(textBottom - textTop)).c_str());
    }
    ImGui::DestroyContext();
    std::puts("Preparation overlay: text/bar geometry, ETA states and four window sizes pass; title pill corner placement passes");
} catch (const std::exception& error) {
    std::fprintf(stderr, "FAIL: %s\n", error.what());
    return 1;
}
