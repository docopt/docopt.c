#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "log.h"

typedef enum {Option, Argument, Command, None} ElementType;

typedef struct {
    ElementType type;
    /*
     * Should probably be union for storage efficiency, but during development
     * it's easier to work without it.
     */
    /* union { */
        struct {
            char *oshort;
            char *olong;
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
  * TokenStream
  */

typedef struct {
    int argc;
    char **argv;
    int i;
    char *current;
} TokenStream;

TokenStream TokenStream_create(int argc, char **argv) {
    TokenStream ts;
    ts.argc = argc;
    ts.argv = argv;
    ts.i = 0;
    ts.current = argv[ts.i];
    return ts;
}

TokenStream TokenStream_move(TokenStream ts) {
    if (ts.i < ts.argc) {
        ts.current = ts.argv[++ts.i];
    }
    if (ts.i == ts.argc) {
        ts.current = NULL;
    }
    return ts;
}

 /*
  * parse_shorts
  */

TokenStream parse_shorts(TokenStream ts, Element options[]) {
    char *raw = &ts.current[1];
    ts = TokenStream_move(ts);
    Element *parsed = NULL;
    while (raw[0] != '\0') {
        int i = 0;
        Element *o = &options[i];
        while (o->type != None) {
            if (o->type == Option && o->option.oshort != NULL
                                  && o->option.oshort[1] == raw[0]) {
                break;
            }
            o = &options[++i];
        }
        if (o->type == None) {  // TODO -%s is specified ambiguously %d times
            printf("-%c is not recognized", raw[0]);
            exit(1);
        }
        raw++;
        if (!o->option.argcount) {
            o->option.value = true;
        } else {
            if (raw[0] == '\0') {
                if (ts.current == NULL) {
                    printf("%s requires argument", o->option.oshort);
                    exit(1);
                }
                raw = ts.current;
                ts = TokenStream_move(ts);
            }
            o->option.argument = raw;
            break;
        }
    }
    return ts;
}

 /*
  * parse_long
  */

TokenStream parse_long(TokenStream ts, Element options[]) {
    char *eq = strchr(ts.current, '=');
    char *argument = NULL;
    if (eq != NULL) {
        *eq = '\0'; // "--option=value\0" => "--option\0value\0"
        argument = eq + 1;
    }
    int i = 0;
    Element *o = &options[i];
    while (o->type != None) {
        if (o->type == Option &&
                strncmp(ts.current, o->option.olong, strlen(ts.current)) == 0) {
            break;
        }
        o = &options[++i];
    }
    if (o->type == None) {  // TODO '%s is not a unique prefix
        printf("%s is not recognized", ts.current);
        exit(1);
    }
    ts = TokenStream_move(ts);
    if (o->option.argcount) {
        if (argument == NULL) {
            if (ts.current == NULL) {
                printf("%s requires argument", o->option.olong);
                exit(1);
            }
            o->option.argument = ts.current;
            ts = TokenStream_move(ts);
        } else {
            o->option.argument = argument;
        }
    } else {
        if (argument != NULL) {
            printf("%s must not have an argument", o->option.olong);
            exit(1);
        }
        o->option.value = true;
    }
    return ts;
}

 /*
  * parse_args
  */

TokenStream parse_args(TokenStream ts, Element options[]) {
    while (ts.current != NULL) {
        if (strcmp(ts.current, "--") == 0) {
            // not implemented yet
            return ts;
            //return parsed + [Argument(None, v) for v in tokens]
        } else if (ts.current[0] == '-' && ts.current[1] == '-') {
            ts = parse_long(ts, options);
        } else if (ts.current[0] == '-' ) {
            ts = parse_shorts(ts, options);
        } else {
            // not implemented yet
            ts = TokenStream_move(ts); // just skip for now
            //parsed.append(Argument(None, tokens.move()))
        }
    }
    return ts;
}

/* This is how the generated struct may look like */
typedef struct {
    /* flag options */
    bool drifting;
    bool help;
    bool moored;
    bool version;
    /* options with argument */
    char *speed;
    /* positional arguments */
    char *x;
    char *y;
    /* repeating positional arguments */
    char **name;
    /* commands */
    bool mine;
    bool move;
    bool new;
    bool remove;
    bool set;
    bool ship;
    bool shoot;
} DocoptArgs;


DocoptArgs docopt(int argc, char *argv[], bool help, char *version) {
    DocoptArgs args = {true, false, true, false, "15", "100", "200",
                       NULL, true, false, true, false, true, false, true};
    return args;
}
