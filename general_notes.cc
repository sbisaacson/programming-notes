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

// The type std::unique_ptr<T, void (*)(T *)> is rarely what we
// want. Instead, we should define a type with operator(): then the
// destructor function isn't stored in the unique_ptr, and the
// unique_ptr can be default-constructed. (We pay the cost of the
// destructor in bloated code instead.) We can do that with a closure
// or an explicit class. Note that C++ has some odd rules that make
// working with zero-size types difficult. All objects have a minimum
// size of a single byte (AFAIK) and all instances of an object must
// have unique addresses. Fields that are zero-sized get upgraded to a
// single byte (plus whatever is needed for padding), but classes that
// inherit from a zero-sized type don't reserve any space for the
// parent.

template <typename T> struct FreeDeleter {
    void operator()(T *ptr) { free(static_cast<void *>(ptr)); }
};

TEST(TypeErasure, UniquePtr) {
    std::unique_ptr<void, void (*)(void *)> p1(malloc(4), free);
    EXPECT_EQ(sizeof(void *) + sizeof(void (*)(void *)), sizeof(p1));
    // The following fails to compile:
    //
    //     std::unique_ptr<void, void (*)(void *)> q;
    //
    // We have to say q(nullptr, free), which makes using this type in
    // a collection very challenging.
    //
    // We can use a type-erased type (which is more appropriate than a
    // function pointer---it's roughly the equivalent of Box<dyn
    // FnOnce ...> in this case). Surprisingly, we can default-construct one:
    std::unique_ptr<void, std::function<void(void *)>> p2(malloc(4), free);
    EXPECT_EQ(sizeof(void *) + sizeof(std::function<void(void *)>), sizeof(p2));
    std::unique_ptr<void, std::function<void(void *)>> p2_empty;
    // Note that std::function<void(void *)> is much larger than a
    // function pointer.

    std::unique_ptr<int, FreeDeleter<int>> p3(
        static_cast<int *>(malloc(sizeof(int))));
    // I don't know if the standard requires the following assertion
    // to be true, but it is:
    EXPECT_EQ(1, sizeof(FreeDeleter<int>));
    EXPECT_EQ(sizeof(int *), sizeof(p3));
    std::unique_ptr<int, FreeDeleter<int>> p3_empty;

    auto deleter = [](int *p) -> void { free(static_cast<void *>(p)); };
    EXPECT_EQ(1, sizeof(deleter));
    std::unique_ptr<int, decltype(deleter)> p4(
        static_cast<int *>(malloc(sizeof(int))), deleter);
    EXPECT_EQ(sizeof(int *), sizeof(p4));
}

TEST(Closures, Mutable) {
    auto c0 = make_counter();
    auto c1 = make_counter();
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(i, c0());
        EXPECT_EQ(i, c1());
    }
    // Copying closures is allowed:
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

// invoke and bind, and function in the standard library make working
// with function-like objects easier.

class ClassWithVirtualFunction {
  public:
    ClassWithVirtualFunction(int x) : x_(x) {}
    virtual int f(int a, int b) { return a * x_ + b; }

  protected:
    int x_;
};

class ChildClassWithVirtualFunction : public ClassWithVirtualFunction {
  public:
    ChildClassWithVirtualFunction(int x) : ClassWithVirtualFunction(x) {}
    int f(int a, int b) override { return 10 * (a * x_ + b); }
};

TEST(Functional, PointerToMemberFunction) {
    using RawMemberFunctionPointer =
        int (ClassWithVirtualFunction::*)(int, int);
    ChildClassWithVirtualFunction obj(10);
    RawMemberFunctionPointer ptr = &ClassWithVirtualFunction::f;
    // C++ introduces the .* and ->* operator for invoking pointers to
    // member functions. These objects are larger than function
    // pointers (since they need to store whether the function is
    // virtual and handle virtual and multiple inheritance).
    EXPECT_EQ(230, (obj.*ptr)(2, 3));
    EXPECT_EQ(230, ((&obj)->*ptr)(2, 3));
    // We can also use std::invoke, which explicitly passes the 'this'
    // argument:
    EXPECT_EQ(230, std::invoke(ptr, obj, 2, 3));
    EXPECT_EQ(230, std::invoke(ptr, &obj, 2, 3));
    // Functions that need a function argument should probably use
    // std::function. We can create some type-erased versions of our
    // function pointer, though the syntax is unbelievable:
    std::function<int(int, int)> bound_ptr =
        std::bind(&ClassWithVirtualFunction::f, &obj, std::placeholders::_1,
                  std::placeholders::_2);
    EXPECT_EQ(230, std::invoke(bound_ptr, 2, 3));
    // Closure notation is arguably easier to read than std::bind with
    // std::placeholders, but it is still a lot of syntax:
    std::function<int(int, int)> alt_bound_ptr = [&obj](int a, int b) -> int {
        return obj.f(a, b);
    };
    EXPECT_EQ(230, std::invoke(alt_bound_ptr, 2, 3));
    // As far as I know, you can't just write &obj.f.
}

} // namespace

// [1]:
// https://stackoverflow.com/questions/18453145/how-is-stdfunction-implemented
// [2]:
// https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md
