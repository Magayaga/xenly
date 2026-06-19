package main

import (
	"bytes"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"testing"

	"xvm/pkg/bytecode"
	"xvm/pkg/compiler"
)

func compileTestBytecode(t *testing.T, out string) {
	t.Helper()
	lexer := compiler.NewLexer(`print("xenlyimg-test")`)
	tokens, lexErrs := lexer.Tokenize()
	if len(lexErrs) > 0 {
		t.Fatalf("lex errors: %v", lexErrs)
	}
	parser := compiler.NewParser(tokens)
	ast, parseErrs := parser.Parse()
	if len(parseErrs) > 0 {
		t.Fatalf("parse errors: %v", parseErrs)
	}
	mod, compErrs := compiler.NewCompiler().Compile(ast)
	if len(compErrs) > 0 {
		t.Fatalf("compile errors: %v", compErrs)
	}
	if err := bytecode.Write(mod, out); err != nil {
		t.Fatalf("write bytecode: %v", err)
	}
}

func TestSplitTarget(t *testing.T) {
	goos, goarch, err := splitTarget("linux/amd64")
	if err != nil {
		t.Fatalf("splitTarget returned error: %v", err)
	}
	if goos != "linux" || goarch != "amd64" {
		t.Fatalf("splitTarget = %s/%s, want linux/amd64", goos, goarch)
	}
	if _, _, err := splitTarget("linux-amd64"); err == nil {
		t.Fatal("splitTarget accepted malformed target")
	}
}

func TestFindModuleRootFrom(t *testing.T) {
	root, ok := findModuleRootFrom(".")
	if !ok {
		t.Fatal("findModuleRootFrom did not locate xvm module from package cwd")
	}
	if _, err := os.Stat(filepath.Join(root, "go.mod")); err != nil {
		t.Fatalf("module root does not contain go.mod: %v", err)
	}
}

func TestXenlyimgBuildsRelativeOutput(t *testing.T) {
	if testing.Short() {
		t.Skip("skipping native image integration test in short mode")
	}
	if _, err := exec.LookPath("go"); err != nil {
		t.Skipf("go toolchain not available: %v", err)
	}

	tmp := t.TempDir()
	bytecodePath := filepath.Join(tmp, "hello.xebc")
	compileTestBytecode(t, bytecodePath)

	outputName := "hello-native"
	if runtime.GOOS == "windows" {
		outputName += ".exe"
	}
	cmd := exec.Command("go", "run", ".", bytecodePath, "-o", outputName, "--no-color")
	cmd.Dir = "."
	cmd.Env = append(os.Environ(), "XVM_ROOT=..")
	var combined bytes.Buffer
	cmd.Stdout = &combined
	cmd.Stderr = &combined
	if err := cmd.Run(); err != nil {
		t.Fatalf("xenlyimg failed: %v\n%s", err, combined.String())
	}

	outputPath, err := filepath.Abs(outputName)
	if err != nil {
		t.Fatalf("resolve output path: %v", err)
	}
	if _, err := os.Stat(outputPath); err != nil {
		t.Fatalf("relative output was not created in caller directory: %v\n%s", err, combined.String())
	}
	defer os.Remove(outputPath)

	run := exec.Command(outputPath)
	var runOut bytes.Buffer
	run.Stdout = &runOut
	run.Stderr = &runOut
	if err := run.Run(); err != nil {
		t.Fatalf("native image failed: %v\n%s", err, runOut.String())
	}
	if got := runOut.String(); got != "xenlyimg-test\n" {
		t.Fatalf("native image output = %q, want %q", got, "xenlyimg-test\n")
	}
}
