/*
 * XENLY VIRTUAL MACHINE (XVM) - high-performance of the Virtual Machine
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * It is available for the Linux, macOS, and Windows operating systems.
 *
 */

package main

import (
	"bytes"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
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
	mod, compErrs := compiler.NewCompiler(".").Compile(ast)
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

func TestMultiTargetOutput(t *testing.T) {
	dir := filepath.Join("out", "dir")
	cases := []struct{ input, goos, goarch, want string }{
		{"hello.xebc", "linux", "amd64", filepath.Join(dir, "hello-linux-amd64")},
		{"path/to/app.xebc", "darwin", "arm64", filepath.Join(dir, "app-darwin-arm64")},
		{"hello.xebc", "windows", "amd64", filepath.Join(dir, "hello-windows-amd64.exe")},
	}
	for _, c := range cases {
		if got := multiTargetOutput(dir, c.input, c.goos, c.goarch); got != c.want {
			t.Errorf("multiTargetOutput(%q, %s/%s) = %q, want %q", c.input, c.goos, c.goarch, got, c.want)
		}
	}
}

func TestBuildEnvOverridesExistingKeys(t *testing.T) {
	t.Setenv("GOOS", "plan9")
	t.Setenv("XENLYIMG_TEST_KEEP", "kept")
	env := buildEnv(map[string]string{"GOOS": "linux", "GOARCH": "arm64"})
	counts := map[string]int{}
	values := map[string]string{}
	for _, kv := range env {
		k, v, _ := strings.Cut(kv, "=")
		counts[k]++
		values[k] = v
	}
	// Exactly one entry per overridden key, so the result never depends on
	// which duplicate a given consumer happens to honor.
	if counts["GOOS"] != 1 || values["GOOS"] != "linux" {
		t.Errorf("GOOS: got %d entries, value %q; want exactly 1 = linux", counts["GOOS"], values["GOOS"])
	}
	if counts["GOARCH"] != 1 || values["GOARCH"] != "arm64" {
		t.Errorf("GOARCH: got %d entries, value %q; want exactly 1 = arm64", counts["GOARCH"], values["GOARCH"])
	}
	if values["XENLYIMG_TEST_KEEP"] != "kept" {
		t.Errorf("unrelated variable was dropped or changed: %q", values["XENLYIMG_TEST_KEEP"])
	}
}

func TestPersistentGoCache(t *testing.T) {
	root := t.TempDir()
	dir, warm := persistentGoCache(root)
	if dir == "" || warm {
		t.Fatalf("first call: dir=%q warm=%v, want a created dir and warm=false", dir, warm)
	}
	if st, err := os.Stat(dir); err != nil || !st.IsDir() {
		t.Fatalf("cache dir not created: %v", err)
	}
	if _, warm := persistentGoCache(root); !warm {
		t.Error("second call should report the cache as already warm")
	}
	// A module dir that can't hold the cache must degrade to Go's default
	// (empty string), never fail the build.
	file := filepath.Join(root, "afile")
	if err := os.WriteFile(file, []byte("x"), 0o644); err != nil {
		t.Fatal(err)
	}
	if dir, _ := persistentGoCache(file); dir != "" {
		t.Errorf("unwritable module dir: got %q, want empty fallback", dir)
	}
}
