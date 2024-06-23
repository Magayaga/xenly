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

**Xenly** (formerly known as **Xenon**) is a **free and open-source** compiled high-level, multi-paradigm, general-purpose programming language designed primarily for command-line interfaces, web servers, and applications. It was originally written in **C**, **Go**, and **Rust** programming languages.

Xenly programming language should be command-line interfaces, web servers, and desktop applications. It is static and dynamic typing, readability, usability, and flexibility.

![Introduction to Xenly](assets/xenly_introduction.gif)

## Examples
### Hello, World! program
The following shows how a **"Hello, World!"** program is written in Xenly programming language:

```swift
// “Hello, World!” program
print("Hello, World!")
```

A **Xenly** variable is created the moment you first assign a value to it.

```swift
// "var" variable to print()
var hi = "Hello, World!"
print($hi)
```

I know all high-level programming languages like **Python**, **Ruby**, **Lua**, and **Julia** were initially written in C programming language.

The following shows how a **"Hello, World!"** program is written in [Hyzero](https://github.com/magayaga/Hyzero) programming language:

```python
# “Hello, World!” program
write("Hello, World!")
```

Here is the Hyzero programming language that was initially written in Python programming language like interpreter, high-level, and functional.

## Getting Started

### 1. Download the Xenly's source code

You can download the git clone of the `xenly` programming language. It is the available for the **Windows** and **Linux** operating systems.


```bash
# Download the Xenly's source code
$ git clone https://github.com/xenly
$ xenly
```

### 2. Install the Xenly's source code

You can run the program `makefile` for Linux, `make.bat` for Windows, and `main.sh` for both Windows and Linux.

```bash
# Run the program (makefile, bash script, or batchfile)
$ make
$ bash main.sh
$ ./make.bat

# Run the binary code
$ ./xenly -v
```

### 3. Set the Library paths

Make sure the shared library path is correctly set so that the system can find the libraries. Set the `LD_LIBRARY_PATH` environment variable to include the directory containing libraries (`math.so`, `graphics.so`, and more).

```bash
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
```

### 4. Check permissions and paths

Ensure that the libraries has the correct permissions and is in the directory where `xenly` expects to find it. Ensure the library is readable and executable:

```bash
$ chmod +rx <libraries>
```

For example,

```bash
$ chmod +rx math.so
```

Confirm that `math.so` is in the same directory as xenly or in a directory listed in `LD_LIBRARY_PATH`.

### 5. Testing programs

```bash
# Xenly's version information
./xenly -v

# Running xenly's examples
./xenly examples/print/main.xe
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

If you are new to the **Xenly**, you may want to check out these additional resources.

* [Official website of Xenly (Getting Started)](https://magayaga.github.io/xenly/get-xenly.html)
* [Documentation of Xenly (Getting Started)](doc/getting-started.md)

## Copyright

Copyright (c) 2023-2024 [Cyril John Magayaga](https://github.com/magayaga). All rights reserved.

Licensed under the [MIT](LICENSE) license.
