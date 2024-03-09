# Getting started with Xenly

You'll need to install our build dependencies (git, gcc, clang) and check out the Xenly repository, for example on Windows or Linux:

```bash
# Xenly's source code
$ git clone https://github.com/magayaga/xenly.git
$ cd xenly
```

Build and run the application
```bash
# Windows
$ ./make.bat
$ bash main.sh

# Linux
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
