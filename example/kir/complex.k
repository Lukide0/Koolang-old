// complex.k

trait Something
{
    fn add(a : i32, b : i32);
}

pub impl Something
{
    pub fn add(a : i32, b : i32)
    {
        return a;
    }
}


/* const B : u8 = 2; */
/* const A_COPY : i8 = 1 << 6; */
/* fn calc_bit_shift(bit : u8) : u64 { */
/*     return 1 << (bit - 1); */
/* } */

/* short example(values): */
/*
const B : i32 = C;
const C : i32 = B;
*/
/* short example(types): */
/*
struct D { e : E; }
struct E { d : D; }
*/
/* short example(self): */
/*
struct F{ f : F; }

fn rec(i : i8) {
    if (i > 0) { rec(i - 1); }
}

fn main() : u8 {
    return A_COPY; // should return 5
}
*/
