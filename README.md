# Xenly programming language

**Xenly** (formerly known as **Xenon**) is a high-level, multi-paradigm, general-purpose programming language designed primarily for command-line interfaces, web servers, and applications. It was originally written in **C programming language** and created and developed by [Cyril John Magayaga](https://github.com/magayaga), who is best known for the creating [Hyzero](https://github.com/magayaga/hyzero) programming language, [Nanomath](https://github.com/magayaga/nanomath) command-based scientific and programming calculator, and [Concat](https://github.com/magayaga/concat) command-based concatenate and print files.

Xenly programming language should be command-line interfaces, web servers, and desktop applications. It is static and dynamic typing, readability, usability, and flexibility.

### Hello, World! program
The following shows how a **"Hello, World!"** program is written in Xenly programming language:

```c
// “Hello, World!” program
print("Hello, World!")
```

I know all high-level programming languages like Python, Ruby, Lua, and Julia were initially written in C programming language.

The following shows how a **"Hello, World!"** program is written in Hyzero programming language:

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
$ git clone https://github.com/xenly xenly-pre-linux
$ cd xenly-pre-linux
$ git checkout pre-linux
```

### 2. Installing Xenly's source code

You can run the program (`makefile` for Linux and `main.sh` for both Windows and Linux).

```bash
# Run the program (makefile or bash script)
$ make
$ bash main.sh

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

## Copyright

Copyright (c) 2023-2024 Cyril John Magayaga. All rights reserved.

Licensed under the [MIT](LICENSE) license.
