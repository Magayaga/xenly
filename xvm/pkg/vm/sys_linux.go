//go:build linux

package vm

import (
	"syscall"
	"time"
	"unsafe"
)

// ── stat time fields ──────────────────────────────────────────────────────────
func sysStatMtime(st syscall.Stat_t) float64 { return float64(st.Mtim.Sec) }
func sysStatAtime(st syscall.Stat_t) float64 { return float64(st.Atim.Sec) }

// ── fdatasync ─────────────────────────────────────────────────────────────────
func sysFdatasync(fd int) error { return syscall.Fdatasync(fd) }

// ── fork ─────────────────────────────────────────────────────────────────────
// Linux amd64/arm64 have no SYS_FORK; use SYS_CLONE with SIGCHLD.
func sysFork() (int, error) {
	r, _, errno := syscall.RawSyscall(syscall.SYS_CLONE,
		uintptr(syscall.SIGCHLD), 0, 0)
	if errno != 0 {
		return -1, errno
	}
	return int(r), nil
}

// ── clocks ────────────────────────────────────────────────────────────────────
func sysClockRealtime() (int64, int64) {
	var ts syscall.Timespec
	syscall.ClockGettime(syscall.CLOCK_REALTIME, &ts)
	return ts.Sec, ts.Nsec
}
func sysClockMonotonic() (int64, int64) {
	var ts syscall.Timespec
	syscall.ClockGettime(syscall.CLOCK_MONOTONIC, &ts)
	return ts.Sec, ts.Nsec
}
func sysClockProcess() (int64, int64) {
	var ts syscall.Timespec
	syscall.ClockGettime(syscall.CLOCK_PROCESS_CPUTIME_ID, &ts)
	return ts.Sec, ts.Nsec
}
func sysNanosleep(sec, nsec int64) int {
	req := syscall.Timespec{Sec: sec, Nsec: nsec}
	var rem syscall.Timespec
	if err := syscall.Nanosleep(&req, &rem); err != nil {
		return -1
	}
	return 0
}
func sysGettimeofday() (int64, int64) {
	var tv syscall.Timeval
	syscall.Gettimeofday(&tv)
	return tv.Sec, int64(tv.Usec)
}

// ── poll ──────────────────────────────────────────────────────────────────────
type sysPollFd = syscall.PollFd

func sysPoll(fds []syscall.PollFd, timeout int) (int, error) {
	return syscall.Poll(fds, timeout)
}

const (
	sysPOLLIN  = int(syscall.POLLIN)
	sysPOLLOUT = int(syscall.POLLOUT)
	sysPOLLERR = int(syscall.POLLERR)
	sysPOLLHUP = int(syscall.POLLHUP)
)

// ── mmap anonymous flag ───────────────────────────────────────────────────────
const sysMAP_ANON = int(syscall.MAP_ANONYMOUS)

// ── fcntl advisory lock ───────────────────────────────────────────────────────
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
	flags, _, errno := syscall.Syscall(syscall.SYS_FCNTL,
		uintptr(fd), uintptr(syscall.F_GETFL), 0)
	if errno != 0 {
		return -1
	}
	return int(flags)
}
func sysFcntlSetfl(fd, flags int) int {
	_, _, errno := syscall.Syscall(syscall.SYS_FCNTL,
		uintptr(fd), uintptr(syscall.F_SETFL), uintptr(flags))
	if errno != 0 {
		return -1
	}
	return 0
}

// ── sysinfo ───────────────────────────────────────────────────────────────────
func sysSysinfo() (totalPages, freePages uint64, err error) {
	var si syscall.Sysinfo_t
	if e := syscall.Sysinfo(&si); e != nil {
		return 0, 0, e
	}
	pageSize := uint64(syscall.Getpagesize())
	if pageSize == 0 {
		pageSize = 4096
	}
	return si.Totalram * uint64(si.Unit) / pageSize,
		si.Freeram * uint64(si.Unit) / pageSize,
		nil
}

// ── uname ─────────────────────────────────────────────────────────────────────
func sysUname() []string {
	var u syscall.Utsname
	if err := syscall.Uname(&u); err != nil {
		return []string{"", "", "", "", ""}
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

// ── rlimit ────────────────────────────────────────────────────────────────────
const sysRLIM_INFINITY = ^uint64(0)

func sysGetrlimit(resource int) (soft, hard float64, ok bool) {
	var rl syscall.Rlimit
	if err := syscall.Getrlimit(resource, &rl); err != nil {
		return 0, 0, false
	}
	s := float64(rl.Cur)
	if rl.Cur == sysRLIM_INFINITY {
		s = -1
	}
	h := float64(rl.Max)
	if rl.Max == sysRLIM_INFINITY {
		h = -1
	}
	return s, h, true
}
func sysSetrlimit(resource int, soft, hard float64) int {
	rl := syscall.Rlimit{}
	if soft < 0 {
		rl.Cur = sysRLIM_INFINITY
	} else {
		rl.Cur = uint64(soft)
	}
	if hard < 0 {
		rl.Max = sysRLIM_INFINITY
	} else {
		rl.Max = uint64(hard)
	}
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

// ── mmap/munmap ───────────────────────────────────────────────────────────────
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
