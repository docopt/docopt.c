Work in progress
-------------------------------------------------------------------------------

C-code generator for docopt language
===============================================================================

Use `docopt_c.py` script to generate `docopt.c` and `docopt.h` files:

```bash
$ echo "Usage: my_program --option <argument>" | docopt_c.py
$ ls
docopt.c  docopt.h
```

```c
#include "./docopt.h"

int main(int argc, char *argv[]) {

    DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ "1.0rc2");

    printf("<argument> is %s\n", args.argument);
    printf("--option is %s\n", args.option ? "true" : "false");
}
```



