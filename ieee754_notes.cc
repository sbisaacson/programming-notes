#include "gtest/gtest.h"

#include <cinttypes>
#include <cstring>
#include <random>

namespace {

template <typename To, typename From> To transmute(From x) {
    static_assert(sizeof(To) == sizeof(From), "size mismatch");
    To y;
    memcpy(&y, &x, sizeof(y));
    return y;
}

// NB. Right shift of negative values is technically
// implementation-defined in C but practically is arithmetic shift:
uint64_t asr(uint64_t x, uint32_t shift) {
    return static_cast<int64_t>(x) >> shift;
}

TEST(IEEE754Notes, Transmute) {
    ASSERT_EQ(0x8000000000000000, transmute<uint64_t>(-0.0));
    ASSERT_EQ(0x3ff0000000000000, transmute<uint64_t>(1.0));
}

// On x86, clang uses a trick to convert uint64_t to double:
TEST(IEEE754Notes, ClangConvert) {
    const int ITERATIONS = 65536;
    std::random_device urandom;
    std::default_random_engine rng(urandom());
    std::uniform_int_distribution<uint64_t> random_bits;
    for (int iteration = 0; iteration < ITERATIONS; ++iteration) {
        uint64_t value = random_bits(rng);
        double expected = static_cast<double>(value);
        uint64_t low32 = value & 0xffffffff;
        uint64_t high32 = value >> 32;
        uint64_t c1 = transmute<uint64_t>(0x1p52);
        uint64_t c2 = transmute<uint64_t>(0x1p84);
        // low and high are computed with punpckldq, then a single subpd:
        double low = transmute<double>(c1 | low32) - 0x1p52;
        double high = transmute<double>(c2 | high32) - 0x1p84;
        double observed = high + low;
        ASSERT_EQ(expected, observed);
    }
}

// We can impose a total order on doubles that refines the comparison
// order (note that all comparisons with NaN are false):
int64_t double_key(double d) {
    uint64_t x = transmute<uint64_t>(d);
    return x ^ (asr(x, 63) >> 1);
}

TEST(IEEE754Notes, RefinedOrder) {
    const int ITERATIONS = 65536;
    std::random_device urandom;
    std::default_random_engine rng(urandom());
    std::uniform_int_distribution<uint64_t> random_bits;
    for (int iteration = 0; iteration < ITERATIONS; ++iteration) {
        uint64_t value1 = random_bits(rng);
	uint64_t value2 = random_bits(rng);
	ASSERT_TRUE(!(value1 < value2) || double_key(value1) < double_key(value2));
	// We can also use uint64_t keys:
        uint64_t key_value1 = double_key(value1) ^ 0x8000000000000000;
	uint64_t key_value2 = double_key(value2) ^ 0x8000000000000000;
        ASSERT_TRUE(!(value1 < value2) || key_value1 < key_value2);
    }
}

} // namespace
