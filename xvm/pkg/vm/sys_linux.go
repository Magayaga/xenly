//go:build linux

package vm

import (
	"syscall"
	"time"
)

func sysStatMtime(st syscall.Stat_t) float64 { return float64(st.Mtim.Sec) }
func sysStatAtime(st syscall.Stat_t) float64 { return float64(st.Atim.Sec) }

func sysFdatasync(fd int) error { return syscall.Fdatasync(fd) }

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

type sysPollFd = syscall.PollFd

func sysPoll(fds []syscall.PollFd, timeout int) (int, error) {
	return syscall.Poll(fds, timeout)
}

func sysGettimeofday() (int64, int64) {
	var tv syscall.Timeval
	syscall.Gettimeofday(&tv)
	return tv.Sec, int64(tv.Usec)
}

func sysSysinfo() (totalPages, freePages uint64, err error) {
	var si syscall.Sysinfo_t
	if e := syscall.Sysinfo(&si); e != nil {
		return 0, 0, e
	}
	pageSize := uint64(syscall.Getpagesize())
	if pageSize == 0 {
		pageSize = 4096
	}
	total := si.Totalram * uint64(si.Unit) / pageSize
	free := si.Freeram * uint64(si.Unit) / pageSize
	return total, free, nil
}

// sysMonoNow returns monotonic nanoseconds — used as a portable fallback.
func sysMonoNow() int64 { return time.Now().UnixNano() }

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
	return []string{toStr(u.Sysname), toStr(u.Nodename), toStr(u.Release), toStr(u.Version), toStr(u.Machine)}
}
