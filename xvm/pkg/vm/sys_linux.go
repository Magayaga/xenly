/*
 * XENLY VIRTUAL MACHINE (XVM) - high-performance of the Virtual Machine
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * It is available for the Linux, macOS, and Windows operating systems.
 *
 */
//go:build linux

package vm

import (
	"syscall"
	"time"
	"unsafe"
)

func sysStatMtime(st syscall.Stat_t) float64 { return float64(st.Mtim.Sec) }
func sysStatAtime(st syscall.Stat_t) float64 { return float64(st.Atim.Sec) }

func sysFdatasync(fd int) error { return syscall.Fdatasync(fd) }

// Linux amd64/arm64: no SYS_FORK — use SYS_CLONE(SIGCHLD).
func sysFork() (int, error) {
	r, _, errno := syscall.RawSyscall(syscall.SYS_CLONE, uintptr(syscall.SIGCHLD), 0, 0)
	if errno != 0 {
		return -1, errno
	}
	return int(r), nil
}

// Clocks: use time.Now() — avoids syscall.ClockGettime which is in x/sys/unix.
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

// poll: syscall.PollFd and syscall.Poll are in x/sys/unix, not syscall.
// Define the struct manually (layout is identical on all POSIX targets).
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

// Poll event constants — stable on Linux.
const (
	sysPOLLIN  = 0x0001
	sysPOLLOUT = 0x0004
	sysPOLLERR = 0x0008
	sysPOLLHUP = 0x0010
)

// MAP_ANONYMOUS on Linux = 0x20. Hardcoded to avoid syscall.MAP_ANONYMOUS
// being missing on some Go versions.
const sysMAP_ANON = 0x20

// fcntl advisory lock.
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
	if err := syscall.FcntlFlock(uintptr(fd), syscall.F_SETLK, &fl); err != nil {
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

func sysSysinfo() (totalPages, freePages uint64, err error) {
	var si syscall.Sysinfo_t
	if e := syscall.Sysinfo(&si); e != nil {
		return 0, 0, e
	}
	ps := uint64(syscall.Getpagesize())
	if ps == 0 {
		ps = 4096
	}
	return si.Totalram * uint64(si.Unit) / ps,
		si.Freeram * uint64(si.Unit) / ps, nil
}

func sysUname() []string {
	var u syscall.Utsname
	if err := syscall.Uname(&u); err != nil {
		return []string{"linux", "unknown", "unknown", "unknown", "unknown"}
	}
	toStr := func(b [65]int8) string {
		s := make([]byte, 0, 65)
		for _, c := range b {
			if c == 0 {
				break
			}
			s = append(s, byte(c))
		}
		return string(s)
	}
	return []string{
		toStr(u.Sysname), toStr(u.Nodename),
		toStr(u.Release), toStr(u.Version), toStr(u.Machine),
	}
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
