/*
 * XENLY - Xenly Virtual Machine (XVM)
 * xenlybyc — Xenly Bytecode Compiler
 *
 * Translates human-readable Xenly source code (.xe) into
 * portable XVM bytecode (.xyc) that runs on the Xenly Virtual Machine.
 *
 * Created, designed, and developed by Cyril John Magayaga.
 * XVM Go implementation — available for Windows, macOS, and Linux.
 *
 * Usage:
 *   xenlybyc <source.xe> [-o output.xyc]
 *   xenlybyc --disasm  <source.xe | source.xyc>
 *   xenlybyc --version | --help
 *
 * Pipeline:
 *   source.xe → Lexer → Parser → AST → Compiler → .xyc (XVM bytecode)
 */
package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"xvm/pkg/bytecode"
	"xvm/pkg/compiler"
)

const (
	xvmVersion     = "1.0.0"
	xvmReleaseDate = "2025-XX-XX"
	xvmAuthor      = "Cyril John Magayaga"
	xvmAuthorEmail = "cjmagayaga957@gmail.com"
)

// ANSI colour helpers
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
	fmt.Printf("\n  %sxenlybyc%s  Xenly Bytecode Compiler  %sv%s%s\n",
		col("1;36"), reset(), col("1;33"), xvmVersion, reset())
	fmt.Printf("\n  %sUsage:%s   %s [options] <file.xe>\n", col("1;33"), reset(), prog)
	fmt.Printf("\n  %sOptions:%s\n", col("1;33"), reset())
	fmt.Printf("    %s-o <file>%s        Output bytecode file  %s(default: <input>.xyc)%s\n",
		col("1"), reset(), col("2"), reset())
	fmt.Printf("    %s--disasm%s         Disassemble .xyc file and print to stdout\n", col("1"), reset())
	fmt.Printf("    %s--emit-ast%s       Dump parsed AST to stdout, then exit\n", col("1"), reset())
	fmt.Printf("    %s--emit-tokens%s    Dump lexer token stream, then exit\n", col("1"), reset())
	fmt.Printf("    %s--no-color%s       Disable ANSI colour output\n", col("1"), reset())
	fmt.Printf("    %s--time%s           Show compilation time breakdown\n", col("1"), reset())
	fmt.Printf("    %s-h, --help%s       Show this help\n", col("1"), reset())
	fmt.Printf("    %s-v, --version%s    Show version\n", col("1"), reset())
	fmt.Printf("    %s--author%s         Show author information\n", col("1"), reset())
	fmt.Printf("\n  %sExamples:%s\n", col("1;32"), reset())
	fmt.Printf("    %s hello.xe                 %s→ hello.xyc\n", prog, col("2"))
	fmt.Printf("    %s%s hello.xe -o out.xyc     %s→ out.xyc\n", reset(), prog, col("2"))
	fmt.Printf("    %s%s --disasm hello.xyc      %s→ disassembly listing\n", reset(), prog, col("2"))
	fmt.Printf("    %s%s --emit-ast hello.xe     %s→ AST dump\n%s\n", reset(), prog, col("2"), reset())
}

func printVersion() {
	fmt.Printf("\n  %sxenlybyc%s v%s  (Xenly Bytecode Compiler)\n", col("1;36"), reset(), xvmVersion)
	fmt.Printf("  Compiles .xe source → .xyc portable XVM bytecode\n")
	fmt.Printf("  Supported platforms: Windows, macOS, Linux\n\n")
}

func swapExt(path, ext string) string {
	base := strings.TrimSuffix(path, filepath.Ext(path))
	return base + ext
}

func main() {
	var (
		inputFile  string
		outputFile string
		doDisasm   bool
		emitAST    bool
		emitTokens bool
		doTime     bool
	)

	args := os.Args[1:]
	for i := 0; i < len(args); i++ {
		switch args[i] {
		case "-h", "--help":
			printUsage(os.Args[0])
			os.Exit(0)
		case "-v", "--version":
			printVersion()
			os.Exit(0)
		case "--author":
			fmt.Printf("\n  %sxenlybyc%s — created, designed, and developed by\n",
				col("1;36"), reset())
			fmt.Printf("  %s <%s>\n\n", xvmAuthor, xvmAuthorEmail)
			os.Exit(0)
		case "--no-color":
			useColor = false
		case "--disasm":
			doDisasm = true
		case "--emit-ast":
			emitAST = true
		case "--emit-tokens":
			emitTokens = true
		case "--time":
			doTime = true
		case "-o":
			i++
			if i >= len(args) {
				fmt.Fprintf(os.Stderr, "%s[xenlybyc]%s -o requires a filename\n", col("1;31"), reset())
				os.Exit(1)
			}
			outputFile = args[i]
		default:
			if strings.HasPrefix(args[i], "-") {
				fmt.Fprintf(os.Stderr, "%s[xenlybyc]%s unknown flag: %s\n", col("1;31"), reset(), args[i])
				os.Exit(1)
			}
			inputFile = args[i]
		}
	}

	// ── Disassemble mode ────────────────────────────────────────────────
	if doDisasm {
		if inputFile == "" {
			fmt.Fprintf(os.Stderr, "%s[xenlybyc]%s --disasm requires an input file\n", col("1;31"), reset())
			os.Exit(1)
		}
		mod, err := bytecode.Read(inputFile)
		if err != nil {
			fmt.Fprintf(os.Stderr, "%s[xenlybyc]%s cannot read %s: %v\n",
				col("1;31"), reset(), inputFile, err)
			os.Exit(1)
		}
		bytecode.Disassemble(mod, os.Stdout)
		os.Exit(0)
	}

	if inputFile == "" {
		printUsage(os.Args[0])
		os.Exit(0)
	}

	// ── Read source ──────────────────────────────────────────────────────
	t0 := time.Now()
	src, err := os.ReadFile(inputFile)
	if err != nil {
		fmt.Fprintf(os.Stderr, "%s[xenlybyc]%s cannot open %q: %v\n",
			col("1;31"), reset(), inputFile, err)
		os.Exit(1)
	}
	tRead := time.Now()

	// ── Lex ─────────────────────────────────────────────────────────────
	lexer := compiler.NewLexer(string(src))
	tokens, lexErrs := lexer.Tokenize()
	tLex := time.Now()

	if len(lexErrs) > 0 {
		for _, e := range lexErrs {
			fmt.Fprintf(os.Stderr, "%s[xenlybyc]%s %s: %s\n", col("1;31"), reset(), inputFile, e)
		}
		os.Exit(1)
	}

	// ── --emit-tokens ───────────────────────────────────────────────────
	if emitTokens {
		fmt.Printf("=== Token stream: %s ===\n", inputFile)
		for _, tok := range tokens {
			fmt.Printf("  %4d:%-4d  %T  %q\n", tok.Line, tok.Column, tok.Type, tok.Lexeme)
		}
		os.Exit(0)
	}

	// ── Parse ────────────────────────────────────────────────────────────
	parser := compiler.NewParser(tokens)
	ast, parseErrs := parser.Parse()
	tParse := time.Now()

	if len(parseErrs) > 0 {
		for _, e := range parseErrs {
			fmt.Fprintf(os.Stderr, "%s[xenlybyc]%s %s: %s\n", col("1;31"), reset(), inputFile, e)
		}
		os.Exit(1)
	}

	// ── --emit-ast ──────────────────────────────────────────────────────
	if emitAST {
		fmt.Printf("=== AST: %s ===\n", inputFile)
		dumpAST(ast, 0)
		os.Exit(0)
	}

	// ── Compile ──────────────────────────────────────────────────────────
	comp := compiler.NewCompiler()
	mod, compErrs := comp.Compile(ast)
	tCompile := time.Now()

	if len(compErrs) > 0 {
		for _, e := range compErrs {
			fmt.Fprintf(os.Stderr, "%s[xenlybyc]%s %s: %s\n", col("1;31"), reset(), inputFile, e)
		}
		os.Exit(1)
	}

	// ── Write .xyc ──────────────────────────────────────────────────────
	if outputFile == "" {
		outputFile = swapExt(inputFile, ".xyc")
	}
	if err := bytecode.Write(mod, outputFile); err != nil {
		fmt.Fprintf(os.Stderr, "%s[xenlybyc]%s failed to write %q: %v\n",
			col("1;31"), reset(), outputFile, err)
		os.Exit(1)
	}
	tWrite := time.Now()

	fmt.Fprintf(os.Stderr, "%s[xenlybyc]%s OK  →  %s\n",
		col("1;32"), reset(), outputFile)

	if doTime {
		fmt.Fprintf(os.Stderr, "\n%s[xenlybyc] compile times:%s\n", col("1;36"), reset())
		fmt.Fprintf(os.Stderr, "  read     %6.2f ms\n", float64(tRead.Sub(t0).Microseconds())/1000)
		fmt.Fprintf(os.Stderr, "  lex      %6.2f ms\n", float64(tLex.Sub(tRead).Microseconds())/1000)
		fmt.Fprintf(os.Stderr, "  parse    %6.2f ms\n", float64(tParse.Sub(tLex).Microseconds())/1000)
		fmt.Fprintf(os.Stderr, "  compile  %6.2f ms\n", float64(tCompile.Sub(tParse).Microseconds())/1000)
		fmt.Fprintf(os.Stderr, "  write    %6.2f ms\n", float64(tWrite.Sub(tCompile).Microseconds())/1000)
		fmt.Fprintf(os.Stderr, "  total    %6.2f ms\n", float64(tWrite.Sub(t0).Microseconds())/1000)
	}
}

// ─── AST dump helper ──────────────────────────────────────────────────────────

func dumpAST(n *compiler.ASTNode, depth int) {
	if n == nil {
		return
	}
	indent := strings.Repeat("  ", depth)
	prefix := ""
	if depth > 0 {
		prefix = "├─ "
	}
	label := kindName(n.Kind)
	extra := ""
	if n.StrVal != "" {
		extra += fmt.Sprintf(" %q", n.StrVal)
	}
	if n.Kind == compiler.NodeNumber {
		extra += fmt.Sprintf(" %g", n.NumVal)
	}
	if n.Kind == compiler.NodeBool {
		if n.BoolVal {
			extra += " true"
		} else {
			extra += " false"
		}
	}
	fmt.Printf("%s%s%s%s  [%d children]\n", indent, prefix, label, extra, len(n.Children))
	for _, child := range n.Children {
		dumpAST(child, depth+1)
	}
}

func kindName(k compiler.NodeKind) string {
	names := []string{
		"Program", "VarDecl", "ConstDecl", "FnDecl", "ClassDecl", "EnumDecl",
		"TypeAlias", "Namespace", "Export", "Import",
		"Block", "ExprStmt", "Assign", "CompoundAssign", "Return",
		"If", "While", "DoWhile", "For", "ForIn", "Switch",
		"Break", "Continue", "Increment", "Decrement",
		"Print", "Input", "Sleep", "Assert", "Requires", "Ensures", "Invariant",
		"Spawn", "Await",
		"Binary", "Unary", "Ternary", "Nullish",
		"Number", "String", "Bool", "Null", "Ident",
		"FnCall", "MethodCall", "CallExpr",
		"PropertyGet", "PropertySet", "Index", "IndexAssign",
		"This", "Super", "New", "Typeof", "Instanceof",
		"ArrayLiteral", "ObjectLiteral", "ArrowFn",
		"EnumVariant", "Match", "MatchArm", "Pattern",
		"NamedArg", "TupleLiteral", "NamespaceAccess", "Spread",
	}
	if int(k) < len(names) {
		return names[k]
	}
	return fmt.Sprintf("Node(%d)", k)
}
