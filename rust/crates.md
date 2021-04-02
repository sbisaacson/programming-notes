# Bookmarks on crates

Rust crates all live in one giant namespace on
[`crates.io`](https://crates.io) and don't necessarily have obvious
names. 

# Useful links:

* [`lib.rs`](https://lib.rs) is an unofficial curated index of crates.
* [`crates.io`](https://crates.io) is the official Rust crate registery.
* [`docs.rs`](https://docs.rs) has searchable documentation for many crates.
* [Standard library documentation](https://doc.rust-lang.org/std/index.html)
* [Reference](https://doc.rust-lang.org/reference/index.html)
* [Rustonomicon](https://doc.rust-lang.org/nomicon/index.html) (reference for
  `unsafe` Rust)
* [Cargo documentation](https://doc.rust-lang.org/cargo/index.html)
* [Rustdoc documentation](https://doc.rust-lang.org/rustdoc/index.html)
* [Rust API guidelines](https://rust-lang.github.io/api-guidelines/about.html)

## Data structures and general programming

### `bytes`

[`bytes`](https://crates.io/crates/bytes) is a crate for reading data
from and writing data to byte buffers.

### `hashbrown`

[`hashbrown`](https://crates.io/crates/hashbrown) is a
high-performance hash table. It is now the implementation of
`std::collections::HashMap`, but note that a `HashMap` uses
[SipHash](https:://en.wikipedia.org/Siphash), which is very slow.
Alternative hashes are

- [`ahash`](https://crates.io/crates/ahash), a
  "non-cryptographic hash" that uses AES-NI on x86
- [`fxhash`](https://crates.io/crates/fxhash), a fast hash that is not
  collision-resistant.
- [`fnv`](https://crates.io/crates/fnv) is a fast hash for short items
  that is not collision-resistant.

### `regex`

[`regex`](https://crates.io/crates/regex) is the standard regular
expression engine.

### `lazy_static`

[`lazy_static`](https://crates.io/crates/lazy_static) allows you to
initialize static variables at run-time.

### `priority-queue`

[`priority-queue`](https://crates.io/crates/priority-queue) is an
implementation of priority queues. Unlike
`std::collections::BinaryHeap`, it allows efficiently changing the
priority of an item. Both `BinaryHeap` and
`priority_queue::PriorityQueue` are max-heaps and you are advised to
use `std::cmp::Reverse` to wrap items to form a min-heap.

### `smallvec`

[`smallvec`](https://crates.io/crates/smallvec) defines a
`SmallVec<[T; n]>` type that stores up to `n` items within the value
before making a heap allocation.

### `bumpalo`

[`bumpalo`](https://crates.io/crates/bumpalo) is a bump allocator
for Rust.

### `jemallocator`

[`jemallocator`](https://crates.io/crates/jemallocator) lets you
use [`jemalloc`](http://jemalloc.net) in place of the standard allocator.

### `itertools`

[`itertools`](https://crates.io/crates/itertools) is similar to the Python
library of the same name.

## OS and IO

### `libc`

[`libc`](https://crates.io/crates/libc) is the standard binding to
`libc`. Almost everything is `unsafe`.

### `nix`

[`nix`](https://crates.io/crates/nix) is a high-level `libc` binding.

### `io-uring`

[`io-uring`](https://crates.io/crates/io-uring) is a binding to the Linux
`io-uring` interface.

### `memmap`

[`memmap`](https://crates.io/crates/memmap) supports the `mmap(2)`
system call. In my experience it's better to use `pread` and `pwrite`
unless absolutely necessary. Note that the `read_at` and `write_at`
methods (exposed by the UNIX prelude) take a `&self` receiver, so you
can share a file among threads.

## Parallel programming

### `rayon`

[`rayon`](https://crates.io/crates/rayon) is a parallel programming
framework akin to TBB. (The `par_chunks()` method gives an interface
similar to `tbb::parallel_for` with an explicit grain size).

### `crossbeam`

[`crossbeam`](https://crates.io/crates/crossbeam) implements channels,
scoped threads, and other utilities for multicore programming. The
channels in crossbeam are more feature-rich than the ones in the
standard library.

### `parking_lot`

[`parking_lot`](https://crates.io/crates/parking_lot) implements
versions of `Mutex`, `RwLock`, `Condvar`, and `Once` that are faster
than the standard library.

## Serialization

### `serde`

[`serde`](https://serde.rs) is the standard crate for serialization and
deserialization. Most crates include serde support as an optional
feature. It supports zero-copy deserialization for some formats
(prefer `Cow<'a, str>` to an owned `String`).

### `serde_json`

[`serde_json`](https://crates.io/crates/serde_json) supports JSON
serialization and deserialization for `serde`.

### `bincode`

[`bincode`](https://crates.io/crates/bincode) is a compact binary
format for `serde`. The authors claim that `bincode`-serialized objects
are no larger than they are in memory.

### `flexbuffers`

[`flexbuffers`](https://crates.io/crates/flexbuffers) is a binary
format for `serde` developed by Google.

### `yaml-rust`

[`yaml-rust`](https://crates.io/crates/yaml-rust) is a pure Rust
implementation of [YAML](https://yaml.org). [Avoid YAML](https://github.com/cblp/yaml-sucks)
unless it is necessary.

### `serde_yaml`

[`serde_yaml`](https://crates.io/crates/serde_yaml) provides glue
to use YAML as a `serde` format (using `yaml-rust` as the parser).

### `protobuf`

[`protobuf`](https://crates.io/crates/protobuf) is a pure Rust
implementation of [Google
`protobuf`](https://developers.google.com/protocol-buffers).

## Numerical code

### `rug`

[`rug`](https://crates.io/crates/rug) is a GMP wrapper.

### `ndarray`

[`ndarray`](https://crates.io/crates/ndarray) is a multi-dimensional
array library (roughly similar to `numpy`). The companion package
[`ndarray-linalg`](https://crates.io/crates/ndarray-linalg) supports
BLAS through an external backend (use the `openblas`, `netlib`, or
`intel-mkl` features when compiling as part of an executable).

### `num`

[`num`](https://crates.io/crates/num) has a lot of functionality:
* Traits for primitive integer types, allowing methods that are
  generic over `u64`, `u32`, etc.
* Arithmetic shift methods on unsigned integers (`signed_shr`)
* Complex numbers

### `GSL`

[`GSL`](https://crates.io/crates/GSL) is a binding for the [GNU Scientific
Library](https://www.gnu.org/software/gsl/).

## Error handling

### `backtrace`

[`backtrace`](https://crates.io/crates/backtrace) is analogous to
`backtrace(7)` in GLIBC and allows you to obtain a backtrace at
runtime. It's useful for error reporting: include a
`backtrace::Backtrace` in the error arm of a `Result`. Otherwise, by
the time the error bubbles up the callchain, you will lose the
backtrace.

### `snafu`

[`snafu`](https://crates.io/crates/snafu) is a crate that makes error
handling more ergonomic.

## CLI

### `structopt`

[`structopt`](https://crates.io/crates/structopt) simplifies CLI
implementation: you just have to decorate an argument structure with
some macros.

## Testing

### `quickcheck`

[`quickcheck`](https://crates.io/crates/quickcheck) allows
`hypothesis`-style tests.

### `proptest`

[`proptest`](https://crates.io/crates/proptest) also allows
`hypothesis`-style tests, but has fancier strategies.

### `afl`

[`afl`](https://crates.io/crates/afl) is a Rust interface for the
[American Fuzzy Lop](https://lcamtuf.coredump.cx/afl/) fuzzer. See
John Regehr's posts on
[fuzzing](https://blog.regehr.org/archives/1687) and
[assertions](https://blog.regehr.org/archives/1091).

### `cargo-fuzz`

[`cargo-fuzz`](https://crates.io/crates/cargo-fuzz) is an alternative
fuzzer.

### `honggfuzz`

[`honggfuzz`](https://crates.io/crates/honggfuzz) is a fuzzer
developed by Google. It takes some work to set up: you need to install

- `build-essentials`
- `binutils-dev`
- `libunwind-dev`
- `libblocksruntime-dev`
- `liblzma-dev`

before running `cargo install honggfuzz`.

### `criterion`

[`criterion`](https://crates.io/crates/criterion) is a library for
microbenchmarking.

### `bencher`

[`bencher`](https://crates.io/crates/bencher) is a port of libtest to
stable Rust.

### `benchcmp`

[`benchcmp`](https://crates.io/crates/benchcmp) is a tool for
comparing the output of two runs of `cargo bench`.

## Databases

### `rusqlite`

[`rusqlite`](https://crates.io/crates/rusqlite) is a
[`SQLite`](https://sqlite.org) binding.

## Logging

### `log`

[`log`](https://crates.io/crates/log) is the standard logging
crate. Executables also need a logger implementation;
[`env_logger`](https://crates.io/crates/env_logger) is what I usually
use.

### `slog`

[`slog`](https://crates.io/crates/slog) provides structure logging,
which should eventually be added to `log`.

### `tracing`

[`tracing`](https://crates.io/crates/tracing) facilitates
instrumenting code with diagnostics. It is part of the `tokio`
project.

## Date and time handling

### `chrono`

[`chrono`](https://crates.io/crates/chrono) provides date and time
handling and provides a superset of the
[`time`](https://crates.io/crates/time) functionality.

## FFI and Rust development

### `cc`

[`cc`](https://crates.io/crates/cc) is the standard way to invoke a C
compiler in a `build.rs` script.

### `bindgen`

[`bindgen`](https://crates.io/crates/bindgen) generates Rust bindings
for C (and a restricted subset of C++) headers.

### `rlua`

[`rlua`](https://crates.io/crates/rlua) is a safe binding for Lua.

### `pyo3`

[`pyo3`](https://pyo3.rs/) is a binding for Python. It has an
associated package ([`maturin`](https://github.com/PyO3/maturin)) for
building Python packages with Rust code; install with `python -m pip
install maturin`.

### `guppy`

[`guppy`](https://crates.io/crates/guppy) is a tool for parsing `cargo
metadata` output and can be used to make dependency graphs.

## Debugging and binary analysis

### `object`

[`object`](https://crates.io/crates/object) is a library for reading
ELF files.

### `gimli`

[`gimli`](https://crates.io/crates/gimli) is a library for reading
DWARF debugging data.

### `addr2line`

[`addr2line`](https://crates.io/crates/addr2line) is a library that
converts instruction addresses to debugging data (using `gimli`).

## Parsing

### `lalrpop`

[`lalrpop`](https://crates.io/crates/lalrpop) is a LALR(1) parser
generator.

### `nom`

[`nom`](https://crates.io/crates/nom) is a parser combinator library.

### `combine`

[`combine`](https://crates.io/crates/combine) is also a parser
combinator library.

## Async

### `async-std`

[`async-std`](https://crates.io/crates/async-std) is an async
replacement for some standard library facilities.

### `tokio`

[`tokio`](https://crates.io/crates/tokio) is the standard async
runtime.

### `mio`

[`mio`](https://crates.io/crates/mio), also developed by the `tokio`
developers, is a low-level library for async network communication.

### `io-uring`

[`io-uring`](https://crates.io/crates/io-uring) is an interface for
the Linux `io_uring` system calls. It is developed by the `tokio`
developers.
