/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * `xenly_math.go` is the similar to the `xenly_math.c` in C programming language and `xenly_math.rs` in Rust
 * programming language.
 * 
 * It is available for Linux and Windows operating systems.
 *
 */
package main

import (
	"C"
	"math"
	"strconv"
)

// Define and export mathematical constants and universal constants
const (
	MATH_PI = 3.14159265358979323846
	MATH_TAU = 6.28318530717958647692
    MATH_E = 2.71828182845904523536
    MATH_GOLDEN_RATIO = 1.61803398874989484820
    MATH_SILVER_RATIO = 2.41421356237309504880
    MATH_SUPERGOLDEN_RATIO = 1.46557123187676802665
	PHYSICAL_SPEED_OF_LIGHT_MS = 299792458
	PHYSICAL_SPEED_OF_LIGHT_KMH = 1080000000
	PHYSICAL_SPEED_OF_LIGHT_MileS = 186000
	PHYSICAL_GRAVTIATIONAL_CONSTANT_N_M2__KG2 = 0.0000000000667430
	PHYSICAL_GRAVTIATIONAL_CONSTANT_DYN_CM2__G2 = 0.0000000667430
)

// Export mathematical constants using functions
//export pi
func pi() C.double {
    return C.double(MATH_PI)
}

//export tau
func tau() C.double {
	return C.double(MATH_TAU)
}

//export e
func e() C.double {
	return C.double(MATH_E)
}

//export goldenRatio
func goldenRatio() C.double {
	return C.double(MATH_GOLDEN_RATIO)
}

//export silverRatio
func silverRatio() C.double {
	return C.double(MATH_SILVER_RATIO)
}

//export superGoldenRatio
func superGoldenRatio() C.double {
	return C.double(MATH_SUPERGOLDEN_RATIO)
}

// Export universal constants using functions
//export speedOfLight
func speedOfLight() C.double {
	return C.double(PHYSICAL_SPEED_OF_LIGHT_MS)
}

//export speedOfLight_kmh
func speedOfLight_kmh() C.double {
	return C.double(PHYSICAL_SPEED_OF_LIGHT_KMH)
}

//export speedOfLight_MileS
func speedOfLight_MileS() C.double {
	return C.double(PHYSICAL_SPEED_OF_LIGHT_MileS)
}

//export gravitationalConstant
func gravitationalConstant() C.double {
	return C.double(PHYSICAL_GRAVTIATIONAL_CONSTANT_N_M2__KG2)
}

//export gravitationalConstant_dyncm2g2
func gravitationalConstant_dyncm2g2() C.double {
	return C.double(PHYSICAL_GRAVTIATIONAL_CONSTANT_DYN_CM2__G2)
}

// Define a function to calculate the square root
//export xenly_sqrt
func xenly_sqrt(x C.double) C.double {
    return C.double(math.Sqrt(float64(x)))
}

// Define a function to calculate the cube root
//export xenly_cbrt
func xenly_cbrt(x C.double) C.double {
    return C.double(math.Cbrt(float64(x)))
}

// Define a function to calculate the fifth root
//export xenly_ffrt
func xenly_ffrt(x C.double) C.double {
	return C.double(float64(x) / float64(1/5))
}

// Define a function to calculate the power
//export xenly_pow
func xenly_pow(base, exp C.double) C.double {
    return C.double(math.Pow(float64(base), float64(exp)))
}

// Define a function to calculate the sine
//export xenly_sin
func xenly_sin(x C.double) C.double {
	return C.double(math.Sin(float64(x)))
}

// Define a function to calculate the cosine
//export xenly_cos
func xenly_cos(x C.double) C.double {
	return C.double(math.Cos(float64(x)))
}

// Define a function to calculate the tangent
//export xenly_tan
func xenly_tan(x C.double) C.double {
	return C.double(math.Tan(float64(x)))
}

// Define a function to calculate the cosecant
//export xenly_csc
func xenly_csc(x C.double) C.double {
	return C.double(1 / math.Sin(float64(x)))
}

// Define a function to calculate the secant
//export xenly_sec
func xenly_sec(x C.double) C.double {
	return C.double(1 / math.Cos(float64(x)))
}

// Define a function to calculate the cotangent
//export xenly_cot
func xenly_cot(x C.double) C.double {
	return C.double(1 / math.Tan(float64(x)))
}

// Define a function to calculate the minimum function (xe_min and xenly_min)
//export xe_min
func xe_min(x C.double, y C.double) C.double {
    if x < y {
		return x
	}
	return y
}

//export xenly_min
func xenly_min(numbers []C.double, count int) C.double {
    if count <= 0 {
        panic("count must be positive")
    }
    min_value := numbers[0]
    for i := 1; i < count; i++ {
        min_value = xe_min(min_value, numbers[i])
    }
    return min_value
}

// Define a function to calculate the maximum function (xe_max and xenly_max)
//export xe_max
func xe_max(x C.double, y C.double) C.double {
    if x > y {
		return x
	}
	return y
}

//export xenly_max
func xenly_max(numbers []C.double, count int) C.double {
    if count <= 0 {
        panic("count must be positive")
    }
    max_value := numbers[0]
    for i := 1; i < count; i++ {
        max_value = xe_max(max_value, numbers[i])
    }
    return max_value
}

// Define a function to calculate the absolute value function (xe_abs and xenly_abs)
//export xe_abs
func xe_abs(x C.double) C.double {
    return C.double(math.Abs(float64(x)))
}

//export xenly_abs
func xenly_abs(arg *C.char) C.double {
	x, err := strconv.ParseFloat(C.GoString(arg), 64)
	if err != nil {
		panic(err)
	}
	return xe_abs(C.double(x))
}


func main() {}