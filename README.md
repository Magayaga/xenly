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

**Xenly** (formerly known as **Xenon**) is a **free and open-source** high-level, multi-paradigm, general-purpose programming language designed primarily for command-line interfaces, web servers, and applications. It was originally written in **C** programming language.

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

```javascript
// "var" variable to print()
var hi = "Hello, World!"
print(hi)
```

## Getting Started

### 1. Download Xenly's source code

You can download the git clone of the `xenly` programming language project. It is available for the **macOS**, **WSL** (Windows Subsystem for Linux), and **Linux** operating systems.


```bash
# Download the Xenly's source code
$ git clone https://github.com/xenly
$ xenly
```

### 2. Install Xenly's source code

You can run the program `makefile` for Linux and `main.sh` for both WSL and Linux.

```bash
# Run the program (makefile or bash script)
$ make
$ bash main.sh

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

Copyright (c) 2023-2026 [Cyril John Magayaga](https://github.com/magayaga). All rights reserved.

Licensed under the [MIT](LICENSE) license.




