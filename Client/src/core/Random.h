#include <cstdint>

class Random {
    uint64_t current = 1;

    static constexpr uint64_t a = 6364136223846793005ULL;
    static constexpr uint64_t c = 1442695040888963407ULL;
public:
    Random() : Random(0) {}
    Random(uint64_t seed) : current(seed) {}

    inline uint64_t next() {
        current = a * current + c;
        return current;
    }

    inline uint32_t nextUInt() {
        return static_cast<uint32_t>(next());
    }
    inline uint32_t nextUInt(uint32_t Max) {
        return nextUInt() % Max;
    }
    inline uint32_t nextUInt(uint32_t Min, uint32_t Max) {
        return (nextUInt() % (Max - Min + 1)) + Min;
    }
};