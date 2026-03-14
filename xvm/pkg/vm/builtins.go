/*
 * XENLY - Xenly Virtual Machine (XVM)
 * VM: Built-in standard library functions
 */
package vm

import (
	"fmt"
	"io"
	"math"
	"math/rand"
	"os"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"time"
)

// stdout is the writer used by print/OP_PRINT. Defaults to os.Stdout.
// SetStdout replaces it (e.g. with a bufio.Writer for Windows flushing).
var stdout io.Writer = os.Stdout

// SetStdout replaces the output writer used by all print operations.
func SetStdout(w io.Writer) { stdout = w }

// RegisterBuiltins installs all built-in functions and modules into env.
func RegisterBuiltins(env *Env) {
	// ── Core ──────────────────────────────────────────────────────────────
	native(env, "print", builtinPrint)
	native(env, "println", builtinPrint)
	native(env, "input", builtinInput)
	native(env, "len", builtinLen)
	native(env, "type", builtinType)
	native(env, "str", builtinStr)
	native(env, "num", builtinNum)
	native(env, "bool", builtinBool)
	native(env, "range", builtinRange)
	native(env, "sleep", builtinSleep)
	native(env, "exit", builtinExit)
	native(env, "assert", builtinAssert)
	native(env, "error", builtinError)

	// ── Math module ────────────────────────────────────────────────────────
	mathObj := Object(map[string]*Value{
		"PI":  Number(math.Pi),
		"E":   Number(math.E),
		"TAU": Number(2 * math.Pi),
		"INF": Number(math.Inf(1)),
		"NAN": Number(math.NaN()),
		"abs": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Abs(a[0].NumVal)), nil
		}),
		"sqrt": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Sqrt(a[0].NumVal)), nil
		}),
		"cbrt": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Cbrt(a[0].NumVal)), nil
		}),
		"pow": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			return Number(math.Pow(a[0].NumVal, a[1].NumVal)), nil
		}),
		"exp": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(1), nil
			}
			return Number(math.Exp(a[0].NumVal)), nil
		}),
		"log": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Log(a[0].NumVal)), nil
		}),
		"log2": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Log2(a[0].NumVal)), nil
		}),
		"log10": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Log10(a[0].NumVal)), nil
		}),
		"sin": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Sin(a[0].NumVal)), nil
		}),
		"cos": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(1), nil
			}
			return Number(math.Cos(a[0].NumVal)), nil
		}),
		"tan": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Tan(a[0].NumVal)), nil
		}),
		"asin": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Asin(a[0].NumVal)), nil
		}),
		"acos": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Acos(a[0].NumVal)), nil
		}),
		"atan": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Atan(a[0].NumVal)), nil
		}),
		"atan2": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			return Number(math.Atan2(a[0].NumVal, a[1].NumVal)), nil
		}),
		"sinh": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Sinh(a[0].NumVal)), nil
		}),
		"cosh": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(1), nil
			}
			return Number(math.Cosh(a[0].NumVal)), nil
		}),
		"tanh": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Tanh(a[0].NumVal)), nil
		}),
		"floor": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Floor(a[0].NumVal)), nil
		}),
		"ceil": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Ceil(a[0].NumVal)), nil
		}),
		"round": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Round(a[0].NumVal)), nil
		}),
		"trunc": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(math.Trunc(a[0].NumVal)), nil
		}),
		"sign": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			v := a[0].NumVal
			if v == 0 {
				return Number(0), nil
			}
			if v > 0 {
				return Number(1), nil
			}
			return Number(-1), nil
		}),
		"min": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(math.Inf(1)), nil
			}
			m := a[0].NumVal
			for _, v := range a[1:] {
				if v.NumVal < m {
					m = v.NumVal
				}
			}
			return Number(m), nil
		}),
		"max": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(math.Inf(-1)), nil
			}
			m := a[0].NumVal
			for _, v := range a[1:] {
				if v.NumVal > m {
					m = v.NumVal
				}
			}
			return Number(m), nil
		}),
		"random": builtin(func(a []*Value) (*Value, error) {
			return Number(rand.Float64()), nil
		}),
		"isNaN": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return False(), nil
			}
			return Bool(math.IsNaN(a[0].NumVal)), nil
		}),
		"isFinite": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return False(), nil
			}
			return Bool(!math.IsInf(a[0].NumVal, 0) && !math.IsNaN(a[0].NumVal)), nil
		}),
		"hypot": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			return Number(math.Hypot(a[0].NumVal, a[1].NumVal)), nil
		}),
		"clamp": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 {
				return Null(), nil
			}
			v, lo, hi := a[0].NumVal, a[1].NumVal, a[2].NumVal
			return Number(math.Max(lo, math.Min(hi, v))), nil
		}),
		"lerp": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 {
				return Null(), nil
			}
			return Number(a[0].NumVal + (a[1].NumVal-a[0].NumVal)*a[2].NumVal), nil
		}),
		"fmod": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(math.NaN()), nil
			}
			return Number(math.Mod(a[0].NumVal, a[1].NumVal)), nil
		}),
		"gcd": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			x, y := math.Abs(a[0].NumVal), math.Abs(a[1].NumVal)
			for y != 0 {
				x, y = y, math.Mod(x, y)
			}
			return Number(x), nil
		}),
		"lcm": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			x, y := math.Abs(a[0].NumVal), math.Abs(a[1].NumVal)
			if x == 0 || y == 0 {
				return Number(0), nil
			}
			gcd := x
			for tmp := y; tmp != 0; gcd, tmp = tmp, math.Mod(gcd, tmp) {
			}
			return Number(x * y / gcd), nil
		}),
	})
	env.Define("math", mathObj, true)
	env.Define("Math", mathObj, true) // alias

	// ── String module ─────────────────────────────────────────────────────
	strObj := Object(map[string]*Value{
		"fromCharCode": builtin(func(a []*Value) (*Value, error) {
			var sb strings.Builder
			for _, v := range a {
				sb.WriteRune(rune(int(v.NumVal)))
			}
			return String(sb.String()), nil
		}),
		"format": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(fmt.Sprint(a[0].String())), nil
		}),
	})
	env.Define("str", strObj, true)
	env.Define("String", strObj, true)

	// ── Array module ──────────────────────────────────────────────────────
	arrObj := Object(map[string]*Value{
		"isArray": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return False(), nil
			}
			return Bool(a[0].Tag == TypeArray), nil
		}),
		"from": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Array(nil), nil
			}
			if a[0].Tag == TypeArray {
				return a[0], nil
			}
			if a[0].Tag == TypeString {
				runes := []rune(a[0].StrVal)
				items := make([]*Value, len(runes))
				for i, r := range runes {
					items[i] = String(string(r))
				}
				return Array(items), nil
			}
			return Array([]*Value{a[0]}), nil
		}),
		"of": builtin(func(a []*Value) (*Value, error) {
			return Array(a), nil
		}),
		"sort": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Null(), nil
			}
			arr := a[0].ArrayVal
			sort.Slice(arr, func(i, j int) bool {
				if arr[i].Tag == TypeNumber && arr[j].Tag == TypeNumber {
					return arr[i].NumVal < arr[j].NumVal
				}
				return arr[i].String() < arr[j].String()
			})
			return a[0], nil
		}),
	})
	env.Define("Array", arrObj, true)

	// ── sys module ──────────────────────────────────────────────────────
	registerSysModule(env)

	// ── json module ───────────────────────────────────────────────────────
	jsonObj := Object(map[string]*Value{
		"stringify": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String("null"), nil
			}
			return String(jsonStringify(a[0])), nil
		}),
		"parse": builtin(func(a []*Value) (*Value, error) {
			// Minimal JSON parser: delegate to string representation for now
			return String(a[0].String()), nil
		}),
	})
	env.Define("json", jsonObj, true)
	env.Define("JSON", jsonObj, true)

	// ── time module ───────────────────────────────────────────────────────
	timeObj := Object(map[string]*Value{
		"now": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(time.Now().UnixMilli())), nil
		}),
		"sleep": builtin(func(a []*Value) (*Value, error) {
			if len(a) > 0 {
				time.Sleep(time.Duration(a[0].NumVal) * time.Millisecond)
			}
			return Null(), nil
		}),
		"format": builtin(func(a []*Value) (*Value, error) {
			return String(time.Now().Format("2006-01-02 15:04:05")), nil
		}),
	})
	env.Define("time", timeObj, true)
	env.Define("Time", timeObj, true)

	// ── number parsing ────────────────────────────────────────────────────
	native(env, "parseInt", func(args []*Value) (*Value, error) {
		if len(args) == 0 {
			return Number(math.NaN()), nil
		}
		base := 10
		if len(args) > 1 {
			base = int(args[1].NumVal)
		}
		n, err := strconv.ParseInt(strings.TrimSpace(args[0].String()), base, 64)
		if err != nil {
			return Number(math.NaN()), nil
		}
		return Number(float64(n)), nil
	})
	native(env, "parseFloat", func(args []*Value) (*Value, error) {
		if len(args) == 0 {
			return Number(math.NaN()), nil
		}
		n, err := strconv.ParseFloat(strings.TrimSpace(args[0].String()), 64)
		if err != nil {
			return Number(math.NaN()), nil
		}
		return Number(n), nil
	})
	native(env, "isNaN", func(args []*Value) (*Value, error) {
		if len(args) == 0 {
			return True(), nil
		}
		return Bool(math.IsNaN(args[0].NumVal)), nil
	})
	native(env, "isFinite", func(args []*Value) (*Value, error) {
		if len(args) == 0 {
			return False(), nil
		}
		v := args[0].NumVal
		return Bool(!math.IsInf(v, 0) && !math.IsNaN(v)), nil
	})

	// Seed random
	rand.Seed(time.Now().UnixNano())

	// ── array module (Xenly stdlib functional API) ───────────────────────
	arrayObj := Object(map[string]*Value{
		// array.of(a, b, c, ...) → [a, b, c]
		"of": builtin(func(a []*Value) (*Value, error) {
			items := make([]*Value, len(a))
			copy(items, a)
			return Array(items), nil
		}),
		// array.empty() → []
		"empty": builtin(func(a []*Value) (*Value, error) {
			return Array(nil), nil
		}),
		// array.range(end) | array.range(start, end) | array.range(start, end, step) → array
		"range": builtin(func(a []*Value) (*Value, error) {
			var start, end, step float64 = 0, 0, 1
			switch len(a) {
			case 0:
				return Array(nil), nil
			case 1:
				end = a[0].NumVal
			case 2:
				start, end = a[0].NumVal, a[1].NumVal
			default:
				start, end, step = a[0].NumVal, a[1].NumVal, a[2].NumVal
			}
			if step == 0 {
				return Array(nil), nil
			}
			var items []*Value
			if step > 0 {
				for v := start; v < end; v += step {
					items = append(items, Number(v))
				}
			} else {
				for v := start; v > end; v += step {
					items = append(items, Number(v))
				}
			}
			return Array(items), nil
		}),
		// array.len(arr) → number
		"len": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Number(0), nil
			}
			return Number(float64(len(a[0].ArrayVal))), nil
		}),
		// array.get(arr, idx) → value
		"get": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return Null(), nil
			}
			idx := int(a[1].NumVal)
			arr := a[0].ArrayVal
			if idx < 0 {
				idx = len(arr) + idx
			}
			if idx < 0 || idx >= len(arr) {
				return Null(), nil
			}
			return arr[idx], nil
		}),
		// array.set(arr, idx, val) → arr  (mutates in-place)
		"set": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 || a[0].Tag != TypeArray {
				return Null(), nil
			}
			idx := int(a[1].NumVal)
			arr := a[0].ArrayVal
			if idx < 0 {
				idx = len(arr) + idx
			}
			if idx >= 0 && idx < len(arr) {
				arr[idx] = a[2]
			}
			return a[0], nil
		}),
		// array.push(arr, val) → arr  (mutates in-place, returns array for chaining)
		"push": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return Null(), nil
			}
			a[0].ArrayVal = append(a[0].ArrayVal, a[1])
			return a[0], nil
		}),
		// array.pop(arr) → last element (mutates)
		"pop": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray || len(a[0].ArrayVal) == 0 {
				return Null(), nil
			}
			n := len(a[0].ArrayVal) - 1
			v := a[0].ArrayVal[n]
			a[0].ArrayVal = a[0].ArrayVal[:n]
			return v, nil
		}),
		// array.shift(arr) → first element (mutates)
		"shift": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray || len(a[0].ArrayVal) == 0 {
				return Null(), nil
			}
			v := a[0].ArrayVal[0]
			a[0].ArrayVal = a[0].ArrayVal[1:]
			return v, nil
		}),
		// array.concat(a, b) → new array
		"concat": builtin(func(a []*Value) (*Value, error) {
			var items []*Value
			for _, arr := range a {
				if arr.Tag == TypeArray {
					items = append(items, arr.ArrayVal...)
				}
			}
			return Array(items), nil
		}),
		// array.slice(arr, start, end?) → new array
		"slice": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Array(nil), nil
			}
			arr := a[0].ArrayVal
			start, end := 0, len(arr)
			if len(a) > 1 {
				start = int(a[1].NumVal)
			}
			if len(a) > 2 {
				end = int(a[2].NumVal)
			}
			if start < 0 {
				start = len(arr) + start
			}
			if end < 0 {
				end = len(arr) + end
			}
			if start < 0 {
				start = 0
			}
			if end > len(arr) {
				end = len(arr)
			}
			result := make([]*Value, end-start)
			copy(result, arr[start:end])
			return Array(result), nil
		}),
		// array.join(arr, sep?) → string
		"join": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return String(""), nil
			}
			sep := ","
			if len(a) > 1 {
				sep = a[1].String()
			}
			parts := make([]string, len(a[0].ArrayVal))
			for i, v := range a[0].ArrayVal {
				parts[i] = v.String()
			}
			return String(strings.Join(parts, sep)), nil
		}),
		// array.reverse(arr) → arr (mutates)
		"reverse": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Null(), nil
			}
			arr := a[0].ArrayVal
			for i, j := 0, len(arr)-1; i < j; i, j = i+1, j-1 {
				arr[i], arr[j] = arr[j], arr[i]
			}
			return a[0], nil
		}),
		// array.contains(arr, val) → bool
		"contains": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return False(), nil
			}
			for _, v := range a[0].ArrayVal {
				if v.Equal(a[1]) {
					return True(), nil
				}
			}
			return False(), nil
		}),
		// array.indexOf(arr, val) → number
		"indexOf": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return Number(-1), nil
			}
			for i, v := range a[0].ArrayVal {
				if v.Equal(a[1]) {
					return Number(float64(i)), nil
				}
			}
			return Number(-1), nil
		}),
		// array.sum(arr) → number
		"sum": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Number(0), nil
			}
			var s float64
			for _, v := range a[0].ArrayVal {
				if v.Tag == TypeNumber {
					s += v.NumVal
				}
			}
			return Number(s), nil
		}),
		// array.isArray(val) → bool
		"isArray": builtin(func(a []*Value) (*Value, error) {
			return Bool(len(a) > 0 && a[0].Tag == TypeArray), nil
		}),
		// array.create(len, fill?) → new array of length n
		"create": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Array(nil), nil
			}
			n := int(a[0].NumVal)
			fill := Null()
			if len(a) > 1 {
				fill = a[1]
			}
			items := make([]*Value, n)
			for i := range items {
				items[i] = fill
			}
			return Array(items), nil
		}),
	})
	env.Define("array", arrayObj, true)

	// ── io module ─────────────────────────────────────────────────────────
	ioObj := Object(map[string]*Value{
		// io.write(v, ...) — print without newline
		"write": builtin(func(a []*Value) (*Value, error) {
			for _, v := range a {
				fmt.Fprint(stdout, v.String())
			}
			return Null(), nil
		}),
		// io.writeln(v?, ...) — print with newline
		"writeln": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				fmt.Fprintln(stdout)
			} else {
				parts := make([]string, len(a))
				for i, v := range a {
					parts[i] = v.String()
				}
				fmt.Fprintln(stdout, strings.Join(parts, " "))
			}
			return Null(), nil
		}),
		// io.readln(prompt?) — read a line from stdin
		"readln": builtin(func(a []*Value) (*Value, error) {
			if len(a) > 0 {
				fmt.Fprint(stdout, a[0].String())
				if f, ok := stdout.(interface{ Flush() error }); ok {
					f.Flush()
				}
			}
			var line string
			fmt.Scanln(&line)
			return String(line), nil
		}),
		// io.print(v, ...) — same as print statement
		"print": builtin(func(a []*Value) (*Value, error) {
			parts := make([]string, len(a))
			for i, v := range a {
				parts[i] = v.String()
			}
			fmt.Fprintln(stdout, strings.Join(parts, " "))
			return Null(), nil
		}),
	})
	env.Define("io", ioObj, true)

	// ── string module ─────────────────────────────────────────────────────
	stringObj := Object(map[string]*Value{
		"len": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(float64(len([]rune(a[0].String())))), nil
		}),
		"toString": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(a[0].String()), nil
		}),
		"toNumber": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			v, err := strconv.ParseFloat(strings.TrimSpace(a[0].String()), 64)
			if err != nil {
				return Number(0), nil
			}
			return Number(v), nil
		}),
		"upper": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(strings.ToUpper(a[0].String())), nil
		}),
		"lower": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(strings.ToLower(a[0].String())), nil
		}),
		"contains": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return False(), nil
			}
			return Bool(strings.Contains(a[0].String(), a[1].String())), nil
		}),
		"startsWith": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return False(), nil
			}
			return Bool(strings.HasPrefix(a[0].String(), a[1].String())), nil
		}),
		"endsWith": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return False(), nil
			}
			return Bool(strings.HasSuffix(a[0].String(), a[1].String())), nil
		}),
		"indexOf": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(-1), nil
			}
			return Number(float64(strings.Index(a[0].String(), a[1].String()))), nil
		}),
		"lastIndexOf": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(-1), nil
			}
			return Number(float64(strings.LastIndex(a[0].String(), a[1].String()))), nil
		}),
		"charAt": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return String(""), nil
			}
			runes := []rune(a[0].String())
			idx := int(a[1].NumVal)
			if idx < 0 {
				idx = len(runes) + idx
			}
			if idx < 0 || idx >= len(runes) {
				return String(""), nil
			}
			return String(string(runes[idx])), nil
		}),
		"charCodeAt": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			runes := []rune(a[0].String())
			idx := int(a[1].NumVal)
			if idx < 0 {
				idx = len(runes) + idx
			}
			if idx < 0 || idx >= len(runes) {
				return Number(math.NaN()), nil
			}
			return Number(float64(runes[idx])), nil
		}),
		"fromCharCode": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(string(rune(int(a[0].NumVal)))), nil
		}),
		"repeat": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return String(""), nil
			}
			return String(strings.Repeat(a[0].String(), int(a[1].NumVal))), nil
		}),
		"reverse": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			runes := []rune(a[0].String())
			for i, j := 0, len(runes)-1; i < j; i, j = i+1, j-1 {
				runes[i], runes[j] = runes[j], runes[i]
			}
			return String(string(runes)), nil
		}),
		"trim": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(strings.TrimSpace(a[0].String())), nil
		}),
		"trimStart": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(strings.TrimLeftFunc(a[0].String(), func(r rune) bool { return r == ' ' || r == '\t' || r == '\n' || r == '\r' })), nil
		}),
		"trimEnd": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(strings.TrimRightFunc(a[0].String(), func(r rune) bool { return r == ' ' || r == '\t' || r == '\n' || r == '\r' })), nil
		}),
		"replace": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 {
				return safeArg(a, 0), nil
			}
			return String(strings.Replace(a[0].String(), a[1].String(), a[2].String(), 1)), nil
		}),
		"replaceAll": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 {
				return safeArg(a, 0), nil
			}
			return String(strings.ReplaceAll(a[0].String(), a[1].String(), a[2].String())), nil
		}),
		"substr": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return String(""), nil
			}
			runes := []rune(a[0].String())
			start := int(a[1].NumVal)
			if start < 0 {
				start = 0
			}
			end := len(runes)
			if len(a) >= 3 {
				end = start + int(a[2].NumVal)
			}
			if end > len(runes) {
				end = len(runes)
			}
			if start >= end {
				return String(""), nil
			}
			return String(string(runes[start:end])), nil
		}),
		"slice": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return String(""), nil
			}
			runes := []rune(a[0].String())
			start := int(a[1].NumVal)
			end := len(runes)
			if len(a) >= 3 {
				end = int(a[2].NumVal)
			}
			if start < 0 {
				start = len(runes) + start
			}
			if end < 0 {
				end = len(runes) + end
			}
			if start < 0 {
				start = 0
			}
			if end > len(runes) {
				end = len(runes)
			}
			if start >= end {
				return String(""), nil
			}
			return String(string(runes[start:end])), nil
		}),
		"padStart": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return safeArg(a, 0), nil
			}
			s := a[0].String()
			n := int(a[1].NumVal)
			pad := " "
			if len(a) >= 3 {
				pad = a[2].String()
			}
			for len([]rune(s)) < n {
				s = pad + s
			}
			runes := []rune(s)
			if len(runes) > n {
				s = string(runes[len(runes)-n:])
			}
			return String(s), nil
		}),
		"padEnd": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return safeArg(a, 0), nil
			}
			s := a[0].String()
			n := int(a[1].NumVal)
			pad := " "
			if len(a) >= 3 {
				pad = a[2].String()
			}
			for len([]rune(s)) < n {
				s = s + pad
			}
			runes := []rune(s)
			if len(runes) > n {
				s = string(runes[:n])
			}
			return String(s), nil
		}),
		"split": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Array(nil), nil
			}
			parts := strings.Split(a[0].String(), a[1].String())
			items := make([]*Value, len(parts))
			for i, p := range parts {
				items[i] = String(p)
			}
			return Array(items), nil
		}),
		"join": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			// string.join(arr, sep)
			if a[0].Tag != TypeArray {
				return String(""), nil
			}
			sep := ""
			if len(a) > 1 {
				sep = a[1].String()
			}
			parts := make([]string, len(a[0].ArrayVal))
			for i, v := range a[0].ArrayVal {
				parts[i] = v.String()
			}
			return String(strings.Join(parts, sep)), nil
		}),
		"format": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			// Simple: string.format(fmt, args...) — basic %s/%d/%f substitution
			result := a[0].String()
			for _, arg := range a[1:] {
				if idx := strings.IndexByte(result, '%'); idx >= 0 && idx+1 < len(result) {
					result = result[:idx] + arg.String() + result[idx+2:]
				}
			}
			return String(result), nil
		}),
	})
	env.Define("string", stringObj, true)

	// ── os module (aliases to sys) ─────────────────────────────────────────
	osObj := Object(map[string]*Value{
		"platform": builtin(func(a []*Value) (*Value, error) {
			return String(runtime.GOOS), nil
		}),
		"exit": builtin(func(a []*Value) (*Value, error) {
			code := 0
			if len(a) > 0 {
				code = int(a[0].NumVal)
			}
			os.Exit(code)
			return Null(), nil
		}),
		"args": builtin(func(a []*Value) (*Value, error) {
			items := make([]*Value, len(os.Args))
			for i, arg := range os.Args {
				items[i] = String(arg)
			}
			return Array(items), nil
		}),
		"env": builtin(func(a []*Value) (*Value, error) {
			if len(a) > 0 {
				return String(os.Getenv(a[0].String())), nil
			}
			return Null(), nil
		}),
		"getenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Null(), nil
			}
			return String(os.Getenv(a[0].String())), nil
		}),
		"getcwd": builtin(func(a []*Value) (*Value, error) {
			cwd, _ := os.Getwd()
			return String(cwd), nil
		}),
		"pid": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(os.Getpid())), nil
		}),
	})
	env.Define("os", osObj, true)
}

// ─── helpers ──────────────────────────────────────────────────────────────────

func native(env *Env, name string, fn BuiltinFn) {
	env.Define(name, &Value{Tag: TypeBuiltin, BuiltinV: fn}, false)
}

func builtin(fn BuiltinFn) *Value {
	return &Value{Tag: TypeBuiltin, BuiltinV: fn}
}

func safeArg(args []*Value, i int) *Value {
	if i < len(args) {
		return args[i]
	}
	return Null()
}

// ─── core built-ins ───────────────────────────────────────────────────────────

func builtinPrint(args []*Value) (*Value, error) {
	parts := make([]string, len(args))
	for i, a := range args {
		parts[i] = a.String()
	}
	fmt.Fprintln(stdout, strings.Join(parts, " "))
	return Null(), nil
}

func builtinInput(args []*Value) (*Value, error) {
	if len(args) > 0 {
		fmt.Fprint(stdout, args[0].String())
		if f, ok := stdout.(interface{ Flush() error }); ok {
			f.Flush()
		}
	}
	var line string
	fmt.Scanln(&line)
	return String(line), nil
}

func builtinLen(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return Number(0), nil
	}
	a := args[0]
	switch a.Tag {
	case TypeArray:
		return Number(float64(len(a.ArrayVal))), nil
	case TypeString:
		return Number(float64(len([]rune(a.StrVal)))), nil
	case TypeObject:
		return Number(float64(len(a.ObjVal))), nil
	}
	return Number(0), nil
}

func builtinType(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return String("null"), nil
	}
	return String(args[0].TypeName()), nil
}

func builtinStr(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return String(""), nil
	}
	return String(args[0].String()), nil
}

func builtinNum(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return Number(0), nil
	}
	a := args[0]
	switch a.Tag {
	case TypeNumber:
		return a, nil
	case TypeBool:
		if a.BoolVal {
			return Number(1), nil
		}
		return Number(0), nil
	case TypeString:
		n, err := strconv.ParseFloat(strings.TrimSpace(a.StrVal), 64)
		if err != nil {
			return Number(math.NaN()), nil
		}
		return Number(n), nil
	}
	return Number(math.NaN()), nil
}

func builtinBool(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return False(), nil
	}
	return Bool(args[0].Truthy()), nil
}

func builtinRange(args []*Value) (*Value, error) {
	var start, end, step float64
	switch len(args) {
	case 1:
		start, end, step = 0, args[0].NumVal, 1
	case 2:
		start, end, step = args[0].NumVal, args[1].NumVal, 1
	case 3:
		start, end, step = args[0].NumVal, args[1].NumVal, args[2].NumVal
	default:
		return Array(nil), nil
	}
	if step == 0 {
		return Array(nil), fmt.Errorf("range step cannot be zero")
	}
	var items []*Value
	for i := start; (step > 0 && i < end) || (step < 0 && i > end); i += step {
		items = append(items, Number(i))
	}
	return Array(items), nil
}

func builtinSleep(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return Null(), nil
	}
	time.Sleep(time.Duration(args[0].NumVal) * time.Millisecond)
	return Null(), nil
}

func builtinExit(args []*Value) (*Value, error) {
	code := 0
	if len(args) > 0 {
		code = int(args[0].NumVal)
	}
	os.Exit(code)
	return Null(), nil
}

func builtinAssert(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return Null(), nil
	}
	if !args[0].Truthy() {
		msg := "assertion failed"
		if len(args) > 1 {
			msg = args[1].String()
		}
		return Null(), fmt.Errorf("%s", msg)
	}
	return Null(), nil
}

func builtinError(args []*Value) (*Value, error) {
	msg := "error"
	if len(args) > 0 {
		msg = args[0].String()
	}
	return Null(), fmt.Errorf("%s", msg)
}

// ─── JSON stringify ────────────────────────────────────────────────────────────

func jsonStringify(v *Value) string {
	if v == nil || v.Tag == TypeNull {
		return "null"
	}
	switch v.Tag {
	case TypeBool:
		if v.BoolVal {
			return "true"
		}
		return "false"
	case TypeNumber:
		if math.IsNaN(v.NumVal) || math.IsInf(v.NumVal, 0) {
			return "null"
		}
		return strconv.FormatFloat(v.NumVal, 'f', -1, 64)
	case TypeString:
		return strconv.Quote(v.StrVal)
	case TypeArray:
		parts := make([]string, len(v.ArrayVal))
		for i, item := range v.ArrayVal {
			parts[i] = jsonStringify(item)
		}
		return "[" + strings.Join(parts, ",") + "]"
	case TypeObject:
		parts := make([]string, 0, len(v.ObjVal))
		for k, val := range v.ObjVal {
			parts = append(parts, fmt.Sprintf("%s:%s", strconv.Quote(k), jsonStringify(val)))
		}
		return "{" + strings.Join(parts, ",") + "}"
	}
	return "null"
}

func goPlatform() string {
	return runtime.GOOS + "/" + runtime.GOARCH
}
