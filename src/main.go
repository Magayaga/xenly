package main

import "C"

//export print_version
func print_version() {
    println("Xenly 0.1.0-preview3")
    println("Copyright (c) 2023-2024 Cyril John Magayaga")
}

//export print_dumpversion
func print_dumpversion() {
    println("0.1.0-preview3")
}

//export print_help
func print_help() {
    println("Usage: xenly [input file]")
    println("Options:")
    println("  -h, --help                   Display this information.")
    println("  -v, --version                Display compiler version information.")
    println("  -dv, --dumpversion           Display the version of the compiler.")
    println("  -dm, --dumpmachine           Display the compiler's target processor.")
    println("  -os, --operatingsystem       Display the operating system.")
    println("  -p, --path                   Display the path to the Xenly compiler executable.")
    println("  --author                     Display the author information.")
    println("\nFor bug reporting instructions, please see:")
    println("<https://github.com/magayaga/xenly>")
}

//export print_author
func print_author() {
	println("Cyril John Magayaga is the original author of Xenly programming language.")
}

func main() {}