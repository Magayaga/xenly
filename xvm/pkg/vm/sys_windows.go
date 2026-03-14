//go:build windows

package vm

import (
	"fmt"
	"math"
	"math/bits"
	"os"
	"os/exec"
	"runtime"
	"syscall"
	"time"
	"unsafe"
)

// nosys returns -1 (ENOSYS equivalent) — used for unavailable POSIX calls.
func nosys() (*Value, error) { return Number(-1), nil }

func registerSysModule(env *Env) {
	sysObj := Object(map[string]*Value{

		// ── Level 1: Process Control ──────────────────────────────────
		"exit": builtin(func(a []*Value) (*Value, error) {
			code := 0
			if len(a) >= 1 { code = int(a[0].NumVal) }
			os.Exit(code); return Null(), nil
		}),
		"abort":   builtin(func(a []*Value) (*Value, error) { os.Exit(3); return Null(), nil }),
		"getpid":  builtin(func(a []*Value) (*Value, error) { return Number(float64(os.Getpid())), nil }),
		"getppid": builtin(func(a []*Value) (*Value, error) { return Number(float64(os.Getppid())), nil }),
		"fork":    builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"exec": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return nosys() }
			argv := []string{a[0].String()}
			if len(a) >= 2 && a[1].Tag == TypeArray {
				for _, v := range a[1].ArrayVal { argv = append(argv, v.String()) }
			}
			cmd := exec.Command(argv[0], argv[1:]...)
			cmd.Stdout, cmd.Stderr, cmd.Stdin = os.Stdout, os.Stderr, os.Stdin
			cmd.Run(); os.Exit(0); return Null(), nil
		}),
		"wait":    builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"waitpid": builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"kill":    builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"raise":   builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"getuid":  builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"getgid":  builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"geteuid": builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"getegid": builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"getrlimit": builtin(func(a []*Value) (*Value, error) {
			return Array([]*Value{Number(-1), Number(-1)}), nil
		}),
		"setrlimit": builtin(func(a []*Value) (*Value, error) { return nosys() }),

		// ── Level 2: File Descriptor I/O ──────────────────────────────
		"open": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return nosys() }
			flags := os.O_RDONLY
			if len(a) >= 2 { flags = int(a[1].NumVal) }
			perm := os.FileMode(0644)
			if len(a) >= 3 { perm = os.FileMode(int(a[2].NumVal)) }
			f, err := os.OpenFile(a[0].String(), flags, perm)
			if err != nil { return Number(-1), nil }
			return Number(float64(f.Fd())), nil
		}),
		"close": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return nosys() }
			h := syscall.Handle(uintptr(int(a[0].NumVal)))
			if err := syscall.CloseHandle(h); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"read": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Null(), nil }
			count := int(a[1].NumVal)
			if count <= 0 || count > 67108864 { return Null(), nil }
			buf := make([]byte, count)
			f := os.NewFile(uintptr(int(a[0].NumVal)), "")
			n, err := f.Read(buf)
			if err != nil || n < 0 { return Null(), nil }
			return String(string(buf[:n])), nil
		}),
		"write": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return nosys() }
			f := os.NewFile(uintptr(int(a[0].NumVal)), "")
			n, err := f.Write([]byte(a[1].String()))
			if err != nil { return Number(-1), nil }
			return Number(float64(n)), nil
		}),
		"seek": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 { return nosys() }
			f := os.NewFile(uintptr(int(a[0].NumVal)), "")
			off, err := f.Seek(int64(a[1].NumVal), int(a[2].NumVal))
			if err != nil { return Number(-1), nil }
			return Number(float64(off)), nil
		}),
		"dup":       builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"dup2":      builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"pipe":      builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"fsync": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return nosys() }
			h := syscall.Handle(uintptr(int(a[0].NumVal)))
			if err := syscall.FlushFileBuffers(h); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"fdatasync": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return nosys() }
			h := syscall.Handle(uintptr(int(a[0].NumVal)))
			if err := syscall.FlushFileBuffers(h); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"truncate": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return nosys() }
			if err := os.Truncate(a[0].String(), int64(a[1].NumVal)); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"ftruncate": builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"mmap":      builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"munmap":    builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"mmap_read":  builtin(func(a []*Value) (*Value, error) { return Null(), nil }),
		"mmap_write": builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"poll":       builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"fcntl_lock": builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"fcntl_setfl": builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"fcntl_getfl": builtin(func(a []*Value) (*Value, error) { return nosys() }),

		// ── Level 3: Filesystem ───────────────────────────────────────
		"stat": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			fi, err := os.Stat(a[0].String())
			if err != nil { return Null(), nil }
			mtime := float64(fi.ModTime().Unix())
			return Array([]*Value{
				Number(float64(fi.Size())), Number(float64(fi.Mode())),
				Number(0), Number(0), Number(mtime), Number(mtime), Number(1),
			}), nil
		}),
		"lstat": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			fi, err := os.Lstat(a[0].String())
			if err != nil { return Null(), nil }
			mtime := float64(fi.ModTime().Unix())
			return Array([]*Value{
				Number(float64(fi.Size())), Number(float64(fi.Mode())),
				Number(0), Number(0), Number(mtime), Number(mtime), Number(1),
			}), nil
		}),
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
		"islnk": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return False(), nil }
			fi, err := os.Lstat(a[0].String())
			if err != nil { return False(), nil }
			return Bool(fi.Mode()&os.ModeSymlink != 0), nil
		}),
		"access": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return nosys() }
			_, err := os.Stat(a[0].String())
			if err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"mkdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return nosys() }
			if err := os.Mkdir(a[0].String(), 0755); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"rmdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return nosys() }
			if err := os.Remove(a[0].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"unlink": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return nosys() }
			if err := os.Remove(a[0].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"rename": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return nosys() }
			if err := os.Rename(a[0].String(), a[1].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"link":     builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"symlink": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return nosys() }
			if err := os.Symlink(a[0].String(), a[1].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"readlink": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			t, err := os.Readlink(a[0].String())
			if err != nil { return Null(), nil }
			return String(t), nil
		}),
		"chmod": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return nosys() }
			if err := os.Chmod(a[0].String(), os.FileMode(int(a[1].NumVal))); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"chown":  builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"getcwd": builtin(func(a []*Value) (*Value, error) {
			cwd, err := os.Getwd()
			if err != nil { return Null(), nil }
			return String(cwd), nil
		}),
		"chdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return nosys() }
			if err := os.Chdir(a[0].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"mkfifo": builtin(func(a []*Value) (*Value, error) { return nosys() }),

		// ── Level 4: Networking (stubs on Windows for now) ───────────
		"socket":     builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"connect":    builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"bind":       builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"listen":     builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"accept":     builtin(func(a []*Value) (*Value, error) { return Null(), nil }),
		"send":       builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"recv":       builtin(func(a []*Value) (*Value, error) { return Null(), nil }),
		"setsockopt": builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"inet_aton":  builtin(func(a []*Value) (*Value, error) { return nosys() }),
		"inet_ntoa":  builtin(func(a []*Value) (*Value, error) { return Null(), nil }),
		"htons":      builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			return Number(float64(bits.ReverseBytes16(uint16(a[0].NumVal)))), nil
		}),
		"ntohs": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			return Number(float64(bits.ReverseBytes16(uint16(a[0].NumVal)))), nil
		}),
		"htonl": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			return Number(float64(bits.ReverseBytes32(uint32(a[0].NumVal)))), nil
		}),
		"ntohl": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(0), nil }
			return Number(float64(bits.ReverseBytes32(uint32(a[0].NumVal)))), nil
		}),

		// ── Level 5: Time & Clocks ────────────────────────────────────
		"clock_realtime": builtin(func(a []*Value) (*Value, error) {
			now := time.Now()
			return Array([]*Value{Number(float64(now.Unix())), Number(float64(now.Nanosecond()))}), nil
		}),
		"clock_monotonic": builtin(func(a []*Value) (*Value, error) {
			now := time.Now()
			return Array([]*Value{Number(float64(now.Unix())), Number(float64(now.Nanosecond()))}), nil
		}),
		"clock_process": builtin(func(a []*Value) (*Value, error) {
			now := time.Now()
			return Array([]*Value{Number(float64(now.Unix())), Number(float64(now.Nanosecond()))}), nil
		}),
		"nanosleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) >= 2 {
				time.Sleep(time.Duration(a[0].NumVal)*time.Second + time.Duration(a[1].NumVal)*time.Nanosecond)
			}
			return Number(0), nil
		}),
		"time":  builtin(func(a []*Value) (*Value, error) { return Number(float64(time.Now().Unix())), nil }),
		"clock": builtin(func(a []*Value) (*Value, error) { return Number(float64(time.Now().UnixNano()) / 1e9), nil }),
		"gettimeofday": builtin(func(a []*Value) (*Value, error) {
			now := time.Now()
			return Array([]*Value{Number(float64(now.Unix())), Number(float64(now.Nanosecond() / 1000))}), nil
		}),
		"sleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) >= 1 { time.Sleep(time.Duration(a[0].NumVal) * time.Second) }
			return Number(0), nil
		}),
		"usleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) >= 1 { time.Sleep(time.Duration(a[0].NumVal) * time.Microsecond) }
			return Number(0), nil
		}),

		// ── Level 6: System Info ──────────────────────────────────────
		"uname": builtin(func(a []*Value) (*Value, error) {
			return Array([]*Value{
				String("Windows"), String("unknown"), String("unknown"),
				String("unknown"), String(runtime.GOARCH),
			}), nil
		}),
		"hostname": builtin(func(a []*Value) (*Value, error) {
			h, err := os.Hostname()
			if err != nil { return Null(), nil }
			return String(h), nil
		}),
		"nproc":        builtin(func(a []*Value) (*Value, error) { return Number(float64(runtime.NumCPU())), nil }),
		"nproc_conf":   builtin(func(a []*Value) (*Value, error) { return Number(float64(runtime.NumCPU())), nil }),
		"phys_pages":   builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"avphys_pages": builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"PAGE_SIZE":    builtin(func(a []*Value) (*Value, error) { return Number(4096), nil }),
		"endian":       builtin(func(a []*Value) (*Value, error) { return String("little"), nil }),
		"sizeof_ptr": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(unsafe.Sizeof(uintptr(0)))), nil
		}),
		"getenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			v := os.Getenv(a[0].String())
			if v == "" { return Null(), nil }
			return String(v), nil
		}),
		"setenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return nosys() }
			if err := os.Setenv(a[0].String(), a[1].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"unsetenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return nosys() }
			os.Unsetenv(a[0].String()); return Number(0), nil
		}),
		"errno":    builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"strerror": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return String("unknown error"), nil }
			return String(fmt.Sprintf("error %d", int(a[0].NumVal))), nil
		}),
		"system": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return nosys() }
			cmd := exec.Command("cmd", "/C", a[0].String())
			cmd.Stdout, cmd.Stderr = os.Stdout, os.Stderr
			if err := cmd.Run(); err != nil {
				if e, ok := err.(*exec.ExitError); ok { return Number(float64(e.ExitCode())), nil }
				return Number(-1), nil
			}
			return Number(0), nil
		}),
		"proc_status": builtin(func(a []*Value) (*Value, error) { return Null(), nil }),
		"proc_maps":   builtin(func(a []*Value) (*Value, error) { return Null(), nil }),

		// ── Level 7: Bit Operations ───────────────────────────────────
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
			v, n := uint64(int64(a[0].NumVal)), int(a[1].NumVal)
			if n >= 0 { return Number(float64(int64(v << uint(n)))), nil }
			return Number(float64(int64(v >> uint(-n)))), nil
		}),
		"shr": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			v, n := uint64(int64(a[0].NumVal)), int(a[1].NumVal)
			if n >= 0 { return Number(float64(int64(v >> uint(n)))), nil }
			return Number(float64(int64(v << uint(-n)))), nil
		}),
		"sar": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			return Number(float64(int64(a[0].NumVal) >> uint(int(a[1].NumVal)))), nil
		}),
		"rol": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			return Number(float64(int64(bits.RotateLeft32(uint32(int64(a[0].NumVal)), int(a[1].NumVal)&31)))), nil
		}),
		"ror": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			return Number(float64(int64(bits.RotateLeft32(uint32(int64(a[0].NumVal)), -(int(a[1].NumVal)&31))))), nil
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
			return Number(float64((uint64(int64(a[0].NumVal)) >> uint(a[1].NumVal)) & 1)), nil
		}),
		"bit_set": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			return Number(float64(int64(uint64(int64(a[0].NumVal)) | (1 << uint(a[1].NumVal))))), nil
		}),
		"bit_clear": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			return Number(float64(int64(uint64(int64(a[0].NumVal)) & ^(uint64(1) << uint(a[1].NumVal))))), nil
		}),
		"bit_toggle": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			return Number(float64(int64(uint64(int64(a[0].NumVal)) ^ (1 << uint(a[1].NumVal))))), nil
		}),
		"align_up": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			n, al := uint64(int64(a[0].NumVal)), uint64(int64(a[1].NumVal))
			if al == 0 { return Number(float64(int64(n))), nil }
			return Number(float64(int64((n + al - 1) & ^(al - 1)))), nil
		}),
		"align_down": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			n, al := uint64(int64(a[0].NumVal)), uint64(int64(a[1].NumVal))
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

		// ── Level 8: IPC & Logging (stubs) ───────────────────────────
		"openlog":  builtin(func(a []*Value) (*Value, error) { return Null(), nil }),
		"syslog":   builtin(func(a []*Value) (*Value, error) { return Null(), nil }),
		"closelog": builtin(func(a []*Value) (*Value, error) { return Null(), nil }),

		// ── Level 9: Open flags (Windows values differ from POSIX) ────
		"O_RDONLY":   builtin(func(a []*Value) (*Value, error) { return Number(float64(os.O_RDONLY)), nil }),
		"O_WRONLY":   builtin(func(a []*Value) (*Value, error) { return Number(float64(os.O_WRONLY)), nil }),
		"O_RDWR":     builtin(func(a []*Value) (*Value, error) { return Number(float64(os.O_RDWR)), nil }),
		"O_CREAT":    builtin(func(a []*Value) (*Value, error) { return Number(float64(os.O_CREATE)), nil }),
		"O_TRUNC":    builtin(func(a []*Value) (*Value, error) { return Number(float64(os.O_TRUNC)), nil }),
		"O_APPEND":   builtin(func(a []*Value) (*Value, error) { return Number(float64(os.O_APPEND)), nil }),
		"O_NONBLOCK": builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"O_SYNC":     builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"O_EXCL":     builtin(func(a []*Value) (*Value, error) { return Number(float64(os.O_EXCL)), nil }),

		// ── Signal constants (Windows subset) ─────────────────────────
		"SIGHUP":  builtin(func(a []*Value) (*Value, error) { return Number(1), nil }),
		"SIGINT":  builtin(func(a []*Value) (*Value, error) { return Number(2), nil }),
		"SIGQUIT": builtin(func(a []*Value) (*Value, error) { return Number(3), nil }),
		"SIGKILL": builtin(func(a []*Value) (*Value, error) { return Number(9), nil }),
		"SIGTERM": builtin(func(a []*Value) (*Value, error) { return Number(15), nil }),
		"SIGUSR1": builtin(func(a []*Value) (*Value, error) { return Number(10), nil }),
		"SIGUSR2": builtin(func(a []*Value) (*Value, error) { return Number(12), nil }),
		"SIGCHLD": builtin(func(a []*Value) (*Value, error) { return Number(17), nil }),
		"SIGPIPE": builtin(func(a []*Value) (*Value, error) { return Number(13), nil }),
		"SIGALRM": builtin(func(a []*Value) (*Value, error) { return Number(14), nil }),
		"SIGSEGV": builtin(func(a []*Value) (*Value, error) { return Number(11), nil }),

		// ── Syslog constants ──────────────────────────────────────────
		"LOG_EMERG": builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"LOG_ALERT": builtin(func(a []*Value) (*Value, error) { return Number(1), nil }),
		"LOG_CRIT":  builtin(func(a []*Value) (*Value, error) { return Number(2), nil }),
		"LOG_ERR":   builtin(func(a []*Value) (*Value, error) { return Number(3), nil }),
		"LOG_WARNING": builtin(func(a []*Value) (*Value, error) { return Number(4), nil }),
		"LOG_NOTICE":  builtin(func(a []*Value) (*Value, error) { return Number(5), nil }),
		"LOG_INFO":    builtin(func(a []*Value) (*Value, error) { return Number(6), nil }),
		"LOG_DEBUG":   builtin(func(a []*Value) (*Value, error) { return Number(7), nil }),

		// ── Poll constants ────────────────────────────────────────────
		"POLLIN":  builtin(func(a []*Value) (*Value, error) { return Number(0x0001), nil }),
		"POLLOUT": builtin(func(a []*Value) (*Value, error) { return Number(0x0004), nil }),
		"POLLERR": builtin(func(a []*Value) (*Value, error) { return Number(0x0008), nil }),
		"POLLHUP": builtin(func(a []*Value) (*Value, error) { return Number(0x0010), nil }),

		// ── mmap constants (stubs) ────────────────────────────────────
		"PROT_READ":  builtin(func(a []*Value) (*Value, error) { return Number(1), nil }),
		"PROT_WRITE": builtin(func(a []*Value) (*Value, error) { return Number(2), nil }),
		"PROT_EXEC":  builtin(func(a []*Value) (*Value, error) { return Number(4), nil }),
		"PROT_NONE":  builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"MAP_SHARED":    builtin(func(a []*Value) (*Value, error) { return Number(1), nil }),
		"MAP_PRIVATE":   builtin(func(a []*Value) (*Value, error) { return Number(2), nil }),
		"MAP_ANONYMOUS": builtin(func(a []*Value) (*Value, error) { return Number(0x20), nil }),

		// ── Socket constants (stubs) ──────────────────────────────────
		"AF_INET":     builtin(func(a []*Value) (*Value, error) { return Number(2), nil }),
		"AF_INET6":    builtin(func(a []*Value) (*Value, error) { return Number(23), nil }),
		"AF_UNIX":     builtin(func(a []*Value) (*Value, error) { return Number(1), nil }),
		"SOCK_STREAM": builtin(func(a []*Value) (*Value, error) { return Number(1), nil }),
		"SOCK_DGRAM":  builtin(func(a []*Value) (*Value, error) { return Number(2), nil }),

		// ── RLIMIT constants (stubs) ──────────────────────────────────
		"RLIMIT_CPU":    builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"RLIMIT_FSIZE":  builtin(func(a []*Value) (*Value, error) { return Number(1), nil }),
		"RLIMIT_STACK":  builtin(func(a []*Value) (*Value, error) { return Number(3), nil }),
		"RLIMIT_NOFILE": builtin(func(a []*Value) (*Value, error) { return Number(7), nil }),
		"RLIMIT_AS":     builtin(func(a []*Value) (*Value, error) { return Number(9), nil }),

		// ── Level 10: FD constants & numeric limits ───────────────────
		"STDIN":    builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"STDOUT":   builtin(func(a []*Value) (*Value, error) { return Number(1), nil }),
		"STDERR":   builtin(func(a []*Value) (*Value, error) { return Number(2), nil }),
		"INT_MAX":  builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxInt32)), nil }),
		"INT_MIN":  builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MinInt32)), nil }),
		"UINT_MAX": builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxUint32)), nil }),
		"LONG_MAX": builtin(func(a []*Value) (*Value, error) { return Number(float64(math.MaxInt64)), nil }),
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
}
