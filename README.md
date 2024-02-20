<p align="center">
  <a href="https://github.com/magayaga/xenly">
    <img src=".github/logo/xenly.svg" width="100%" height="100%">
  </a>
</p>

<h1 align="center">Xenly programming language</h1>

<p align="center">Dense, multiverse, and the successor of the C programming language</p>

<p align="center">
  <a href="https://github.com/magayaga/xenly">
    <img src=".github/logo/cjm_official.svg" width="75px" height="100%">
  </a>
</p>

**Xenly** (formerly known as **Xenon**) is a **free and open-source** compiled high-level, multi-paradigm, general-purpose programming language designed primarily for command-line interfaces, web servers, and applications. It was originally written in **C programming language** and created and developed by [Cyril John Magayaga](https://github.com/magayaga), who is best known for creating [Hyzero](https://github.com/magayaga/hyzero) programming language, [Nanomath](https://github.com/magayaga/nanomath) command-based scientific and programming calculator, and [Concat](https://github.com/magayaga/concat) command-based concatenate and print files.

Xenly programming language should be command-line interfaces, web servers, and desktop applications. It is static and dynamic typing, readability, usability, and flexibility.



https://github.com/Magayaga/xenly/assets/93117076/dd924975-eea3-4bc6-8b0d-c322bc4092b3


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
print(hi)
```

I know all high-level programming languages like **Python**, **Ruby**, **Lua**, and **Julia** were initially written in C programming language.

The following shows how a **"Hello, World!"** program is written in [Hyzero](https://github.com/magayaga/Hyzero) programming language:

```python
# “Hello, World!” program
write("Hello, World!")
```

A **Hyzero** variable is created the moment you first assign a value to it.
```python
# "VAR" variable to print()
VAR hi = "Hello, World!"
print(hi)
```

Here is the Hyzero programming language that was initially written in Python programming language like interpreter, high-level, and functional.

## Source Code Organization

The Xenly source code is organized as follows:

|          Directory           |                    Contents                     |
|:----------------------------:|:-----------------------------------------------:|
| `doc/`                       | documentation for Xenly programming language    |
| `docs/`                      | official website for Xenly programming language |
| `examples/`                  | example code for Xenly programming language     |
| `src/`                       | source for Xenly programming language           |

## Getting started

If you do want to try out Xenly locally, you'll need to install our build dependencies (git, and compilers such as **Clang**, **GCC**, or **tcc**) and check out the Xenly repository, for example on Debian or Ubuntu, Windows, and macOS:

1. **Debian or Ubuntu** operating systems:
   ```shell

   # Update sudo
   $ sudo apt update

   # Update sudo with fixing missing
   $ sudo apt-get update --fix-missing

   # Install tools; Choose the Clang, GCC, or tcc
   $ sudo apt install gcc git
   $ sudo apt install clang git

   # Download Xenly's code
   $ git clone https://github.com/magayaga/xenly.git
   $ cd xenly
   ```

   Then you can build and run the code:
   ```shell
   # Makefile
   $ make

   # Bash
   $ bash main.sh
   ```

   Last, Open the application:
   ```shell
   ./xenly
   ```

2. **Windows** operating system:

   Choose the [**Microsoft Visual Studio**](https://visualstudio.microsoft.com/), [**MinGW Installer**](https://sourceforge.net/projects/mingw-w64/), [**Cygwin**](https://www.cygwin.com/), [**MSYS2**](https://www.msys2.org/), [**tcc**](https://bellard.org/tcc/) on Windows operating system. Then, the download the [Git on Windows](https://git-scm.com/).

   ```shell
   # Download Xenly's code
   $ git clone https://github.com/magayaga/xenly.git
   $ cd xenly
   ```

   Then you can build and run the code:
   ```shell
   # Make.bat
   $ ./make.bat

   # Python
   $ py install.py
   ```

   Last, Open the application:
   ```shell
   ./xenly
   ```

3. **macOS** operating system:

   ```shell
   # Install tools; Choose the Clang, GCC, or tcc
   $ brew install gcc git
   $ brew install clang git

   # Download Xenly's code
   $ git clone https://github.com/magayaga/xenly.git
   $ cd xenly
   ```

   Then you can build and run the code:
   ```shell
   # Bash
   $ bash main.sh

   # Python
   $ py install.py
   ```

   Last, Open the Application:
   
   ```shell
   ./xenly
   ```

## Copyright

Copyright (c) 2023-2024 [Cyril John Magayaga](https://github.com/magayaga). All rights reserved.

Licensed under the [MIT](LICENSE) license.
