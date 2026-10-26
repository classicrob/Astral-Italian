// The app includes this header from C++17 TUs. Keep this target at C++17.
#include "cache_sha256.hpp"
#include <cstdio>
#include <string>

int main() {
    namespace C = PetariPipelineCache;
    if (C::digest("") != "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855") return 1;
    if (C::digest("abc") != "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") return 2;
    if (C::digest(std::string(1000000, 'a')) != "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0") return 3;
    C::Sha256 stream;
    for (char c : std::string("abc")) stream.update(&c, 1);
    if (stream.finish() != C::digest("abc")) return 4;
    std::puts("C++17 SHA256 known vectors and streaming PASS");
    return 0;
}
