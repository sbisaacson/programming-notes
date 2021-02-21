#include <algorithm>
#include <cinttypes>

uint64_t gcd(uint64_t a, uint64_t b) {
    if (a == 0)
        return b;
    uint32_t v = __builtin_ctzll(a | b);
    a >>= v;
    b >>= v;
    if (b & 1)
        std::swap(a, b);
    while (b) {
        b >>= __builtin_ctzll(b);
        if (a > b)
            std::swap(a, b);
        b -= a;
    }
    return a << v;
}
