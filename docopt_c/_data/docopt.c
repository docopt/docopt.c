#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#else
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#endif


struct DocoptArgs {
    /* commands */
    size_t create;
    size_t mine;
    size_t move;
    size_t remove;
    size_t set;
    size_t ship;
    size_t shoot;
    /* arguments */
    char *name;
    char *x;
    char *y;
    /* options without arguments */
    size_t drifting;
    size_t help;
    size_t moored;
    size_t version;
    /* options with arguments */
    char *speed;
    /* special */
    const char *usage_pattern;
    const char *help_message;
};

const char help_message[] =
"Naval Fate.\n"
"\n"
"Usage:\n"
"  naval_fate ship create <name>...\n"
"  naval_fate ship <name> move <x> <y> [--speed=<kn>]\n"
"  naval_fate ship shoot <x> <y>\n"
"  naval_fate mine (set|remove) <x> <y> [--moored|--drifting]\n"
"  naval_fate --help\n"
"  naval_fate --version\n"
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
"  naval_fate ship create <name>...\n"
"  naval_fate ship <name> move <x> <y> [--speed=<kn>]\n"
"  naval_fate ship shoot <x> <y>\n"
"  naval_fate mine (set|remove) <x> <y> [--moored|--drifting]\n"
"  naval_fate --help\n"
"  naval_fate --version";

struct Command {
    const char *name;
    bool value;
};

struct Argument {
    const char *name;
    char *value;
    char **array;
};

struct Option {
    const char *oshort;
    const char *olong;
    bool argcount;
    bool value;
    char *argument;
};

struct Elements {
    size_t n_commands;
    size_t n_arguments;
    size_t n_options;
    struct Command *commands;
    struct Argument *arguments;
    struct Option *options;
};


/*
 * Tokens object
 */

struct Tokens {
    size_t argc;
    char **argv;
    size_t i;
    char *current;
};

Tokens tokens_new(int argc, char **argv) {
    struct Tokens ts = {argc, argv, 0, argv[0]};
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

size_t parse_doubledash(struct Tokens *ts, struct Elements *elements) {
    //size_t n_commands = elements->n_commands;
    //size_t n_arguments = elements->n_arguments;
    //Command *commands = elements->commands;
    //Argument *arguments = elements->arguments;

    // not implemented yet
    // return parsed + [Argument(None, v) for v in tokens]
    return 0;
}

size_t parse_long(struct Tokens *ts, struct Elements *elements) {
    size_t i;
    size_t len_prefix;
    size_t n_options = elements->n_options;
    char *eq = strchr(ts->current, '=');
    struct Option *option;
    struct Option *options = elements->options;

    len_prefix = (eq-(ts->current))/sizeof(char);
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
    } else {
        if (eq != NULL) {
            fprintf(stderr, "%s must not have an argument\n", option->olong);
            return 1;
        }
        option->value = true;
    }
    return 0;
}

size_t parse_shorts(struct Tokens *ts, struct Elements *elements) {
    char *raw;
    size_t i;
    size_t n_options = elements->n_options;
    struct Option *option;
    struct Option *options = elements->options;

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

size_t parse_argcmd(struct Tokens *ts, struct Elements *elements) {
    size_t i;
    size_t n_commands = elements->n_commands;
    //size_t n_arguments = elements->n_arguments;
    struct Command *command;
    struct Command *commands = elements->commands;
    //Argument *arguments = elements->arguments;

    for (i=0; i < n_commands; i++) {
        command = &commands[i];
        if (!strcmp(command->name, ts->current)){
            command->value = true;
            tokens_move(ts);
            return 0;
        }
    }
    // not implemented yet, just skip for now
    // parsed.append(Argument(None, tokens.move()))
    /*fprintf(stderr, "! argument '%s' has been ignored\n", ts->current);
    fprintf(stderr, "  '");
    for (i=0; i<ts->argc ; i++)
        fprintf(stderr, "%s ", ts->argv[i]);
    fprintf(stderr, "'\n");*/
    tokens_move(ts);
    return 0;
}

size_t parse_args(struct Tokens *ts, struct Elements *elements) {
    size_t ret;

    while (ts->current != NULL) {
        if (strcmp(ts->current, "--") == 0) {
            ret = parse_doubledash(ts, elements);
            if (!ret) break;
        } else if (ts->current[0] == '-' && ts->current[1] == '-') {
            ret = parse_long(ts, elements);
        } else if (ts->current[0] == '-' && ts->current[1] != '\0') {
            ret = parse_shorts(ts, elements);
        } else
            ret = parse_argcmd(ts, elements);
        if (ret) return ret;
    }
    return 0;
}

size_t elems_to_args(struct Elements *elements, struct DocoptArgs *args, const bool help,
                     const char *version){
    struct Command *command;
    struct Argument *argument;
    struct Option *option;
    size_t i;

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
    return 0;
}


/*
 * Main docopt function
 */

struct DocoptArgs docopt(size_t argc, char *argv[], bool help, const char *version) {
    struct DocoptArgs args = {
        0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, 0, 0, 0, 0, (char*) "10",
        usage_pattern, help_message
    };
    struct Tokens ts;
    struct Command commands[] = {
        {"create", 0},
        {"mine", 0},
        {"move", 0},
        {"remove", 0},
        {"set", 0},
        {"ship", 0},
        {"shoot", 0}
    };
    struct Argument arguments[] = {
        {"<name>", NULL, NULL},
        {"<x>", NULL, NULL},
        {"<y>", NULL, NULL}
    };
    struct Option options[] = {
        {NULL, "--drifting", 0, 0, NULL},
        {"-h", "--help", 0, 0, NULL},
        {NULL, "--moored", 0, 0, NULL},
        {NULL, "--version", 0, 0, NULL},
        {NULL, "--speed", 1, 0, NULL}
    };
    struct Elements elements = {7, 3, 5, commands, arguments, options};

    ts = tokens_new(argc, argv);
    if (parse_args(&ts, &elements))
        exit(EXIT_FAILURE);
    if (elems_to_args(&elements, &args, help, version))
        exit(EXIT_SUCCESS);
    return args;
}
