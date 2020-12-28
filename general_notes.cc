// Miscellaneous notes on C++. In general, I prefer writing Rust, but
// sometimes C++ is necessary.

#include <cinttypes>
#include <functional>
#include <memory>
#include <new>
#include <numeric>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include <gsl/span>

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
    EXPECT_EQ(0, aligned_address & 4095) << "Address is not aligned";
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
        EXPECT_EQ(0, owned.get()[i]) << "Memory is not initialized";
}

// The CRTP is the C++ analogue for default trait methods in
// Rust. When overloading a function, a using statement is necessary.
// We have to cast to the base class.

template <typename T> class CrtpBase {
  public:
    int g_add_1(int x) const { return static_cast<const T &>(*this).g(x) + 1; }

    int f(int x, int y) const {
        const T &derived = static_cast<const T &>(*this);
        return derived.f(x) + derived.f(y);
    }
};

class CrtpDerived : public CrtpBase<CrtpDerived> {
  public:
    using CrtpBase::f;
    explicit CrtpDerived(int a) : a_(a) {}
    int f(int x) const { return x + a_; }
    int g(int y) const { return 2 * y - a_; }

  private:
    int a_;
};

TEST(CRTP, Example) {
    CrtpDerived obj(2);
    EXPECT_EQ(3, obj.f(1));
    EXPECT_EQ(7, obj.f(1, 2));
    EXPECT_EQ(0, obj.g(1));
    EXPECT_EQ(1, obj.g_add_1(1));
}

// Type erasure (dyn Trait in Rust or existential types in Haskell) is
// achieved with two (or sometimes three) classes. Essentially, we
// have to virtualize something.  See [1].  Note that trait objects in
// Rust always include drop as the first method in the vtable
// (followed by the size and alignment of the object); here we have to
// make that explicit.

class DisplayBase {
  public:
    virtual ~DisplayBase() = default;
    virtual std::ostream &display(std::ostream &stream) const = 0;
};

template <typename T> class Display : public DisplayBase {
  public:
    Display(T obj) : obj_(obj) {}
    std::ostream &display(std::ostream &stream) const override {
        return stream << obj_;
    }

  private:
    T obj_;
};

class DisplayOwned {
  public:
    template <typename T> DisplayOwned(T obj) : owned(new Display<T>(obj)) {}
    std::ostream &display(std::ostream &stream) const {
        return owned->display(stream);
    }

  private:
    std::unique_ptr<DisplayBase> owned;
};

TEST(TypeErasure, Example) {
    std::vector<DisplayOwned> vec;
    vec.emplace(vec.end(), 5);
    vec.emplace(vec.end(), std::string(" hello"));
    std::ostringstream buf;
    std::for_each(vec.begin(), vec.end(), [&](auto &it) { it.display(buf); });
    EXPECT_EQ("5 hello", buf.str());
}

// C++ lambdas allow closures to move values into a closure; to mutate
// them across calls, the mutable keyword is required. The syntax is a
// little funny, but in general, keywords that modify the implicit
// receiver (i.e, "this") come after the arguments.
std::function<int()> make_counter() {
    int count = 0;
    return [=]() mutable -> int { return count++; };
}

TEST(Closures, Mutable) {
    auto c0 = make_counter();
    auto c1 = make_counter();
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(i, c0());
        EXPECT_EQ(i, c1());
    }
    // Surprisingly, cloning closures is allowed:
    auto c2 = c0;
    EXPECT_EQ(10, c2());
    EXPECT_EQ(11, c2());
    EXPECT_EQ(10, c0());
}

// span<T> in C++20 is analogous to NonNull<[T]> in Rust (C++ doesn't
// have lifetimes). There's not an obvious analogy to an owned slice
// (Box<[T]>). Spans have a lot of ergonomic conversions.  Passing
// span<T> is preferable to (T*, size) (see [2], but note that a lot
// of their advice is controversial).  It's available in C++17 with
// the Microsoft GSL (Guidelines Support Library, not GNU Scientific
// Library; libmsgsl-dev).
TEST(Spans, Example) {
    // The "sum" operator is not idiomatic C++-functions should accept
    // iterators unless they need a contiguous range of memory.
    auto sum = [](gsl::span<const int> sp) -> int {
        return std::accumulate(sp.begin(), sp.end(), 0);
    };
    std::vector<int> v(10);
    std::iota(v.begin(), v.end(), 0);
    EXPECT_EQ(45, sum(v));          // Automatically convert vector to span
    EXPECT_EQ(10, sum({&v[0], 5})); // Use (pointer, size) constructor
    EXPECT_EQ(7, sum({&v[3], 2}));
    gsl::span<const int> vs{v};
    EXPECT_EQ(45, sum(vs));
    EXPECT_EQ(10, sum(vs.subspan(0, 5))); // subspan(start, size)
    EXPECT_EQ(7, sum(vs.subspan(3, 2)));
    int primes[5] = {2, 3, 5, 7, 11};
    EXPECT_EQ(28, sum(primes)); // spans can be constructed from
                                // C-style arrays
}

} // namespace

// [1]:
// https://stackoverflow.com/questions/18453145/how-is-stdfunction-implemented
// [2]:
// https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md
