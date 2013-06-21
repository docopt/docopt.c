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


typedef struct {
    /* commands */
    int create;
    int mine;
    int move;
    int remove;
    int set;
    int ship;
    int shoot;
    /* arguments */
    char *name;
    char *x;
    char *y;
    /* options without arguments */
    int drifting;
    int help;
    int moored;
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
"  naval_fate.py --help\n"
"  naval_fate.py --version";

typedef struct {
    const char *name;
    bool value;
} Command;

typedef struct {
    const char *name;
    char *value;
    char **array;
} Argument;

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

Tokens* tokens_new(Tokens *ts, int argc, char **argv) {
    struct Tokens update = {argc, argv, 0, argv[0]};
    (*ts) = update;
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
 * ARGV parsing functions
 */

Tokens* parse_shorts(Tokens *ts, int n_options, Option *options) {
    Option *option;
    char *raw;
    int i;

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
            exit(1);
        }
        raw++;
        if (!option->argcount) {
            option->value = true;
        } else {
            if (raw[0] == '\0') {
                if (ts->current == NULL) {
                    fprintf(stderr, "%s requires argument\n", option->oshort);
                    exit(1);
                }
                raw = ts->current;
                tokens_move(ts);
            }
            option->argument = raw;
            break;
        }
    }
    return ts;
}

Tokens* parse_long(Tokens *ts, int n_options, Option *options) {
    char *eq = strchr(ts->current, '=');
    char *argument = NULL;
    int i;
    Option *option;

    if (eq != NULL) {
        // "--option=value\0" => "--option\0value\0"
        *eq = '\0';
        argument = eq + 1;
    }
    for (i=0; i < n_options; i++) {
        option = &options[i];
        if (!strncmp(ts->current, option->olong, strlen(ts->current)))
            break;
    }
    if (i == n_options) {
        // TODO '%s is not a unique prefix
        fprintf(stderr, "%s is not recognized\n", ts->current);
        exit(1);
    }
    tokens_move(ts);
    if (option->argcount) {
        if (argument == NULL) {
            if (ts->current == NULL) {
                fprintf(stderr, "%s requires argument\n", option->olong);
                exit(1);
            }
            option->argument = ts->current;
            tokens_move(ts);
        } else {
            option->argument = argument;
        }
    } else {
        if (argument != NULL) {
            fprintf(stderr, "%s must not have an argument\n", option->olong);
            exit(1);
        }
        option->value = true;
    }
    return ts;
}

Tokens* parse_args(Tokens *ts, Elements *elements) {
    while (ts->current != NULL) {
        if (strcmp(ts->current, "--") == 0) {
            // not implemented yet
            // return parsed + [Argument(None, v) for v in tokens]
            return ts;
        } else if (ts->current[0] == '-' && ts->current[1] == '-') {
            parse_long(ts, elements->n_options, elements->options);
        } else if (ts->current[0] == '-' && ts->current[1] != '\0') {
            parse_shorts(ts, elements->n_options, elements->options);
        } else {
            // not implemented yet, just skip for now
            // parsed.append(Argument(None, tokens.move()))
            tokens_move(ts);
        }
    }
    return ts;
}

void elems_to_args(Elements *elements, DocoptArgs *args, bool help,
                   const char *version){
    Command *command;
    Argument *argument;
    Option *option;
    int i;

    /* options */
    for (i=0; i < elements->n_options; i++) {
        option = &elements->options[i];
        if (help && option->value && !strcmp(option->olong, "--help")) {
            printf("%s", args->help_message);
            exit(0);
        } else if (version && option->value &&
                   !strcmp(option->olong, "--version")) {
            printf("%s\n", version);
            exit(0);
        } else if (!strcmp(option->olong, "--drifting")) {
            args->drifting = option->value;
        } else if (!strcmp(option->olong, "--help")) {
            args->help = option->value;
        } else if (!strcmp(option->olong, "--moored")) {
            args->moored = option->value;
        } else if (!strcmp(option->olong, "--version")) {
            args->version = option->value;
        } else if (!strcmp(option->olong, "--speed")) {
            args->speed = option->argument;
        }
    }
    /* commands */
    for (i=0; i < elements->n_commands; i++) {
        command = &elements->commands[i];
        if (!strcmp(command->name, "create")) {
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
        if (!strcmp(argument->name, "<name>")) {
            args->name = argument->value;
        } else if (!strcmp(argument->name, "<x>")) {
            args->x = argument->value;
        } else if (!strcmp(argument->name, "<y>")) {
            args->y = argument->value;
        }
    }
    return;
}


/*
 * Main docopt function
 */

DocoptArgs docopt(int argc, char *argv[], bool help, const char *version) {
    DocoptArgs args = {
        0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, 0, 0, 0, 0, (char*) "10",
        usage_pattern, help_message
    };
    Tokens ts;
    Command commands[] = {
        {"create", 0},
        {"mine", 0},
        {"move", 0},
        {"remove", 0},
        {"set", 0},
        {"ship", 0},
        {"shoot", 0}
    };
    Argument arguments[] = {
        {"<name>", NULL, NULL},
        {"<x>", NULL, NULL},
        {"<y>", NULL, NULL}
    };
    Option options[] = {
        {NULL, "--drifting", 0, 0, NULL},
        {"-h", "--help", 0, 0, NULL},
        {NULL, "--moored", 0, 0, NULL},
        {NULL, "--version", 0, 0, NULL},
        {NULL, "--speed", 1, 0, NULL}
    };
    Elements elements = {7, 3, 5, commands, arguments, options};

    tokens_new(&ts, argc, argv);
    parse_args(&ts, &elements);
    elems_to_args(&elements, &args, help, version);
    return args;
}
