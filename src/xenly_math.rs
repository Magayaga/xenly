/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * `xenly_math.rs` is the similar to the `xenly_math.c` in C programming language and `xenly_math.go` in Go
 * programming language.
 * 
 * It is available for Linux and Windows operating systems.
 *
 */

 // Define and export mathematical constants
pub const MATH_PI: f64 = 3.14159265358979323846;
pub const MATH_TAU: f64 = 6.28318530717958647692;
pub const MATH_E: f64 = 2.71828182845904523536;
pub const MATH_GOLDEN_RATIO: f64 = 1.61803398874989484820;
pub const MATH_SILVER_RATIO: f64 = 2.41421356237309504880;
pub const MATH_SUPERGOLDEN_RATIO: f64 = 1.46557123187676802665;

// Export constants using functions
#[no_mangle]
pub extern "C" fn pi() -> f64 {
    MATH_PI
}

#[no_mangle]
pub extern "C" fn tau() -> f64 {
    MATH_TAU
}

#[no_mangle]
pub extern "C" fn e() -> f64 {
    MATH_E
}

#[no_mangle]
pub extern "C" fn goldenRatio() -> f64 {
    MATH_GOLDEN_RATIO
}

#[no_mangle]
pub extern "C" fn silverRatio() -> f64 {
    MATH_SILVER_RATIO
}

#[no_mangle]
pub extern "C" fn superGoldenRatio() -> f64 {
    MATH_SUPERGOLDEN_RATIO
}

#[no_mangle]
pub extern "C" fn xenly_sqrt(x: f64) -> f64 {
    x.sqrt()
}

#[no_mangle]
pub extern "C" fn xenly_cbrt(x: f64) -> f64 {
    x.cbrt()
}

#[no_mangle]
pub extern "C" fn xenly_pow(base: f64, exp: f64) -> f64 {
    base.powf(exp)
}

#[no_mangle]
pub extern "C" fn xenly_sin(x: f64) -> f64 {
    x.sin()
}

#[no_mangle]
pub extern "C" fn xenly_cos(x: f64) -> f64 {
    x.cos()
}

#[no_mangle]
pub extern "C" fn xenly_tan(x: f64) -> f64 {
    x.tan()
}
