//go:build darwin

package vm

import (
	"syscall"
	"time"
	"unsafe"
)

func sysStatMtime(st syscall.Stat_t) float64 { return float64(st.Mtimespec.Sec) }
func sysStatAtime(st syscall.Stat_t) float64 { return float64(st.Atimespec.Sec) }

// fdatasync on macOS: use fcntl(F_FULLFSYNC) = 51 for true durability,
// fall back to fsync if that fails.
func sysFdatasync(fd int) error {
	const F_FULLFSYNC = 51
	_, _, errno := syscall.Syscall(syscall.SYS_FCNTL, uintptr(fd), uintptr(F_FULLFSYNC), 0)
	if errno != 0 {
		return syscall.Fsync(fd)
	}
	return nil
}

// macOS syscall package doesn't expose ClockGettime; use mach_absolute_time
// approximation via time package — accurate enough for Xenly programs.
func sysClockRealtime() (int64, int64) {
	now := time.Now()
	return now.Unix(), int64(now.Nanosecond())
}

func sysClockMonotonic() (int64, int64) {
	// time.Now() uses monotonic clock internally on macOS.
	now := time.Now()
	return now.Unix(), int64(now.Nanosecond())
}

func sysClockProcess() (int64, int64) {
	now := time.Now()
	return now.Unix(), int64(now.Nanosecond())
}

func sysNanosleep(sec, nsec int64) int {
	d := time.Duration(sec)*time.Second + time.Duration(nsec)*time.Nanosecond
	time.Sleep(d)
	return 0
}

// macOS syscall package uses Timeval with int32 Usec on some arch.
func sysGettimeofday() (int64, int64) {
	now := time.Now()
	return now.Unix(), int64(now.Nanosecond() / 1000)
}

// sysPollFd mirrors the pollfd struct; on macOS syscall doesn't export PollFd.
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

func sysSysinfo() (totalPages, freePages uint64, err error) {
	// macOS: use sysctl hw.memsize for total RAM, vm_stat for free.
	// Simplified: return 0 to avoid CGO dependency.
	return 0, 0, nil
}

func sysMonoNow() int64 { return time.Now().UnixNano() }

func sysUname() []string {
	var u syscall.Utsname
	if err := syscall.Uname(&u); err != nil {
		return []string{"", "", "", "", ""}
	}
	// On Darwin/Go, Utsname fields are [256]int8
	toStr := func(b [256]int8) string {
		s := make([]byte, 0, 256)
		for _, c := range b {
			if c == 0 {
				break
			}
			s = append(s, byte(c))
		}
		return string(s)
	}
	return []string{toStr(u.Sysname), toStr(u.Nodename), toStr(u.Release), toStr(u.Version), toStr(u.Machine)}
}
