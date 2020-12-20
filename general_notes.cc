// Miscellaneous notes on C++.

#include <cinttypes>
#include <memory>
#include <new>
#include <ostream>
#include <sstream>
#include <vector>

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
    ASSERT_EQ(3, obj.f(1));
    ASSERT_EQ(7, obj.f(1, 2));
    ASSERT_EQ(0, obj.g(1));
    ASSERT_EQ(1, obj.g_add_1(1));
}

// Type erasure (dyn Trait in Rust or existential types in Haskell) is
// achieved with two (or sometimes three) classes. Essentially, we
// have to virtualize something.  See [1].  Note that trait objects in
// Rust always include drop as the first method in the vtable
// (followed by the size and alignment of the object); here we have to
// make that explicit.
//
// [1]:
// https://stackoverflow.com/questions/18453145/how-is-stdfunction-implemented

class DisplayBase {
  public:
    virtual ~DisplayBase() = default;
    virtual std::ostream &display(std::ostream &stream) = 0;
};

template <typename T> class Display : public DisplayBase {
  public:
    Display(T obj) : obj_(obj) {}
    std::ostream &display(std::ostream &stream) override {
        return stream << obj_;
    }

  private:
    T obj_;
};

class DisplayOwned {
  public:
    template <typename T> DisplayOwned(T obj) : owned(new Display<T>(obj)) {}
    std::ostream &display(std::ostream &stream) {
        return owned->display(stream);
    }

  private:
    std::unique_ptr<DisplayBase> owned;
};

TEST(TypeErasure, Example) {
    std::vector<DisplayOwned> vec;
    vec.emplace(vec.end(), 5);
    vec.emplace(vec.end(), " hello");
    std::ostringstream buf;
    std::for_each(vec.begin(), vec.end(), [&](auto &it) { it.display(buf); });
    ASSERT_EQ("5 hello", buf.str());
}

} // namespace
