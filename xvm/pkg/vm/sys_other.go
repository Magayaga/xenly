//go:build !linux && !darwin

package vm

import (
	"syscall"
	"time"
	"unsafe"
)

func sysStatMtime(st syscall.Stat_t) float64 { return 0 }
func sysStatAtime(st syscall.Stat_t) float64 { return 0 }
func sysFdatasync(fd int) error               { return syscall.Fsync(fd) }
func sysFork() (int, error)                   { return -1, syscall.ENOSYS }

func sysClockRealtime() (int64, int64) {
	now := time.Now(); return now.Unix(), int64(now.Nanosecond())
}
func sysClockMonotonic() (int64, int64) {
	now := time.Now(); return now.Unix(), int64(now.Nanosecond())
}
func sysClockProcess() (int64, int64) {
	now := time.Now(); return now.Unix(), int64(now.Nanosecond())
}
func sysNanosleep(sec, nsec int64) int {
	time.Sleep(time.Duration(sec)*time.Second + time.Duration(nsec)*time.Nanosecond)
	return 0
}
func sysGettimeofday() (int64, int64) {
	now := time.Now(); return now.Unix(), int64(now.Nanosecond() / 1000)
}

type sysPollFd struct {
	Fd      int32
	Events  int16
	Revents int16
}

func sysPoll(fds []sysPollFd, timeout int) (int, error) { return -1, syscall.ENOSYS }

const (
	sysPOLLIN  = 0x0001
	sysPOLLOUT = 0x0004
	sysPOLLERR = 0x0008
	sysPOLLHUP = 0x0010
)

const sysMAP_ANON = 0x20 // MAP_ANONYMOUS on Linux; reasonable default

func sysFcntlLock(fd, lockType int) int    { return -1 }
func sysFcntlGetfl(fd int) int             { return -1 }
func sysFcntlSetfl(fd, flags int) int      { return -1 }
func sysSysinfo() (uint64, uint64, error)  { return 0, 0, nil }
func sysUname() []string {
	return []string{"unknown", "unknown", "unknown", "unknown", "unknown"}
}

func sysGetrlimit(resource int) (soft, hard float64, ok bool) { return 0, 0, false }
func sysSetrlimit(resource int, soft, hard float64) int       { return -1 }

const (
	sysRLIMIT_CPU    = 0
	sysRLIMIT_FSIZE  = 1
	sysRLIMIT_STACK  = 3
	sysRLIMIT_NOFILE = 7
	sysRLIMIT_AS     = 9
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
