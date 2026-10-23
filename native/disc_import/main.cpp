#include "disc_import.hpp"
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iomanip>
#include <iostream>

namespace fs = std::filesystem;
using namespace PetariNative::DiscImport;
namespace {
volatile std::sig_atomic_t interrupted = 0;
void cancel(int) { interrupted = 1; }
const char* formatName(Format f) {
    switch (f) {
    case Format::GcmIso: return "disc image (ISO/GCM/CISO/GCZ/NFS/TGC)";
    case Format::Wbfs: return "WBFS";
    case Format::Rvz: return "RVZ/WIA";
    case Format::ExtractedFolder: return "extracted folder";
    default: return "unknown";
    }
}
}
int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: petari_disc_import probe <image-or-folder>\n"
                     "       petari_disc_import import <image> <destination> [--cancel-at <percent>]\n"
                     "       petari_disc_import validate <folder> [--quick]\n";
        return 2;
    }
    const std::string command = argv[1];
    const auto start = std::chrono::steady_clock::now();
    if (command == "probe" && argc == 3) {
        const auto info = probe(argv[2]);
        std::cout << "format=" << formatName(info.format) << "\ngame=" << info.title << "\nid=" << info.gameId
                  << "\nrevision=" << info.revision << "\nsource_bytes=" << info.sourceBytes << '\n';
        if (!info.problem.empty()) std::cerr << info.problem << '\n';
        return info.isSupportedGame ? 0 : 1;
    }
    std::string error;
    if (command == "validate" && (argc == 3 || (argc == 4 && std::string(argv[3]) == "--quick"))) {
        const bool ok = validateExtracted(argv[2], argc == 4, &error);
        std::cout << "validate=" << (ok ? "PASS" : "FAIL") << " seconds="
                  << std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() << '\n';
        if (!ok) std::cerr << error << '\n';
        return ok ? 0 : 1;
    }
    if (command == "import" && (argc == 4 || (argc == 6 && std::string(argv[4]) == "--cancel-at"))) {
        double cancelAt = 101;
        if (argc == 6) {
            char* end = nullptr;
            cancelAt = std::strtod(argv[5], &end);
            if (end == argv[5] || *end || !(cancelAt >= 0 && cancelAt <= 100)) {
                std::cerr << "Choose a cancellation percentage from 0 to 100.\n";
                return 2;
            }
        }
        std::signal(SIGINT, cancel);
        std::signal(SIGTERM, cancel);
        auto result = importDisc(argv[2], argv[3], [&](const Progress& p) {
            const double percent = p.totalBytes ? 100.0 * p.doneBytes / p.totalBytes : 0;
            std::cerr << std::fixed << std::setprecision(1) << percent << "% " << p.doneBytes << '/' << p.totalBytes
                      << " " << p.currentFile << '\n';
            return !interrupted && percent < cancelAt;
        }, &error);
        std::cout << "import=" << (result == Result::Ok ? "PASS" : result == Result::Cancelled ? "CANCELLED" : "FAIL")
                  << " seconds=" << std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() << '\n';
        if (result == Result::Ok && probe(argv[2]).format != Format::ExtractedFolder) {
            std::uint64_t files = 0, bytes = 0;
            for (const auto& entry : fs::recursive_directory_iterator(argv[3])) {
                if (entry.is_regular_file()) { ++files; bytes += entry.file_size(); }
            }
            std::cout << "files=" << files << " bytes=" << bytes << '\n';
        }
        if (!error.empty()) std::cerr << error << '\n';
        return result == Result::Ok ? 0 : result == Result::Cancelled ? 130 : 1;
    }
    std::cerr << "Unknown command or arguments. Run petari_disc_import without arguments for usage.\n";
    return 2;
}
