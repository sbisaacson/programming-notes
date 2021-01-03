# Bookmarks on crates

## Data structures and general programming

### `hashbrown`

[`hashbrown`](https://crates.io/crates/hashbrown) is a
high-performance hash table.

### `regex`

[`regex`](https://crates.io/crates/regex) is the standard regular
expression engine.

### `lazy_static`

[`lazy_static`](https://crates.io/crates/lazy_static) allows you to
initialize static variables at run-time.

## OS and IO

### `memmap`

[`memmap`](https://crates.io/crates/memmap) supports the `mmap(2)`
system call. In my experience it's better to use `pread` and `pwrite`
unless absolutely necessary.

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

[Serde](https://serde.rs) is the standard crate for serialization and
deserialization. Most crates include serde support as an optional
feature. It supports zero-copy deserialization for some formats
(prefer `Cow<'a, str>` to an owned `String`).

### `serde_json`

[`serde_json`](https://crates.io/crates/serde_json) supports JSON
serialization and deserialization for `serde`.

### `bincode`

[`bincode`](https://crates.io/crates/bincode) is a compact binary
format for `serde`.

## Numerics

### `rug`

[`rug`](https://crates.io/crates/rug) is a GMP wrapper.

### `num-complex`

[`num-complex`](https://crates.io/crates/num-complex) is the standard
implementation of complex numbers.

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

## FFI

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
