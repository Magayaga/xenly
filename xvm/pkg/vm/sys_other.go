//go:build !linux && !darwin

package vm

import (
	"syscall"
	"time"
)

func sysStatMtime(st syscall.Stat_t) float64 { return 0 }
func sysStatAtime(st syscall.Stat_t) float64 { return 0 }
func sysFdatasync(fd int) error               { return syscall.Fsync(fd) }

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

type sysPollFd struct {
	Fd      int32
	Events  int16
	Revents int16
}

func sysPoll(fds []sysPollFd, timeout int) (int, error) { return -1, nil }

func sysSysinfo() (totalPages, freePages uint64, err error) { return 0, 0, nil }

func sysMonoNow() int64 { return time.Now().UnixNano() }

func sysUname() []string {
	return []string{"unknown", "unknown", "unknown", "unknown", "unknown"}
}
