use std::cell::Cell;

fn main() {
    let arr: &mut [u64] = &mut [0, 3];

    // The borrow-checker in Rust 1.49 rejects the following code:
    //
    // fn tr(a: &mut u64, b: &mut u64, s: u32, m: u64) {
    //     let t = (*a ^ (*b >> s)) & m;
    //     *a ^= t;
    //     *b ^= t << s;
    // }
    // tr(&mut arr[0], &mut arr[1], 1, u64::max_value() / 3);
    //
    // However, by using Cell projection, we can write something that
    // works. See [1].
    //
    // [1]: https://www.abubalay.com/blog/2020/01/05/cell-field-projection

    fn tr(a: &Cell<u64>, b: &Cell<u64>, s: u32, m: u64) {
        let ar = a.get();
        let br = b.get();
        let t = (ar ^ (br >> s)) & m;
        a.set(ar ^ t);
        b.set(br ^ t);
    }
    // Roughly:
    //
    //     Cell::from:mut : &mut [T] -> Cell<[T]>
    //     as_slice_of_cells : &Cell<[T]> -> &[Cell<T>]
    let arr_cell_view = Cell::from_mut(arr).as_slice_of_cells();
    tr(
        &arr_cell_view[0],
        &arr_cell_view[1],
        1,
        u64::max_value() / 3,
    );
    dbg!(arr);
}
