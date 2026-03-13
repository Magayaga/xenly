/*
 * XENLY - Xenly Virtual Machine (XVM)
 * xenlyrun — Xenly Virtual Machine Launcher
 *
 * Loads compiled XVM bytecode (.xebc) and executes it using the
 * Xenly Virtual Machine (XVM).
 *
 * Created, designed, and developed by Cyril John Magayaga.
 * XVM Go implementation — available for Windows, macOS, and Linux.
 *
 * Usage:
 *   xenlyrun <file.xebc> [args...]
 *   xenlyrun --run <file.xe>     (compile and run in one step)
 *   xenlyrun --version | --help
 *
 * The XVM is a stack-based virtual machine that executes portable
 * Xenly bytecode. Programs compiled with xenlybyc run identically
 * on all supported platforms.
 */
package main

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"time"

	"xvm/pkg/bytecode"
	"xvm/pkg/compiler"
	"xvm/pkg/vm"
)

const (
	xvmVersion     = "0.1.0"
	xvmReleaseDate = "2026-XX-XX"
	xvmAuthor      = "Cyril John Magayaga"
	xvmAuthorEmail = "cjmagayaga957@gmail.com"
)

var useColor = true

func col(code string) string {
	if !useColor {
		return ""
	}
	return "\033[" + code + "m"
}
func reset() string {
	if !useColor {
		return ""
	}
	return "\033[0m"
}

func printUsage(prog string) {
	fmt.Printf("\n  %sxenlyrun%s  Xenly Virtual Machine  %sv%s%s\n",
		col("1;36"), reset(), col("1;33"), xvmVersion, reset())
	fmt.Printf("\n  %sUsage:%s   %s [options] <file.xebc> [args...]\n", col("1;33"), reset(), prog)
	fmt.Printf("\n  %sOptions:%s\n", col("1;33"), reset())
	fmt.Printf("    %s--run <file.xe>%s   Compile .xe and run immediately  %s(no .xebc written)%s\n",
		col("1"), reset(), col("2"), reset())
	fmt.Printf("    %s--no-color%s        Disable ANSI colour output\n", col("1"), reset())
	fmt.Printf("    %s--time%s            Show execution time\n", col("1"), reset())
	fmt.Printf("    %s--dumpmachine%s     Print target machine triple\n", col("1"), reset())
	fmt.Printf("    %s--dumpversion%s     Print version string\n", col("1"), reset())
	fmt.Printf("    %s--os%s              Print host operating system\n", col("1"), reset())
	fmt.Printf("    %s-h, --help%s        Show this help\n", col("1"), reset())
	fmt.Printf("    %s-v, --version%s     Show version\n", col("1"), reset())
	fmt.Printf("    %s--author%s          Show author information\n", col("1"), reset())
	fmt.Printf("\n  %sExamples:%s\n", col("1;32"), reset())
	fmt.Printf("    %s hello.xebc              %s→ run compiled bytecode\n", prog, col("2"))
	fmt.Printf("    %s%s --run hello.xe        %s→ compile + run\n", reset(), prog, col("2"))
	fmt.Printf("    %s%s --run hello.xe -- arg1%s→ pass args to program\n%s\n",
		reset(), prog, col("2"), reset())
}

func printVersion() {
	fmt.Printf("\n  %sxenlyrun%s v%s  (Xenly Virtual Machine)\n", col("1;36"), reset(), xvmVersion)
	fmt.Printf("  Executes .xebc XVM bytecode programs\n")
	fmt.Printf("  Supported platforms: Windows, macOS, Linux\n\n")
}

func xvmMachine() string {
	arch := runtime.GOARCH
	switch arch {
	case "amd64":
		arch = "x86_64"
	case "arm64":
		arch = "aarch64"
	}
	return arch + "-" + runtime.GOOS
}

func main() {
	var (
		inputFile string
		runMode   bool   // --run: compile .xe and execute directly
		doTime    bool
	)

	args := os.Args[1:]
	i := 0
	for i < len(args) {
		switch args[i] {
		case "-h", "--help":
			printUsage(os.Args[0])
			os.Exit(0)
		case "-v", "--version":
			printVersion()
			os.Exit(0)
		case "--author":
			fmt.Printf("\n  %sxenlyrun%s — created, designed, and developed by\n",
				col("1;36"), reset())
			fmt.Printf("  %s <%s>\n\n", xvmAuthor, xvmAuthorEmail)
			os.Exit(0)
		case "--dumpmachine":
			fmt.Println(xvmMachine())
			os.Exit(0)
		case "--dumpversion":
			fmt.Println(xvmVersion)
			os.Exit(0)
		case "--os":
			fmt.Printf("%s (%s)\n", runtime.GOOS, runtime.GOARCH)
			os.Exit(0)
		case "--no-color":
			useColor = false
		case "--time":
			doTime = true
		case "--run":
			runMode = true
			i++
			if i >= len(args) {
				fmt.Fprintf(os.Stderr, "%s[xenlyrun]%s --run requires a .xe source file\n",
					col("1;31"), reset())
				os.Exit(1)
			}
			inputFile = args[i]
		case "--":
			// remaining args passed to program
			i++
			continue
		default:
			if strings.HasPrefix(args[i], "-") && inputFile == "" {
				fmt.Fprintf(os.Stderr, "%s[xenlyrun]%s unknown flag: %s\n",
					col("1;31"), reset(), args[i])
				os.Exit(1)
			}
			if inputFile == "" {
				inputFile = args[i]
			}
		}
		i++
	}

	if inputFile == "" {
		printUsage(os.Args[0])
		os.Exit(0)
	}

	t0 := time.Now()
	var mod *bytecode.Module

	if runMode {
		// ── Compile .xe on the fly ─────────────────────────────────────
		var err error
		mod, err = compileSource(inputFile)
		if err != nil {
			fmt.Fprintf(os.Stderr, "%s[xenlyrun]%s %v\n", col("1;31"), reset(), err)
			os.Exit(1)
		}
	} else {
		// ── Load .xebc bytecode ──────────────────────────────────────────
		if filepath.Ext(inputFile) == ".xe" {
			// Convenience: user passed .xe to xenlyrun — auto-compile
			fmt.Fprintf(os.Stderr, "%s[xenlyrun]%s note: compiling .xe on the fly "+
				"(use xenlybyc to save .xebc)\n", col("1;33"), reset())
			var err error
			mod, err = compileSource(inputFile)
			if err != nil {
				fmt.Fprintf(os.Stderr, "%s[xenlyrun]%s %v\n", col("1;31"), reset(), err)
				os.Exit(1)
			}
		} else {
			var err error
			mod, err = bytecode.Read(inputFile)
			if err != nil {
				fmt.Fprintf(os.Stderr, "%s[xenlyrun]%s cannot load %q: %v\n",
					col("1;31"), reset(), inputFile, err)
				os.Exit(1)
			}
		}
	}

	tLoad := time.Now()

	// ── Execute ──────────────────────────────────────────────────────────
	machine := vm.NewXVM(mod)
	if err := machine.Run(); err != nil {
		fmt.Fprintf(os.Stderr, "%s[xenlyrun]%s runtime error: %v\n",
			col("1;31"), reset(), err)
		os.Exit(1)
	}

	tExec := time.Now()

	if doTime {
		fmt.Fprintf(os.Stderr, "\n%s[xenlyrun] execution times:%s\n", col("1;36"), reset())
		fmt.Fprintf(os.Stderr, "  load   %6.2f ms\n", float64(tLoad.Sub(t0).Microseconds())/1000)
		fmt.Fprintf(os.Stderr, "  exec   %6.2f ms\n", float64(tExec.Sub(tLoad).Microseconds())/1000)
		fmt.Fprintf(os.Stderr, "  total  %6.2f ms\n", float64(tExec.Sub(t0).Microseconds())/1000)
	}
}

// compileSource compiles a .xe file to a bytecode.Module without writing a file.
func compileSource(path string) (*bytecode.Module, error) {
	src, err := os.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("cannot open %q: %w", path, err)
	}

	lexer := compiler.NewLexer(string(src))
	tokens, lexErrs := lexer.Tokenize()
	if len(lexErrs) > 0 {
		return nil, fmt.Errorf("lex errors in %s:\n  %s", path, strings.Join(lexErrs, "\n  "))
	}

	parser := compiler.NewParser(tokens)
	ast, parseErrs := parser.Parse()
	if len(parseErrs) > 0 {
		return nil, fmt.Errorf("parse errors in %s:\n  %s", path, strings.Join(parseErrs, "\n  "))
	}

	comp := compiler.NewCompiler()
	mod, compErrs := comp.Compile(ast)
	if len(compErrs) > 0 {
		return nil, fmt.Errorf("compile errors in %s:\n  %s", path, strings.Join(compErrs, "\n  "))
	}

	return mod, nil
}
