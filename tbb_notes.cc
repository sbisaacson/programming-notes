// Miscellaneous notes on Intel TBB
//
// ## References
//
// [1]:
// https://www.threadingbuildingblocks.org/docs/help/reference/algorithms/parallel_scan_func.html

#include <algorithm>
#include <cinttypes>
#include <functional>
#include <numeric>
#include <random>
#include <vector>

#include <tbb/blocked_range.h>
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_scan.h>

#include <gmp.h>

#include "gtest/gtest.h"

namespace {

// Simple example of parallel reduction.
TEST(TBBNotes, ParallelReduce) {
    const size_t NUM_ELEMENTS = 1048576;
    std::random_device urandom;
    std::default_random_engine rng(urandom());
    std::uniform_int_distribution<uint64_t> random_bits;
    std::vector<uint64_t> data(NUM_ELEMENTS);
    std::generate(data.begin(), data.end(), [&] { return random_bits(rng); });
    auto reducer = [](uint32_t running, uint64_t obj) -> uint32_t {
        return running + __builtin_popcountll(obj);
    };
    uint64_t expected_reduction =
        std::accumulate(data.begin(), data.end(), 0, reducer);
    uint64_t reduction = tbb::parallel_reduce(
        tbb::blocked_range<const uint64_t *>(&*data.begin(), &*data.end()), 0,
        [&](const auto &range, uint32_t state) {
            return std::accumulate(range.begin(), range.end(), state, reducer);
        },
        std::plus<uint32_t>());
    ASSERT_EQ(expected_reduction, reduction);
}

// Inclusive parallel prefix sum. See [1].
TEST(TBBNotes, ParallelPrefixSum) {
    const size_t NUM_ELEMENTS = 65536;
    std::vector<uint64_t> data(NUM_ELEMENTS);
    std::iota(data.begin(), data.end(), 0);
    tbb::parallel_scan(
        tbb::blocked_range<size_t>(0, NUM_ELEMENTS, 1024),
        0, // identity element for operation
        [&](const auto &range, uint64_t running_sum, bool is_final) {
            for (size_t i = range.begin(); i < range.end(); ++i) {
                running_sum += data[i];
                if (is_final)
                    data[i] = running_sum;
            }
            return running_sum;
        },
        [](uint64_t left, uint64_t right) { return left + right; });
    for (size_t i = 0; i < NUM_ELEMENTS; ++i)
        ASSERT_EQ(i * (i + 1) / 2, data[i]) << "Mismatch in index " << i;
}

/// Parallel scan operation that distinguishes between left and right.
/// See [1].
class Horner {
  public:
    Horner(uint64_t multiplier, uint64_t *data)
        : num_terms_(0), multiplier_(multiplier), sum_(0), data_(data) {}

    Horner(Horner &other, tbb::split)
        : num_terms_(0), multiplier_(other.multiplier_), sum_(0),
          data_(other.data_) {}

    uint64_t get_sum() const { return sum_; }

    template <typename Tag>
    void operator()(const tbb::blocked_range<size_t> &range, Tag) {
        for (size_t i = range.begin(); i < range.end(); ++i) {
            sum_ = (multiplier_ * sum_) + data_[i];
            // NB. Tag::is_final_scan is not a constexpr.
            if (Tag::is_final_scan()) {
                data_[i] = sum_;
            }
        }
        num_terms_ += range.size();
    }

    void reverse_join(Horner &left) {
        sum_ = exponentiate(multiplier_, num_terms_) * left.sum_ + sum_;
        num_terms_ += left.num_terms_;
    }

    void assign(Horner &other) {
        sum_ = other.sum_;
        num_terms_ = other.num_terms_;
    }

    static uint64_t exponentiate(uint64_t base, uint64_t power) {
        if (power == 0)
            return 1;
        uint64_t acc = base;
        for (int pos = 62 - __builtin_clzll(power); pos >= 0; --pos) {
            acc = acc * acc;
            if ((power >> pos) & 1)
                acc *= base;
        }
        return acc;
    }

  private:
    size_t num_terms_;
    uint64_t multiplier_;
    uint64_t sum_;
    uint64_t *data_;
};

TEST(TBBNotes, ParallelPolynomialEvaluate) {
    const size_t NUM_ELEMENTS = 65536;
    const uint64_t MULTIPLIER = 3;
    std::vector<uint64_t> data(NUM_ELEMENTS);
    std::vector<uint64_t> expected_result(NUM_ELEMENTS);
    std::iota(data.begin(), data.end(), 0);
    uint64_t running_sum = 0;
    for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
        running_sum = MULTIPLIER * running_sum + data[i];
        expected_result[i] = running_sum;
    }
    Horner worker(MULTIPLIER, data.data());
    tbb::parallel_scan(tbb::blocked_range<size_t>(0, NUM_ELEMENTS, 1024),
                       worker);

    for (size_t i = 0; i < NUM_ELEMENTS; ++i)
        ASSERT_EQ(expected_result[i], data[i]) << "Mismatch in index " << i;
}

// Use of a concurrent vector within a `parallel_for` to store
// follow-on work.
uint64_t add(uint64_t *rp, uint64_t *s1p, uint64_t *s2p, size_t num_limbs) {
    tbb::concurrent_vector<size_t> unresolved_carries;
    tbb::parallel_for(tbb::blocked_range<size_t>(0, num_limbs, 1024),
                      [&](const auto &range) {
                          if (mpn_add_n(rp + range.begin(), s1p + range.begin(),
                                        s2p + range.begin(), range.size()))
                              unresolved_carries.push_back(range.end());
                      });
    uint64_t rval = 0;
    for (const auto &it : unresolved_carries) {
        rval |=
            (it == num_limbs) || mpn_add_1(rp + it, rp + it, num_limbs - it, 1);
    }
    return rval;
}

TEST(TBBNotes, ConcurrentVector) {
    const size_t NUM_LIMBS = 1048576;
    std::random_device urandom;
    std::default_random_engine rng(urandom());
    std::uniform_int_distribution<uint64_t> random_bits;
    std::vector<uint64_t> s1(NUM_LIMBS);
    std::vector<uint64_t> s2(NUM_LIMBS);
    std::vector<uint64_t> r(NUM_LIMBS);
    std::vector<uint64_t> expected_r(NUM_LIMBS);
    std::generate(s1.begin(), s1.end(), [&] { return random_bits(rng); });
    std::generate(s2.begin(), s2.end(), [&] { return random_bits(rng); });
    uint64_t expected_carry =
        mpn_add_n(expected_r.data(), s1.data(), s2.data(), NUM_LIMBS);
    uint64_t carry = add(r.data(), s1.data(), s2.data(), NUM_LIMBS);
    ASSERT_EQ(expected_carry, carry);
    ASSERT_EQ(r, expected_r);
}

} // namespace
