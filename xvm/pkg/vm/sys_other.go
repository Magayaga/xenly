/*
 * XENLY VIRTUAL MACHINE (XVM) - high-performance of the Virtual Machine
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * It is available for the Linux, macOS, and Windows operating systems.
 *
 */
//go:build !linux && !darwin && !windows

package vm

import (
	"os"
	"runtime"
	"time"
)

// registerSysModule stub for non-Linux/Darwin/Windows targets (e.g. FreeBSD, Plan9).
func registerSysModule(env *Env) {
	sysObj := Object(map[string]*Value{
		"exit": builtin(func(a []*Value) (*Value, error) {
			code := 0
			if len(a) >= 1 { code = int(a[0].NumVal) }
			os.Exit(code); return Null(), nil
		}),
		"getpid":     builtin(func(a []*Value) (*Value, error) { return Number(float64(os.Getpid())), nil }),
		"nproc":      builtin(func(a []*Value) (*Value, error) { return Number(float64(runtime.NumCPU())), nil }),
		"nproc_conf": builtin(func(a []*Value) (*Value, error) { return Number(float64(runtime.NumCPU())), nil }),
		"time":  builtin(func(a []*Value) (*Value, error) { return Number(float64(time.Now().Unix())), nil }),
		"clock": builtin(func(a []*Value) (*Value, error) { return Number(float64(time.Now().UnixNano()) / 1e9), nil }),
		"sleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) >= 1 { time.Sleep(time.Duration(a[0].NumVal) * time.Second) }
			return Number(0), nil
		}),
		"getcwd": builtin(func(a []*Value) (*Value, error) {
			cwd, err := os.Getwd()
			if err != nil { return Null(), nil }
			return String(cwd), nil
		}),
		"hostname": builtin(func(a []*Value) (*Value, error) {
			h, err := os.Hostname()
			if err != nil { return Null(), nil }
			return String(h), nil
		}),
	})
	env.Define("sys", sysObj, true)
}
