#pragma once
#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <vector>
namespace PetariNative::Headless {
const std::vector<std::string>& traceColumns();
struct State {
    std::map<std::string, std::string> fields;
    void text(const std::string& key, std::string value);
    void integer(const std::string& key, std::uint64_t value);
    void real(const std::string& key, float value);
};
class Trace {
public:
    explicit Trace(std::ostream& out);
    void write(std::uint64_t frame, const State& state);
    void finish();
private:
    std::ostream& mOut;
    std::uint64_t mNext = 0;
};
State observeGame();
}
