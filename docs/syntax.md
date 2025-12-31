### Bear Lang Syntax (out-of-date, TODO update soon)
- **Work In Progress:** this document will be the main syntax reference before moving to a more rigorous system of seperate documents for each language feature.
- BearLang is a C-Like language inspired by elements of Rust, and C++.
#### Project structure:
- Bear uses a module-based structure. At first, modules will also have to be compiled before I can add static library + metadata files.
- This is the basics of the system:
```
// in main.br 
import my_mod // a file containing a module

fn main([&]str args) -> i32 {
    my_mod..do_thing();
    return 0;
}
```
- From the root dir, the compiler will search for ./my_mod then try ./my_mod/mod.br 
- Whichever one it finds is the root of that module
```
// in ./my_mod
mod my_mod; // the rest of the file will be apart of the "my_mod" namespace/module

fn do_thing() {
    println("I'm doing a thing\n");
}
```
#### Built-in types:
- `i8, u8, ..., i64, u64` supported
- `f32` single and `f64` double precision
- char (unicode 32-bit)
- str (ascii string)
- bool
#### Type-related features (will come later)
- type-deduction: `var x = func_returning_i32(); // x will be i32`
- compt: `compt i32 x = some_compt_func();`
#### Moves / Copies 
- All objects are copyable and moveable, unless marked with `NoCopy`, `NoMove`
- `=` for copies, `<-` for moves during assignment 
- All copies and moves are bitwise
- Moved-from objects will never have their dtor's called
- You cannot move fields out of objects
#### Variables
```
i32 a; // default init (zero-init)
i32 x = 0; // assignment
i32 y <- x; // move assignment
i32 z = crunch_numbers();
BigStruct big_thing = get_big_thing();
ResrcManager manager1 = ResrcManager::new();
ResrcManager manager2 <- manager1; // transfer ownership through move
```
#### Arrays
```
[10]i32 my_arr;      // holding {elem_0, elem_1, ...}
[&]i32 my_arr_ref = &my_arr; // slice
```
```
fn take_a_slice([&mut]i32 slice) -> [&]i32 {
    transform_slice(slice);
    return slice;
}
```

#### References & Ptrs
```
i32 x = 1;
i32* x_ptr = &x;  // same as C/C++
i32& x_ref = &x;  // & borrow syntax
var y = &x;       // defaults to reference
var& y = &x;      // also fine for getting reference
var* y = &x;      // to get a ptr
var* z = &z;      // will be deduced to i32**

// East/West rules (mut applies left->right or right->left iff nothing is to the left of the mut)
mut i32& z = &x;     // (const) ref to mutable i32
i32& mut z = &x;     // mutable ref to (const) i32
mut i32& mut z = &x; // mutabe ref to mutable i32 
mut i32* mut z = &x; // same but ptr
i32 mut *mut z = &x; // also legal, but not preferred
```

```
contract Contiguous {
    fn size();
    fn data(); 
}

struct I32SliceWrapper has Contiguous { 
    hid usize size;
    hid i32& data;

    // ctor
    fn new([&]i32 from_slice) -> Self {
        Self self = {
            .size = from_slice.size;
            .data = from_slice.data;
        };
        return self;
    }
    
    // "getters"
    mt size -> usize {
        return self.size;
    }
    mt data() -> i32& {
        return self.data;
    }
}
```

```
struct Thing {
    i32 my_int;
    fn new() -> Thing {
        Thing thing;
        thing.my_int = 42;
        return thing;
    }
}
```
### Built-in Constructs

#### Slices 
```
[16]i32 arr;
[&]i32 slice = std..slice(&arr, 8); 
```

#### Generics
```
MyTemplatedType<param1,param2,param3> thing;
box::MyTemplatedType<param1,param2,param3> boxedThing;

fn foo<A, B, C>() -> bar<A, B, C> {
    do_work();
}

struct Foo<A, B, C> {

}

```

#### Variants
- The `is` contract is move-only (circumvent with `foo.copy is ...`)
```
variant Foo {
    Bar,
    Boo,
    BarWithInt(i32 num),
}

fn main() {
    Foo foo1 = way1();
    Foo foo2 = way2();

    if foo1 is Foo..BarWithInt(var x) {
        println("foo 1 is {}", x);
    } else if foo2 is Foo..BarWithInt(var x) {
        println("foo 2 is {}", x);
    }

}

fn way1() -> Foo {
    Foo foo = Foo..BarWithInt{.num = 5};
    return foo;
}

fn way2() -> Foo {
    Foo foo = Foo..BarWithInt(5);
    return foo;
}
```
