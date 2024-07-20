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
use std::ffi::CStr;
use std::os::raw::c_char;

// Define and export mathematical constants and universal constants
pub const MATH_PI: f64 = 3.14159265358979323846;
pub const MATH_TAU: f64 = 6.28318530717958647692;
pub const MATH_E: f64 = 2.71828182845904523536;
pub const MATH_GOLDEN_RATIO: f64 = 1.61803398874989484820;
pub const MATH_SILVER_RATIO: f64 = 2.41421356237309504880;
pub const MATH_SUPERGOLDEN_RATIO: f64 = 1.46557123187676802665;
pub const PHYSICAL_SPEED_OF_LIGHT_MS: f64 = 299_792_458.0;
pub const PHYSICAL_SPEED_OF_LIGHT_KMH: f64 = 1_080_000_000.0;
pub const PHYSICAL_SPEED_OF_LIGHT_MILES: f64 = 186_000.0;
pub const PHYSICAL_GRAVTIATIONAL_CONSTANT_N_M2__KG2: f64 = 0.0000000000667430;
pub const PHYSICAL_GRAVTIATIONAL_CONSTANT_DYN_CM2__G2: f64 = 0.0000000667430;

// Export mathematical constants using functions
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

// Export universal constants using functions
#[no_mangle]
pub extern "C" fn speedOfLight() -> f64 {
    PHYSICAL_SPEED_OF_LIGHT_MS
}

#[no_mangle]
pub extern "C" fn speedOfLight_kmh() -> f64 {
    PHYSICAL_SPEED_OF_LIGHT_KMH
}

#[no_mangle]
pub extern "C" fn speedOfLight_MileS() -> f64 {
    PHYSICAL_SPEED_OF_LIGHT_MILES
}

#[no_mangle]
pub extern "C" fn gravitationalConstant() -> f64 {
    PHYSICAL_GRAVTIATIONAL_CONSTANT_N_M2__KG2
}

#[no_mangle]
pub extern "C" fn gravitationalConstant_dyncm2g2() -> f64 {
    PHYSICAL_GRAVTIATIONAL_CONSTANT_DYN_CM2__G2
}

// Define a function to calculate the square root
#[no_mangle]
pub extern "C" fn xenly_sqrt(x: f64) -> f64 {
    x.sqrt()
}

// Define a function to calculate the cube root
#[no_mangle]
pub extern "C" fn xenly_cbrt(x: f64) -> f64 {
    x.cbrt()
}

// Define a function to calculate the fifth root
#[no_mangle]
pub extern "C" fn xenly_ffrt(x: f64) -> f64 {
    x / (1.0/5.0)
}

// Define a function to calculate the power
#[no_mangle]
pub extern "C" fn xenly_pow(base: f64, exp: f64) -> f64 {
    base.powf(exp)
}

// Define a function to calculate the sine
#[no_mangle]
pub extern "C" fn xenly_sin(x: f64) -> f64 {
    x.sin()
}

// Define a function to calculate the cosine
#[no_mangle]
pub extern "C" fn xenly_cos(x: f64) -> f64 {
    x.cos()
}

// Define a function to calculate the tangent
#[no_mangle]
pub extern "C" fn xenly_tan(x: f64) -> f64 {
    x.tan()
}

// Define a function to calculate the cosecant
#[no_mangle]
pub extern "C" fn xenly_csc(x: f64) -> f64 {
    1.0 / x.sin()
}

// Define a function to calculate the secant
#[no_mangle]
pub extern "C" fn xenly_sec(x: f64) -> f64 {
    1.0 / x.cos()
}

// Define a function to calculate the cotangent
#[no_mangle]
pub extern "C" fn xenly_cot(x: f64) -> f64 {
    1.0 / x.tan()
}

// Define a function to calculate the minimum function (xe_min and xenly_min)
#[no_mangle]
pub extern "C" fn xe_min(x: f64, y: f64) -> f64 {
    if x < y {
        x
    }
    
    else {
        y
    }
}

#[no_mangle]
pub extern "C" fn xenly_min(numbers: *const f64, count: usize) -> f64 {
    let numbers = unsafe {
        std::slice::from_raw_parts(numbers, count)
    };
    
    let mut min_value = numbers[0];
    for &num in &numbers[1..] {
        min_value = xe_min(min_value, num);
    }
    min_value
}

// Define a function to calculate the maximum function (xe_max and xenly_max)
#[no_mangle]
pub extern "C" fn xe_max(x: f64, y: f64) -> f64 {
    if x > y {
        x
    }
    
    else {
        y
    }
}

#[no_mangle]
pub extern "C" fn xenly_max(numbers: *const f64, count: usize) -> f64 {
    let numbers = unsafe {
        std::slice::from_raw_parts(numbers, count)
    };
    
    let mut max_value = numbers[0];
    for &num in &numbers[1..] {
        max_value = xe_max(max_value, num);
    }
    max_value
}

// Define a function to calculate the absolute value function (xe_abs and xenly_abs)
#[no_mangle]
pub extern "C" fn xe_abs(x: f64) -> f64 {
    if x < 0.0 {
        -x
    }
    
    else {
        x
    }
}

#[no_mangle]
pub extern "C" fn xenly_abs(arg: *const c_char) -> f64 {
    let c_str = unsafe {
        assert!(!arg.is_null());

        CStr::from_ptr(arg)
    };

    let str_utf8 = c_str.to_str().expect("Invalid UTF-8 string");
    let x: f64 = str_utf8.parse().expect("Failed to parse input as number");

    xe_abs(x)
}
