//go:build windows

package vm

import (
	"math"
	"os"
	"os/exec"
	"runtime"
	"time"
)

// registerSysModule provides a minimal sys module on Windows.
// Full POSIX syscall wrappers are not available on Windows.
func registerSysModule(env *Env) {
	sysObj := Object(map[string]*Value{
		"exit": builtin(func(a []*Value) (*Value, error) {
			code := 0
			if len(a) >= 1 { code = int(a[0].NumVal) }
			os.Exit(code); return Null(), nil
		}),
		"getpid":  builtin(func(a []*Value) (*Value, error) { return Number(float64(os.Getpid())), nil }),
		"getenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			v := os.Getenv(a[0].String())
			if v == "" { return Null(), nil }
			return String(v), nil
		}),
		"setenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			if err := os.Setenv(a[0].String(), a[1].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"unsetenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			os.Unsetenv(a[0].String()); return Number(0), nil
		}),
		"getcwd": builtin(func(a []*Value) (*Value, error) {
			cwd, err := os.Getwd()
			if err != nil { return Null(), nil }
			return String(cwd), nil
		}),
		"hostname": builtin(func(a []*Value) (*Value, error) {
			h, err := os.Hostname()
			if err != nil { return Null(), nil }
			return String(h), nil
		}),
		"nproc":      builtin(func(a []*Value) (*Value, error) { return Number(float64(runtime.NumCPU())), nil }),
		"nproc_conf": builtin(func(a []*Value) (*Value, error) { return Number(float64(runtime.NumCPU())), nil }),
		"time":  builtin(func(a []*Value) (*Value, error) { return Number(float64(time.Now().Unix())), nil }),
		"clock": builtin(func(a []*Value) (*Value, error) { return Number(float64(time.Now().UnixNano()) / 1e9), nil }),
		"sleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) >= 1 { time.Sleep(time.Duration(a[0].NumVal) * time.Second) }
			return Number(0), nil
		}),
		"usleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) >= 1 { time.Sleep(time.Duration(a[0].NumVal) * time.Microsecond) }
			return Number(0), nil
		}),
		"nanosleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) >= 2 {
				time.Sleep(time.Duration(a[0].NumVal)*time.Second + time.Duration(a[1].NumVal)*time.Nanosecond)
			}
			return Number(0), nil
		}),
		"clock_realtime": builtin(func(a []*Value) (*Value, error) {
			now := time.Now()
			return Array([]*Value{Number(float64(now.Unix())), Number(float64(now.Nanosecond()))}), nil
		}),
		"clock_monotonic": builtin(func(a []*Value) (*Value, error) {
			now := time.Now()
			return Array([]*Value{Number(float64(now.Unix())), Number(float64(now.Nanosecond()))}), nil
		}),
		"gettimeofday": builtin(func(a []*Value) (*Value, error) {
			now := time.Now()
			return Array([]*Value{Number(float64(now.Unix())), Number(float64(now.Nanosecond() / 1000))}), nil
		}),
		"system": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			cmd := exec.Command("cmd", "/C", a[0].String())
			cmd.Stdout, cmd.Stderr = os.Stdout, os.Stderr
			if err := cmd.Run(); err != nil {
				if e, ok := err.(*exec.ExitError); ok { return Number(float64(e.ExitCode())), nil }
				return Number(-1), nil
			}
			return Number(0), nil
		}),
		"uname": builtin(func(a []*Value) (*Value, error) {
			return Array([]*Value{
				String("Windows"), String("unknown"), String("unknown"),
				String("unknown"), String(runtime.GOARCH),
			}), nil
		}),
		"sizeof_ptr": builtin(func(a []*Value) (*Value, error) { return Number(float64(8)), nil }),
		"endian":     builtin(func(a []*Value) (*Value, error) { return String("little"), nil }),
		"errno":      builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"strerror":   builtin(func(a []*Value) (*Value, error) { return String("unknown error"), nil }),
		"isfile": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return False(), nil }
			fi, err := os.Stat(a[0].String())
			if err != nil { return False(), nil }
			return Bool(!fi.IsDir()), nil
		}),
		"isdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return False(), nil }
			fi, err := os.Stat(a[0].String())
			if err != nil { return False(), nil }
			return Bool(fi.IsDir()), nil
		}),
		"mkdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			if err := os.Mkdir(a[0].String(), 0755); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"rmdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			if err := os.Remove(a[0].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"unlink": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			if err := os.Remove(a[0].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"rename": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			if err := os.Rename(a[0].String(), a[1].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"chdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			if err := os.Chdir(a[0].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		// Numeric limits
		"STDIN": builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"STDOUT": builtin(func(a []*Value) (*Value, error) { return Number(1), nil }),
		"STDERR": builtin(func(a []*Value) (*Value, error) { return Number(2), nil }),
		"INT_MAX":  builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxInt32)), nil }),
		"INT_MIN":  builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MinInt32)), nil }),
		"UINT_MAX": builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxUint32)), nil }),
		"LONG_MAX": builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxInt64)), nil }),
		"INT8_MAX": builtin(func(a []*Value) (*Value, error) { return Number(127), nil }),
		"INT8_MIN": builtin(func(a []*Value) (*Value, error) { return Number(-128), nil }),
		"INT16_MAX": builtin(func(a []*Value) (*Value, error) { return Number(32767), nil }),
		"INT16_MIN": builtin(func(a []*Value) (*Value, error) { return Number(-32768), nil }),
		"INT32_MAX": builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxInt32)), nil }),
		"INT32_MIN": builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MinInt32)), nil }),
		"UINT8_MAX":  builtin(func(a []*Value) (*Value, error) { return Number(255), nil }),
		"UINT16_MAX": builtin(func(a []*Value) (*Value, error) { return Number(65535), nil }),
		"UINT32_MAX": builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxUint32)), nil }),
	})
	env.Define("sys", sysObj, true)
}
