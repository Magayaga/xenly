/*
 * XENLY VIRTUAL MACHINE (XVM) - high-performance of the Virtual Machine
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * It is available for the Linux, macOS, and Windows operating systems.
 *
 */
//go:build darwin

package vm

import (
	"os/exec"
	"runtime"
	"strings"
	"syscall"
	"time"
	"unsafe"
)

func sysStatMtime(st syscall.Stat_t) float64 { return float64(st.Mtimespec.Sec) }
func sysStatAtime(st syscall.Stat_t) float64 { return float64(st.Atimespec.Sec) }

func sysFdatasync(fd int) error {
	const F_FULLFSYNC = 51
	_, _, errno := syscall.Syscall(syscall.SYS_FCNTL, uintptr(fd), uintptr(F_FULLFSYNC), 0)
	if errno != 0 {
		return syscall.Fsync(fd)
	}
	return nil
}

func sysFork() (int, error) {
	r, _, errno := syscall.RawSyscall(syscall.SYS_FORK, 0, 0, 0)
	if errno != 0 {
		return -1, errno
	}
	return int(r), nil
}

func sysClockRealtime() (int64, int64) {
	now := time.Now()
	return now.Unix(), int64(now.Nanosecond())
}
func sysClockMonotonic() (int64, int64) {
	now := time.Now()
	return now.Unix(), int64(now.Nanosecond())
}
func sysClockProcess() (int64, int64) {
	now := time.Now()
	return now.Unix(), int64(now.Nanosecond())
}
func sysNanosleep(sec, nsec int64) int {
	time.Sleep(time.Duration(sec)*time.Second + time.Duration(nsec)*time.Nanosecond)
	return 0
}
func sysGettimeofday() (int64, int64) {
	now := time.Now()
	return now.Unix(), int64(now.Nanosecond() / 1000)
}

// sysPollFd mirrors struct pollfd — same layout on Darwin and Linux.
type sysPollFd struct {
	Fd      int32
	Events  int16
	Revents int16
}

func sysPoll(fds []sysPollFd, timeout int) (int, error) {
	if len(fds) == 0 {
		return 0, nil
	}
	r, _, errno := syscall.Syscall(syscall.SYS_POLL,
		uintptr(unsafe.Pointer(&fds[0])),
		uintptr(len(fds)),
		uintptr(timeout))
	if errno != 0 {
		return -1, errno
	}
	return int(r), nil
}

const (
	sysPOLLIN  = 0x0001
	sysPOLLOUT = 0x0004
	sysPOLLERR = 0x0008
	sysPOLLHUP = 0x0010
)

const sysMAP_ANON = int(syscall.MAP_ANON)

func sysFcntlLock(fd, lockType int) int {
	var lt int16
	switch lockType {
	case 0:
		lt = syscall.F_RDLCK
	case 1:
		lt = syscall.F_WRLCK
	default:
		lt = syscall.F_UNLCK
	}
	fl := syscall.Flock_t{Type: lt, Whence: int16(0)}
	_, _, errno := syscall.Syscall(syscall.SYS_FCNTL,
		uintptr(fd), uintptr(syscall.F_SETLK),
		uintptr(unsafe.Pointer(&fl)))
	if errno != 0 {
		return -1
	}
	return 0
}
func sysFcntlGetfl(fd int) int {
	flags, _, errno := syscall.Syscall(syscall.SYS_FCNTL, uintptr(fd), uintptr(syscall.F_GETFL), 0)
	if errno != 0 {
		return -1
	}
	return int(flags)
}
func sysFcntlSetfl(fd, flags int) int {
	_, _, errno := syscall.Syscall(syscall.SYS_FCNTL, uintptr(fd), uintptr(syscall.F_SETFL), uintptr(flags))
	if errno != 0 {
		return -1
	}
	return 0
}

func sysSysinfo() (uint64, uint64, error) { return 0, 0, nil }

// sysUname: avoid syscall.Uname/Utsname which may not be available in all
// Go/Darwin versions. Use exec.Command("uname") instead.
func sysUname() []string {
	getField := func(flag string) string {
		out, err := exec.Command("uname", flag).Output()
		if err != nil {
			return "unknown"
		}
		return strings.TrimSpace(string(out))
	}
	sysname := getField("-s")   // Darwin
	nodename := getField("-n")  // hostname
	release := getField("-r")   // kernel release
	version := getField("-v")   // kernel version
	machine := runtime.GOARCH  // amd64 / arm64
	return []string{sysname, nodename, release, version, machine}
}

func sysGetrlimit(resource int) (soft, hard float64, ok bool) {
	var rl syscall.Rlimit
	if err := syscall.Getrlimit(resource, &rl); err != nil {
		return 0, 0, false
	}
	const inf = ^uint64(0)
	s, h := float64(rl.Cur), float64(rl.Max)
	if rl.Cur == inf { s = -1 }
	if rl.Max == inf { h = -1 }
	return s, h, true
}
func sysSetrlimit(resource int, soft, hard float64) int {
	const inf = ^uint64(0)
	rl := syscall.Rlimit{}
	if soft < 0 { rl.Cur = inf } else { rl.Cur = uint64(soft) }
	if hard < 0 { rl.Max = inf } else { rl.Max = uint64(hard) }
	if err := syscall.Setrlimit(resource, &rl); err != nil {
		return -1
	}
	return 0
}

const (
	sysRLIMIT_CPU    = int(syscall.RLIMIT_CPU)
	sysRLIMIT_FSIZE  = int(syscall.RLIMIT_FSIZE)
	sysRLIMIT_STACK  = int(syscall.RLIMIT_STACK)
	sysRLIMIT_NOFILE = int(syscall.RLIMIT_NOFILE)
	sysRLIMIT_AS     = int(syscall.RLIMIT_AS)
)

func sysMmap(size, prot, flags, fd int, offset int64) (uintptr, error) {
	b, err := syscall.Mmap(fd, offset, size, prot, flags)
	if err != nil {
		return 0, err
	}
	return uintptr(unsafe.Pointer(&b[0])), nil
}
func sysMunmap(ptr uintptr, size int) int {
	b := unsafe.Slice((*byte)(unsafe.Pointer(ptr)), size)
	if err := syscall.Munmap(b); err != nil {
		return -1
	}
	return 0
}

func sysMonoNow() int64 { return time.Now().UnixNano() }
