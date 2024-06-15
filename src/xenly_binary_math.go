/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * `xenly_binary_math.go` is the similar to the `xenly_binary_math.c` in C programming language and and
 * `xenly_binary_math.rs` in Rust programming language.
 *
 * It is available for Linux and Windows operating systems.
 */
package main

import (
	"C"
	"strconv"
)

// Convert binary string to decimal
//export xenly_bindec
func xenly_bindec(binary *C.char) float64 {
	binaryStr := C.GoString(binary)
	result, _ := strconv.ParseInt(binaryStr, 2, 64)
	return float64(result)
}

// Convert decimal to binary string
//export xenly_decbin
func xenly_decbin(decimal C.int) *C.char {
	binaryStr := strconv.FormatInt(int64(decimal), 2)
	return C.CString(binaryStr)
}

func main() {}
