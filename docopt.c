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


typedef enum {Command, Argument, Option, None} ElementType;

typedef struct {
    ElementType type;
    struct {
        const char *name;
        bool value;
    } command;
    struct {
        const char *name;
        char *value;
        char **array;
    } argument;
    struct {
        const char *oshort;
        const char *olong;
        bool argcount;
        bool value;
        char *argument;
    } option;
} Element;


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

Tokens* parse_shorts(Tokens *ts, Element options[]) {
    Element *o;
    char *raw;

    raw = &ts->current[1];
    tokens_move(ts);
    while (raw[0] != '\0') {
        int i = 0;
        o = &options[i];
        while (o->type != None) {
            if (o->type == Option && o->option.oshort != NULL
                                  && o->option.oshort[1] == raw[0]) {
                break;
            }
            o = &options[++i];
        }
        if (o->type == None) {  // TODO -%s is specified ambiguously %d times
            fprintf(stderr, "-%c is not recognized\n", raw[0]);
            exit(1);
        }
        raw++;
        if (!o->option.argcount) {
            o->option.value = true;
        } else {
            if (raw[0] == '\0') {
                if (ts->current == NULL) {
                    fprintf(stderr, "%s requires argument\n",
                            o->option.oshort);
                    exit(1);
                }
                raw = ts->current;
                tokens_move(ts);
            }
            o->option.argument = raw;
            break;
        }
    }
    return ts;
}

Tokens* parse_long(Tokens *ts, Element options[]) {
    char *eq = strchr(ts->current, '=');
    char *argument = NULL;
    int i = 0;
    Element *o;

    if (eq != NULL) {
        *eq = '\0'; // "--option=value\0" => "--option\0value\0"
        argument = eq + 1;
    }
    o = &options[i];
    while (o->type != None) {
        if (o->type == Option &&
                strncmp(ts->current, o->option.olong, strlen(ts->current)) == 0) {
            break;
        }
        o = &options[++i];
    }
    if (o->type == None) {  // TODO '%s is not a unique prefix
        fprintf(stderr, "%s is not recognized\n", ts->current);
        exit(1);
    }
    tokens_move(ts);
    if (o->option.argcount) {
        if (argument == NULL) {
            if (ts->current == NULL) {
                fprintf(stderr, "%s requires argument\n", o->option.olong);
                exit(1);
            }
            o->option.argument = ts->current;
            tokens_move(ts);
        } else {
            o->option.argument = argument;
        }
    } else {
        if (argument != NULL) {
            fprintf(stderr, "%s must not have an argument\n", o->option.olong);
            exit(1);
        }
        o->option.value = true;
    }
    return ts;
}

Tokens* parse_args(Tokens *ts, Element options[]) {
    while (ts->current != NULL) {
        if (strcmp(ts->current, "--") == 0) {
            // not implemented yet
            return ts;
            //return parsed + [Argument(None, v) for v in tokens]
        } else if (ts->current[0] == '-' && ts->current[1] == '-') {
            parse_long(ts, options);
        } else if (ts->current[0] == '-' ) {
            parse_shorts(ts, options);
        } else {
            // not implemented yet
            tokens_move(ts); // just skip for now
            //parsed.append(Argument(None, tokens.move()))
        }
    }
    return ts;
}


/*
 * Main docopt function
 */

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

DocoptArgs docopt(int argc, char *argv[], bool help, const char *version) {
    int i = 0;
    Tokens ts;
    DocoptArgs args = {
        0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, 0, 0, 0, 0, (char*) "10",
        usage_pattern, help_message
    };
    Element options[] = {
        {Command, {"create", 0}, {NULL, NULL, NULL}, {NULL, NULL, false, false, NULL}},
        {Command, {"mine", 0}, {NULL, NULL, NULL}, {NULL, NULL, false, false, NULL}},
        {Command, {"move", 0}, {NULL, NULL, NULL}, {NULL, NULL, false, false, NULL}},
        {Command, {"remove", 0}, {NULL, NULL, NULL}, {NULL, NULL, false, false, NULL}},
        {Command, {"set", 0}, {NULL, NULL, NULL}, {NULL, NULL, false, false, NULL}},
        {Command, {"ship", 0}, {NULL, NULL, NULL}, {NULL, NULL, false, false, NULL}},
        {Command, {"shoot", 0}, {NULL, NULL, NULL}, {NULL, NULL, false, false, NULL}},
        {Argument, {NULL, false}, {"<name>", NULL, NULL}, {NULL, NULL, false, false, NULL}},
        {Argument, {NULL, false}, {"<x>", NULL, NULL}, {NULL, NULL, false, false, NULL}},
        {Argument, {NULL, false}, {"<y>", NULL, NULL}, {NULL, NULL, false, false, NULL}},
        {Option, {NULL, false}, {NULL, NULL, NULL}, {NULL, "--drifting", 0, 0, NULL}},
        {Option, {NULL, false}, {NULL, NULL, NULL}, {"-h", "--help", 0, 0, NULL}},
        {Option, {NULL, false}, {NULL, NULL, NULL}, {NULL, "--moored", 0, 0, NULL}},
        {Option, {NULL, false}, {NULL, NULL, NULL}, {NULL, "--version", 0, 0, NULL}},
        {Option, {NULL, false}, {NULL, NULL, NULL}, {NULL, "--speed", 1, 0, NULL}},
        {None}
    };
    Element *o;

    tokens_new(&ts, argc, argv);
    parse_args(&ts, options);

    o = &options[i];
    while (o->type != None) {
        if (o->type == Option && help && o->option.value
                 && strcmp(o->option.olong, "--help") == 0) {
            printf("%s", args.help_message);
            exit(0);
        } else if (o->type == Option && version && o->option.value
                           && strcmp(o->option.olong, "--version") == 0) {
            printf("%s\n", version);
            exit(0);
        } else if (o->type == Command &&
                   strcmp(o->command.name, "create") == 0) {
            args.create = o->command.value;
        } else if (o->type == Command &&
                   strcmp(o->command.name, "mine") == 0) {
            args.mine = o->command.value;
        } else if (o->type == Command &&
                   strcmp(o->command.name, "move") == 0) {
            args.move = o->command.value;
        } else if (o->type == Command &&
                   strcmp(o->command.name, "remove") == 0) {
            args.remove = o->command.value;
        } else if (o->type == Command &&
                   strcmp(o->command.name, "set") == 0) {
            args.set = o->command.value;
        } else if (o->type == Command &&
                   strcmp(o->command.name, "ship") == 0) {
            args.ship = o->command.value;
        } else if (o->type == Command &&
                   strcmp(o->command.name, "shoot") == 0) {
            args.shoot = o->command.value;
        } else if (o->type == Argument &&
                   strcmp(o->argument.name, "<name>") == 0) {
            args.name = o->argument.value;
        } else if (o->type == Argument &&
                   strcmp(o->argument.name, "<x>") == 0) {
            args.x = o->argument.value;
        } else if (o->type == Argument &&
                   strcmp(o->argument.name, "<y>") == 0) {
            args.y = o->argument.value;
        } else if (o->type == Option &&
                   strcmp(o->option.olong, "--drifting") == 0) {
            args.drifting = o->option.value;
        } else if (o->type == Option &&
                   strcmp(o->option.olong, "--help") == 0) {
            args.help = o->option.value;
        } else if (o->type == Option &&
                   strcmp(o->option.olong, "--moored") == 0) {
            args.moored = o->option.value;
        } else if (o->type == Option &&
                   strcmp(o->option.olong, "--version") == 0) {
            args.version = o->option.value;
        } else if (o->option.argument &&
                   strcmp(o->option.olong, "--speed") == 0) {
            args.speed = o->option.argument;
        }
        o = &options[++i];
    }
    return args;
}

