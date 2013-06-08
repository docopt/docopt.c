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


typedef enum {Option, Argument, Command, None} ElementType;

typedef struct {
    ElementType type;
    /*
     * Should probably be union for storage efficiency, but during development
     * it's easier to work without it.
     */
    /* union { */
        struct {
            const char *oshort;
            const char *olong;
            bool argcount;
            bool value;
            char *argument;
        } option;
        struct {
            char *name;
            bool repeating;
            char *value;
            char **array;
        } argument;
        struct {
            char *name;
            bool value;
        } command;
    /* } data; */
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

typedef struct {$flag_options$options_with_arguments
    /* special */
    const char *usage_pattern;
    const char *help_message;
} DocoptArgs;

const char help_message[] =
$help_message;

const char usage_pattern[] =
$usage_pattern;

DocoptArgs docopt(int argc, char *argv[], bool help, const char *version) {
    int i = 0;
    Tokens ts;
    DocoptArgs args = {$defaults
        usage_pattern, help_message
    };
    Element options[] = {$options
        {None}
    };
    Element *o;

    tokens_new(&ts, argc, argv);
    parse_args(&ts, options);
    o = &options[i];
    while (o->type != None) {
        if (help && o->option.value
                 && strcmp(o->option.olong, "--help") == 0) {
            printf("%s", args.help_message);
            exit(0);
        } else if (version && o->option.value
                           && strcmp(o->option.olong, "--version") == 0) {
            printf("%s\n", version);
            exit(0);
        }$if_flag$if_not_flag
        o = &options[++i];
    }
    return args;
}
