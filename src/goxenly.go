/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 */
package main

import "C"
import "fmt"
import "runtime"

//export print_version
func print_version() {
    fmt.Println("Xenly 0.1.0-preview4 (Pre-alpha release)")
    fmt.Println("Copyright (c) 2023-2024 Cyril John Magayaga")
}

//export print_dumpversion
func print_dumpversion() {
    fmt.Println("0.1.0-preview4")
}

//export print_dumpreleasedate
func print_dumpreleasedate() {
    fmt.Println("February 29, 2024")
}

//export print_help
func print_help() {
    fmt.Println("Usage: xenly [input file]")
    fmt.Println("Options:")
    fmt.Println("  -h, --help                   Display this information.")
    fmt.Println("  -v, --version                Display compiler version information.")
    fmt.Println("  -dv, --dumpversion           Display the version of the compiler.")
    fmt.Println("  -dm, --dumpmachine           Display the compiler's target processor.")
    fmt.Println("  -os, --operatingsystem       Display the operating system.")
    fmt.Println("  -p, --path                   Display the path to the Xenly compiler executable.")
    fmt.Println("  --author                     Display the author information.")
    fmt.Println("\nFor bug reporting instructions, please see:")
    fmt.Println("<https://github.com/magayaga/xenly>")
}

//export print_author
func print_author() {
	fmt.Println("Copyright (c) 2023-2024 Cyril John Magayaga")
}

//export print_operatingsystem
func print_operatingsystem() {
    switch os := runtime.GOOS; os {
    case "windows":
        fmt.Println("Windows")
    case "linux":
        fmt.Println("Linux")
    case "darwin":
        fmt.Println("macOS")
    case "freebsd":
        fmt.Println("FreeBSD")
    case "dragonfly":
        fmt.Println("DragonFlyBSD")
    case "openbsd":
        fmt.Println("OpenBSD")
    case "netbsd":
        fmt.Println("NetBSD")
    case "android":
        fmt.Println("Android")
    default:
        fmt.Println("Unknown")
    }
}

func main() {}