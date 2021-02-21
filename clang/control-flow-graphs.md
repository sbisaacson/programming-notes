# Control-flow graphs

![Control-flow graph of `gcd`](gcd.svg)

The `clang` compiler can produce control-flow graphs with `graphviz`.
The following code is in `cfg.cc`:

```C++
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
```

Run
```sh
clang++ -O3 -Wall -Wextra -emit-llvm -S -o cfg.ll cfg.cc
opt -analyze --dot-cfg cfg.ll
mv ._Z3gcdmm.dot gcd.dot
dot -Tsvg -o gcd.svg gcd.dot
```
to produce the graph shown above.
