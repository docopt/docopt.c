C-code generator for docopt language
====================================
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![PyPi publish](https://github.com/offscale/docopt.c/actions/workflows/main.yml/badge.svg)](https://github.com/offscale/docopt.c/actions/workflows/main.yml)
[![PyPi: release](https://img.shields.io/pypi/v/docopt_c.svg?maxAge=3600)](https://pypi.org/project/docopt_c)

Note, *at this point the code generator handles only options*
(positional arguments, commands and pattern matching will follow).

### Step 1. Describe your CLI in docopt language

```
Naval Fate.

Usage:
  naval_fate ship create <name>...
  naval_fate ship <name> move <x> <y> [--speed=<kn>]
  naval_fate ship shoot <x> <y>
  naval_fate mine (set|remove) <x> <y> [--moored|--drifting]
  naval_fate --help
  naval_fate --version

Options:
  -h --help     Show this screen.
  --version     Show version.
  --speed=<kn>  Speed in knots [default: 10].
  --moored      Moored (anchored) mine.
  --drifting    Drifting mine.
```

### Step 2. Generate the C code

```bash
$ python -m docopt_c -o docopt.c example.docopt
```

or by using pipe

```bash
$ cat example.docopt | python -m docopt_c > docopt.c
```

### Step 3. Include the generated `docopt.c` into your program

```c
#include <stdlib.h>
#include "docopt.c"

int main(int argc, char *argv[])
{
    DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ "2.0rc2");

    printf("Commands\n");
    printf("    mine == %s\n", args.mine ? "true" : "false");
    printf("    move == %s\n", args.move ? "true" : "false");
    printf("    create == %s\n", args.create ? "true" : "false");
    printf("    remove == %s\n", args.remove ? "true" : "false");
    printf("    set == %s\n", args.set ? "true" : "false");
    printf("    ship == %s\n", args.ship ? "true" : "false");
    printf("    shoot == %s\n", args.shoot ? "true" : "false");
    printf("Arguments\n");
    printf("    x == %s\n", args.x);
    printf("    y == %s\n", args.y);
    printf("Flags\n");
    printf("    --drifting == %s\n", args.drifting ? "true" : "false");
    printf("    --help == %s\n", args.help ? "true" : "false");
    printf("    --moored == %s\n", args.moored ? "true" : "false");
    printf("    --version == %s\n", args.version ? "true" : "false");
    printf("Options\n");
    printf("    --speed == %s\n", args.speed);

    return EXIT_SUCCESS;
}
```

### Step 4. Profit!

```bash
$ c99 example.c -o example.out
$ ./example.out mine --drifting --speed=20
Commands
    mine == true
    move == false
    create == false
    remove == false
    set == false
    ship == false
    shoot == false
Arguments
    x == (null)
    y == (null)
Flags
    --drifting == true
    --help == false
    --moored == false
    --version == false
Options
    --speed == 20
```

Development
===========

See the [Python version's page](http://github.com/docopt/docopt) for more
info on developing.
