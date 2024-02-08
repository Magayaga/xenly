package main

import "C"
import "fmt"

//export print_version
func print_version() {
    fmt.Println("Xenly 0.1.0-preview3")
    fmt.Println("Copyright (c) 2023-2024 Cyril John Magayaga")
}

//export print_dumpversion
func print_dumpversion() {
    fmt.Println("0.1.0-preview3")
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
	fmt.Println("Cyril John Magayaga is the original author of Xenly programming language.")
}

func main() {}