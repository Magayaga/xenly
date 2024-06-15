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
import "math"
import "strings"
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
    fmt.Println("Xenly 0.1.0-preview8 (Pre-alpha release)")
    fmt.Println("Copyright (c) 2023-2024 Cyril John Magayaga")
}

//export print_dumpversion
func print_dumpversion() {
    fmt.Println("0.1.0-preview8")
}

//export print_dumpreleasedate
func print_dumpreleasedate() {
    fmt.Println("June 27, 2024")
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
    fmt.Println("  --new-project                Create a default Xenly project.");
    fmt.Println("  --create-project             Create a new Xenly project.");
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

/******************************************/
/******************************************/
/*                 ERROR                  */
/******************************************/
/******************************************/

//export error
func error(message string) {
    fmt.Fprintf(os.Stderr, "\x1b[31mError: %s\x1b[0m\n", message)
    os.Exit(1)
}

/******************************************/
/******************************************/
/*               CONSTANTS                */
/******************************************/
/******************************************/

// Constants
const (
    mathPi                 = 3.14159265358979323846
    mathTau                = 6.28318530717958647692
    mathE                  = 2.71828182845904523536
    mathGoldenRatio        = 1.61803398874989484820
    mathSilverRatio        = 2.41421356237309504880
    mathSupergoldenRatio   = 1.46557123187676802665
    speedOfLight_ms        = 299792458
    speedOfLight_kmh       = 1080000000
    speedOfLight_mileS     = 186000

)

/******************************************/
/******************************************/
/*                 PRINT                  */
/******************************************/
/******************************************/

//export execute_print
func execute_print(arg string) {
    arg = strings.TrimSpace(arg)

    // Handle quoted strings
    if (arg[0] == '"' && arg[len(arg)-1] == '"') || (arg[0] == '\'' && arg[len(arg)-1] == '\'') {
        fmt.Println(arg[1 : len(arg)-1])
        return
    }

    // Handle arithmetic expressions
    /* if strings.ContainsAny(arg, "+-*\/%^") {
        result, err := evaluate_arithmetic_expression(arg)
        if err != nil {
            fmt.Println("Error:", err)
            return
        }
        fmt.Println(result)
        return
    } */

    // Handle constants
    switch arg {
    case "pi", "π":
        fmt.Println(mathPi)
    case "tau", "τ":
        fmt.Println(mathTau)
    case "e":
        fmt.Println(mathE)
    case "goldenRatio":
        fmt.Println(mathGoldenRatio)
    case "silverRatio":
        fmt.Println(mathSilverRatio)
    case "supergoldenRatio":
        fmt.Println(mathSupergoldenRatio)
    case "speedOfLight":
        fmt.Println(speedOfLight_ms)
    case "speedOfLight.kmh":
        fmt.Println(speedOfLight_kmh)
    case "speedOfLight.mih":
        fmt.Println(speedOfLight_mileS)
    }

    // Handle functions
    if strings.HasPrefix(arg, "sqrt(") && arg[len(arg)-1] == ')' {
        expr := arg[5 : len(arg)-1]
        value, err := strconv.ParseFloat(expr, 64)
        if err != nil {
            fmt.Println("Error:", err)
            return
        }
        
        if value < 0 {
            fmt.Println("Error: Square root of a negative number is not supported")
            return
        }
        
        fmt.Println(math.Sqrt(value))
        return
    }

    if strings.HasPrefix(arg, "cbrt(") && arg[len(arg)-1] == ')' {
        expr := arg[5 : len(arg)-1]
        value, err := strconv.ParseFloat(expr, 64)
        if err != nil {
            fmt.Println("Error:", err)
            return
        }

        if value < 0 {
            fmt.Println("Error: Cube root of a negative number is not supported")
            return
        }
        
        fmt.Println(math.Cbrt(value))
        return
    }

    // Handle numeric constants or expressions
    value, err := strconv.ParseFloat(arg, 64)
    if err == nil {
        fmt.Println(value)
        return
    }

    // Handle other cases as error
    fmt.Println("Error: Invalid input")
}

func main() {}
