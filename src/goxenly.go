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
// import "strconv"
import "os"
import "runtime"
// import "unicode"

type Variable struct {
    name  string
    value string
}

var variables []Variable
var numVariables int // assuming num_variables is a global variable in C

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
    _, err = sourceFile.WriteString("print(\"Hello, World!\")\nprint(2*9-6/3*5)\n")
    if err != nil {
        fmt.Println("Unable to write to source file:", err)
        return
    }

    // Inform the user that the project has been initialized
    fmt.Println("New Xenly project initialized in 'xenly project' folder.")
}

/*
func isdigit(c byte) bool {
    return '0' <= c && c <= '9'
}

//export evaluate_factor
func evaluate_factor(expression *string) float64 {
    var result float64

    if (*expression)[0] == '(' {
        *expression = (*expression)[1:] // Move past the opening parenthesis
        result = evaluate_arithmetic_expression(expression)
        if (*expression)[0] == ')' {
            *expression = (*expression)[1:] // Move past the closing parenthesis
        } else {
            panic("Mismatched parentheses")
        }
    } else {
        // Parse the number from the expression
        for i := 0; i < len(*expression) && (isdigit((*expression)[i]) || (*expression)[i] == '.'); i++ {
            // Accumulate the characters that form the number
            result, _ = strconv.ParseFloat((*expression)[:i+1], 64)
        }
        // Move past the characters that form the number
        *expression = (*expression)[len(strconv.FormatFloat(result, 'f', -1, 64)):]
    }

    return result
}

//export evaluate_term
func evaluate_term(expression *string) float64 {
    result := evaluate_factor(expression)
    for len(*expression) > 0 {
        operator := (*expression)[0]
        if operator == '*' || operator == '/' || operator == '%' {
            *expression = (*expression)[1:] // Move past the operator
            factor := evaluate_factor(expression)
            if operator == '*' {
                result *= factor
            } else if operator == '/' {
                if factor != 0 {
                    result /= factor
                } else {
                    panic("Division by zero")
                }
            } else if operator == '%' {
                if factor != 0 {
                    result = math.Mod(result, factor)
                } else {
                    panic("Modulo by zero")
                }
            }
        } else {
            break // Not an operator, exit the loop
        }
    }
    return result
}

//export evaluate_arithmetic_expression
func evaluate_arithmetic_expression(expression string) float64 {
    result := evaluate_term(expression)
    for len(*expression) > 0 {
        operator := (*expression)[0]
        if operator == '+' || operator == '-' {
            *expression = (*expression)[1:] // Move past the operator
            term := evaluate_term(expression)
            if operator == '+' {
                result += term
            } else {
                result -= term
            }
        } else {
            break // Not an operator, exit the loop
        }
    }
    return result
}

//export execute_print
func execute_print(arg string, variables []Variable) {
    if arg[0] == '"' && arg[len(arg)-1] == '"' {
        fmt.Println(arg[1 : len(arg)-1])
    } else if strings.ContainsAny(arg, "+-*%/^") {
        result, _ := evaluate_arithmetic_expression(arg)
        fmt.Println(result)
    } else if arg == "pi" {
        fmt.Println(math.Pi)
    } else if arg == "tau" {
        fmt.Println(math.Pi * 2)
    } else if arg == "e" {
        fmt.Println(math.E)
    } else if arg == "goldenRatio" {
        fmt.Println(math.Phi)
    } else if arg == "silverRatio" {
        fmt.Println(1 + math.Sqrt(2))
    } else if arg == "supergoldenRatio" {
        fmt.Println((1 + math.Sqrt(5)) / 2)
    } else if strings.HasPrefix(arg, "sqrt(") && arg[len(arg)-1] == ')' {
        expr := arg[5 : len(arg)-1]
        result, _ := evaluate_condition(expr)
        if result >= 0 {
            fmt.Println(math.Sqrt(result))
        } else {
            fmt.Println("Square root of a negative number is not supported")
        }
    } else if strings.HasPrefix(arg, "cbrt(") && arg[len(arg)-1] == ')' {
        expr := arg[5 : len(arg)-1]
        result, _ := evaluate_condition(expr)
        fmt.Println(math.Cbrt(result))
    } else if _, err := strconv.ParseFloat(arg, 64); err == nil {
        result, _ := evaluate_condition(arg)
        fmt.Println(result)
    } else {
        is_variable := false
        for _, v := range variables {
            if v.Name == arg {
                fmt.Println(v.Value)
                is_variable = true
                break
            }
        }
        if !is_variable {
            result, _ := evaluate_condition(arg)
            fmt.Println(result)
        }
    }
}

//export evaluate_condition
func evaluate_condition(condition string, variables []Variable) float64 {
    if condition == "true" {
        return 1.0
    } else if condition == "false" {
        return 0.0
    } else if strings.ContainsAny(condition, "+-*%/^") {
        return evaluate_arithmetic_expression(&condition)
    } else if strings.HasPrefix(condition, "pow(") && condition[len(condition)-1] == ')' {
        // Extract the base and exponent values from the argument
        arguments := strings.Split(condition[4:len(condition)-1], ",")
        base := evaluate_condition(strings.TrimSpace(arguments[0]), variables)
        exponent := evaluate_condition(strings.TrimSpace(arguments[1]), variables)
        return math.Pow(base, exponent)
    } else if strings.HasPrefix(condition, "sin(") && condition[len(condition)-1] == ')' {
        innerResult := evaluate_condition(condition[4:len(condition)-1], variables)
        return math.Sin(innerResult)
    } else if strings.HasPrefix(condition, "cos(") && condition[len(condition)-1] == ')' {
        innerResult := evaluate_condition(condition[4:len(condition)-1], variables)
        return math.Cos(innerResult)
    } else if strings.HasPrefix(condition, "tan(") && condition[len(condition)-1] == ')' {
        innerResult := evaluate_condition(condition[4:len(condition)-1], variables)
        return math.Tan(innerResult)
    } else if strings.HasPrefix(condition, "gamma(") && condition[len(condition)-1] == ')' {
        innerResult := evaluate_condition(condition[6:len(condition)-1], variables)
        return math.Gamma(innerResult)
    } else if strings.HasPrefix(condition, "fibonacci(") && condition[len(condition)-1] == ')' {
        innerResult := evaluate_condition(condition[10:len(condition)-1], variables)
        if innerResult < 0 {
            panic("Fibonacci of a negative number is not supported")
        }
        return float64(fibonacci(int(innerResult)))
    } else if strings.HasPrefix(condition, "factorial(") && condition[len(condition)-1] == ')' {
        innerResult := evaluate_condition(condition[10:len(condition)-1], variables)
        if innerResult < 0 {
            panic("Factorial of a negative number is not supported")
        }
        return float64(factorial(int(innerResult)))
    } else if condition[0] == '(' && condition[len(condition)-1] == ')' {
        expressionResult := evaluate_condition(condition[1:len(condition)-1], variables)
        return expressionResult
    } else if strings.ContainsAny(condition, "<>=") {
        parts := strings.Fields(condition)
        leftValue, _ := strconv.Atoi(parts[0])
        rightValue, _ := strconv.Atoi(parts[2])
        operator := parts[1]
        switch operator {
        case "<":
            return boolToFloat64(leftValue < rightValue)
        case ">":
            return boolToFloat64(leftValue > rightValue)
        case "=":
            return boolToFloat64(leftValue == rightValue)
        }
    } else if strings.HasPrefix(condition, "sqrt(") && condition[len(condition)-1] == ')' {
        innerResult := evaluate_condition(condition[5:len(condition)-1], variables)
        if innerResult >= 0 {
            return math.Sqrt(innerResult)
        } else {
            panic("Square root of a negative number is not supported")
        }
    } else if strings.HasPrefix(condition, "cbrt(") && condition[len(condition)-1] == ')' {
        innerResult := evaluate_condition(condition[5:len(condition)-1], variables)
        return math.Cbrt(innerResult)
    } else {
        for _, v := range variables {
            if v.Name == condition {
                if v.Value == "true" {
                    return 1.0
                } else {
                    return 0.0
                }
            }
        }
    }

    // Convert string to float64
    result, _ := strconv.ParseFloat(condition, 64)
    return result
}
*/

func main() {}
