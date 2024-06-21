# Getting started with Xenly

You'll need to install our build dependencies (`git`, `gcc` for Windows, `clang` for Linux / macOS) and check out the Xenly repository, for example on **Windows**, **Linux**, or **macOS**:

1. ## Windows operating system:

   You can download the `GCC` compiler (version 9 or above) available for **MSYS2** and **Cygwin**, and `Git` for the Windows operating system.

   `Clang` was not working because all fixing errors, bugs, and warnings.
   
   ```bash
   # Download the Git and GCC
   $ winget install --id Git.Git -e --source winget
   $ winget install MSYS2.MSYS2
   ```

   Open the Xenly's source code.
   ```bash
   # Xenly's source code
   $ git clone https://github.com/magayaga/xenly.git
   $ cd xenly
   ```

   Build and run the application
   
   ```bash
   # Windows (batchfile or bash script)
   $ ./make.bat
   $ bash main.sh
   ```

   To run code in a file non-interactively, you can give it as the first argument to the **Xenly** command:

   ```shell
   $ ./xenly examples/print/main.xe
   $ ./xenly examples/math/operations.xe
   ```

   You can pass additional arguments to **Xenly**, and to your program `main.xe` or `operations.xe`.

2. ## Linux operating system:

   You can download the `GCC` compiler (version 9 or above) or `Clang` compiler (version 13 or above), and `Git` for the Linux operating system like **Ubuntu**, **Fedora**, **Android**, and more Linux-like and Linux distros.

   You can choose the `GCC` compiler or `Clang` compiler.
   ```bash
   # Download the Git and GCC
   $ sudo apt-get install git gcc

   # or Download the Git and Clang
   $ sudo apt-get install git clang
   ```

   Open the Xenly's source code.
   ```bash
   # Xenly's source code
   $ git clone https://github.com/magayaga/xenly.git
   $ cd xenly
   ```

   Build and run the application
   
   ```bash
   # Linux (makefile or bash script)
   $ make
   $ bash main.sh
   ```

   To run code in a file non-interactively, you can give it as the first argument to the **Xenly** command:

   ```shell
   $ ./xenly examples/print/main.xe
   $ ./xenly examples/math/operations.xe
   ```

   You can pass additional arguments to **Xenly**, and to your program `main.xe` or `operations.xe`.

3. ## macOS operating system:

   You can download the `GCC` compiler (version 9 or above) or `Clang` compiler (version 13 or above), and `Git` for the macOS operating system.

   You can choose the `GCC` compiler or `Clang` compiler.
   ```bash
   # Download the Git and GCC
   $ brew install git gcc

   # or Download the Git and Clang
   $ brew install git clang
   ```

   Open the Xenly's source code.
   ```bash
   # Xenly's source code
   $ git clone https://github.com/magayaga/xenly.git
   $ cd xenly
   ```

   Build and run the application
   
   ```bash
   # macOS (makefile or bash script)
   $ make
   $ bash main.sh
   ```

   To run code in a file non-interactively, you can give it as the first argument to the **Xenly** command:

   ```shell
   $ ./xenly examples/print/main.xe
   $ ./xenly examples/math/operations.xe
   ```

   You can pass additional arguments to **Xenly**, and to your program `main.xe` or `operations.xe`.

## Copyright

Copyright (c) 2023-2024 [Cyril John Magayaga](https://github.com/magayaga). All rights reserved.
