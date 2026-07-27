/*
 * XENLY VIRTUAL MACHINE (XVM) - high-performance of the Virtual Machine
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * It is available for the Linux, macOS, and Windows operating systems.
 *
 */
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
	"path/filepath"
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
// processStart marks when this process began, for os.clock().
var processStart = time.Now()

func RegisterBuiltins(env *Env, xvm *XVM) {
	// ── Core ──────────────────────────────────────────────────────────────
	native(env, "print", builtinPrint)
	native(env, "println", builtinPrint)
	native(env, "input", builtinInput)
	native(env, "len", builtinLen)
	{
		isType := func(want ValueType) BuiltinFn {
			return func(a []*Value) (*Value, error) {
				if len(a) == 0 {
					return valFalse, nil
				}
				if a[0].Tag == want {
					return valTrue, nil
				}
				return valFalse, nil
			}
		}
		typeObj := Object(map[string]*Value{
			"typeOf":    builtin(builtinType),
			"isNumber":  builtin(isType(TypeNumber)),
			"isString":  builtin(isType(TypeString)),
			"isBoolean": builtin(isType(TypeBool)),
			"isBool":    builtin(isType(TypeBool)),
			"isArray":   builtin(isType(TypeArray)),
			"isObject":  builtin(isType(TypeObject)),
			"isNull":    builtin(isType(TypeNull)),
			"toString":  builtin(builtinStr),
			"toNumber":  builtin(builtinNum),
			"toBool":    builtin(builtinBool),
		})
		env.Define("type", typeObj, true)
	}
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
				return valFalse, nil
			}
			return Bool(math.IsNaN(a[0].NumVal)), nil
		}),
		"isFinite": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return valFalse, nil
			}
			return Bool(!math.IsInf(a[0].NumVal, 0) && !math.IsNaN(a[0].NumVal)), nil
		}),
		"isInf": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return valFalse, nil
			}
			return Bool(math.IsInf(a[0].NumVal, 0)), nil
		}),
		// math.complex(re, im?) → [re, im] represented as a 2-element array
		"complex": builtin(func(a []*Value) (*Value, error) {
			re, im := 0.0, 0.0
			if len(a) > 0 {
				re = a[0].NumVal
			}
			if len(a) > 1 {
				im = a[1].NumVal
			}
			return Array([]*Value{Number(re), Number(im)}), nil
		}),
		"complexAdd": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray || a[1].Tag != TypeArray ||
				len(a[0].ArrayVal) < 2 || len(a[1].ArrayVal) < 2 {
				return valNull, nil
			}
			r1, i1 := a[0].ArrayVal[0].NumVal, a[0].ArrayVal[1].NumVal
			r2, i2 := a[1].ArrayVal[0].NumVal, a[1].ArrayVal[1].NumVal
			return Array([]*Value{Number(r1 + r2), Number(i1 + i2)}), nil
		}),
		"complexMul": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray || a[1].Tag != TypeArray ||
				len(a[0].ArrayVal) < 2 || len(a[1].ArrayVal) < 2 {
				return valNull, nil
			}
			r1, i1 := a[0].ArrayVal[0].NumVal, a[0].ArrayVal[1].NumVal
			r2, i2 := a[1].ArrayVal[0].NumVal, a[1].ArrayVal[1].NumVal
			return Array([]*Value{Number(r1*r2 - i1*i2), Number(r1*i2 + i1*r2)}), nil
		}),
		"complexAbs": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray || len(a[0].ArrayVal) < 2 {
				return Number(0), nil
			}
			re, im := a[0].ArrayVal[0].NumVal, a[0].ArrayVal[1].NumVal
			return Number(math.Hypot(re, im)), nil
		}),
		"complexConj": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray || len(a[0].ArrayVal) < 2 {
				return valNull, nil
			}
			return Array([]*Value{Number(a[0].ArrayVal[0].NumVal), Number(-a[0].ArrayVal[1].NumVal)}), nil
		}),
		"complexPhase": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray || len(a[0].ArrayVal) < 2 {
				return Number(0), nil
			}
			return Number(math.Atan2(a[0].ArrayVal[1].NumVal, a[0].ArrayVal[0].NumVal)), nil
		}),
		"hypot": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			return Number(math.Hypot(a[0].NumVal, a[1].NumVal)), nil
		}),
		"clamp": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 {
				return valNull, nil
			}
			v, lo, hi := a[0].NumVal, a[1].NumVal, a[2].NumVal
			return Number(math.Max(lo, math.Min(hi, v))), nil
		}),
		"lerp": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 {
				return valNull, nil
			}
			return Number(a[0].NumVal + (a[1].NumVal-a[0].NumVal)*a[2].NumVal), nil
		}),
		"degrees": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(a[0].NumVal * 180.0 / math.Pi), nil
		}),
		"radians": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(a[0].NumVal * math.Pi / 180.0), nil
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
		"randomInt": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			lo, hi := int(a[0].NumVal), int(a[1].NumVal)
			if hi <= lo {
				return Number(float64(lo)), nil
			}
			return Number(float64(lo + rand.Intn(hi-lo))), nil
		}),
		// math.sum(arr) / math.product(arr) — over an array, or over varargs
		"sum": builtin(func(a []*Value) (*Value, error) {
			nums := numsFromArrayOrArgs(a)
			total := 0.0
			for _, n := range nums {
				total += n
			}
			return Number(total), nil
		}),
		"product": builtin(func(a []*Value) (*Value, error) {
			nums := numsFromArrayOrArgs(a)
			total := 1.0
			for _, n := range nums {
				total *= n
			}
			return Number(total), nil
		}),
		"mean": builtin(func(a []*Value) (*Value, error) {
			nums := numsFromArrayOrArgs(a)
			if len(nums) == 0 {
				return Number(0), nil
			}
			total := 0.0
			for _, n := range nums {
				total += n
			}
			return Number(total / float64(len(nums))), nil
		}),
		"median": builtin(func(a []*Value) (*Value, error) {
			nums := numsFromArrayOrArgs(a)
			if len(nums) == 0 {
				return Number(0), nil
			}
			sort.Float64s(nums)
			mid := len(nums) / 2
			if len(nums)%2 == 0 {
				return Number((nums[mid-1] + nums[mid]) / 2), nil
			}
			return Number(nums[mid]), nil
		}),
		"variance": builtin(func(a []*Value) (*Value, error) {
			nums := numsFromArrayOrArgs(a)
			if len(nums) == 0 {
				return Number(0), nil
			}
			mean := 0.0
			for _, n := range nums {
				mean += n
			}
			mean /= float64(len(nums))
			v := 0.0
			for _, n := range nums {
				d := n - mean
				v += d * d
			}
			return Number(v / float64(len(nums))), nil
		}),
		"stddev": builtin(func(a []*Value) (*Value, error) {
			nums := numsFromArrayOrArgs(a)
			if len(nums) == 0 {
				return Number(0), nil
			}
			mean := 0.0
			for _, n := range nums {
				mean += n
			}
			mean /= float64(len(nums))
			v := 0.0
			for _, n := range nums {
				d := n - mean
				v += d * d
			}
			return Number(math.Sqrt(v / float64(len(nums)))), nil
		}),
		"factorial": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(1), nil
			}
			n := int(a[0].NumVal)
			if n < 0 {
				return Number(math.NaN()), nil
			}
			if n > 170 {
				return Number(math.Inf(1)), nil
			}
			result := 1.0
			for i := 2; i <= n; i++ {
				result *= float64(i)
			}
			return Number(result), nil
		}),
		"combinations": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			n, k := int(a[0].NumVal), int(a[1].NumVal)
			if k > n || k < 0 || n < 0 {
				return Number(0), nil
			}
			if k == 0 || k == n {
				return Number(1), nil
			}
			if k > n-k {
				k = n - k
			}
			result := 1.0
			for i := 0; i < k; i++ {
				result *= float64(n - i)
				result /= float64(i + 1)
			}
			return Number(result), nil
		}),
		"permutations": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(0), nil
			}
			n, k := int(a[0].NumVal), int(a[1].NumVal)
			if k > n || k < 0 || n < 0 {
				return Number(0), nil
			}
			result := 1.0
			for i := 0; i < k; i++ {
				result *= float64(n - i)
			}
			return Number(result), nil
		}),
	})
	env.Define("math", mathObj, true)
	env.Define("Math", mathObj, true) // alias

	// ── Array module ──────────────────────────────────────────────────────
	arrObj := Object(map[string]*Value{
		"isArray": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return valFalse, nil
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
				return valNull, nil
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
			return valNull, nil
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
			return valTrue, nil
		}
		return Bool(math.IsNaN(args[0].NumVal)), nil
	})
	native(env, "isFinite", func(args []*Value) (*Value, error) {
		if len(args) == 0 {
			return valFalse, nil
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
			for i, v := range a {
				// Only clone mutable types for performance
				if v.Tag == TypeArray || v.Tag == TypeObject {
					items[i] = v.Clone()
				} else {
					items[i] = v
				}
			}
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
		// array.length(arr) → number (alias for len)
		"length": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Number(0), nil
			}
			return Number(float64(len(a[0].ArrayVal))), nil
		}),
		// array.fill(arr, value) → arr, mutated so every element is value
		"fill": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return valNull, nil
			}
			for i := range a[0].ArrayVal {
				a[0].ArrayVal[i] = a[1]
			}
			return a[0], nil
		}),
		// array.get(arr, idx) → value
		"get": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return valNull, nil
			}
			idx := int(a[1].NumVal)
			arr := a[0].ArrayVal
			if idx < 0 {
				idx = len(arr) + idx
			}
			if idx < 0 || idx >= len(arr) {
				return valNull, nil
			}
			return arr[idx], nil
		}),
		// array.set(arr, idx, val) → arr  (mutates in-place)
		"set": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 3 || a[0].Tag != TypeArray {
				return valNull, nil
			}
			idx := int(a[1].NumVal)
			arr := a[0].ArrayVal
			if idx < 0 {
				idx = len(arr) + idx
			}
			if idx >= 0 && idx < len(arr) {
				arr[idx] = a[2].Clone()
			}
			return a[0], nil
		}),
		// array.push(arr, val...) → arr  (mutates in-place, returns array for chaining)
		"push": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 || a[0].Tag != TypeArray {
				return valNull, nil
			}
			// Performance optimization: only clone when necessary
			for i := 1; i < len(a); i++ {
				val := a[i]
				// Only clone mutable types to prevent reference sharing issues
				if val.Tag == TypeArray || val.Tag == TypeObject {
					a[0].ArrayVal = append(a[0].ArrayVal, val.Clone())
				} else {
					a[0].ArrayVal = append(a[0].ArrayVal, val)
				}
			}
			return a[0], nil
		}),
		// array.unshift(arr, val...) → arr  (mutates in-place, returns array for chaining)
		"unshift": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 1 || a[0].Tag != TypeArray {
				return valNull, nil
			}
			// Performance optimization: only clone when necessary
			clonedArgs := make([]*Value, len(a)-1)
			for i := 1; i < len(a); i++ {
				val := a[i]
				// Only clone mutable types
				if val.Tag == TypeArray || val.Tag == TypeObject {
					clonedArgs[i-1] = val.Clone()
				} else {
					clonedArgs[i-1] = val
				}
			}
			newItems := append(clonedArgs, a[0].ArrayVal...)
			a[0].ArrayVal = newItems
			return a[0], nil
		}),
		// array.pop(arr) → last element (mutates)
		"pop": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray || len(a[0].ArrayVal) == 0 {
				return valNull, nil
			}
			n := len(a[0].ArrayVal) - 1
			v := a[0].ArrayVal[n]
			a[0].ArrayVal = a[0].ArrayVal[:n]
			return v, nil
		}),
		// array.shift(arr) → first element (mutates)
		"shift": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray || len(a[0].ArrayVal) == 0 {
				return valNull, nil
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
					for _, item := range arr.ArrayVal {
						// Only clone mutable types
						if item.Tag == TypeArray || item.Tag == TypeObject {
							items = append(items, item.Clone())
						} else {
							items = append(items, item)
						}
					}
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
			for i := start; i < end; i++ {
				// Only clone mutable types for performance
				if arr[i].Tag == TypeArray || arr[i].Tag == TypeObject {
					result[i-start] = arr[i].Clone()
				} else {
					result[i-start] = arr[i]
				}
			}
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
				return valNull, nil
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
				return valFalse, nil
			}
			for _, v := range a[0].ArrayVal {
				if v.Equal(a[1]) {
					return valTrue, nil
				}
			}
			return valFalse, nil
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
			fill := valNull
			if len(a) > 1 {
				fill = a[1]
			}
			items := make([]*Value, n)
			// Performance: only clone if fill value is mutable
			if fill.Tag == TypeArray || fill.Tag == TypeObject {
				for i := range items {
					items[i] = fill.Clone()
				}
			} else {
				for i := range items {
					items[i] = fill
				}
			}
			return Array(items), nil
		}),
		// array.map(arr, fn) → new array of fn(x) for each x
		"map": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return Array(nil), nil
			}
			src := a[0].ArrayVal
			out := make([]*Value, len(src))
			for i, v := range src {
				r, err := xvm.callValue(a[1], []*Value{v}, nil, nil)
				if err != nil {
					return valNull, err
				}
				out[i] = r
			}
			return Array(out), nil
		}),
		// array.filter(arr, fn) → new array of elements where fn(x) is truthy
		"filter": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return Array(nil), nil
			}
			var out []*Value
			for _, v := range a[0].ArrayVal {
				r, err := xvm.callValue(a[1], []*Value{v}, nil, nil)
				if err != nil {
					return valNull, err
				}
				if r.Truthy() {
					out = append(out, v)
				}
			}
			return Array(out), nil
		}),
		// array.reduce(arr, fn, initial?) → accumulated value
		"reduce": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return valNull, nil
			}
			src := a[0].ArrayVal
			var acc *Value
			start := 0
			if len(a) > 2 {
				acc = a[2]
			} else {
				if len(src) == 0 {
					return valNull, nil
				}
				acc = src[0]
				start = 1
			}
			for i := start; i < len(src); i++ {
				r, err := xvm.callValue(a[1], []*Value{acc, src[i]}, nil, nil)
				if err != nil {
					return valNull, err
				}
				acc = r
			}
			return acc, nil
		}),
		// array.forEach(arr, fn) → null (side-effecting)
		"forEach": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return valNull, nil
			}
			for _, v := range a[0].ArrayVal {
				if _, err := xvm.callValue(a[1], []*Value{v}, nil, nil); err != nil {
					return valNull, err
				}
			}
			return valNull, nil
		}),
		// array.find(arr, fn) → first element where fn(x) is truthy, or null
		"find": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return valNull, nil
			}
			for _, v := range a[0].ArrayVal {
				r, err := xvm.callValue(a[1], []*Value{v}, nil, nil)
				if err != nil {
					return valNull, err
				}
				if r.Truthy() {
					return v, nil
				}
			}
			return valNull, nil
		}),
		// array.some(arr, fn) → true if any element passes fn
		"some": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return valFalse, nil
			}
			for _, v := range a[0].ArrayVal {
				r, err := xvm.callValue(a[1], []*Value{v}, nil, nil)
				if err != nil {
					return valNull, err
				}
				if r.Truthy() {
					return valTrue, nil
				}
			}
			return valFalse, nil
		}),
		// array.every(arr, fn) → true if all elements pass fn
		"every": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 || a[0].Tag != TypeArray {
				return valTrue, nil
			}
			for _, v := range a[0].ArrayVal {
				r, err := xvm.callValue(a[1], []*Value{v}, nil, nil)
				if err != nil {
					return valNull, err
				}
				if !r.Truthy() {
					return valFalse, nil
				}
			}
			return valTrue, nil
		}),
		// array.sort(arr) / array.sortDesc(arr) → new sorted array
		"sort": builtin(func(a []*Value) (*Value, error) {
			return sortArrayValue(a, false), nil
		}),
		"sortDesc": builtin(func(a []*Value) (*Value, error) {
			return sortArrayValue(a, true), nil
		}),
		// array.unique(arr) → new array with duplicates removed
		"unique": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Array(nil), nil
			}
			var out []*Value
			for _, v := range a[0].ArrayVal {
				dup := false
				for _, o := range out {
					if v.Equal(o) {
						dup = true
						break
					}
				}
				if !dup {
					out = append(out, v)
				}
			}
			return Array(out), nil
		}),
		// array.flatten(arr) → new array, one level of nested arrays flattened
		"flatten": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 || a[0].Tag != TypeArray {
				return Array(nil), nil
			}
			var out []*Value
			for _, v := range a[0].ArrayVal {
				if v.Tag == TypeArray {
					out = append(out, v.ArrayVal...)
				} else {
					out = append(out, v)
				}
			}
			return Array(out), nil
		}),
		// array.min(arr) / array.max(arr) → number
		"min": builtin(func(a []*Value) (*Value, error) {
			return minMaxArrayValue(a, true), nil
		}),
		"max": builtin(func(a []*Value) (*Value, error) {
			return minMaxArrayValue(a, false), nil
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
			return valNull, nil
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
			return valNull, nil
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
			return valNull, nil
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
		"unicodeLength": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			return Number(float64(len([]rune(a[0].String())))), nil
		}),
		"unicodeCharAt": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return String(""), nil
			}
			runes := []rune(a[0].String())
			idx := int(a[1].NumVal)
			if idx < 0 || idx >= len(runes) {
				return String(""), nil
			}
			return String(string(runes[idx])), nil
		}),
		"codePointAt": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return Number(-1), nil
			}
			runes := []rune(a[0].String())
			idx := int(a[1].NumVal)
			if idx < 0 || idx >= len(runes) {
				return Number(-1), nil
			}
			return Number(float64(runes[idx])), nil
		}),
		"fromCodePoint": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(string(rune(int(a[0].NumVal)))), nil
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
				return valFalse, nil
			}
			return Bool(strings.Contains(a[0].String(), a[1].String())), nil
		}),
		"startsWith": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return valFalse, nil
			}
			return Bool(strings.HasPrefix(a[0].String(), a[1].String())), nil
		}),
		"endsWith": builtin(func(a []*Value) (*Value, error) {
			if len(a) < 2 {
				return valFalse, nil
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
	env.Define("str", stringObj, true)
	env.Define("String", stringObj, true)

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
			return valNull, nil
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
			return valNull, nil
		}),
		"getenv": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return valNull, nil
			}
			return String(os.Getenv(a[0].String())), nil
		}),
		"getcwd": builtin(func(a []*Value) (*Value, error) {
			cwd, _ := os.Getwd()
			return String(cwd), nil
		}),
		"cwd": builtin(func(a []*Value) (*Value, error) {
			cwd, _ := os.Getwd()
			return String(cwd), nil
		}),
		"time": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(time.Now().Unix())), nil
		}),
		"clock": builtin(func(a []*Value) (*Value, error) {
			return Number(time.Since(processStart).Seconds()), nil
		}),
		"mkdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return valFalse, nil
			}
			err := os.MkdirAll(a[0].String(), 0755)
			return Bool(err == nil), nil
		}),
		"exists": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return valFalse, nil
			}
			_, err := os.Stat(a[0].String())
			return Bool(err == nil), nil
		}),
		"isDir": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return valFalse, nil
			}
			info, err := os.Stat(a[0].String())
			if err != nil {
				return valFalse, nil
			}
			return Bool(info.IsDir()), nil
		}),
		"isFile": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return valFalse, nil
			}
			info, err := os.Stat(a[0].String())
			if err != nil {
				return valFalse, nil
			}
			return Bool(!info.IsDir()), nil
		}),
		"listdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Array(nil), nil
			}
			entries, err := os.ReadDir(a[0].String())
			if err != nil {
				return Array(nil), nil
			}
			items := make([]*Value, len(entries))
			for i, e := range entries {
				items[i] = String(e.Name())
			}
			return Array(items), nil
		}),
		"rmdir": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return valFalse, nil
			}
			err := os.Remove(a[0].String())
			return Bool(err == nil), nil
		}),
		"pid": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(os.Getpid())), nil
		}),
		"getpid": builtin(func(a []*Value) (*Value, error) {
			return Number(float64(os.Getpid())), nil
		}),
	})
	env.Define("os", osObj, true)

	// ── crypto module (simple non-cryptographic hashing utilities) ─────────
	cryptoObj := Object(map[string]*Value{
		// crypto.hash(s) → DJB2 hash
		"hash": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			var h uint32 = 5381
			for _, b := range []byte(a[0].String()) {
				h = ((h << 5) + h) + uint32(b) // h*33 + b
			}
			return Number(float64(h)), nil
		}),
		// crypto.fnv1a(s) → FNV-1a 32-bit hash
		"fnv1a": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			var h uint32 = 2166136261
			for _, b := range []byte(a[0].String()) {
				h ^= uint32(b)
				h *= 16777619
			}
			return Number(float64(h)), nil
		}),
		// crypto.murmur3(s, seed) → simplified MurmurHash3-style 32-bit hash
		"murmur3": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			var seed uint32
			if len(a) > 1 {
				seed = uint32(a[1].NumVal)
			}
			data := []byte(a[0].String())
			var h uint32 = seed
			const c1, c2 uint32 = 0xcc9e2d51, 0x1b873593
			i := 0
			for ; i+4 <= len(data); i += 4 {
				k := uint32(data[i]) | uint32(data[i+1])<<8 | uint32(data[i+2])<<16 | uint32(data[i+3])<<24
				k *= c1
				k = (k << 15) | (k >> 17)
				k *= c2
				h ^= k
				h = (h << 13) | (h >> 19)
				h = h*5 + 0xe6546b64
			}
			var k uint32
			switch len(data) - i {
			case 3:
				k ^= uint32(data[i+2]) << 16
				fallthrough
			case 2:
				k ^= uint32(data[i+1]) << 8
				fallthrough
			case 1:
				k ^= uint32(data[i])
				k *= c1
				k = (k << 15) | (k >> 17)
				k *= c2
				h ^= k
			}
			h ^= uint32(len(data))
			h ^= h >> 16
			h *= 0x85ebca6b
			h ^= h >> 13
			h *= 0xc2b2ae35
			h ^= h >> 16
			return Number(float64(h)), nil
		}),
		// crypto.checksum(s) → sum of byte values, truncated to 32 bits
		"checksum": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Number(0), nil
			}
			var sum uint32
			for _, b := range []byte(a[0].String()) {
				sum += uint32(b)
			}
			return Number(float64(sum)), nil
		}),
		// crypto.randomBytes(n) → array of n random integers 0-255
		"randomBytes": builtin(func(a []*Value) (*Value, error) {
			n := 0
			if len(a) > 0 {
				n = int(a[0].NumVal)
			}
			items := make([]*Value, n)
			for i := range items {
				items[i] = Number(float64(rand.Intn(256)))
			}
			return Array(items), nil
		}),
		// crypto.uuid() → random UUID v4-shaped string (not cryptographically secure)
		"uuid": builtin(func(a []*Value) (*Value, error) {
			b := make([]byte, 16)
			for i := range b {
				b[i] = byte(rand.Intn(256))
			}
			b[6] = (b[6] & 0x0f) | 0x40 // version 4
			b[8] = (b[8] & 0x3f) | 0x80 // variant 10
			return String(fmt.Sprintf("%x-%x-%x-%x-%x", b[0:4], b[4:6], b[6:8], b[8:10], b[10:16])), nil
		}),
		"MD5_SIZE":    builtin(func(a []*Value) (*Value, error) { return Number(16), nil }),
		"SHA1_SIZE":   builtin(func(a []*Value) (*Value, error) { return Number(20), nil }),
		"SHA256_SIZE": builtin(func(a []*Value) (*Value, error) { return Number(32), nil }),
	})
	env.Define("crypto", cryptoObj, true)

	// ── path module ─────────────────────────────────────────────────────────
	pathObj := Object(map[string]*Value{
		"join": builtin(func(a []*Value) (*Value, error) {
			parts := make([]string, len(a))
			for i, v := range a {
				parts[i] = v.String()
			}
			return String(filepath.Join(parts...)), nil
		}),
		"basename": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(filepath.Base(a[0].String())), nil
		}),
		"dirname": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(filepath.Dir(a[0].String())), nil
		}),
		"extname": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(filepath.Ext(a[0].String())), nil
		}),
		"ext": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(filepath.Ext(a[0].String())), nil
		}),
		"isAbs": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return valFalse, nil
			}
			return Bool(filepath.IsAbs(a[0].String())), nil
		}),
		"clean": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return String(""), nil
			}
			return String(filepath.Clean(a[0].String())), nil
		}),
		"split": builtin(func(a []*Value) (*Value, error) {
			if len(a) == 0 {
				return Array([]*Value{String(""), String("")}), nil
			}
			dir, file := filepath.Split(a[0].String())
			return Array([]*Value{String(dir), String(file)}), nil
		}),
	})
	env.Define("path", pathObj, true)
}

// ─── helpers ──────────────────────────────────────────────────────────────────

// sortArrayValue implements array.sort/array.sortDesc: a[0] must be an array.
// Sorts numerically if the first element is a number, lexically if it's a
// string; returns a new array (does not mutate the input).
func sortArrayValue(a []*Value, desc bool) *Value {
	if len(a) == 0 || a[0].Tag != TypeArray {
		return Array(nil)
	}
	src := a[0].ArrayVal
	out := make([]*Value, len(src))
	copy(out, src)
	byStr := len(out) > 0 && out[0].Tag == TypeString
	sort.SliceStable(out, func(i, j int) bool {
		var less bool
		if byStr {
			less = out[i].StrVal < out[j].StrVal
		} else {
			less = out[i].NumVal < out[j].NumVal
		}
		if desc {
			return !less && out[i] != out[j]
		}
		return less
	})
	return Array(out)
}

// minMaxArrayValue implements array.min/array.max over numeric elements.
func minMaxArrayValue(a []*Value, wantMin bool) *Value {
	if len(a) == 0 || a[0].Tag != TypeArray || len(a[0].ArrayVal) == 0 {
		return valNull
	}
	var m float64
	found := false
	for _, v := range a[0].ArrayVal {
		if v.Tag != TypeNumber {
			continue
		}
		if !found || (wantMin && v.NumVal < m) || (!wantMin && v.NumVal > m) {
			m = v.NumVal
			found = true
		}
	}
	if !found {
		return valNull
	}
	return Number(m)
}

// numsFromArrayOrArgs extracts a []float64 from either a single array
// argument (math.sum([1,2,3])) or a varargs list (math.sum(1,2,3)).
func numsFromArrayOrArgs(a []*Value) []float64 {
	if len(a) == 1 && a[0].Tag == TypeArray {
		nums := make([]float64, 0, len(a[0].ArrayVal))
		for _, v := range a[0].ArrayVal {
			if v.Tag == TypeNumber {
				nums = append(nums, v.NumVal)
			}
		}
		return nums
	}
	nums := make([]float64, 0, len(a))
	for _, v := range a {
		if v.Tag == TypeNumber {
			nums = append(nums, v.NumVal)
		}
	}
	return nums
}

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
	return valNull
}

// ─── core built-ins ───────────────────────────────────────────────────────────

func builtinPrint(args []*Value) (*Value, error) {
	parts := make([]string, len(args))
	for i, a := range args {
		parts[i] = a.String()
	}
	fmt.Fprintln(stdout, strings.Join(parts, " "))
	return valNull, nil
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
		return valFalse, nil
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
		return valNull, nil
	}
	time.Sleep(time.Duration(args[0].NumVal) * time.Millisecond)
	return valNull, nil
}

func builtinExit(args []*Value) (*Value, error) {
	code := 0
	if len(args) > 0 {
		code = int(args[0].NumVal)
	}
	os.Exit(code)
	return valNull, nil
}

func builtinAssert(args []*Value) (*Value, error) {
	if len(args) == 0 {
		return valNull, nil
	}
	if !args[0].Truthy() {
		msg := "assertion failed"
		if len(args) > 1 {
			msg = args[1].String()
		}
		return valNull, fmt.Errorf("%s", msg)
	}
	return valNull, nil
}

func builtinError(args []*Value) (*Value, error) {
	msg := "error"
	if len(args) > 0 {
		msg = args[0].String()
	}
	return valNull, fmt.Errorf("%s", msg)
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
