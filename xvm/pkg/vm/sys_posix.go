/*
 * XENLY VIRTUAL MACHINE (XVM) - high-performance of the Virtual Machine
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * It is available for the Linux, macOS, and Windows operating systems.
 *
 */
//go:build linux || darwin

package vm

import (
	"encoding/binary"
	"fmt"
	"math"
	"math/bits"
	"net"
	"os"
	"os/exec"
	"runtime"
	"syscall"
	"time"
	"unsafe"
)

func registerSysModule(env *Env) {
	sysObj := Object(map[string]*Value{

		// ── Level 1: Process Control ──────────────────────────────────
		"exit": builtin(func(a []*Value) (*Value, error) {
			code := 0
			if len(a) >= 1 { code = int(a[0].NumVal) }
			os.Exit(code); return Null(), nil
		}),
		"abort": builtin(func(a []*Value) (*Value, error) {
			syscall.Kill(os.Getpid(), syscall.SIGABRT)
			return Null(), nil
		}),
		"getpid":  builtin(func(a []*Value) (*Value, error) { return Number(float64(os.Getpid())), nil }),
		"getppid": builtin(func(a []*Value) (*Value, error) { return Number(float64(os.Getppid())), nil }),
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
			if err := syscall.Exec(path, argv, os.Environ()); err != nil { return Number(-1), nil }
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
			if err := syscall.Kill(int(a[0].NumVal), syscall.Signal(int(a[1].NumVal))); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"raise": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			if err := syscall.Kill(os.Getpid(), syscall.Signal(int(a[0].NumVal))); err != nil { return Number(-1), nil }
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

		// ── Level 2: File Descriptor I/O ──────────────────────────────
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
			if err := syscall.Close(int(a[0].NumVal)); err != nil { return Number(-1), nil }
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
			n, err := syscall.Write(int(a[0].NumVal), []byte(a[1].String()))
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
			if err := syscall.Dup2(int(a[0].NumVal), int(a[1].NumVal)); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"pipe": builtin(func(a []*Value) (*Value, error) {
			fds := make([]int, 2)
			if err := syscall.Pipe(fds); err != nil { return Null(), nil }
			return Array([]*Value{Number(float64(fds[0])), Number(float64(fds[1]))}), nil
		}),
		"fsync": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			if err := syscall.Fsync(int(a[0].NumVal)); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"fdatasync": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			if err := sysFdatasync(int(a[0].NumVal)); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"truncate": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			if err := syscall.Truncate(a[0].String(), int64(a[1].NumVal)); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"ftruncate": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			if err := syscall.Ftruncate(int(a[0].NumVal), int64(a[1].NumVal)); err != nil { return Number(-1), nil }
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

		// ── Level 3: Filesystem ───────────────────────────────────────
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
			if err := syscall.Access(a[0].String(), uint32(a[1].NumVal)); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"mkdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			mode := uint32(0755)
			if len(a) >= 2 { mode = uint32(a[1].NumVal) }
			if err := syscall.Mkdir(a[0].String(), mode); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"rmdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			if err := syscall.Rmdir(a[0].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"unlink": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			if err := syscall.Unlink(a[0].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"rename": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			if err := syscall.Rename(a[0].String(), a[1].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"link": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			if err := syscall.Link(a[0].String(), a[1].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"symlink": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			if err := syscall.Symlink(a[0].String(), a[1].String()); err != nil { return Number(-1), nil }
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
			if err := syscall.Chmod(a[0].String(), uint32(a[1].NumVal)); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"chown": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 { return Number(-1), nil }
			if err := syscall.Chown(a[0].String(), int(a[1].NumVal), int(a[2].NumVal)); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"getcwd": builtin(func(a []*Value) (*Value, error) {
			cwd, err := os.Getwd()
			if err != nil { return Null(), nil }
			return String(cwd), nil
		}),
		"chdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			if err := syscall.Chdir(a[0].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"mkfifo": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			mode := uint32(0644)
			if len(a) >= 2 { mode = uint32(a[1].NumVal) }
			if err := syscall.Mkfifo(a[0].String(), mode); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),

		// ── Level 4: Networking ───────────────────────────────────────
		"socket": builtin(func(a []*Value) (*Value, error) {
			domain, typ, proto := syscall.AF_INET, syscall.SOCK_STREAM, 0
			if len(a) >= 1 { domain = int(a[0].NumVal) }
			if len(a) >= 2 { typ = int(a[1].NumVal) }
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
			if err := syscall.Connect(int(a[0].NumVal), addr); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"bind": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 { return Number(-1), nil }
			fd := int(a[0].NumVal)
			addr := &syscall.SockaddrInet4{Port: int(a[2].NumVal)}
			if len(a) >= 2 && a[1].String() != "" && a[1].String() != "0.0.0.0" {
				if ip := net.ParseIP(a[1].String()).To4(); ip != nil { copy(addr.Addr[:], ip) }
			}
			syscall.SetsockoptInt(fd, syscall.SOL_SOCKET, syscall.SO_REUSEADDR, 1)
			if err := syscall.Bind(fd, addr); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"listen": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			if err := syscall.Listen(int(a[0].NumVal), int(a[1].NumVal)); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"accept": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			nfd, sa, err := syscall.Accept(int(a[0].NumVal))
			if err != nil { return Null(), nil }
			ip, port := "", 0
			if s4, ok := sa.(*syscall.SockaddrInet4); ok {
				ip = fmt.Sprintf("%d.%d.%d.%d", s4.Addr[0], s4.Addr[1], s4.Addr[2], s4.Addr[3])
				port = s4.Port
			}
			return Array([]*Value{Number(float64(nfd)), String(ip), Number(float64(port))}), nil
		}),
		"send": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			data := []byte(a[1].String())
			if err := syscall.Sendto(int(a[0].NumVal), data, 0, nil); err != nil {
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
			if err := syscall.SetsockoptInt(int(a[0].NumVal), int(a[1].NumVal), int(a[2].NumVal), int(a[3].NumVal)); err != nil { return Number(-1), nil }
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
			ip := make(net.IP, 4)
			binary.BigEndian.PutUint32(ip, uint32(a[0].NumVal))
			return String(ip.String()), nil
		}),
		"htons": builtin(func(a []*Value) (*Value, error) {
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
		"time":  builtin(func(a []*Value) (*Value, error) { return Number(float64(time.Now().Unix())), nil }),
		"clock": builtin(func(a []*Value) (*Value, error) { return Number(float64(time.Now().UnixNano()) / 1e9), nil }),
		"gettimeofday": builtin(func(a []*Value) (*Value, error) {
			sec, usec := sysGettimeofday()
			return Array([]*Value{Number(float64(sec)), Number(float64(usec))}), nil
		}),
		"sleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) >= 1 { time.Sleep(time.Duration(a[0].NumVal) * time.Second) }
			return Number(0), nil
		}),
		"usleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) >= 1 { time.Sleep(time.Duration(a[0].NumVal) * time.Microsecond) }
			return Number(0), nil
		}),

		// ── Level 6: System Info & Environment ───────────────────────
		"uname":     builtin(func(a []*Value) (*Value, error) {
			fields := sysUname()
			r := make([]*Value, len(fields))
			for i, f := range fields { r[i] = String(f) }
			return Array(r), nil
		}),
		"hostname": builtin(func(a []*Value) (*Value, error) {
			h, err := os.Hostname()
			if err != nil { return Null(), nil }
			return String(h), nil
		}),
		"nproc":      builtin(func(a []*Value) (*Value, error) { return Number(float64(runtime.NumCPU())), nil }),
		"nproc_conf": builtin(func(a []*Value) (*Value, error) { return Number(float64(runtime.NumCPU())), nil }),
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
		"PAGE_SIZE": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.Getpagesize())), nil }),
		"endian": builtin(func(a []*Value) (*Value, error) {
			t := uint16(0x0001)
			if *(*uint8)(unsafe.Pointer(&t)) == 1 { return String("little"), nil }
			return String("big"), nil
		}),
		"sizeof_ptr": builtin(func(a []*Value) (*Value, error) { return Number(float64(unsafe.Sizeof(uintptr(0)))), nil }),
		"getenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Null(), nil }
			v, ok := syscall.Getenv(a[0].String())
			if !ok { return Null(), nil }
			return String(v), nil
		}),
		"setenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(-1), nil }
			if err := os.Setenv(a[0].String(), a[1].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"unsetenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			if err := os.Unsetenv(a[0].String()); err != nil { return Number(-1), nil }
			return Number(0), nil
		}),
		"errno":    builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"strerror": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return String("Unknown error"), nil }
			return String(syscall.Errno(int(a[0].NumVal)).Error()), nil
		}),
		"system": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 { return Number(-1), nil }
			cmd := exec.Command("sh", "-c", a[0].String())
			cmd.Stdout, cmd.Stderr = os.Stdout, os.Stderr
			if err := cmd.Run(); err != nil {
				if e, ok := err.(*exec.ExitError); ok { return Number(float64(e.ExitCode())), nil }
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

		// ── Level 7: Bit & Memory Operations ─────────────────────────
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
			v, n := uint32(int64(a[0].NumVal)), int(a[1].NumVal)&31
			return Number(float64(int64(bits.RotateLeft32(v, n)))), nil
		}),
		"ror": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 { return Number(0), nil }
			v, n := uint32(int64(a[0].NumVal)), int(a[1].NumVal)&31
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

		// ── Level 9: Open flags ───────────────────────────────────────
		"O_RDONLY":  builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_RDONLY)), nil }),
		"O_WRONLY":  builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_WRONLY)), nil }),
		"O_RDWR":    builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_RDWR)), nil }),
		"O_CREAT":   builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_CREAT)), nil }),
		"O_TRUNC":   builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_TRUNC)), nil }),
		"O_APPEND":  builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_APPEND)), nil }),
		"O_NONBLOCK": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_NONBLOCK)), nil }),
		"O_SYNC":    builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_SYNC)), nil }),
		"O_EXCL":    builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.O_EXCL)), nil }),

		// ── Signal constants ──────────────────────────────────────────
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

		// ── Syslog priority constants ─────────────────────────────────
		"LOG_EMERG": builtin(func(a []*Value) (*Value, error) { return Number(0), nil }),
		"LOG_ALERT": builtin(func(a []*Value) (*Value, error) { return Number(1), nil }),
		"LOG_CRIT":  builtin(func(a []*Value) (*Value, error) { return Number(2), nil }),
		"LOG_ERR":   builtin(func(a []*Value) (*Value, error) { return Number(3), nil }),
		"LOG_WARNING": builtin(func(a []*Value) (*Value, error) { return Number(4), nil }),
		"LOG_NOTICE":  builtin(func(a []*Value) (*Value, error) { return Number(5), nil }),
		"LOG_INFO":    builtin(func(a []*Value) (*Value, error) { return Number(6), nil }),
		"LOG_DEBUG":   builtin(func(a []*Value) (*Value, error) { return Number(7), nil }),

		// ── Poll constants ────────────────────────────────────────────
		"POLLIN":  builtin(func(a []*Value) (*Value, error) { return Number(float64(sysPOLLIN)), nil }),
		"POLLOUT": builtin(func(a []*Value) (*Value, error) { return Number(float64(sysPOLLOUT)), nil }),
		"POLLERR": builtin(func(a []*Value) (*Value, error) { return Number(float64(sysPOLLERR)), nil }),
		"POLLHUP": builtin(func(a []*Value) (*Value, error) { return Number(float64(sysPOLLHUP)), nil }),

		// ── mmap prot/flags constants ─────────────────────────────────
		"PROT_READ":     builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.PROT_READ)), nil }),
		"PROT_WRITE":    builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.PROT_WRITE)), nil }),
		"PROT_EXEC":     builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.PROT_EXEC)), nil }),
		"PROT_NONE":     builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.PROT_NONE)), nil }),
		"MAP_SHARED":    builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.MAP_SHARED)), nil }),
		"MAP_PRIVATE":   builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.MAP_PRIVATE)), nil }),
		"MAP_ANONYMOUS": builtin(func(a []*Value) (*Value, error) { return Number(float64(sysMAP_ANON)), nil }),

		// ── Socket domain/type constants ──────────────────────────────
		"AF_INET":     builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.AF_INET)), nil }),
		"AF_INET6":    builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.AF_INET6)), nil }),
		"AF_UNIX":     builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.AF_UNIX)), nil }),
		"SOCK_STREAM": builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SOCK_STREAM)), nil }),
		"SOCK_DGRAM":  builtin(func(a []*Value) (*Value, error) { return Number(float64(syscall.SOCK_DGRAM)), nil }),

		// ── Resource limit constants ──────────────────────────────────
		"RLIMIT_CPU":    builtin(func(a []*Value) (*Value, error) { return Number(float64(sysRLIMIT_CPU)), nil }),
		"RLIMIT_FSIZE":  builtin(func(a []*Value) (*Value, error) { return Number(float64(sysRLIMIT_FSIZE)), nil }),
		"RLIMIT_STACK":  builtin(func(a []*Value) (*Value, error) { return Number(float64(sysRLIMIT_STACK)), nil }),
		"RLIMIT_NOFILE": builtin(func(a []*Value) (*Value, error) { return Number(float64(sysRLIMIT_NOFILE)), nil }),
		"RLIMIT_AS":     builtin(func(a []*Value) (*Value, error) { return Number(float64(sysRLIMIT_AS)), nil }),

		// ── Level 10: FD constants & numeric limits ───────────────────
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
}
