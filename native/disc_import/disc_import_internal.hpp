#pragma once
#include "disc_import.hpp"

namespace PetariNative::DiscImport::Detail {
// Test seams: public entry points always use actual free space and the repository's DOL digest.
using FreeSpaceFn = std::function<std::uintmax_t(const std::filesystem::path&)>;
Result importWithSpace(const std::filesystem::path& source, const std::filesystem::path& dest,
                       const ProgressFn& progress, std::string* error, const FreeSpaceFn& freeSpace);
bool validateTree(const std::filesystem::path& root, bool quick, std::string* error, const char* expectedSha1);
}
