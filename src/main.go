package main

import "C"
import "fmt"

const xenlyVersion = "0.1.0-preview3"

func print_version() {
    fmt.Printf("Xenly %s\n", xenlyVersion)
    fmt.Println("Copyright (c) 2023-2024 Cyril John Magayaga")
}

func print_dumpversion() {
    fmt.Println(xenlyVersion)
}

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

func print_author() {
	fmt.Println("Cyril John Magayaga is the original author of Xenly programming language.")
}

func main() {
	print_version()
	print_help()
	print_author()
}