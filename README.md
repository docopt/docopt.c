C-code generator for docopt language
===============================================================================

At this point the code generator handles only options (positional arguments
and commands will follow).

### Step 1. Create file in docopt lanugage, describing your CLI

`example.docopt`

    Usage:
        the_program --tcp [--host=HOST] [--port=PORT] [--timeout=SECONDS]
        the_program --serial [--port=PORT] [--baud=BAUD] [--timeout=SECONDS]
        the_program -h | --help | --version

    Options:
      -h, --help             Show this screen.
      --version              Print version and exit.
      --tcp                  TCP mode.
      --serial               Serial mode.
      --host HOST            Target host [default: localhost].
      -p, --port PORT        Target port [default: 1234].
      -t, --timeout SECONDS  Timeout time in seconds [default: 10]
      -b, --baud BAUD        Target port [default: 9600].

### Step 2. Generate code (`docopt.c`) using `docopt_c.py`

```bash
$ cat example.docopt | python docopt_c.py > docopt.c
```

### Step 3. Include `docopt.c` into your program

`example.c`

```c
#include "./docopt.c"

int main(int argc, char *argv[]) {

    DocoptArgs args = docopt(argc, argv, /* help */ 1, "2.0rc2");

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

### Step 4. Profit

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
