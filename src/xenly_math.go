/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * `xenly_math.go` is the similar to the `xenly_math.c` in C programming language.
 * 
 * It is available for Linux and Windows operating systems.
 *
 */
package main

import (
	"C"
	"math"
)

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

func main() {}