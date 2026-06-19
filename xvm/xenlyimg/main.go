/*
 * XENLY - Xenly Virtual Machine (XVM)
 * xenlyimg — XVM Native Image Builder
 *
 * Compiles portable XVM bytecode (.xebc/.xyc) into a standalone,
 * platform-specific native executable by embedding the bytecode image into a
 * small optimized XVM runner and invoking the Go AOT toolchain.
 */
package main

import (
	"encoding/base64"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"time"

	"xvm/pkg/bytecode"
)

const (
	xenlyimgVersion = "1.0.0"
	xenlyimgAuthor  = "Cyril John Magayaga"
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

func usage(prog string) {
	fmt.Printf("\n  %sxenlyimg%s  XVM Native Image Builder  %sv%s%s\n", col("1;36"), reset(), col("1;33"), xenlyimgVersion, reset())
	fmt.Printf("\n  %sUsage:%s   %s [options] <file.xebc|file.xyc>\n", col("1;33"), reset(), prog)
	fmt.Printf("\n  %sOptions:%s\n", col("1;33"), reset())
	fmt.Printf("    %s-o <file>%s             Output native executable (default: input name)\n", col("1"), reset())
	fmt.Printf("    %s--target <os/arch>%s    Build for linux/amd64, darwin/arm64, windows/amd64, ...\n", col("1"), reset())
	fmt.Printf("    %s--workdir <dir>%s       Keep generated native-image build directory\n", col("1"), reset())
	fmt.Printf("    %s--no-color%s            Disable ANSI colour output\n", col("1"), reset())
	fmt.Printf("    %s--time%s                Show image build time\n", col("1"), reset())
	fmt.Printf("    %s-h, --help%s            Show this help\n", col("1"), reset())
	fmt.Printf("    %s-v, --version%s         Show version\n", col("1"), reset())
	fmt.Printf("\n  %sPipeline:%s\n", col("1;32"), reset())
	fmt.Printf("    xenlybyc hello.xe -o hello.xebc\n")
	fmt.Printf("    %s hello.xebc -o hello%s\n\n", prog, reset())
}

func defaultOutput(input, targetOS string) string {
	base := strings.TrimSuffix(input, filepath.Ext(input))
	if targetOS == "windows" && !strings.HasSuffix(strings.ToLower(base), ".exe") {
		return base + ".exe"
	}
	return base
}

func splitTarget(target string) (string, string, error) {
	if target == "" {
		return runtime.GOOS, runtime.GOARCH, nil
	}
	parts := strings.Split(target, "/")
	if len(parts) != 2 || parts[0] == "" || parts[1] == "" {
		return "", "", fmt.Errorf("target must be in GOOS/GOARCH form, got %q", target)
	}
	return parts[0], parts[1], nil
}

func findModuleRootFrom(start string) (string, bool) {
	if start == "" {
		return "", false
	}
	start, err := filepath.Abs(start)
	if err != nil {
		return "", false
	}
	info, err := os.Stat(start)
	if err == nil && !info.IsDir() {
		start = filepath.Dir(start)
	}
	for {
		gomod := filepath.Join(start, "go.mod")
		if data, err := os.ReadFile(gomod); err == nil && strings.Contains(string(data), "module xvm") {
			return start, true
		}
		parent := filepath.Dir(start)
		if parent == start {
			return "", false
		}
		start = parent
	}
}

func moduleRoot() (string, error) {
	if root, ok := findModuleRootFrom(os.Getenv("XVM_ROOT")); ok {
		return root, nil
	}
	if cwd, err := os.Getwd(); err == nil {
		if root, ok := findModuleRootFrom(cwd); ok {
			return root, nil
		}
	}
	if exe, err := os.Executable(); err == nil {
		if root, ok := findModuleRootFrom(exe); ok {
			return root, nil
		}
	}

	cmd := exec.Command("go", "env", "GOMOD")
	out, err := cmd.Output()
	if err == nil {
		gomod := strings.TrimSpace(string(out))
		if gomod != "" && gomod != os.DevNull {
			if root, ok := findModuleRootFrom(gomod); ok {
				return root, nil
			}
		}
	}
	return "", fmt.Errorf("cannot locate xvm/go.mod; run from the xvm tree or set XVM_ROOT")
}

func writeBuildFiles(dir, moduleDir, encoded string) error {
	gomod := "module xenlyimgbuild\n\ngo 1.21\n\nrequire xvm v0.0.0\n\nreplace xvm => " + filepath.ToSlash(moduleDir) + "\n"
	if err := os.WriteFile(filepath.Join(dir, "go.mod"), []byte(gomod), 0o644); err != nil {
		return err
	}
	mainSrc := fmt.Sprintf(`package main

import (
	"bufio"
	"encoding/base64"
	"fmt"
	"os"

	"xvm/pkg/bytecode"
	"xvm/pkg/vm"
)

const embeddedBytecode = %q

func main() {
	data, err := base64.StdEncoding.DecodeString(embeddedBytecode)
	if err != nil {
		fmt.Fprintf(os.Stderr, "[xenlyimg] embedded bytecode error: %%v\n", err)
		os.Exit(1)
	}
	mod, err := bytecode.ReadBytes(data)
	if err != nil {
		fmt.Fprintf(os.Stderr, "[xenlyimg] bytecode decode error: %%v\n", err)
		os.Exit(1)
	}
	stdout := bufio.NewWriter(os.Stdout)
	vm.SetStdout(stdout)
	defer stdout.Flush()
	if err := vm.NewXVM(mod).Run(); err != nil {
		fmt.Fprintf(os.Stderr, "[xenlyimg] runtime error: %%v\n", err)
		os.Exit(1)
	}
}
`, encoded)
	return os.WriteFile(filepath.Join(dir, "main.go"), []byte(mainSrc), 0o644)
}

func main() {
	var inputFile, outputFile, target, keepWorkdir string
	var showTime bool

	args := os.Args[1:]
	for i := 0; i < len(args); i++ {
		switch args[i] {
		case "-h", "--help":
			usage(os.Args[0])
			return
		case "-v", "--version":
			fmt.Printf("xenlyimg v%s — XVM Native Image Builder\n", xenlyimgVersion)
			return
		case "--author":
			fmt.Printf("xenlyimg — created, designed, and developed by %s\n", xenlyimgAuthor)
			return
		case "--no-color":
			useColor = false
		case "--time":
			showTime = true
		case "-o":
			i++
			if i >= len(args) {
				fmt.Fprintln(os.Stderr, "[xenlyimg] -o requires a file")
				os.Exit(1)
			}
			outputFile = args[i]
		case "--target":
			i++
			if i >= len(args) {
				fmt.Fprintln(os.Stderr, "[xenlyimg] --target requires GOOS/GOARCH")
				os.Exit(1)
			}
			target = args[i]
		case "--workdir":
			i++
			if i >= len(args) {
				fmt.Fprintln(os.Stderr, "[xenlyimg] --workdir requires a directory")
				os.Exit(1)
			}
			keepWorkdir = args[i]
		default:
			if strings.HasPrefix(args[i], "-") {
				fmt.Fprintf(os.Stderr, "[xenlyimg] unknown option %q\n", args[i])
				os.Exit(1)
			}
			inputFile = args[i]
		}
	}
	if inputFile == "" {
		usage(os.Args[0])
		os.Exit(1)
	}

	t0 := time.Now()
	if _, err := bytecode.Read(inputFile); err != nil {
		fmt.Fprintf(os.Stderr, "[xenlyimg] cannot load %q: %v\n", inputFile, err)
		os.Exit(1)
	}
	data, err := os.ReadFile(inputFile)
	if err != nil {
		fmt.Fprintf(os.Stderr, "[xenlyimg] cannot read %q: %v\n", inputFile, err)
		os.Exit(1)
	}
	goos, goarch, err := splitTarget(target)
	if err != nil {
		fmt.Fprintf(os.Stderr, "[xenlyimg] %v\n", err)
		os.Exit(1)
	}
	if outputFile == "" {
		outputFile = defaultOutput(inputFile, goos)
	}
	outputFile, err = filepath.Abs(outputFile)
	if err != nil {
		fmt.Fprintf(os.Stderr, "[xenlyimg] cannot resolve output path: %v\n", err)
		os.Exit(1)
	}
	moduleDir, err := moduleRoot()
	if err != nil {
		fmt.Fprintf(os.Stderr, "[xenlyimg] %v\n", err)
		os.Exit(1)
	}
	workdir := keepWorkdir
	if workdir == "" {
		workdir, err = os.MkdirTemp("", "xenlyimg-*")
		if err != nil {
			fmt.Fprintf(os.Stderr, "[xenlyimg] %v\n", err)
			os.Exit(1)
		}
		defer os.RemoveAll(workdir)
	} else if err := os.MkdirAll(workdir, 0o755); err != nil {
		fmt.Fprintf(os.Stderr, "[xenlyimg] %v\n", err)
		os.Exit(1)
	}
	if err := writeBuildFiles(workdir, moduleDir, base64.StdEncoding.EncodeToString(data)); err != nil {
		fmt.Fprintf(os.Stderr, "[xenlyimg] cannot write build files: %v\n", err)
		os.Exit(1)
	}

	cmd := exec.Command("go", "build", "-trimpath", "-ldflags=-s -w", "-o", outputFile, ".")
	cmd.Dir = workdir
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Env = append(os.Environ(), "GOOS="+goos, "GOARCH="+goarch)
	if err := cmd.Run(); err != nil {
		fmt.Fprintf(os.Stderr, "[xenlyimg] native image build failed: %v\n", err)
		os.Exit(1)
	}
	fmt.Printf("%s[xenlyimg]%s native executable: %s (%s/%s)\n", col("1;32"), reset(), outputFile, goos, goarch)
	if keepWorkdir != "" {
		fmt.Printf("%s[xenlyimg]%s build directory: %s\n", col("1;33"), reset(), workdir)
	}
	if showTime {
		fmt.Printf("%s[xenlyimg]%s image build time: %.2f ms\n", col("1;36"), reset(), float64(time.Since(t0).Microseconds())/1000)
	}
}
