#include "gtest/gtest.h"

#include <cinttypes>
#include <x86intrin.h>

namespace {

// The unpack intrinsics effect the following transposes:
//
//     a.b.cdef -> c.b.defa (epi8)
//     a.b.cde  -> c.b.def  (epi16)
//     a.b.cd   -> c.b.da   (epi32)
//     a.b.c    -> c.b.a    (epi64)
TEST(AVX2, UnpackDword) {
    __m256i x = _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0);
    __m256i y = _mm256_add_epi32(x, _mm256_set1_epi32(8));
    __m256i z = _mm256_unpacklo_epi32(x, y);
    __m256i w = _mm256_unpackhi_epi32(x, y);
    __m256i z_expected = _mm256_set_epi32(13, 5, 12, 4, 9, 1, 8, 0);
    __m256i w_expected = _mm256_set_epi32(15, 7, 14, 6, 11, 3, 10, 2);
    EXPECT_EQ(0xffffffff,
              _mm256_movemask_epi8(_mm256_cmpeq_epi32(z_expected, z)));
    EXPECT_EQ(0xffffffff,
              _mm256_movemask_epi8(_mm256_cmpeq_epi32(w_expected, w)));
}

TEST(AVX2, UnpackQword) {
    __m256i x = _mm256_set_epi64x(3, 2, 1, 0);
    __m256i y = _mm256_set_epi64x(7, 6, 5, 4);
    __m256i z = _mm256_unpacklo_epi64(x, y);
    __m256i w = _mm256_unpackhi_epi64(x, y);
    __m256i z_expected = _mm256_set_epi64x(6, 2, 4, 0);
    __m256i w_expected = _mm256_set_epi64x(7, 3, 5, 1);
    EXPECT_EQ(0xffffffff,
              _mm256_movemask_epi8(_mm256_cmpeq_epi64(z_expected, z)));
    EXPECT_EQ(0xffffffff,
              _mm256_movemask_epi8(_mm256_cmpeq_epi64(w_expected, w)));
}

// Cross-lane operations are very expensive---llvm-mca reports that
// vperm2f128 has 100-cycle latency on my Ryzen.
TEST(AVX2, Transpose4x4) {
    __m256i x0 = _mm256_set_epi64x(0xA3, 0xA2, 0xA1, 0xA0);
    __m256i x1 = _mm256_set_epi64x(0xB3, 0xB2, 0xB1, 0xB0);
    __m256i x2 = _mm256_set_epi64x(0xC3, 0xC2, 0xC1, 0xC0);
    __m256i x3 = _mm256_set_epi64x(0xD3, 0xD2, 0xD1, 0xD0);

    __m256i y0 = _mm256_unpacklo_epi64(x0, x1);
    __m256i y1 = _mm256_unpackhi_epi64(x0, x1);
    __m256i y2 = _mm256_unpacklo_epi64(x2, x3);
    __m256i y3 = _mm256_unpackhi_epi64(x2, x3);

    __m256i z0 = _mm256_permute2x128_si256(y0, y2, 0x20);
    __m256i z1 = _mm256_permute2x128_si256(y1, y3, 0x20);
    __m256i z2 = _mm256_permute2x128_si256(y0, y2, 0x31);
    __m256i z3 = _mm256_permute2x128_si256(y1, y3, 0x31);

    __m256i w0 = _mm256_set_epi64x(0xD0, 0xC0, 0xB0, 0xA0);
    __m256i w1 = _mm256_set_epi64x(0xD1, 0xC1, 0xB1, 0xA1);
    __m256i w2 = _mm256_set_epi64x(0xD2, 0xC2, 0xB2, 0xA2);
    __m256i w3 = _mm256_set_epi64x(0xD3, 0xC3, 0xB3, 0xA3);

    EXPECT_EQ(0xffffffff, _mm256_movemask_epi8(_mm256_cmpeq_epi64(w0, z0)));
    EXPECT_EQ(0xffffffff, _mm256_movemask_epi8(_mm256_cmpeq_epi64(w1, z1)));
    EXPECT_EQ(0xffffffff, _mm256_movemask_epi8(_mm256_cmpeq_epi64(w2, z2)));
    EXPECT_EQ(0xffffffff, _mm256_movemask_epi8(_mm256_cmpeq_epi64(w3, z3)));
}

} // namespace
