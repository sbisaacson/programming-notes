// Miscellaneous notes on C++.

#include <cinttypes>
#include <memory>
#include <new>

#include "gtest/gtest.h"

namespace {

// Allocate uninitialized aligned memory with placement new.
//
// NB. valgrind 3.16.1 complains about a mismatched free / delete /
// delete[] in this function when compiling with clang++ 9.0.0.
TEST(Memory, AlignedNew) {
    std::unique_ptr<uint64_t[]> aligned(new (std::align_val_t(4096))
                                            uint64_t[5]);
    uintptr_t aligned_address = reinterpret_cast<uintptr_t>(aligned.get());
    ASSERT_EQ(0, aligned_address & 4095) << "Address is not aligned";
}

// Owned initialized memory (compare Option<Box<[u64]>> in Rust).
//
// The generated assembly for make_unique (in clang) calls
//
//     memset(<result of new>, 0, 0x28)
//
// and valgrind doesn't complain about unitialized memory; the actual
// test would be undefined behavior otherwise.
TEST(Memory, OwnedInitialized) {
    auto owned = std::make_unique<uint64_t[]>(5);
    for (int i = 0; i < 5; ++i)
	ASSERT_EQ(0, owned.get()[i]) << "Memory is not initialized";
}

} // namespace
