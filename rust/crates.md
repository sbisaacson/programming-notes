# Bookmarks on crates

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

## Error handling

### `backtrace`

[`backtrace`](https://crates.io/crates/backtrace) is analogous to
`backtrace(7)` in GLIBC and allows you to obtain a backtrace at
runtime. It's useful for errors. Otherwise, by the time the error
bubbles up the callchain, you will lose the backtrace.

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
