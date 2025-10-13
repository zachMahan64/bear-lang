BearLang Syntax (as of 20250929)
- BearLang is a C-Like language inspired by elements of Java, Python, Rust, and C++.
- Types:
    - Floating Point: single/double precision -> `flt` or `doub`, respectively 
    - Integers (all are signed): 32bit/64bit -> `int` or `long`, respectively 
    - Strings (heap managed): str 
- Type mods:
    - Box (automatically dereferenced unique ptr): `box::SomeType`
        - Can be reassigned into another box:
        - `box::Foo myFoo = new Foo();`
        - `myFoo = new Foo();` // release previously held Foo
    - Bag (automatically dereferenced shared ptr): `bag::SomeType`
        - Can be assigned by new or by another bag:
        - `bag::Foo myFoo = new Foo();`
        - `bag::Foo myFoo = otherFoo; // inc ref count`
        - `myFoo = new Foo(); // reassign and dec ref count Foo`
    - Both Bag and Box dec reference counter/release resource on destruction
    - References:
        - Similar to references in Rust or C++, they are essentially non-nullable automatically dereferenced pointers. 
        - Example usages: 
            - `ref::box::Foo = existingBoxedFoo;`
            - `ref::int = existingInt;`
            - `fn myFunc(ref::str myString) {doWork(myString)} // take string by reference and then doWork on it`
- Functions: 
    - `fn function_name(type_name arg) -> return_type {...}`
    - return type can be elided if it's void:
        - `fn myFunc (int x) {...}`
- Scope:
    space my_space {...}
- Use ".." as scope resolution symbol:
    - Example:
```
    fn main() -> int {
        return Foo..bar;
    }
    struct Foo {
        static int MAGIC_ZERO = 0; 
        fn bar() -> int {
            return MAGIC_FOO;
        }
    }
```
