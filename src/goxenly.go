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
// import "math"
// import "strings"
import "strconv"
import "os"
import "runtime"
// import "unicode"

type Variable struct {
    name  string
    value string
}

var variables []Variable
var numVariables int // assuming num_variables is a global variable in C

/******************************************/
/******************************************/
/*                 PRINT                  */
/******************************************/
/******************************************/

//export print_version
func print_version() {
    fmt.Println("Xenly 0.1.0-preview5 (Pre-alpha release)")
    fmt.Println("Copyright (c) 2023-2024 Cyril John Magayaga")
}

//export print_dumpversion
func print_dumpversion() {
    fmt.Println("0.1.0-preview5")
}

//export print_dumpreleasedate
func print_dumpreleasedate() {
    fmt.Println("March 30, 2024")
}

//export print_help
func print_help() {
    fmt.Println("Usage: xenly [input file]")
    fmt.Println("Options:")
    fmt.Println("  -h, --help                   Display this information.");
    fmt.Println("  -v, --version                Display compiler version information.");
    fmt.Println("  -dm, --dumpmachine           Display the compiler's target processor.");
    fmt.Println("  -drd, --dumpreleasedate      Display the release date of the compiler.");
    fmt.Println("  -dv, --dumpversion           Display the version of the compiler.");
    fmt.Println("  -os, --operatingsystem       Display the operating system.");
    fmt.Println("  --author                     Display the author information.");
    fmt.Println("  --new-project                Create a new xenly project.");
    fmt.Println("For bug reporting instructions, please see:")
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

/******************************************/
/******************************************/
/*                PROJECT                 */
/******************************************/
/******************************************/

//export initialize_project
func initialize_project() {
    // Create a new folder for the project
    err := os.Mkdir("xenly_project", 0755)
    if err != nil {
        fmt.Println("Unable to create project folder:", err)
        return
    }

    // Change directory to the newly created folder
    err = os.Chdir("xenly_project")
    if err != nil {
        fmt.Println("Unable to change directory:", err)
        return
    }

    // Create a new Xenly source file
    sourceFile, err := os.Create("main.xe")
    if err != nil {
        fmt.Println("Unable to create source file:", err)
        return
    }
    defer sourceFile.Close()

    // Write default "hello world" program to the source file
    _, err = sourceFile.WriteString("print(\"Hello, World!\")print(2*9-6/3*5)")
    if err != nil {
        fmt.Println("Unable to write to source file:", err)
        return
    }

    // Inform the user that the project has been initialized
    fmt.Println("New Xenly project initialized in 'xenly project' folder.")
}

//export create_initialize_project
func create_initialize_project(projectName string) {
    // Create a new folder for the project
    if err := os.Mkdir(projectName, 0755); err != nil {
        fmt.Println("Unable to create project directory:", err)
        return
    }

    // Change directory to the newly created folder
    if err := os.Chdir(projectName); err != nil {
        fmt.Println("Unable to change directory to project folder:", err)
        return
    }

    // Create a new source file
    sourceFile, err := os.Create("main.xe")
    if err != nil {
        fmt.Println("Unable to create source file:", err)
        return
    }
    defer sourceFile.Close()

    // Write default "hello world" program to the source file
    _, err = fmt.Fprintf(sourceFile, "print(\"Hello, World!\")\nprint(2*9-6/3*5)\n")
    if err != nil {
        fmt.Println("Error writing to source file:", err)
        return
    }

    // Inform the user that the project has been initialized
    fmt.Printf("New Xenly project initialized in \x1b[42;1m'%s'\x1b[0m folder.", projectName)
}

/******************************************/
/******************************************/
/*                 XENLY                  */
/******************************************/
/******************************************/

//export evaluately_condition
func evaluately_condition(condition string) bool {
    // Implement a more comprehensive logic for evaluating conditions
    // This is a simplified example
    result, err := strconv.Atoi(condition)
    if err != nil {
        // Handle error, for simplicity returning false
        return false
    }
    return result != 0
}

//export error
func error(message string) {
    fmt.Fprintf(os.Stderr, "\x1b[31mError: %s\x1b[0m\n", message)
    os.Exit(1)
}

func main() {}
