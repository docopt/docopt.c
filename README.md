C-code generator for docopt language
====================================

Note, *at this point the code generator handles only options*
(positional arguments, commands and pattern matching will follow).

### Step 1. Describe your CLI in docopt language

    Usage:
      program --tcp [--host=<host>] [--port=<port>] [--timeout=<seconds>]
      program --serial [--port=<port>] [--baud=<baud>] [--timeout=<seconds>]
      program -h | --help | --version

    Options:
      -h, --help               Show this screen.
      --version                Print version and exit.
      --tcp                    TCP mode.
      --serial                 Serial mode.
      --host=<host>            Target host [default: localhost].
      -p, --port=<port>        Target port [default: 1234].
      -t, --timeout=<seconds>  Timeout time in seconds [default: 10]
      -b, --baud=<baud>        Target port [default: 9600].

### Step 2. Generate the C code

```bash
$ cat example.docopt | python docopt.c.py > docopt.c
```

### Step 3. Include the generated `docopt.c` into your program

```c
#include "docopt.c"

int main(int argc, char *argv[])
{
    DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ "2.0rc2");

    printf("--help == %s\n", args.help ? "true" : "false");
    printf("--version == %s\n", args.version ? "true" : "false");
    printf("--tcp == %s\n", args.tcp ? "true" : "false");
    printf("--serial == %s\n", args.serial ? "true" : "false");
    printf("--host == %s\n", args.host);
    printf("--port == %s\n", args.port);
    printf("--timeout == %s\n", args.timeout);
    printf("--baud == %s\n", args.baud);
}
```

### Step 4. Profit!

```bash
$ c99 example.c -o example.out
$ ./example.out --tcp --host=127.0.0.1 --baud=4800
--help == false
--version == false
--tcp == true
--serial == false
--host == 127.0.0.1
--port == 1234
--timeout == 10
--baud == 4800
```

Development
===========

See the [Python version's page](http://github.com/docopt/docopt) for more
info on developing.
