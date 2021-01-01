// Rust closures implement Clone and Copy if the captured types do so
// (see RFC 2132).

use std::cell::Cell;

fn main() {
    let mut closure = {
        let mut count = 0;
        move || {
            count += 1;
            count
        }
    };
    assert_eq!(closure(), 1);
    assert_eq!(closure(), 2);
    let mut copy = closure;
    assert_eq!(closure(), 3);
    assert_eq!(copy(), 3);

    let global_counter = Cell::new(0i32);
    let closure = || {
        let v = global_counter.get() + 1;
        global_counter.set(v);
        v
    };
    assert_eq!(closure(), 1);
    assert_eq!(closure(), 2);
    let copy = closure;
    assert_eq!(closure(), 3);
    assert_eq!(copy(), 4);
}
