//
// XENLY - high-level and general-purpose programming language
// created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
//
// It is initially written in Zig programming language.
//
// It is available for Linux and Windows operating systems.
//
const std = @import("std");
const math = std.math;

// Mathematical constants
pub const MATH_PI: f64 = math.pi;
pub const MATH_TAU: f64 = math.tau;
pub const MATH_E: f64 = math.e;
pub const MATH_GOLDEN_RATIO: f64 = 1.61803398874989484820;
pub const MATH_SILVER_RATIO: f64 = 2.41421356237309504880;
pub const MATH_SUPERGOLDEN_RATIO: f64 = 1.46557123187676802665;

// Physical constants
pub const PHYSICAL_SPEED_OF_LIGHT_MS: f64 = 299792458;
pub const PHYSICAL_SPEED_OF_LIGHT_KMH: f64 = 1080000000;
pub const PHYSICAL_SPEED_OF_LIGHT_MileS: f64 = 186000;
pub const PHYSICAL_GRAVTIATIONAL_CONSTANT_N_M2__KG2: f64 = 0.0000000000667430;
pub const PHYSICAL_GRAVTIATIONAL_CONSTANT_DYN_CM2__G2: f64 = 0.0000000667430;

// Mathematical constants functions
pub fn pi() f64 {
    return MATH_PI;
}

pub fn tau() f64 {
    return MATH_TAU;
}

pub fn e() f64 {
    return MATH_E;
}

pub fn goldenRatio() f64 {
    return MATH_GOLDEN_RATIO;
}

pub fn silverRatio() f64 {
    return MATH_SILVER_RATIO;
}

pub fn superGoldenRatio() f64 {
    return MATH_SUPERGOLDEN_RATIO;
}

// Universal constants functions
pub fn speedOfLight() f64 {
    return PHYSICAL_SPEED_OF_LIGHT_MS;
}

pub fn speedOfLight_kmh() f64 {
    return PHYSICAL_SPEED_OF_LIGHT_KMH;
}

pub fn speedOfLight_MileS() f64 {
    return PHYSICAL_SPEED_OF_LIGHT_MileS;
}

pub fn gravitationalConstant() f64 {
    return PHYSICAL_GRAVTIATIONAL_CONSTANT_N_M2__KG2;
}

pub fn gravitationalConstant_dyncm2g2() f64 {
    return PHYSICAL_GRAVTIATIONAL_CONSTANT_DYN_CM2__G2;
}

// Mathematical functions
pub fn xenly_sqrt(x: f64) f64 {
    return @sqrt(x);
}

pub fn xenly_cbrt(x: f64) f64 {
    return math.cbrt(x);
}

pub fn xenly_ffrt(x: f64) f64 {
    return math.pow(f64, x, 1.0 / 5.0);
}

pub fn xenly_pow(base: f64, exp: f64) f64 {
    return math.pow(f64, base, exp);
}

pub fn xenly_sin(x: f64) f64 {
    return @sin(x);
}

pub fn xenly_cos(x: f64) f64 {
    return @cos(x);
}

pub fn xenly_tan(x: f64) f64 {
    return @tan(x);
}

pub fn xenly_csc(x: f64) f64 {
    return 1 / @sin(x);
}

pub fn xenly_sec(x: f64) f64 {
    return 1 / @cos(x);
}

pub fn xenly_cot(x: f64) f64 {
    return 1 / @tan(x);
}

pub fn xenly_min(args: []const f64) f64 {
    var min_value: f64 = std.math.inf(f64);
    for (args) |num| {
        min_value = @min(min_value, num);
    }
    return min_value;
}

pub fn xenly_max(args: []const f64) f64 {
    var max_value: f64 = -std.math.inf(f64);
    for (args) |num| {
        max_value = @max(max_value, num);
    }
    return max_value;
}

pub fn xe_abs(x: f64) f64 {
    return @abs(x);
}

pub fn xenly_abs(arg: []const u8) !f64 {
    const x = try std.fmt.parseFloat(f64, arg);
    return xe_abs(x);
}
