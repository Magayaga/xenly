/*
 * XENLY VIRTUAL MACHINE (XVM) - high-performance of the Virtual Machine
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * It is available for the Linux, macOS, and Windows operating systems.
 *
 */
/*
 * XENLY - Xenly Virtual Machine (XVM)
 * xenlyimg — XVM Native Image Builder
 *
 * Compiles portable XVM bytecode (.xebc) into a standalone,
 * platform-specific native executable by embedding the bytecode image into a
 * small optimized XVM runner and invoking the Go AOT toolchain.
 */
package main

import (
	"bytes"
	"encoding/base64"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"sync"
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
	fmt.Printf("\n  %sUsage:%s   %s [options] <file.xebc>\n", col("1;33"), reset(), prog)
	fmt.Printf("\n  %sOptions:%s\n", col("1;33"), reset())
	fmt.Printf("    %s-o <file>%s             Output native executable (default: input name)\n", col("1"), reset())
	fmt.Printf("    %s--target <os/arch>%s    Build for linux/amd64, darwin/arm64, windows/amd64, ...\n", col("1"), reset())
	fmt.Printf("    %s--workdir <dir>%s       Keep generated native-image build directory\n", col("1"), reset())
	fmt.Printf("    %s--no-color%s            Disable ANSI colour output\n", col("1"), reset())
	fmt.Printf("    %s--time%s                Show image build time\n", col("1"), reset())
	fmt.Printf("    %s--no-cache%s            Don't use/create the persistent Go build cache\n", col("1"), reset())
	fmt.Printf("    %s--targets%s LIST        Build several GOOS/GOARCH targets in parallel (comma-separated,\n", col("1"), reset())
	fmt.Printf("                          e.g. linux/amd64,darwin/arm64,windows/amd64); -o then names an\n")
	fmt.Printf("                          output directory, files are named <base>-<goos>-<goarch>\n")
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

// multiTargetOutput names one file among several targets built in the same
// run, since a single -o name can't serve more than one of them.
func multiTargetOutput(dir, input, goos, goarch string) string {
	base := filepath.Base(strings.TrimSuffix(input, filepath.Ext(input)))
	name := fmt.Sprintf("%s-%s-%s", base, goos, goarch)
	if goos == "windows" {
		name += ".exe"
	}
	return filepath.Join(dir, name)
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
	gomod := "module xenlyimgbuild\n\ngo 1.27\n\nrequire xvm v0.0.0\n\nreplace xvm => " + filepath.ToSlash(moduleDir) + "\n"
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

// persistentGoCache returns a build-cache directory that survives across
// invocations regardless of $HOME (many CI runners and containers start
// with a fresh, empty $HOME on every job, which otherwise forces Go to
// recompile its entire standard library from scratch on every single
// image build - roughly 10s for a trivial program, confirmed by
// benchmarking, versus ~0.1-0.2s once GOCACHE is warm). Pinning it inside
// the xvm module tree keeps it warm across those runs as long as the
// checkout itself is reused (e.g. a persisted/cached working directory),
// the same way callers would cache node_modules or a vendor directory.
// Returns "" (meaning: let Go pick its own default, usually under
// os.UserCacheDir()) if the directory can't be created, e.g. a read-only
// checkout - this is a speed optimization, never a hard requirement.
func persistentGoCache(moduleDir string) (dir string, alreadyWarm bool) {
	dir = filepath.Join(moduleDir, ".xenlyimg-cache", "go-build")
	if _, err := os.Stat(dir); err == nil {
		alreadyWarm = true
	}
	if err := os.MkdirAll(dir, 0o755); err != nil {
		return "", false
	}
	return dir, alreadyWarm
}

// buildEnv starts from the caller's environment, drops any existing
// entries for the keys we're about to set (rather than relying on
// duplicate-entry precedence, which differs between C's getenv() and Go's
// os.Getenv() and isn't worth depending on), and appends our values.
func buildEnv(overrides map[string]string) []string {
	env := os.Environ()
	out := make([]string, 0, len(env)+len(overrides))
	for _, kv := range env {
		key := kv
		if idx := strings.IndexByte(kv, '='); idx >= 0 {
			key = kv[:idx]
		}
		if _, skip := overrides[key]; skip {
			continue
		}
		out = append(out, kv)
	}
	for k, v := range overrides {
		out = append(out, k+"="+v)
	}
	return out
}

// targetResult is what one native-image build reports back, so builds can
// run concurrently without interleaving their output on the terminal.
type targetResult struct {
	goos, goarch string
	outputFile   string
	elapsed      time.Duration
	log          []byte
	err          error
}

// buildTarget runs one `go build` for goos/goarch inside workdir (which
// already holds the generated main.go/go.mod; those files are identical for
// every target, since only GOOS/GOARCH differ, and `go build` only reads
// them, so concurrent builds can safely share one workdir). All Go builds
// share one GOCACHE (cacheDir, "" for Go's default), which Go's build cache
// supports for concurrent use.
func buildTarget(workdir, cacheDir, outputFile, goos, goarch string) targetResult {
	res := targetResult{goos: goos, goarch: goarch, outputFile: outputFile}
	t0 := time.Now()
	cmd := exec.Command("go", "build", "-trimpath", "-ldflags=-s -w", "-o", outputFile, ".")
	cmd.Dir = workdir
	var buf bytes.Buffer
	cmd.Stdout = &buf
	cmd.Stderr = &buf
	overrides := map[string]string{"GOOS": goos, "GOARCH": goarch}
	if cacheDir != "" {
		overrides["GOCACHE"] = cacheDir
	}
	cmd.Env = buildEnv(overrides)
	res.err = cmd.Run()
	res.log = buf.Bytes()
	res.elapsed = time.Since(t0)
	return res
}

func main() {
	var inputFile, outputFile, target, targets, keepWorkdir string
	var showTime, noCache bool

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
		case "--no-cache":
			noCache = true
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
		case "--targets":
			i++
			if i >= len(args) {
				fmt.Fprintln(os.Stderr, "[xenlyimg] --targets requires a comma-separated GOOS/GOARCH list")
				os.Exit(1)
			}
			targets = args[i]
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
	if target != "" && targets != "" {
		fmt.Fprintln(os.Stderr, "[xenlyimg] use either --target or --targets, not both")
		os.Exit(1)
	}

	type job struct{ goos, goarch, out string }
	var jobs []job
	multi := targets != ""
	if multi {
		outDir := outputFile
		if outDir == "" {
			outDir = "."
		}
		outDir, err = filepath.Abs(outDir)
		if err != nil {
			fmt.Fprintf(os.Stderr, "[xenlyimg] cannot resolve output directory: %v\n", err)
			os.Exit(1)
		}
		if err := os.MkdirAll(outDir, 0o755); err != nil {
			fmt.Fprintf(os.Stderr, "[xenlyimg] cannot create output directory: %v\n", err)
			os.Exit(1)
		}
		seen := map[string]bool{}
		for _, t := range strings.Split(targets, ",") {
			t = strings.TrimSpace(t)
			if t == "" {
				fmt.Fprintln(os.Stderr, "[xenlyimg] --targets contains an empty entry")
				os.Exit(1)
			}
			if seen[t] {
				continue
			}
			seen[t] = true
			goos, goarch, err := splitTarget(t)
			if err != nil {
				fmt.Fprintf(os.Stderr, "[xenlyimg] %v\n", err)
				os.Exit(1)
			}
			jobs = append(jobs, job{goos, goarch, multiTargetOutput(outDir, inputFile, goos, goarch)})
		}
	} else {
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
		jobs = []job{{goos, goarch, outputFile}}
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

	// Decide the cache location once, before any build starts, so that
	// "was it already warm" is answered consistently rather than racing
	// with the first build that creates the directory.
	cacheDir, cacheWarm := "", false
	if !noCache {
		cacheDir, cacheWarm = persistentGoCache(moduleDir)
	}

	results := make([]targetResult, len(jobs))
	workers := runtime.NumCPU()
	if workers > len(jobs) {
		workers = len(jobs)
	}
	sem := make(chan struct{}, workers)
	var wg sync.WaitGroup
	for idx, j := range jobs {
		wg.Add(1)
		sem <- struct{}{}
		go func(idx int, j job) {
			defer wg.Done()
			defer func() { <-sem }()
			results[idx] = buildTarget(workdir, cacheDir, j.out, j.goos, j.goarch)
		}(idx, j)
	}
	wg.Wait()

	failed := 0
	for _, r := range results {
		if r.err != nil {
			failed++
			fmt.Fprintf(os.Stderr, "%s[xenlyimg]%s native image build failed for %s/%s: %v\n", col("1;31"), reset(), r.goos, r.goarch, r.err)
			os.Stderr.Write(r.log)
			continue
		}
		os.Stdout.Write(r.log)
		fmt.Printf("%s[xenlyimg]%s native executable: %s (%s/%s)\n", col("1;32"), reset(), r.outputFile, r.goos, r.goarch)
		if showTime && multi {
			fmt.Printf("%s[xenlyimg]%s   %s/%s took %.2f ms\n", col("1;36"), reset(), r.goos, r.goarch, float64(r.elapsed.Microseconds())/1000)
		}
	}
	if keepWorkdir != "" {
		fmt.Printf("%s[xenlyimg]%s build directory: %s\n", col("1;33"), reset(), workdir)
	}
	if cacheDir != "" && !cacheWarm && failed == 0 {
		fmt.Printf("%s[xenlyimg]%s build cache warmed - subsequent builds for these targets will be much faster\n", col("1;33"), reset())
	}
	if showTime {
		fmt.Printf("%s[xenlyimg]%s image build time: %.2f ms\n", col("1;36"), reset(), float64(time.Since(t0).Microseconds())/1000)
	}
	if failed > 0 {
		os.Exit(1)
	}
}
