var (x,y) = (5,6);
var x = (5,6);
/*
import std::containers::{
    vector,
    map,
    umap = unordered_map,
};

import a::b;

import std;

// #[name(value), name(value, value)];
pub var global_var = 5;

// #[name(value)];
// #[name(value, value)];
pub const GLOBAL_CONSTANT : i32 = 5;

pub fn function1()
{
    // BODY
}

pub fn function2(a : i32) : i32
{
    // BODY
}

pub fn function3<T : Sized>(a : T)
{
    // BODY
}
*/
struct A
{
    a : i32;

    fn A(value : i32);
}

impl A {
    fn A(value : i32)
    {
        this.a = value * C;
    }
}

struct B<T : Sized>
{
    a : T;
    fn B(value : T);
}

trait TraitA<T : Sized>
{
    fn get() : T;
}

impl B
{
    fn B(value : T)
    {
        this.a = value;
    }
}

impl B : TraitA
{
    fn get() : T
    {
        return this.a;
    }
}

enum F
{
    A,
    B,
    C,
}

enum G<f32>
{
    A,
    B,
    C,
    D
}

variant Option<T : Sized>
{
    void,
    T
}

variant Ok<T : Sized>
{
    T
}

variant Err<T : Sized>
{
    T
}

variant Result
{
    Ok!<T>,
    Err!<E>,
}

variant Result
{
   Ok,Err
}
