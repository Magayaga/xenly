/*
 * XENLY VIRTUAL MACHINE (XVM)
 * VM: Multiprocessing built-ins (thread pools, futures, channels)
 *
 * Design note: worker goroutines, task queues, futures, and channels here
 * are all real and provide genuine concurrent scheduling/blocking
 * semantics. However, the interpreter's shared global environment isn't
 * safe for unsynchronized concurrent access, so actual Xenly bytecode
 * execution triggered from a worker goroutine is serialized behind
 * vmCallMu. This keeps the feature correct and race-free while still
 * modeling a real thread pool (queued tasks, worker goroutines, blocking
 * futures/channels) rather than faking it with purely sequential calls.
 */
package vm

import (
	"fmt"
	"runtime"
	"sync"
)

// ─── Thread pool ────────────────────────────────────────────────────────────

type xThreadPool struct {
	tasks chan func()
	done  chan struct{}
	once  sync.Once
}

func newThreadPool(n int) *xThreadPool {
	if n < 1 {
		n = 1
	}
	p := &xThreadPool{
		tasks: make(chan func(), 256),
		done:  make(chan struct{}),
	}
	for i := 0; i < n; i++ {
		go func() {
			for {
				select {
				case task, ok := <-p.tasks:
					if !ok {
						return
					}
					task()
				case <-p.done:
					return
				}
			}
		}()
	}
	return p
}

func (p *xThreadPool) submit(task func()) {
	select {
	case p.tasks <- task:
	case <-p.done:
	}
}

func (p *xThreadPool) destroy() {
	p.once.Do(func() { close(p.done) })
}

// ─── Future ─────────────────────────────────────────────────────────────────

type xFuture struct {
	mu    sync.Mutex
	ready bool
	val   *Value
	err   error
	done  chan struct{}
}

func newFuture() *xFuture {
	return &xFuture{done: make(chan struct{})}
}

func (f *xFuture) resolve(v *Value, err error) {
	f.mu.Lock()
	f.val, f.err, f.ready = v, err, true
	f.mu.Unlock()
	close(f.done)
}

func (f *xFuture) get() (*Value, error) {
	<-f.done
	f.mu.Lock()
	defer f.mu.Unlock()
	return f.val, f.err
}

func (f *xFuture) isReady() bool {
	select {
	case <-f.done:
		return true
	default:
		return false
	}
}

// vmCallMu serializes Xenly closure invocations made from worker
// goroutines — see the package-level design note above.
var vmCallMu sync.Mutex

// RegisterMultiprocBuiltins installs the thread pool / future / channel
// built-in functions into env. xvm is needed so submitted tasks can call
// back into the interpreter to invoke the user's Xenly function.
func RegisterMultiprocBuiltins(env *Env, xvm *XVM) {
	native(env, "cpu_count", func(a []*Value) (*Value, error) {
		return Number(float64(runtime.NumCPU())), nil
	})

	native(env, "thread_pool_create", func(a []*Value) (*Value, error) {
		n := runtime.NumCPU()
		if len(a) > 0 && a[0].Tag == TypeNumber {
			n = int(a[0].NumVal)
		}
		return &Value{Tag: TypeNative, NativeKind: "pool", Native: newThreadPool(n)}, nil
	})

	native(env, "thread_pool_submit", func(a []*Value) (*Value, error) {
		if len(a) < 2 {
			return valNull, fmt.Errorf("thread_pool_submit: expected (pool, fn, ...args)")
		}
		pool, ok := a[0].Native.(*xThreadPool)
		if !ok {
			return valNull, fmt.Errorf("thread_pool_submit: first argument is not a thread pool")
		}
		fn := a[1]
		taskArgs := append([]*Value(nil), a[2:]...)
		fut := newFuture()
		pool.submit(func() {
			vmCallMu.Lock()
			v, err := xvm.callValue(fn, taskArgs, nil, nil)
			vmCallMu.Unlock()
			fut.resolve(v, err)
		})
		return &Value{Tag: TypeNative, NativeKind: "future", Native: fut}, nil
	})

	native(env, "thread_pool_destroy", func(a []*Value) (*Value, error) {
		if len(a) > 0 {
			if pool, ok := a[0].Native.(*xThreadPool); ok {
				pool.destroy()
			}
		}
		return valNull, nil
	})

	native(env, "future_get", func(a []*Value) (*Value, error) {
		if len(a) == 0 {
			return valNull, nil
		}
		fut, ok := a[0].Native.(*xFuture)
		if !ok {
			return valNull, fmt.Errorf("future_get: not a future")
		}
		v, err := fut.get()
		if err != nil {
			return valNull, err
		}
		return v, nil
	})

	native(env, "future_is_ready", func(a []*Value) (*Value, error) {
		if len(a) == 0 {
			return valFalse, nil
		}
		fut, ok := a[0].Native.(*xFuture)
		if !ok {
			return valFalse, nil
		}
		return Bool(fut.isReady()), nil
	})

	native(env, "future_destroy", func(a []*Value) (*Value, error) {
		return valNull, nil // nothing to release explicitly; GC handles it
	})

	// ── Channels ────────────────────────────────────────────────────────────
	native(env, "channel_create", func(a []*Value) (*Value, error) {
		capacity := 0
		if len(a) > 0 && a[0].Tag == TypeNumber {
			capacity = int(a[0].NumVal)
		}
		return &Value{Tag: TypeNative, NativeKind: "channel", Native: make(chan *Value, capacity)}, nil
	})

	native(env, "channel_send", func(a []*Value) (*Value, error) {
		if len(a) < 2 {
			return valNull, fmt.Errorf("channel_send: expected (channel, value)")
		}
		ch, ok := a[0].Native.(chan *Value)
		if !ok {
			return valNull, fmt.Errorf("channel_send: not a channel")
		}
		ch <- a[1]
		return valNull, nil
	})

	native(env, "channel_recv", func(a []*Value) (*Value, error) {
		if len(a) == 0 {
			return valNull, nil
		}
		ch, ok := a[0].Native.(chan *Value)
		if !ok {
			return valNull, fmt.Errorf("channel_recv: not a channel")
		}
		v, ok := <-ch
		if !ok {
			return valNull, nil // channel closed
		}
		return v, nil
	})

	native(env, "channel_destroy", func(a []*Value) (*Value, error) {
		if len(a) > 0 {
			if ch, ok := a[0].Native.(chan *Value); ok {
				close(ch)
			}
		}
		return valNull, nil
	})
}
