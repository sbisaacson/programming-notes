// Box<Self> receivers allow trait methods to take ownership of a dyn
// Trait object. This generalizes calling closures stored in a Box<dyn
// FnOnce(...) -> ...>.

trait Consumable {
    fn finish(self: Box<Self>);
}

impl Consumable for String {
    fn finish(self: Box<Self>) {
        println!("String {}", self);
    }
}

impl Consumable for u64 {
    fn finish(self: Box<Self>) {
        println!("u64 {}", self);
    }
}

fn main() {
    let mut v: Vec<Box<dyn Consumable>> = Vec::new();
    v.push(Box::new("Hello".to_string()));
    v.push(Box::new(55u64));
    for item in v {
        item.finish();
    }
    let mut v: Vec<Box<dyn FnOnce()>> = Vec::new();
    v.push(Box::new(|| {
        println!("first");
    }));
    v.push(Box::new(|| {
        println!("second");
    }));
    for item in v {
        item();
    }
}
