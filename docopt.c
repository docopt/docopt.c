#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#include <cstring>
#else
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#endif

#ifdef DOCOPT_DEBUG
#define DOCOPT_DPRINTF(...) fprintf(stderr, __VA_ARGS__)
#else
#define DOCOPT_DPRINTF(...)
#endif
#if 0
 /* docopt parsed pattern */
Required
(Either(Required
(Command('ship', False),
 Command('create', False),
 OneOrMore(Argument('<name>', None))),
 Required(Command('ship', False),
 Argument('<name>', None),
 Command('move', False),
 Argument('<x>', None),
 Argument('<y>', None),
 Optional(Option(None, '--speed', 1, '10'))),
 Required(Command('ship', False),
 Command('shoot', False),
 Argument('<x>', None),
 Argument('<y>', None)),
 Required(Command('mine', False),
 Required(Either
(Command('set', False),
 Command('remove', False))),
 Argument('<x>', None),
 Argument('<y>', None),
 Optional(Either
(Option(None, '--moored', 0, False),
 Option(None, '--drifting', 0, False)))),
 Required(Command('anchor', False),
 Optional(Option(None, '--up', 0, False)),
 Optional(Argument('<number>', None))),
 Required(Option('-h', '--help', 0, False)),
 Required(Option(None, '--version', 0, False))))
#endif

typedef struct {
    /* commands */
    int anchor;
    int create;
    int mine;
    int move;
    int remove;
    int set;
    int ship;
    int shoot;
    /* arguments */
    char *name;
    char *number;
    char *x;
    char *y;
    /* options without arguments */
    int drifting;
    int help;
    int moored;
    int up;
    int version;
    /* options with arguments */
    char *speed;
    /* special */
    const char *usage_pattern;
    const char *help_message;
} DocoptArgs;

const char help_message[] =
"Naval Fate.\n"
"\n"
"Usage:\n"
"  naval_fate.py ship create <name>...\n"
"  naval_fate.py ship <name> move <x> <y> [--speed=<kn>]\n"
"  naval_fate.py ship shoot <x> <y>\n"
"  naval_fate.py mine (set|remove) <x> <y> [--moored|--drifting]\n"
"  naval_fate.py anchor [--up] [<number>]\n"
"  naval_fate.py --help\n"
"  naval_fate.py --version\n"
"\n"
"Options:\n"
"  -h --help     Show this screen.\n"
"  --version     Show version.\n"
"  --speed=<kn>  Speed in knots [default: 10].\n"
"  --moored      Moored (anchored) mine.\n"
"  --drifting    Drifting mine.\n"
"\n"
"";

const char usage_pattern[] =
"Usage:\n"
"  naval_fate.py ship create <name>...\n"
"  naval_fate.py ship <name> move <x> <y> [--speed=<kn>]\n"
"  naval_fate.py ship shoot <x> <y>\n"
"  naval_fate.py mine (set|remove) <x> <y> [--moored|--drifting]\n"
"  naval_fate.py anchor [--up] [<number>]\n"
"  naval_fate.py --help\n"
"  naval_fate.py --version";

typedef struct {
    const char *name;
    char *value;
    int count;
    char **array;
} Argument;

typedef struct {
    const char *name;
    bool value;
    Argument *args;
} Command;

typedef struct {
    const char *oshort;
    const char *olong;
    bool argcount;
    bool value;
    char *argument;
} Option;

typedef struct {
    int n_commands;
    int n_arguments;
    int n_options;
    Command *commands;
    Argument *arguments;
    Option *options;
} Elements;


/*
 * Tokens object
 */

typedef struct Tokens {
    int argc;
    char **argv;
    int i;
    char *current;
} Tokens;

Tokens tokens_new(int argc, char **argv) {
    Tokens ts = {argc, argv, 0, argv[0]};
    return ts;
}

Tokens* tokens_move(Tokens *ts) {
    if (ts->i < ts->argc) {
        ts->current = ts->argv[++ts->i];
    }
    if (ts->i == ts->argc) {
        ts->current = NULL;
    }
    return ts;
}

/*
 * Positional Arguments
 */


static Argument docopt_ship[] = {
    {"<name>", NULL, 0, NULL},
    {NULL, NULL, 0, NULL}
};

static Argument docopt_create[] = {
    {"<name>", NULL, 0, NULL},
    {NULL, NULL, 0, NULL}
};

static Argument docopt_move[] = {
    {"<x>", NULL, 0, NULL},
    {"<y>", NULL, 0, NULL},
    {NULL, NULL, 0, NULL}
};

static Argument docopt_shoot[] = {
    {"<x>", NULL, 0, NULL},
    {"<y>", NULL, 0, NULL},
    {NULL, NULL, 0, NULL}
};

static Argument docopt_mine[] = {
    {NULL, NULL, 0, NULL}
};

static Argument docopt_set[] = {
    {"<x>", NULL, 0, NULL},
    {"<y>", NULL, 0, NULL},
    {NULL, NULL, 0, NULL}
};

static Argument docopt_remove[] = {
    {"<x>", NULL, 0, NULL},
    {"<y>", NULL, 0, NULL},
    {NULL, NULL, 0, NULL}
};

static Argument docopt_anchor[] = {
    {"<number>", NULL, 0, NULL},
    {NULL, NULL, 0, NULL}
};


/*
 * ARGV parsing functions
 */

int parse_doubledash(Tokens *ts, Elements *elements) {
    //int n_commands = elements->n_commands;
    //int n_arguments = elements->n_arguments;
    //Command *commands = elements->commands;
    //Argument *arguments = elements->arguments;

    // not implemented yet
    // return parsed + [Argument(None, v) for v in tokens]
    return 0;
}

int parse_long(Tokens *ts, Elements *elements) {
    int i;
    size_t len_prefix;
    int n_options = elements->n_options;
    char *eq = strchr(ts->current, '=');
    Option *option = NULL;
    Option *options = elements->options;

    if (eq) {
        len_prefix = (eq-(ts->current))/sizeof(char);
    }
    else {
        len_prefix = strlen(ts->current);
    }
    for (i=0; i < n_options; i++) {
        option = &options[i];
        if (!strncmp(ts->current, option->olong, len_prefix))
            break;
    }
    if (i == n_options) {
        // TODO '%s is not a unique prefix
        fprintf(stderr, "%s is not recognized\n", ts->current);
        return 1;
    }
    tokens_move(ts);
    if (option->argcount) {
        if (eq == NULL) {
            if (ts->current == NULL) {
                fprintf(stderr, "%s requires argument\n", option->olong);
                return 1;
            }
            option->argument = ts->current;
            tokens_move(ts);
        } else {
            option->argument = eq + 1;
        }
        DOCOPT_DPRINTF("option %s = %s\n", option->olong, option->argument);
    } else {
        if (eq != NULL) {
            fprintf(stderr, "%s must not have an argument\n", option->olong);
            return 1;
        }
        option->value = true;
        DOCOPT_DPRINTF("option %s=true\n", option->olong);
    }
    return 0;
}

int parse_shorts(Tokens *ts, Elements *elements) {
    char *raw;
    int i;
    int n_options = elements->n_options;
    Option *option = NULL;
    Option *options = elements->options;

    raw = &ts->current[1];
    tokens_move(ts);
    while (raw[0] != '\0') {
        for (i=0; i < n_options; i++) {
            option = &options[i];
            if (option->oshort != NULL && option->oshort[1] == raw[0])
                break;
        }
        if (i == n_options) {
            // TODO -%s is specified ambiguously %d times
            fprintf(stderr, "-%c is not recognized\n", raw[0]);
            return 1;
        }
        raw++;
        if (!option->argcount) {
            option->value = true;
        } else {
            if (raw[0] == '\0') {
                if (ts->current == NULL) {
                    fprintf(stderr, "%s requires argument\n", option->oshort);
                    return 1;
                }
                raw = ts->current;
                tokens_move(ts);
            }
            option->argument = raw;
            break;
        }
    }
    return 0;
}

int elems_to_args(Elements *elements, DocoptArgs *args, bool help,
                  const char *version);

int parse_argcmd(Tokens *ts, Elements *elements, DocoptArgs *args) {
    int i;
    int n_commands = elements->n_commands;
    int n_arguments = elements->n_arguments;
    Command *command;
    Command *commands = elements->commands;
    Argument *arguments = elements->arguments;

    for (i=0; i < n_commands; i++) {
        command = &commands[i];
        if (!strcmp(command->name, ts->current)) {
            DOCOPT_DPRINTF("! matched command '%s'\n", command->name);
            command->value = true;
            tokens_move(ts);

            if (command->args) {
                /* Get the subset of arguments for this command */
                int n_args = 0;

                elems_to_args(elements, args, false, NULL);
                /* Count and clear the positional arguments in the subset
                 * TODO: Add the number of arguments to command (ie. ->n_args)
                 */
                for (;command->args[n_args].name; n_args++) {
                    command->args[n_args].value = 0;
                    command->args[n_args].count = 0;
                    command->args[n_args].array = NULL;
                };

                /* replace the argument set with the subset */
                elements->n_arguments = n_args;
                elements->arguments = command->args;
            } /* if command arguments */
            return 0;
        } /* if current token is a command */
    } /* for commands */

    if (n_arguments) {
        /* Parse positional argument */
        /* find first argument position we haven't parsed yet */
        for (i=0; i < n_arguments; i++) {
            if (!arguments[i].array) {
                break;
            }
        }
        if (i < n_arguments) {
            /* a new argument */
            arguments[i].value = ts->current;
            arguments[i].count = 1;
            arguments[i].array = &ts->argv[ts->i];
            DOCOPT_DPRINTF("argument[%d]->name %s value %s count 1\n", i, arguments[i].name, arguments[i].value);
        }
        else { /* i == n_arguments */
            /* count as number of OneOrMore arguments */
            arguments[i-1].count++;
            DOCOPT_DPRINTF("argument[%d]->name %s value %s count %u\n", i-1, arguments[i-1].name, ts->current, arguments[i-1].count);
        }
        tokens_move(ts);
    }
    else {
        /* Skip unknown or unexpected tokens */
        tokens_move(ts);
    }

    return 0;
}

int parse_args(Tokens *ts, Elements *elements, DocoptArgs *args) {
    int ret;

    while (ts->current != NULL) {
        if (strcmp(ts->current, "--") == 0) {
            ret = parse_doubledash(ts, elements);
            if (!ret) break;
        } else if (ts->current[0] == '-' && ts->current[1] == '-') {
            ret = parse_long(ts, elements);
        } else if (ts->current[0] == '-' && ts->current[1] != '\0') {
            ret = parse_shorts(ts, elements);
        } else
            ret = parse_argcmd(ts, elements, args);
        if (ret) return ret;
    }
    return 0;
}

int elems_to_args(Elements *elements, DocoptArgs *args, bool help,
                  const char *version){
    Command *command;
    Argument *argument;
    Option *option;
    int i;

    // fix gcc-related compiler warnings (unused)
    (void)command;
    (void)argument;

    /* options */
    for (i=0; i < elements->n_options; i++) {
        option = &elements->options[i];
        if (help && option->value && !strcmp(option->olong, "--help")) {
            printf("%s", args->help_message);
            return 1;
        } else if (version && option->value &&
                   !strcmp(option->olong, "--version")) {
            printf("%s\n", version);
            return 1;
        } else if (!strcmp(option->olong, "--drifting")) {
            args->drifting = option->value;
        } else if (!strcmp(option->olong, "--help")) {
            args->help = option->value;
        } else if (!strcmp(option->olong, "--moored")) {
            args->moored = option->value;
        } else if (!strcmp(option->olong, "--up")) {
            args->up = option->value;
        } else if (!strcmp(option->olong, "--version")) {
            args->version = option->value;
        } else if (!strcmp(option->olong, "--speed")) {
            if (option->argument)
                args->speed = option->argument;
        }
    }
    /* commands */
    for (i=0; i < elements->n_commands; i++) {
        command = &elements->commands[i];
        if (!strcmp(command->name, "anchor")) {
            args->anchor = command->value;
        } else if (!strcmp(command->name, "create")) {
            args->create = command->value;
        } else if (!strcmp(command->name, "mine")) {
            args->mine = command->value;
        } else if (!strcmp(command->name, "move")) {
            args->move = command->value;
        } else if (!strcmp(command->name, "remove")) {
            args->remove = command->value;
        } else if (!strcmp(command->name, "set")) {
            args->set = command->value;
        } else if (!strcmp(command->name, "ship")) {
            args->ship = command->value;
        } else if (!strcmp(command->name, "shoot")) {
            args->shoot = command->value;
        }
    }
    /* arguments */
    for (i=0; i < elements->n_arguments; i++) {
        argument = &elements->arguments[i];
        if (!argument->name) {return 0;}
        if (!strcmp(argument->name, "<name>")) {
            args->name = argument->value;
        } else if (!strcmp(argument->name, "<number>")) {
            args->number = argument->value;
        } else if (!strcmp(argument->name, "<x>")) {
            args->x = argument->value;
        } else if (!strcmp(argument->name, "<y>")) {
            args->y = argument->value;
        }
    }
    return 0;
}


/*
 * Main docopt function
 */

DocoptArgs docopt(int argc, char *argv[], bool help, const char *version) {
    DocoptArgs args = {
        0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, (char*)
        "10",
        usage_pattern, help_message
    };
    Tokens ts;
    Command commands[] = {
        {"anchor", 0, docopt_anchor},
        {"create", 0, docopt_create},
        {"mine", 0, docopt_mine},
        {"move", 0, docopt_move},
        {"remove", 0, docopt_remove},
        {"set", 0, docopt_set},
        {"ship", 0, docopt_ship},
        {"shoot", 0, docopt_shoot}
    };
    Argument arguments[] = {
        {"<name>", NULL, 0, NULL},
        {"<number>", NULL, 0, NULL},
        {"<x>", NULL, 0, NULL},
        {"<y>", NULL, 0, NULL},
        {NULL, NULL, 0, NULL}
    };
    Option options[] = {
        {NULL, "--drifting", 0, 0, NULL},
        {"-h", "--help", 0, 0, NULL},
        {NULL, "--moored", 0, 0, NULL},
        {NULL, "--up", 0, 0, NULL},
        {NULL, "--version", 0, 0, NULL},
        {NULL, "--speed", 1, 0, NULL}
    };
    Elements elements = {8, 4, 6, commands, arguments, options};

    ts = tokens_new(argc, argv);
    if (parse_args(&ts, &elements, &args))
        exit(EXIT_FAILURE);
    if (elems_to_args(&elements, &args, help, version))
        exit(EXIT_SUCCESS);
    return args;
}

