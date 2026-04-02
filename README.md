<p align="center">
  <a href="https://github.com/magayaga/xenly">
    <img src="assets/xenly.svg" width="100%" height="100%">
  </a>
</p>

<h1 align="center">Xenly programming language</h1>

<p align="center">Dense, multiverse, and the successor of the C programming language</p>

<p align="center">
  <a href="https://github.com/magayaga/xenly">
    <img src=".github/logo/cjm_official.svg" width="75px" height="100%">
  </a>
</p>

**Xenly** (formerly known as **Xenon**) is a **free and open-source** high-level, multi-paradigm, general-purpose programming language designed primarily for command-line interfaces, web servers, and applications. It is written in the **C** and **Go** programming languages.

Xenly programming language should be command-line interfaces, web servers, and desktop applications. It is static and dynamic typing, readability, usability, and flexibility.

![Introduction to Xenly](assets/xenly_introduction.gif)

## Operating system supports
The **Xenly** programming language supports **macOS**, **Windows**, and **Linux** operating systems, and **Windows Subsystem for Linux**.

|   Operating systems  | Interpreter (`xenly`) | Native compiler (`xenlyc`) | Xenly Virtual Machine (`xenlybyc` and `xenlyrun`)   | Build automation system tool and test runner (`bituin`) |
|:--------------------:|:---------------------:|:--------------------------:|:---------------------------------------------------:|:-------------------------------------------------------:|
| Linux 🐧             | Yes                   | Yes                        | Yes                                                 | -                                                       |
| Apple macOS 🍎       | Yes                   | Yes                        | Yes                                                 | -                                                       |
| Windows Subsystem for Linux 🪟 | Yes         | Yes                        | Yes                                                 | -                                                       |
| Windows 🪟           | No (Not available)    | No (Not available)         | Yes                                                 | -                                                       |
| **Programming language** | **C**              | **C**                      | **Go**                                              | **Zig** or **Rust**? |

## How Much Faster?
Here are times to run `xenly`, `xenlyc`, and `xvm` on some Xenly's example codes on GitHub of varying sizes:

   |      Example codes      | Size (LOC) | `xenly` | `xenlyc` | `xvm` (`xenlybyc` + `xenlyrun`) |
   |:-----------------------:|:----------:|:-------:|:--------:|:-------------------------------:|
   | `hello.xe`              | 3          | 0.051s  | 0.034s   | 0.004s                          |
   | `sys_demo.xe`           | 159        | 0.078s  | 0.094s   | 135.861s                        |
   | `math_enhanced_demo.xe` | 223        | 0.005s  | 0.126s   | 0.006s                          |
   | Billion nested loops    | 8          | -       | 3.562s   | -                               |
   
## Examples
### 1. Hello, World! program
The following shows how a **"Hello, World!"** program is written in Xenly programming language:

```rust
// “Hello, World!” program
print("Hello, World!")
```

### 2. Variables
A **Xenly** variable is created the moment you first assign a value to it.

```javascript
// "var" variable to print()
const hi = "Hello, World!"
print(hi)
```

### 3. Xenly's standard library
**Xenly** comes with a standard library that helps establish a set of common types used by **Xenly** libraries and programs.
```typescript
// import modules
import "math"
import "string"
import "array"

// math
print(math.PI())
print(math.E()))
print(math.sqrt(144))

// string
print(string.len("hello"))
print(string.upper("hello"))
print(string.lower("WORLD"))

// array
print(array.create(5, 0))
print(array.of(10, 20, 30, 40, 50))
```

## Getting Started

### 1. Download Xenly's source code

You can download the git clone of the `xenly` programming language project. It is available for the **macOS**, **WSL** (Windows Subsystem for Linux), and **Linux** operating systems.

> [!WARNING]
> The `xenly` interpreter and the `xenlyc` native compiler for the **Windows** operating system are not available, because they are being fixed for errors and bugs.

```bash
# Download the Xenly's source code
$ git clone https://github.com/magayaga/xenly
$ cd xenly
```

### 2. Install Xenly's source code

You can run the program `makefile`, `main.sh`, or `install-c.py` for both WSL and Linux.

```bash
# Run the program (Makefile, Bash script, or Python)
$ make
$ bash main.sh
$ py install-c.py

# Run the binary code
$ ./xenly
$ ./xenlyc
```

### 3. Testing programs

```bash
# Xenly's version information
./xenly -v

# Running xenly's examples (Interpreter)
./xenly examples/hello.xe

# Running xenlyc's examples (Native compiler)
./xenlyc examples/hello.xe -o hello
./hello
```

## Source Code Organization

The Xenly source code is organized as follows:

|          Directory           |                           Contents                          |
|:----------------------------:|:-----------------------------------------------------------:|
| `assets/`                    | Types of files for the Xenly programming language           |
| `doc/`                       | Documentation for the Xenly programming language            |
| `docs/`                      | Official website for the Xenly programming language         |
| `examples/`                  | Example code for the Xenly programming language             |
| `src/`                       | Source code for the Xenly programming language              |
| `xvm/`                       | XVM's source code for the Xenly programming language        |

If you are new to the **Xenly**, you should check out these additional resources.

* [Official website of Xenly (Getting Started)](https://magayaga.github.io/xenly/get-xenly.html)
* [Documentation of Xenly (Getting Started)](doc/getting-started.md)

## Copyright

Copyright (c) 2023-2026 [Cyril John Magayaga](https://github.com/magayaga). All rights reserved.

Licensed under the [MIT](LICENSE) license.












