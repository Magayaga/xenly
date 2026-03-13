/*
 * XENLY - Xenly Virtual Machine (XVM)
 * VM: Built-in standard library functions
 */
package vm

import (
	"encoding/binary"
	"fmt"
	"math"
	"math/bits"
	"math/rand"
	"net"
	"os"
	"os/exec"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"syscall"
	"time"
	"unsafe"
)

// RegisterBuiltins installs all built-in functions and modules into env.
func RegisterBuiltins(env *Env) {
	// ── Core ──────────────────────────────────────────────────────────────
	native(env, "print", builtinPrint)
	native(env, "println", builtinPrint)
	native(env, "input", builtinInput)
	native(env, "len", builtinLen)
	native(env, "type", builtinType)
	native(env, "str", builtinStr)
	native(env, "num", builtinNum)
	native(env, "bool", builtinBool)
	native(env, "range", builtinRange)
	native(env, "sleep", builtinSleep)
	native(env, "exit", builtinExit)
	native(env, "assert", builtinAssert)
	native(env, "error", builtinError)

	// ── Math module ────────────────────────────────────────────────────────
	mathObj := Object(map[string]*Value{
		"PI":    Number(math.Pi),
		"E":     Number(math.E),
		"TAU":   Number(2 * math.Pi),
		"INF":   Number(math.Inf(1)),
		"NAN":   Number(math.NaN()),
		"abs":   builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Abs(a[0].NumVal)), nil }),
		"sqrt":  builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Sqrt(a[0].NumVal)), nil }),
		"cbrt":  builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Cbrt(a[0].NumVal)), nil }),
		"pow":   builtin(func(a []*Value) (*Value, error) { if len(a)<2 {return Number(0),nil}; return Number(math.Pow(a[0].NumVal, a[1].NumVal)), nil }),
		"exp":   builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(1),nil}; return Number(math.Exp(a[0].NumVal)), nil }),
		"log":   builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Log(a[0].NumVal)), nil }),
		"log2":  builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Log2(a[0].NumVal)), nil }),
		"log10": builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Log10(a[0].NumVal)), nil }),
		"sin":   builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Sin(a[0].NumVal)), nil }),
		"cos":   builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(1),nil}; return Number(math.Cos(a[0].NumVal)), nil }),
		"tan":   builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Tan(a[0].NumVal)), nil }),
		"asin":  builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Asin(a[0].NumVal)), nil }),
		"acos":  builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Acos(a[0].NumVal)), nil }),
		"atan":  builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Atan(a[0].NumVal)), nil }),
		"atan2": builtin(func(a []*Value) (*Value, error) { if len(a)<2 {return Number(0),nil}; return Number(math.Atan2(a[0].NumVal, a[1].NumVal)), nil }),
		"sinh":  builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Sinh(a[0].NumVal)), nil }),
		"cosh":  builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(1),nil}; return Number(math.Cosh(a[0].NumVal)), nil }),
		"tanh":  builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Tanh(a[0].NumVal)), nil }),
		"floor": builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Floor(a[0].NumVal)), nil }),
		"ceil":  builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Ceil(a[0].NumVal)), nil }),
		"round": builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Round(a[0].NumVal)), nil }),
		"trunc": builtin(func(a []*Value) (*Value, error) { if len(a)==0 {return Number(0),nil}; return Number(math.Trunc(a[0].NumVal)), nil }),
		"sign": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return Number(0), nil }
			v := a[0].NumVal
			if v == 0 { return Number(0), nil }
			if v > 0 { return Number(1), nil }
			return Number(-1), nil
		}),
		"min": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(math.Inf(1)), nil
			}
			m := a[0].NumVal
			for _, v := range a[1:] {
				if v.NumVal < m {
					m = v.NumVal
				}
			}
			return Number(m), nil
		}),
		"max": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(math.Inf(-1)), nil
			}
			m := a[0].NumVal
			for _, v := range a[1:] {
				if v.NumVal > m {
					m = v.NumVal
				}
			}
			return Number(m), nil
		}),
		"random": builtin(func(a []*Value) (*Value, error) {
			return Number(rand.Float64()), nil
		}),
		"isNaN": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return False(), nil }
			return Bool(math.IsNaN(a[0].NumVal)), nil
		}),
		"isFinite": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return False(), nil }
			return Bool(!math.IsInf(a[0].NumVal, 0) && !math.IsNaN(a[0].NumVal)), nil
		}),
		"hypot": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			return Number(math.Hypot(a[0].NumVal, a[1].NumVal)), nil
		}),
		"clamp": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 {
				return Null(), nil
			}
			v, lo, hi := a[0].NumVal, a[1].NumVal, a[2].NumVal
			return Number(math.Max(lo, math.Min(hi, v))), nil
		}),
		"lerp": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 {
				return Null(), nil
			}
			return Number(a[0].NumVal + (a[1].NumVal-a[0].NumVal)*a[2].NumVal), nil
		}),
		"fmod": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(math.NaN()), nil
			}
			return Number(math.Mod(a[0].NumVal, a[1].NumVal)), nil
		}),
		"gcd": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			x, y := math.Abs(a[0].NumVal), math.Abs(a[1].NumVal)
			for y != 0 {
				x, y = y, math.Mod(x, y)
			}
			return Number(x), nil
		}),
		"lcm": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			x, y := math.Abs(a[0].NumVal), math.Abs(a[1].NumVal)
			if x == 0 || y == 0 { return Number(0), nil }
			gcd := x
			for tmp := y; tmp != 0; gcd, tmp = tmp, math.Mod(gcd, tmp) {}
			return Number(x * y / gcd), nil
		}),
	})
	env.Define("math", mathObj, true)
	env.Define("Math", mathObj, true) // alias

	// ── String module ─────────────────────────────────────────────────────
	strObj := Object(map[string]*Value{
		"fromCharCode": builtin(func(a []*Value) (*Value, error) {
			var sb strings.Builder
			for _, v := range a {
				sb.WriteRune(rune(int(v.NumVal)))
			}
			return String(sb.String()), nil
		}),
		"format": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(fmt.Sprint(a[0].String())), nil
		}),
	})
	env.Define("str", strObj, true)
	env.Define("String", strObj, true)

	// ── Array module ──────────────────────────────────────────────────────
	arrObj := Object(map[string]*Value{
		"isArray": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return False(), nil
			}
			return Bool(a[0].Tag == TypeArray), nil
		}),
		"from": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Array(nil), nil
			}
			if a[0].Tag == TypeArray {
				return a[0], nil
			}
			if a[0].Tag == TypeString {
				runes := []rune(a[0].StrVal)
				items := make([]*Value, len(runes))
				for i, r := range runes {
					items[i] = String(string(r))
				}
				return Array(items), nil
			}
			return Array([]*Value{a[0]}), nil
		}),
		"of": builtin(func(a []*Value) (*Value, error) {
			return Array(a), nil
		}),
		"sort": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Null(), nil
			}
			arr := a[0].ArrayVal
			sort.Slice(arr, func(i, j int) bool {
				if arr[i].Tag == TypeNumber && arr[j].Tag == TypeNumber {
					return arr[i].NumVal < arr[j].NumVal
				}
				return arr[i].String() < arr[j].String()
			})
			return a[0], nil
		}),
	})
	env.Define("Array", arrObj, true)


	// ── sys module ────────────────────────────────────────────────────────
	// Full POSIX syscall wrapper matching Xenly's C modules.c sys module.
	sysObj := Object(map[string]*Value{

		// ── Level 1: Process Control ────────────────────────────────────
		"exit": builtin(func(a []*Value) (*Value, error) {
			code := 0
			if len(a) >= 1 { code = int(a[0].NumVal) }
			os.Exit(code); return Null(), nil
		}),
		"abort": builtin(func(a []*Value) (*Value, error) {
			syscall.Kill(os.Getpid(), syscall.SIGABRT)
			return Null(), nil
		}),
		"getpid": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(os.Getpid())), nil
		}),
		"getppid": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(os.Getppid())), nil
		}),
		"fork": builtin(func(a []*Value) (*Value, error) {
			pid, err := sysFork()
			if err != nil { return Number(-1), nil }
			return Number(float64(pid)), nil
		}),
		"exec": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			path := a[0].String()
			argv := []string{path}
			if len(a) >= 2 && a[1].Tag == TypeArray {
				for _, v := range a[1].ArrayVal { argv = append(argv, v.String()) }
			}
			err := syscall.Exec(path, argv, os.Environ())
			if err != nil { return Number(float64(-1)), nil }
			return Null(), nil
		}),
		"wait": builtin(func(a []*Value) (*Value, error) {
			var ws syscall.WaitStatus
			pid, err := syscall.Wait4(-1, &ws, 0, nil)
			if err != nil { return Number(-1), nil }
			return Number(float64(pid)), nil
		}),
		"waitpid": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			var ws syscall.WaitStatus
			pid, err := syscall.Wait4(int(a[0].NumVal), &ws, 0, nil)
			if err != nil { return Number(-1), nil }
			return Number(float64(pid)), nil
		}),
		"kill": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			err := syscall.Kill(int(a[0].NumVal), syscall.Signal(int(a[1].NumVal)))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"raise": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			err := syscall.Kill(os.Getpid(), syscall.Signal(int(a[0].NumVal)))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"getuid":  builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.Getuid())), nil }),
		"getgid":  builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.Getgid())), nil }),
		"geteuid": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.Geteuid())), nil }),
		"getegid": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.Getegid())), nil }),
		"getrlimit": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			soft, hard, ok := sysGetrlimit(int(a[0].NumVal))
			if !ok { return Null(), nil }
			return Array([]*Value{Number(soft), Number(hard)}), nil
		}),
		"setrlimit": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 { return Number(-1), nil }
			return Number(float64(sysSetrlimit(int(a[0].NumVal), a[1].NumVal, a[2].NumVal))), nil
		}),

		// ── Level 2: File Descriptor I/O ───────────────────────────────
		"open": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			flags := syscall.O_RDONLY
			if len(a) >= 2 { flags = int(a[1].NumVal) }
			mode := uint32(0644)
			if len(a) >= 3 { mode = uint32(a[2].NumVal) }
			fd, err := syscall.Open(a[0].String(), flags, mode)
			if err != nil { return Number(-1), nil }
			return Number(float64(fd)), nil
		}),
		"close": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			err := syscall.Close(int(a[0].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"read": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Null(), nil }
			count := int(a[1].NumVal)
			if count <= 0 || count > 67108864 { return Null(), nil }
			buf := make([]byte, count)
			n, err := syscall.Read(int(a[0].NumVal), buf)
			if err != nil || n < 0 { return Null(), nil }
			return String(string(buf[:n])), nil
		}),
		"write": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			fd := int(a[0].NumVal)
			data := []byte(a[1].String())
			n, err := syscall.Write(fd, data)
			if err != nil { return Number(-1), nil }
			return Number(float64(n)), nil
		}),
		"seek": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 { return Number(-1), nil }
			off, err := syscall.Seek(int(a[0].NumVal), int64(a[1].NumVal), int(a[2].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(float64(off)), nil
		}),
		"dup": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			fd, err := syscall.Dup(int(a[0].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(float64(fd)), nil
		}),
		"dup2": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			err := syscall.Dup2(int(a[0].NumVal), int(a[1].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"pipe": builtin(func(a []*Value) (*Value, error) {
			fds := [2]int{}
			if err := syscall.Pipe(fds[:]); err != nil { return Null(), nil }
			return Array([]*Value{Number(float64(fds[0])), Number(float64(fds[1]))}), nil
		}),
		"fsync": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			err := syscall.Fsync(int(a[0].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"fdatasync": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			err := sysFdatasync(int(a[0].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"truncate": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			err := syscall.Truncate(a[0].String(), int64(a[1].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"ftruncate": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			err := syscall.Ftruncate(int(a[0].NumVal), int64(a[1].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"mmap": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			size := int(a[0].NumVal)
			prot := syscall.PROT_READ | syscall.PROT_WRITE
			if len(a) >= 2 { prot = int(a[1].NumVal) }
			flags := syscall.MAP_PRIVATE | sysMAP_ANON
			if len(a) >= 3 { flags = int(a[2].NumVal) }
			fd := -1
			if len(a) >= 4 { fd = int(a[3].NumVal) }
			off := int64(0)
			if len(a) >= 5 { off = int64(a[4].NumVal) }
			ptr, err := sysMmap(size, prot, flags, fd, off)
			if err != nil { return Number(-1), nil }
			return Number(float64(ptr)), nil
		}),
		"munmap": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			return Number(float64(sysMunmap(uintptr(a[0].NumVal), int(a[1].NumVal)))), nil
		}),
		"mmap_read": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 { return Null(), nil }
			base := uintptr(a[0].NumVal)
			offset := int(a[1].NumVal)
			count := int(a[2].NumVal)
			if count <= 0 || count > 67108864 { return Null(), nil }
			src := unsafe.Slice((*byte)(unsafe.Pointer(base+uintptr(offset))), count)
			return String(string(src)), nil
		}),
		"mmap_write": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 { return Number(-1), nil }
			base := uintptr(a[0].NumVal)
			offset := int(a[1].NumVal)
			data := []byte(a[2].String())
			dst := unsafe.Slice((*byte)(unsafe.Pointer(base+uintptr(offset))), len(data))
			copy(dst, data)
			return Number(float64(len(data))), nil
		}),
		"poll": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 || a[0].Tag != TypeArray { return Number(-1), nil }
			timeout := -1
			if len(a) >= 2 { timeout = int(a[1].NumVal) }
			fds := make([]sysPollFd, len(a[0].ArrayVal))
			for i, pair := range a[0].ArrayVal {
				if pair.Tag == TypeArray && len(pair.ArrayVal) >= 2 {
					fds[i].Fd = int32(pair.ArrayVal[0].NumVal)
					fds[i].Events = int16(pair.ArrayVal[1].NumVal)
				}
			}
			n, err := sysPoll(fds, timeout)
			if err != nil { return Number(-1), nil }
			return Number(float64(n)), nil
		}),
		"fcntl_lock": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			return Number(float64(sysFcntlLock(int(a[0].NumVal), int(a[1].NumVal)))), nil
		}),
		"fcntl_setfl": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			return Number(float64(sysFcntlSetfl(int(a[0].NumVal), int(a[1].NumVal)))), nil
		}),
		"fcntl_getfl": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			return Number(float64(sysFcntlGetfl(int(a[0].NumVal)))), nil
		}),

		// ── Level 3: Filesystem ─────────────────────────────────────────
		"stat": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			var st syscall.Stat_t
			if err := syscall.Stat(a[0].String(), &st); err != nil { return Null(), nil }
			return Array([]*Value{
				Number(float64(st.Size)), Number(float64(st.Mode)),
				Number(float64(st.Uid)), Number(float64(st.Gid)),
				Number(sysStatMtime(st)), Number(sysStatAtime(st)),
				Number(float64(st.Nlink)),
			}), nil
		}),
		"lstat": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			var st syscall.Stat_t
			if err := syscall.Lstat(a[0].String(), &st); err != nil { return Null(), nil }
			return Array([]*Value{
				Number(float64(st.Size)), Number(float64(st.Mode)),
				Number(float64(st.Uid)), Number(float64(st.Gid)),
				Number(sysStatMtime(st)), Number(sysStatAtime(st)),
				Number(float64(st.Nlink)),
			}), nil
		}),
		"isfile": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return False(), nil }
			var st syscall.Stat_t
			if err := syscall.Stat(a[0].String(), &st); err != nil { return False(), nil }
			return Bool(st.Mode&syscall.S_IFMT == syscall.S_IFREG), nil
		}),
		"isdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return False(), nil }
			var st syscall.Stat_t
			if err := syscall.Stat(a[0].String(), &st); err != nil { return False(), nil }
			return Bool(st.Mode&syscall.S_IFMT == syscall.S_IFDIR), nil
		}),
		"islnk": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return False(), nil }
			var st syscall.Stat_t
			if err := syscall.Lstat(a[0].String(), &st); err != nil { return False(), nil }
			return Bool(st.Mode&syscall.S_IFMT == syscall.S_IFLNK), nil
		}),
		"access": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			err := syscall.Access(a[0].String(), uint32(a[1].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"mkdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			mode := uint32(0755)
			if len(a) >= 2 { mode = uint32(a[1].NumVal) }
			err := syscall.Mkdir(a[0].String(), mode)
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"rmdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			err := syscall.Rmdir(a[0].String())
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"unlink": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			err := syscall.Unlink(a[0].String())
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"rename": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			err := syscall.Rename(a[0].String(), a[1].String())
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"link": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			err := syscall.Link(a[0].String(), a[1].String())
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"symlink": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			err := syscall.Symlink(a[0].String(), a[1].String())
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"readlink": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			buf := make([]byte, 4096)
			n, err := syscall.Readlink(a[0].String(), buf)
			if err != nil { return Null(), nil }
			return String(string(buf[:n])), nil
		}),
		"chmod": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			err := syscall.Chmod(a[0].String(), uint32(a[1].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"chown": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 { return Number(-1), nil }
			err := syscall.Chown(a[0].String(), int(a[1].NumVal), int(a[2].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"getcwd": builtin(func(a []*Value) (*Value, error) {
			cwd, err := os.Getwd()
			if err != nil { return Null(), nil }
			return String(cwd), nil
		}),
		"chdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			err := syscall.Chdir(a[0].String())
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"mkfifo": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			mode := uint32(0644)
			if len(a) >= 2 { mode = uint32(a[1].NumVal) }
			err := syscall.Mkfifo(a[0].String(), mode)
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),

		// ── Level 4: Networking ─────────────────────────────────────────
		"socket": builtin(func(a []*Value) (*Value, error) {
			domain := syscall.AF_INET
			if len(a) >= 1 { domain = int(a[0].NumVal) }
			typ := syscall.SOCK_STREAM
			if len(a) >= 2 { typ = int(a[1].NumVal) }
			proto := 0
			if len(a) >= 3 { proto = int(a[2].NumVal) }
			fd, err := syscall.Socket(domain, typ, proto)
			if err != nil { return Number(-1), nil }
			return Number(float64(fd)), nil
		}),
		"connect": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 { return Number(-1), nil }
			ip := net.ParseIP(a[1].String()).To4()
			if ip == nil { return Number(-1), nil }
			addr := &syscall.SockaddrInet4{Port: int(a[2].NumVal)}
			copy(addr.Addr[:], ip)
			err := syscall.Connect(int(a[0].NumVal), addr)
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"bind": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 { return Number(-1), nil }
			fd := int(a[0].NumVal)
			addr := &syscall.SockaddrInet4{Port: int(a[2].NumVal)}
			if len(a) >= 2 && a[1].String() != "" && a[1].String() != "0.0.0.0" {
				ip := net.ParseIP(a[1].String()).To4()
				if ip != nil { copy(addr.Addr[:], ip) }
			}
			opt := 1
			syscall.SetsockoptInt(fd, syscall.SOL_SOCKET, syscall.SO_REUSEADDR, opt)
			err := syscall.Bind(fd, addr)
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"listen": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			err := syscall.Listen(int(a[0].NumVal), int(a[1].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"accept": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			nfd, sa, err := syscall.Accept(int(a[0].NumVal))
			if err != nil { return Null(), nil }
			ip := ""
			port := 0
			if s4, ok := sa.(*syscall.SockaddrInet4); ok {
				ip = fmt.Sprintf("%d.%d.%d.%d", s4.Addr[0], s4.Addr[1], s4.Addr[2], s4.Addr[3])
				port = s4.Port
			}
			return Array([]*Value{Number(float64(nfd)), String(ip), Number(float64(port))}), nil
		}),
		"send": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			flags := 0
			if len(a) >= 3 { flags = int(a[2].NumVal) }
			data := []byte(a[1].String())
			err := syscall.Sendto(int(a[0].NumVal), data, flags, nil)
			if err != nil {
				n, err2 := syscall.Write(int(a[0].NumVal), data)
				if err2 != nil { return Number(-1), nil }
				return Number(float64(n)), nil
			}
			return Number(float64(len(data))), nil
		}),
		"recv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Null(), nil }
			count := int(a[1].NumVal)
			if count <= 0 || count > 67108864 { return Null(), nil }
			buf := make([]byte, count)
			n, _, err := syscall.Recvfrom(int(a[0].NumVal), buf, 0)
			if err != nil { return Null(), nil }
			return String(string(buf[:n])), nil
		}),
		"setsockopt": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 4 { return Number(-1), nil }
			err := syscall.SetsockoptInt(int(a[0].NumVal), int(a[1].NumVal), int(a[2].NumVal), int(a[3].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"inet_aton": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			ip := net.ParseIP(a[0].String()).To4()
			if ip == nil { return Number(-1), nil }
			return Number(float64(binary.BigEndian.Uint32(ip))), nil
		}),
		"inet_ntoa": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			n := uint32(a[0].NumVal)
			ip := make(net.IP, 4)
			binary.BigEndian.PutUint32(ip, n)
			return String(ip.String()), nil
		}),
		"htons": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			v := uint16(a[0].NumVal)
			return Number(float64(bits.ReverseBytes16(v))), nil
		}),
		"ntohs": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			v := uint16(a[0].NumVal)
			return Number(float64(bits.ReverseBytes16(v))), nil
		}),
		"htonl": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			v := uint32(a[0].NumVal)
			return Number(float64(bits.ReverseBytes32(v))), nil
		}),
		"ntohl": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			v := uint32(a[0].NumVal)
			return Number(float64(bits.ReverseBytes32(v))), nil
		}),

		// ── Level 5: Time & Clocks ──────────────────────────────────────
		"clock_realtime": builtin(func(a []*Value) (*Value, error) {
			sec, nsec := sysClockRealtime()
			return Array([]*Value{Number(float64(sec)), Number(float64(nsec))}), nil
		}),
		"clock_monotonic": builtin(func(a []*Value) (*Value, error) {
			sec, nsec := sysClockMonotonic()
			return Array([]*Value{Number(float64(sec)), Number(float64(nsec))}), nil
		}),
		"clock_process": builtin(func(a []*Value) (*Value, error) {
			sec, nsec := sysClockProcess()
			return Array([]*Value{Number(float64(sec)), Number(float64(nsec))}), nil
		}),
		"nanosleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			return Number(float64(sysNanosleep(int64(a[0].NumVal), int64(a[1].NumVal)))), nil
		}),
		"time": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(time.Now().Unix())), nil
		}),
		"clock": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(time.Now().UnixNano()) / 1e9), nil
		}),
		"gettimeofday": builtin(func(a []*Value) (*Value, error) {
			sec, usec := sysGettimeofday()
			return Array([]*Value{Number(float64(sec)), Number(float64(usec))}), nil
		}),
		"sleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			time.Sleep(time.Duration(a[0].NumVal) * time.Second)
			return Number(0), nil
		}),
		"usleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			time.Sleep(time.Duration(a[0].NumVal) * time.Microsecond)
			return Number(0), nil
		}),

		// ── Level 6: System Info & Environment ─────────────────────────
		"uname": builtin(func(a []*Value) (*Value, error) {
			fields := sysUname()
			result := make([]*Value, len(fields))
			for i, f := range fields { result[i] = String(f) }
			return Array(result), nil
		}),
		"hostname": builtin(func(a []*Value) (*Value, error) {
			h, err := os.Hostname()
			if err != nil { return Null(), nil }
			return String(h), nil
		}),
		"nproc": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(runtime.NumCPU())), nil
		}),
		"nproc_conf": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(runtime.NumCPU())), nil
		}),
		"phys_pages": builtin(func(a []*Value) (*Value, error) {
			total, _, err := sysSysinfo()
			if err != nil { return Number(0), nil }
			return Number(float64(total)), nil
		}),
		"avphys_pages": builtin(func(a []*Value) (*Value, error) {
			_, free, err := sysSysinfo()
			if err != nil { return Number(0), nil }
			return Number(float64(free)), nil
		}),
		"PAGE_SIZE": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(syscall.Getpagesize())), nil
		}),
		"endian": builtin(func(a []*Value) (*Value, error) {
			t := uint16(0x0001)
			if *(*uint8)(unsafe.Pointer(&t)) == 1 { return String("little"), nil }
			return String("big"), nil
		}),
		"sizeof_ptr": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(unsafe.Sizeof(uintptr(0)))), nil
		}),
		"getenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			v, ok := syscall.Getenv(a[0].String())
			if !ok { return Null(), nil }
			return String(v), nil
		}),
		"setenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			err := os.Setenv(a[0].String(), a[1].String())
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"unsetenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			err := os.Unsetenv(a[0].String())
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"errno": builtin(func(a []*Value) (*Value, error) {
			return Number(0), nil // No global errno in Go; return 0
		}),
		"strerror": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return String("Unknown error"), nil }
			return String(syscall.Errno(int(a[0].NumVal)).Error()), nil
		}),
		"system": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			cmd := exec.Command("sh", "-c", a[0].String())
			cmd.Stdout = os.Stdout
			cmd.Stderr = os.Stderr
			err := cmd.Run()
			if err != nil {
				if exitErr, ok := err.(*exec.ExitError); ok {
					return Number(float64(exitErr.ExitCode())), nil
				}
				return Number(-1), nil
			}
			return Number(0), nil
		}),
		"proc_status": builtin(func(a []*Value) (*Value, error) {
			b, err := os.ReadFile("/proc/self/status")
			if err != nil { return Null(), nil }
			return String(string(b)), nil
		}),
		"proc_maps": builtin(func(a []*Value) (*Value, error) {
			b, err := os.ReadFile("/proc/self/maps")
			if err != nil { return Null(), nil }
			return String(string(b)), nil
		}),

		// ── Level 7: Bit & Memory Operations ───────────────────────────
		"band": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			return Number(float64(int64(uint64(int64(a[0].NumVal)) & uint64(int64(a[1].NumVal))))), nil
		}),
		"bor": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			return Number(float64(int64(uint64(int64(a[0].NumVal)) | uint64(int64(a[1].NumVal))))), nil
		}),
		"bxor": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			return Number(float64(int64(uint64(int64(a[0].NumVal)) ^ uint64(int64(a[1].NumVal))))), nil
		}),
		"bnot": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			return Number(float64(int64(^uint64(int64(a[0].NumVal))))), nil
		}),
		"shl": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			v := uint64(int64(a[0].NumVal)); n := int(a[1].NumVal)
			if n >= 0 { return Number(float64(int64(v << uint(n)))), nil }
			return Number(float64(int64(v >> uint(-n)))), nil
		}),
		"shr": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			v := uint64(int64(a[0].NumVal)); n := int(a[1].NumVal)
			if n >= 0 { return Number(float64(int64(v >> uint(n)))), nil }
			return Number(float64(int64(v << uint(-n)))), nil
		}),
		"sar": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			return Number(float64(int64(a[0].NumVal) >> uint(int(a[1].NumVal)))), nil
		}),
		"rol": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			v := uint32(int64(a[0].NumVal)); n := int(a[1].NumVal) & 31
			return Number(float64(int64(bits.RotateLeft32(v, n)))), nil
		}),
		"ror": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			v := uint32(int64(a[0].NumVal)); n := int(a[1].NumVal) & 31
			return Number(float64(int64(bits.RotateLeft32(v, -n)))), nil
		}),
		"popcnt": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			return Number(float64(bits.OnesCount64(uint64(int64(a[0].NumVal))))), nil
		}),
		"clz": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(64), nil }
			v := uint64(int64(a[0].NumVal))
			if v == 0 { return Number(64), nil }
			return Number(float64(bits.LeadingZeros64(v))), nil
		}),
		"ctz": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(64), nil }
			v := uint64(int64(a[0].NumVal))
			if v == 0 { return Number(64), nil }
			return Number(float64(bits.TrailingZeros64(v))), nil
		}),
		"bswap32": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			return Number(float64(int64(int32(bits.ReverseBytes32(uint32(int64(a[0].NumVal))))))), nil
		}),
		"bswap64": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			return Number(float64(int64(bits.ReverseBytes64(uint64(int64(a[0].NumVal)))))), nil
		}),
		"parity": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			return Number(float64(bits.OnesCount64(uint64(int64(a[0].NumVal))) & 1)), nil
		}),
		"bit_get": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			v := uint64(int64(a[0].NumVal)); n := uint(a[1].NumVal)
			return Number(float64((v >> n) & 1)), nil
		}),
		"bit_set": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			v := uint64(int64(a[0].NumVal)); n := uint(a[1].NumVal)
			return Number(float64(int64(v | (1 << n)))), nil
		}),
		"bit_clear": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			v := uint64(int64(a[0].NumVal)); n := uint(a[1].NumVal)
			return Number(float64(int64(v & ^(uint64(1) << n)))), nil
		}),
		"bit_toggle": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			v := uint64(int64(a[0].NumVal)); n := uint(a[1].NumVal)
			return Number(float64(int64(v ^ (1 << n)))), nil
		}),
		"align_up": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			n := uint64(int64(a[0].NumVal)); al := uint64(int64(a[1].NumVal))
			if al == 0 { return Number(float64(int64(n))), nil }
			return Number(float64(int64((n + al - 1) & ^(al - 1)))), nil
		}),
		"align_down": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			n := uint64(int64(a[0].NumVal)); al := uint64(int64(a[1].NumVal))
			if al == 0 { return Number(float64(int64(n))), nil }
			return Number(float64(int64(n & ^(al - 1)))), nil
		}),
		"is_pow2": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return False(), nil }
			n := uint64(int64(a[0].NumVal))
			return Bool(n > 0 && (n&(n-1)) == 0), nil
		}),
		"next_pow2": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(1), nil }
			n := uint64(int64(a[0].NumVal))
			if n == 0 { return Number(1), nil }
			return Number(float64(int64(uint64(1) << bits.Len64(n-1)))), nil
		}),

		// ── Level 8: IPC & System Logging (stubs on Go) ────────────────
		"openlog":  builtin(func(a []*Value) (*Value, error) { return Null(), nil }),
		"syslog":   builtin(func(a []*Value) (*Value, error) { return Null(), nil }),
		"closelog": builtin(func(a []*Value) (*Value, error) { return Null(), nil }),

		// ── Level 9: File Open Flags ────────────────────────────────────
		"O_RDONLY":  builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_RDONLY)), nil }),
		"O_WRONLY":  builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_WRONLY)), nil }),
		"O_RDWR":    builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_RDWR)), nil }),
		"O_CREAT":   builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_CREAT)), nil }),
		"O_TRUNC":   builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_TRUNC)), nil }),
		"O_APPEND":  builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_APPEND)), nil }),
		"O_NONBLOCK": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_NONBLOCK)), nil }),
		"O_SYNC":    builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_SYNC)), nil }),
		"O_EXCL":    builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_EXCL)), nil }),

		// ── Signal constants ────────────────────────────────────────────
		"SIGHUP":  builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SIGHUP)), nil }),
		"SIGINT":  builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SIGINT)), nil }),
		"SIGQUIT": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SIGQUIT)), nil }),
		"SIGKILL": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SIGKILL)), nil }),
		"SIGTERM": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SIGTERM)), nil }),
		"SIGUSR1": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SIGUSR1)), nil }),
		"SIGUSR2": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SIGUSR2)), nil }),
		"SIGCHLD": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SIGCHLD)), nil }),
		"SIGPIPE": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SIGPIPE)), nil }),
		"SIGALRM": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SIGALRM)), nil }),
		"SIGSEGV": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SIGSEGV)), nil }),

		// ── Syslog priority constants ───────────────────────────────────
		"LOG_EMERG":   builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"LOG_ALERT":   builtin(func(a []*Value) (*Value, error) { return Number(1), nil }),
		"LOG_CRIT":    builtin(func(a []*Value) (*Value, error) { return Number(2), nil }),
		"LOG_ERR":     builtin(func(a []*Value) (*Value, error) { return Number(3), nil }),
		"LOG_WARNING": builtin(func(a []*Value) (*Value, error) { return Number(4), nil }),
		"LOG_NOTICE":  builtin(func(a []*Value) (*Value, error) { return Number(5), nil }),
		"LOG_INFO":    builtin(func(a []*Value) (*Value, error) { return Number(6), nil }),
		"LOG_DEBUG":   builtin(func(a []*Value) (*Value, error) { return Number(7), nil }),

		// ── Poll event flags ────────────────────────────────────────────
		"POLLIN":  builtin(func(a []*Value) (*Value, error) { return Number(float64(sysPOLLIN)), nil }),
		"POLLOUT": builtin(func(a []*Value) (*Value, error) { return Number(float64(sysPOLLOUT)), nil }),
		"POLLERR": builtin(func(a []*Value) (*Value, error) { return Number(float64(sysPOLLERR)), nil }),
		"POLLHUP": builtin(func(a []*Value) (*Value, error) { return Number(float64(sysPOLLHUP)), nil }),

		// ── mmap prot/flags constants ───────────────────────────────────
		"PROT_READ":     builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.PROT_READ)), nil }),
		"PROT_WRITE":    builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.PROT_WRITE)), nil }),
		"PROT_EXEC":     builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.PROT_EXEC)), nil }),
		"PROT_NONE":     builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.PROT_NONE)), nil }),
		"MAP_SHARED":    builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.MAP_SHARED)), nil }),
		"MAP_PRIVATE":   builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.MAP_PRIVATE)), nil }),
		"MAP_ANONYMOUS": builtin(func(a []*Value) (*Value, error) { return Number(float64(sysMAP_ANON)), nil }),

		// ── Socket domain/type constants ────────────────────────────────
		"AF_INET":     builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.AF_INET)), nil }),
		"AF_INET6":    builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.AF_INET6)), nil }),
		"AF_UNIX":     builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.AF_UNIX)), nil }),
		"SOCK_STREAM": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SOCK_STREAM)), nil }),
		"SOCK_DGRAM":  builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SOCK_DGRAM)), nil }),

		// ── Resource limit constants ─────────────────────────────────────
		"RLIMIT_CPU":    builtin(func(a []*Value) (*Value, error) { return Number(float64(sysRLIMIT_CPU)), nil }),
		"RLIMIT_FSIZE":  builtin(func(a []*Value) (*Value, error) { return Number(float64(sysRLIMIT_FSIZE)), nil }),
		"RLIMIT_STACK":  builtin(func(a []*Value) (*Value, error) { return Number(float64(sysRLIMIT_STACK)), nil }),
		"RLIMIT_NOFILE": builtin(func(a []*Value) (*Value, error) { return Number(float64(sysRLIMIT_NOFILE)), nil }),
		"RLIMIT_AS":     builtin(func(a []*Value) (*Value, error) { return Number(float64(sysRLIMIT_AS)), nil }),

		// ── Level 10: I/O FD constants & numeric limits ─────────────────
		"STDIN":     builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"STDOUT":    builtin(func(a []*Value) (*Value, error) { return Number(1), nil }),
		"STDERR":    builtin(func(a []*Value) (*Value, error) { return Number(2), nil }),
		"INT_MAX":   builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxInt32)), nil }),
		"INT_MIN":   builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MinInt32)), nil }),
		"UINT_MAX":  builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxUint32)), nil }),
		"LONG_MAX":  builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxInt64)), nil }),
		"INT8_MAX":  builtin(func(a []*Value) (*Value, error) { return Number(127), nil }),
		"INT8_MIN":  builtin(func(a []*Value) (*Value, error) { return Number(-128), nil }),
		"INT16_MAX": builtin(func(a []*Value) (*Value, error) { return Number(32767), nil }),
		"INT16_MIN": builtin(func(a []*Value) (*Value, error) { return Number(-32768), nil }),
		"INT32_MAX": builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxInt32)), nil }),
		"INT32_MIN": builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MinInt32)), nil }),
		"UINT8_MAX":  builtin(func(a []*Value) (*Value, error) { return Number(255), nil }),
		"UINT16_MAX": builtin(func(a []*Value) (*Value, error) { return Number(65535), nil }),
		"UINT32_MAX": builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxUint32)), nil }),
	})
	env.Define("sys", sysObj, true)

	// ── json module ───────────────────────────────────────────────────────
	jsonObj := Object(map[string]*Value{
		"stringify": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String("null"), nil
			}
			return String(jsonStringify(a[0])), nil
		}),
		"parse": builtin(func(a []*Value) (*Value, error) {
			// Minimal JSON parser: delegate to string representation for now
			return String(a[0].String()), nil
		}),
	})
	env.Define("json", jsonObj, true)
	env.Define("JSON", jsonObj, true)

	// ── time module ───────────────────────────────────────────────────────
	timeObj := Object(map[string]*Value{
		"now": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(time.Now().UnixMilli())), nil
		}),
		"sleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) > 0 {
				time.Sleep(time.Duration(a[0].NumVal) * time.Millisecond)
			}
			return Null(), nil
		}),
		"format": builtin(func(a []*Value) (*Value, error) {
			return String(time.Now().Format("2006-01-02 15:04:05")), nil
		}),
	})
	env.Define("time", timeObj, true)
	env.Define("Time", timeObj, true)

	// ── number parsing ────────────────────────────────────────────────────
	native(env, "parseInt", func(args []*Value) (*Value, error) {
		if len(args) == 0 {
			return Number(math.NaN()), nil
		}
		base := 10
		if len(args) > 1 {
			base = int(args[1].NumVal)
		}
		n, err := strconv.ParseInt(strings.TrimSpace(args[0].String()), base, 64)
		if err != nil {
			return Number(math.NaN()), nil
		}
		return Number(float64(n)), nil
	})
	native(env, "parseFloat", func(args []*Value) (*Value, error) {
		if len(args) == 0 {
			return Number(math.NaN()), nil
		}
		n, err := strconv.ParseFloat(strings.TrimSpace(args[0].String()), 64)
		if err != nil {
			return Number(math.NaN()), nil
		}
		return Number(n), nil
	})
	native(env, "isNaN", func(args []*Value) (*Value, error) {
		if len(args) == 0 {
			return True(), nil
		}
		return Bool(math.IsNaN(args[0].NumVal)), nil
	})
	native(env, "isFinite", func(args []*Value) (*Value, error) {
		if len(args) == 0 {
			return False(), nil
		}
		v := args[0].NumVal
		return Bool(!math.IsInf(v, 0) && !math.IsNaN(v)), nil
	})

	// Seed random
	rand.Seed(time.Now().UnixNano())

	// ── array module (Xenly stdlib functional API) ───────────────────────
	arrayObj := Object(map[string]*Value{
		// array.of(a, b, c, ...) → [a, b, c]
		"of": builtin(func(a []*Value) (*Value, error) {
			items := make([]*Value, len(a))
			copy(items, a)
			return Array(items), nil
		}),
		// array.empty() → []
		"empty": builtin(func(a []*Value) (*Value, error) {
			return Array(nil), nil
		}),
		// array.range(end) | array.range(start, end) | array.range(start, end, step) → array
		"range": builtin(func(a []*Value) (*Value, error) {
			var start, end, step float64 = 0, 0, 1
			switch len(a) {
			case 0:
				return Array(nil), nil
			case 1:
				end = a[0].NumVal
			case 2:
				start, end = a[0].NumVal, a[1].NumVal
			default:
				start, end, step = a[0].NumVal, a[1].NumVal, a[2].NumVal
			}
			if step == 0 {
				return Array(nil), nil
			}
			var items []*Value
			if step > 0 {
				for v := start; v < end; v += step {
					items = append(items, Number(v))
				}
			} else {
				for v := start; v > end; v += step {
					items = append(items, Number(v))
				}
			}
			return Array(items), nil
		}),
		// array.len(arr) → number
		"len": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Number(0), nil
			}
			return Number(float64(len(a[0].ArrayVal))), nil
		}),
		// array.get(arr, idx) → value
		"get": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return Null(), nil
			}
			idx := int(a[1].NumVal)
			arr := a[0].ArrayVal
			if idx < 0 {
				idx = len(arr) + idx
			}
			if idx < 0 || idx >= len(arr) {
				return Null(), nil
			}
			return arr[idx], nil
		}),
		// array.set(arr, idx, val) → arr  (mutates in-place)
		"set": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 || a[0].Tag != TypeArray {
				return Null(), nil
			}
			idx := int(a[1].NumVal)
			arr := a[0].ArrayVal
			if idx < 0 {
				idx = len(arr) + idx
			}
			if idx >= 0 && idx < len(arr) {
				arr[idx] = a[2]
			}
			return a[0], nil
		}),
		// array.push(arr, val) → new_length  (mutates in-place)
		"push": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return Number(0), nil
			}
			a[0].ArrayVal = append(a[0].ArrayVal, a[1])
			return Number(float64(len(a[0].ArrayVal))), nil
		}),
		// array.pop(arr) → last element (mutates)
		"pop": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray || len(a[0].ArrayVal) == 0 {
				return Null(), nil
			}
			n := len(a[0].ArrayVal) - 1
			v := a[0].ArrayVal[n]
			a[0].ArrayVal = a[0].ArrayVal[:n]
			return v, nil
		}),
		// array.shift(arr) → first element (mutates)
		"shift": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray || len(a[0].ArrayVal) == 0 {
				return Null(), nil
			}
			v := a[0].ArrayVal[0]
			a[0].ArrayVal = a[0].ArrayVal[1:]
			return v, nil
		}),
		// array.concat(a, b) → new array
		"concat": builtin(func(a []*Value) (*Value, error) {
			var items []*Value
			for _, arr := range a {
				if arr.Tag == TypeArray {
					items = append(items, arr.ArrayVal...)
				}
			}
			return Array(items), nil
		}),
		// array.slice(arr, start, end?) → new array
		"slice": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Array(nil), nil
			}
			arr := a[0].ArrayVal
			start, end := 0, len(arr)
			if len(a) > 1 {
				start = int(a[1].NumVal)
			}
			if len(a) > 2 {
				end = int(a[2].NumVal)
			}
			if start < 0 {
				start = len(arr) + start
			}
			if end < 0 {
				end = len(arr) + end
			}
			if start < 0 {
				start = 0
			}
			if end > len(arr) {
				end = len(arr)
			}
			result := make([]*Value, end-start)
			copy(result, arr[start:end])
			return Array(result), nil
		}),
		// array.join(arr, sep?) → string
		"join": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return String(""), nil
			}
			sep := ","
			if len(a) > 1 {
				sep = a[1].String()
			}
			parts := make([]string, len(a[0].ArrayVal))
			for i, v := range a[0].ArrayVal {
				parts[i] = v.String()
			}
			return String(strings.Join(parts, sep)), nil
		}),
		// array.reverse(arr) → arr (mutates)
		"reverse": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Null(), nil
			}
			arr := a[0].ArrayVal
			for i, j := 0, len(arr)-1; i < j; i, j = i+1, j-1 {
				arr[i], arr[j] = arr[j], arr[i]
			}
			return a[0], nil
		}),
		// array.contains(arr, val) → bool
		"contains": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return False(), nil
			}
			for _, v := range a[0].ArrayVal {
				if v.Equal(a[1]) {
					return True(), nil
				}
			}
			return False(), nil
		}),
		// array.indexOf(arr, val) → number
		"indexOf": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return Number(-1), nil
			}
			for i, v := range a[0].ArrayVal {
				if v.Equal(a[1]) {
					return Number(float64(i)), nil
				}
			}
			return Number(-1), nil
		}),
		// array.sum(arr) → number
		"sum": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Number(0), nil
			}
			var s float64
			for _, v := range a[0].ArrayVal {
				if v.Tag == TypeNumber {
					s += v.NumVal
				}
			}
			return Number(s), nil
		}),
		// array.isArray(val) → bool
		"isArray": builtin(func(a []*Value) (*Value, error) {
			return Bool(len(a) > 0 && a[0].Tag == TypeArray), nil
		}),
		// array.create(len, fill?) → new array of length n
		"create": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Array(nil), nil
			}
			n := int(a[0].NumVal)
			fill := Null()
			if len(a) > 1 {
				fill = a[1]
			}
			items := make([]*Value, n)
			for i := range items {
				items[i] = fill
			}
			return Array(items), nil
		}),
	})
	env.Define("array", arrayObj, true)

	// ── io module ─────────────────────────────────────────────────────────
	ioObj := Object(map[string]*Value{
		// io.write(v, ...) — print without newline
		"write": builtin(func(a []*Value) (*Value, error) {
			for _, v := range a {
				fmt.Print(v.String())
			}
			return Null(), nil
		}),
		// io.writeln(v?, ...) — print with newline
		"writeln": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				fmt.Println()
			} else {
				parts := make([]string, len(a))
				for i, v := range a {
					parts[i] = v.String()
				}
				fmt.Println(strings.Join(parts, " "))
			}
			return Null(), nil
		}),
		// io.readln(prompt?) — read a line from stdin
		"readln": builtin(func(a []*Value) (*Value, error) {
			if len(a) > 0 {
				fmt.Print(a[0].String())
			}
			var line string
			fmt.Scanln(&line)
			return String(line), nil
		}),
		// io.print(v, ...) — same as print statement
		"print": builtin(func(a []*Value) (*Value, error) {
			parts := make([]string, len(a))
			for i, v := range a {
				parts[i] = v.String()
			}
			fmt.Println(strings.Join(parts, " "))
			return Null(), nil
		}),
	})
	env.Define("io", ioObj, true)

	// ── string module ─────────────────────────────────────────────────────
	stringObj := Object(map[string]*Value{
		"len": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return Number(0), nil }
			return Number(float64(len([]rune(a[0].String())))), nil
		}),
		"toString": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return String(""), nil }
			return String(a[0].String()), nil
		}),
		"toNumber": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return Number(0), nil }
			v, err := strconv.ParseFloat(strings.TrimSpace(a[0].String()), 64)
			if err != nil { return Number(0), nil }
			return Number(v), nil
		}),
		"upper": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return String(""), nil }
			return String(strings.ToUpper(a[0].String())), nil
		}),
		"lower": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return String(""), nil }
			return String(strings.ToLower(a[0].String())), nil
		}),
		"contains": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return False(), nil }
			return Bool(strings.Contains(a[0].String(), a[1].String())), nil
		}),
		"startsWith": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return False(), nil }
			return Bool(strings.HasPrefix(a[0].String(), a[1].String())), nil
		}),
		"endsWith": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return False(), nil }
			return Bool(strings.HasSuffix(a[0].String(), a[1].String())), nil
		}),
		"indexOf": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			return Number(float64(strings.Index(a[0].String(), a[1].String()))), nil
		}),
		"lastIndexOf": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			return Number(float64(strings.LastIndex(a[0].String(), a[1].String()))), nil
		}),
		"charAt": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return String(""), nil }
			runes := []rune(a[0].String())
			idx := int(a[1].NumVal)
			if idx < 0 { idx = len(runes) + idx }
			if idx < 0 || idx >= len(runes) { return String(""), nil }
			return String(string(runes[idx])), nil
		}),
		"charCodeAt": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			runes := []rune(a[0].String())
			idx := int(a[1].NumVal)
			if idx < 0 { idx = len(runes) + idx }
			if idx < 0 || idx >= len(runes) { return Number(math.NaN()), nil }
			return Number(float64(runes[idx])), nil
		}),
		"fromCharCode": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return String(""), nil }
			return String(string(rune(int(a[0].NumVal)))), nil
		}),
		"repeat": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return String(""), nil }
			return String(strings.Repeat(a[0].String(), int(a[1].NumVal))), nil
		}),
		"reverse": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return String(""), nil }
			runes := []rune(a[0].String())
			for i, j := 0, len(runes)-1; i < j; i, j = i+1, j-1 {
				runes[i], runes[j] = runes[j], runes[i]
			}
			return String(string(runes)), nil
		}),
		"trim": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return String(""), nil }
			return String(strings.TrimSpace(a[0].String())), nil
		}),
		"trimStart": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return String(""), nil }
			return String(strings.TrimLeftFunc(a[0].String(), func(r rune) bool { return r == ' ' || r == '\t' || r == '\n' || r == '\r' })), nil
		}),
		"trimEnd": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return String(""), nil }
			return String(strings.TrimRightFunc(a[0].String(), func(r rune) bool { return r == ' ' || r == '\t' || r == '\n' || r == '\r' })), nil
		}),
		"replace": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 { return safeArg(a, 0), nil }
			return String(strings.Replace(a[0].String(), a[1].String(), a[2].String(), 1)), nil
		}),
		"replaceAll": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 { return safeArg(a, 0), nil }
			return String(strings.ReplaceAll(a[0].String(), a[1].String(), a[2].String())), nil
		}),
		"substr": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return String(""), nil }
			runes := []rune(a[0].String())
			start := int(a[1].NumVal)
			if start < 0 { start = 0 }
			end := len(runes)
			if len(a) >= 3 { end = start + int(a[2].NumVal) }
			if end > len(runes) { end = len(runes) }
			if start >= end { return String(""), nil }
			return String(string(runes[start:end])), nil
		}),
		"slice": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return String(""), nil }
			runes := []rune(a[0].String())
			start := int(a[1].NumVal)
			end := len(runes)
			if len(a) >= 3 { end = int(a[2].NumVal) }
			if start < 0 { start = len(runes) + start }
			if end < 0 { end = len(runes) + end }
			if start < 0 { start = 0 }
			if end > len(runes) { end = len(runes) }
			if start >= end { return String(""), nil }
			return String(string(runes[start:end])), nil
		}),
		"padStart": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return safeArg(a, 0), nil }
			s := a[0].String()
			n := int(a[1].NumVal)
			pad := " "
			if len(a) >= 3 { pad = a[2].String() }
			for len([]rune(s)) < n { s = pad + s }
			runes := []rune(s)
			if len(runes) > n { s = string(runes[len(runes)-n:]) }
			return String(s), nil
		}),
		"padEnd": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return safeArg(a, 0), nil }
			s := a[0].String()
			n := int(a[1].NumVal)
			pad := " "
			if len(a) >= 3 { pad = a[2].String() }
			for len([]rune(s)) < n { s = s + pad }
			runes := []rune(s)
			if len(runes) > n { s = string(runes[:n]) }
			return String(s), nil
		}),
		"split": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Array(nil), nil }
			parts := strings.Split(a[0].String(), a[1].String())
			items := make([]*Value, len(parts))
			for i, p := range parts { items[i] = String(p) }
			return Array(items), nil
		}),
		"join": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return String(""), nil }
			// string.join(arr, sep)
			if a[0].Tag != TypeArray { return String(""), nil }
			sep := ""
			if len(a) > 1 { sep = a[1].String() }
			parts := make([]string, len(a[0].ArrayVal))
			for i, v := range a[0].ArrayVal { parts[i] = v.String() }
			return String(strings.Join(parts, sep)), nil
		}),
		"format": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return String(""), nil }
			// Simple: string.format(fmt, args...) — basic %s/%d/%f substitution
			result := a[0].String()
			for _, arg := range a[1:] {
				if idx := strings.IndexByte(result, '%'); idx >= 0 && idx+1 < len(result) {
					result = result[:idx] + arg.String() + result[idx+2:]
				}
			}
			return String(result), nil
		}),
	})
	env.Define("string", stringObj, true)

	// ── os module (aliases to sys) ─────────────────────────────────────────
	osObj := Object(map[string]*Value{
		"platform": builtin(func(a []*Value) (*Value, error) {
			return String(runtime.GOOS), nil
		}),
		"exit": builtin(func(a []*Value) (*Value, error) {
			code := 0
			if len(a) > 0 { code = int(a[0].NumVal) }
			os.Exit(code)
			return Null(), nil
		}),
		"args": builtin(func(a []*Value) (*Value, error) {
			items := make([]*Value, len(os.Args))
			for i, arg := range os.Args { items[i] = String(arg) }
			return Array(items), nil
		}),
		"env": builtin(func(a []*Value) (*Value, error) {
			if len(a) > 0 {
				return String(os.Getenv(a[0].String())), nil
			}
			return Null(), nil
		}),
		"getenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 { return Null(), nil }
			return String(os.Getenv(a[0].String())), nil
		}),
		"getcwd": builtin(func(a []*Value) (*Value, error) {
			cwd, _ := os.Getwd()
			return String(cwd), nil
		}),
		"pid": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(os.Getpid())), nil
		}),
	})
	env.Define("os", osObj, true)
}

// ─── helpers ──────────────────────────────────────────────────────────────────

func native(env *Env, name string, fn BuiltinFn) {
	env.Define(name, &Value{Tag: TypeBuiltin, BuiltinV: fn}, false)
}

func builtin(fn BuiltinFn) *Value {
	return &Value{Tag: TypeBuiltin, BuiltinV: fn}
}

func safeArg(args []*Value, i int) *Value {
	if i < len(args) {
		return args[i]
	}
	return Null()
}

// ─── core built-ins ───────────────────────────────────────────────────────────

func builtinPrint(args []*Value) (*Value, error) {
	parts := make([]string, len(args))
	for i, a := range args {
		parts[i] = a.String()
	}
	fmt.Println(strings.Join(parts, " "))
	return Null(), nil
}

func builtinInput(args []*Value) (*Value, error) {
	if len(args) > 0 {
		fmt.Print(args[0].String())
	}
	var line string
	fmt.Scanln(&line)
	return String(line), nil
}

func builtinLen(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return Number(0), nil
	}
	a := args[0]
	switch a.Tag {
	case TypeArray:
		return Number(float64(len(a.ArrayVal))), nil
	case TypeString:
		return Number(float64(len([]rune(a.StrVal)))), nil
	case TypeObject:
		return Number(float64(len(a.ObjVal))), nil
	}
	return Number(0), nil
}

func builtinType(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return String("null"), nil
	}
	return String(args[0].TypeName()), nil
}

func builtinStr(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return String(""), nil
	}
	return String(args[0].String()), nil
}

func builtinNum(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return Number(0), nil
	}
	a := args[0]
	switch a.Tag {
	case TypeNumber:
		return a, nil
	case TypeBool:
		if a.BoolVal {
			return Number(1), nil
		}
		return Number(0), nil
	case TypeString:
		n, err := strconv.ParseFloat(strings.TrimSpace(a.StrVal), 64)
		if err != nil {
			return Number(math.NaN()), nil
		}
		return Number(n), nil
	}
	return Number(math.NaN()), nil
}

func builtinBool(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return False(), nil
	}
	return Bool(args[0].Truthy()), nil
}

func builtinRange(args []*Value) (*Value, error) {
	var start, end, step float64
	switch len(args) {
	case 1:
		start, end, step = 0, args[0].NumVal, 1
	case 2:
		start, end, step = args[0].NumVal, args[1].NumVal, 1
	case 3:
		start, end, step = args[0].NumVal, args[1].NumVal, args[2].NumVal
	default:
		return Array(nil), nil
	}
	if step == 0 {
		return Array(nil), fmt.Errorf("range step cannot be zero")
	}
	var items []*Value
	for i := start; (step > 0 && i < end) || (step < 0 && i > end); i += step {
		items = append(items, Number(i))
	}
	return Array(items), nil
}

func builtinSleep(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return Null(), nil
	}
	time.Sleep(time.Duration(args[0].NumVal) * time.Millisecond)
	return Null(), nil
}

func builtinExit(args []*Value) (*Value, error) {
	code := 0
	if len(args) > 0 {
		code = int(args[0].NumVal)
	}
	os.Exit(code)
	return Null(), nil
}

func builtinAssert(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return Null(), nil
	}
	if !args[0].Truthy() {
		msg := "assertion failed"
		if len(args) > 1 {
			msg = args[1].String()
		}
		return Null(), fmt.Errorf("%s", msg)
	}
	return Null(), nil
}

func builtinError(args []*Value) (*Value, error) {
	msg := "error"
	if len(args) > 0 {
		msg = args[0].String()
	}
	return Null(), fmt.Errorf("%s", msg)
}

// ─── JSON stringify ────────────────────────────────────────────────────────────

func jsonStringify(v *Value) string {
	if v == nil || v.Tag == TypeNull {
		return "null"
	}
	switch v.Tag {
	case TypeBool:
		if v.BoolVal {
			return "true"
		}
		return "false"
	case TypeNumber:
		if math.IsNaN(v.NumVal) || math.IsInf(v.NumVal, 0) {
			return "null"
		}
		return strconv.FormatFloat(v.NumVal, 'f', -1, 64)
	case TypeString:
		return strconv.Quote(v.StrVal)
	case TypeArray:
		parts := make([]string, len(v.ArrayVal))
		for i, item := range v.ArrayVal {
			parts[i] = jsonStringify(item)
		}
		return "[" + strings.Join(parts, ",") + "]"
	case TypeObject:
		parts := make([]string, 0, len(v.ObjVal))
		for k, val := range v.ObjVal {
			parts = append(parts, fmt.Sprintf("%s:%s", strconv.Quote(k), jsonStringify(val)))
		}
		return "{" + strings.Join(parts, ",") + "}"
	}
	return "null"
}

func goPlatform() string {
	return runtime.GOOS + "/" + runtime.GOARCH
}
