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

typedef struct {$commands$arguments$flags$options
    /* special */
    const char *usage_pattern;
    const char *help_message;
} DocoptArgs;

const char help_message[] =
$help_message;

const char usage_pattern[] =
$usage_pattern;

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

$positional

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

int parse_argcmd(Tokens *ts, Elements *elements) {
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

                /* Count the number of positional arguments in the subset
                 * TODO: Add the number of arguments to command (ie. ->n_args)
                 */
                for (;command->args[n_args].name; n_args++) {};

                /* replace the argument set with the subset */
                elements->n_arguments = n_args;
                elements->arguments = command->args;
            } /* if command arguments */
            return 0;
        } /* if current token is a command */
    } /* for commands */

    /* Not a command so parse positional arguments */
    for (i=0; i < n_arguments && ts->current; i++) {
        arguments[i].value = ts->current;
        arguments[i].count = ts->argc - ts->i;
        arguments[i].array = &ts->argv[ts->i];
        DOCOPT_DPRINTF("! argument[%d]->name %s value %s \n", i, arguments[i].name, arguments[i].value);
        tokens_move(ts);
    } /* for arguments */

    ts->current = NULL;

    return 0;
}

int parse_args(Tokens *ts, Elements *elements) {
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
            ret = parse_argcmd(ts, elements);
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
        }$if_flag$if_option
    }
    /* commands */
    for (i=0; i < elements->n_commands; i++) {
        command = &elements->commands[i];$if_command
    }
    /* arguments */
    for (i=0; i < elements->n_arguments; i++) {
        argument = &elements->arguments[i];
        if (!argument->name) {return 0;}$if_argument
    }
    return 0;
}


/*
 * Main docopt function
 */

DocoptArgs docopt(int argc, char *argv[], bool help, const char *version) {
    DocoptArgs args = {$defaults
        usage_pattern, help_message
    };
    Tokens ts;
    Command commands[] = {$elems_cmds
    };
    Argument arguments[] = {$elems_args
    };
    Option options[] = {$elems_opts
    };
    Elements elements = {$elems_n, commands, arguments, options};

    ts = tokens_new(argc, argv);
    if (parse_args(&ts, &elements))
        exit(EXIT_FAILURE);
    if (elems_to_args(&elements, &args, help, version))
        exit(EXIT_SUCCESS);
    return args;
}
