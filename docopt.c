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
    int help;
    int version;
    int tcp;
    int serial;
    /* options with arguments */
    char *host;
    char *port;
    char *timeout;
    char *baud;
    /* special */
    const char *usage_pattern;
    const char *help_message;
} DocoptArgs;

const char help_message[] =
"Usage:\n"
"    the_program --tcp [--host=HOST] [--port=PORT] [--timeout=SECONDS]\n"
"    the_program --serial [--port=PORT] [--baud=BAUD] [--timeout=SECONDS]\n"
"    the_program -h | --help | --version\n"
"\n"
"Options:\n"
"  -h, --help             Show this screen.\n"
"  --version              Print version and exit.\n"
"  --tcp                  TCP mode.\n"
"  --serial               Serial mode.\n"
"  --host HOST            Target host [default: localhost].\n"
"  -p, --port PORT        Target port [default: 1234].\n"
"  -t, --timeout SECONDS  Timeout time in seconds [default: 10]\n"
"  -b, --baud BAUD        Target port [default: 9600].\n"
"";

const char usage_pattern[] =
"Usage:\n"
"    the_program --tcp [--host=HOST] [--port=PORT] [--timeout=SECONDS]\n"
"    the_program --serial [--port=PORT] [--baud=BAUD] [--timeout=SECONDS]\n"
"    the_program -h | --help | --version";

DocoptArgs docopt(int argc, char *argv[], bool help, const char *version) {
    DocoptArgs args = {
        0, 0, 0, 0, "localhost", "1234", "10", "9600",
        usage_pattern, help_message
    };
    Element options[] = {
        {Option, {"-h", "--help", 0, 0, NULL}},
        {Option, {NULL, "--version", 0, 0, NULL}},
        {Option, {NULL, "--tcp", 0, 0, NULL}},
        {Option, {NULL, "--serial", 0, 0, NULL}},
        {Option, {NULL, "--host", 1, 0, NULL}},
        {Option, {"-p", "--port", 1, 0, NULL}},
        {Option, {"-t", "--timeout", 1, 0, NULL}},
        {Option, {"-b", "--baud", 1, 0, NULL}},
        {None}
    };
    TokenStream ts = TokenStream_create(argc, argv);
    parse_args(ts, options);
    int i = 0;
    Element *o = &options[i];
    while (o->type != None) {
        if (help && o->option.value
                 && strcmp(o->option.olong, "--help") == 0) {
            printf("%s", args.help_message);
            exit(0);
        } else if (version && o->option.value
                           && strcmp(o->option.olong, "--version") == 0) {
            printf("%s", version);
            exit(0);
        } else if (strcmp(o->option.olong, "--help") == 0) {
            args.help = o->option.value;
        } else if (strcmp(o->option.olong, "--version") == 0) {
            args.version = o->option.value;
        } else if (strcmp(o->option.olong, "--tcp") == 0) {
            args.tcp = o->option.value;
        } else if (strcmp(o->option.olong, "--serial") == 0) {
            args.serial = o->option.value;
        }  else if (o->option.argument && strcmp(o->option.olong, "--host") == 0) {
            args.host = o->option.argument;
        } else if (o->option.argument && strcmp(o->option.olong, "--port") == 0) {
            args.port = o->option.argument;
        } else if (o->option.argument && strcmp(o->option.olong, "--timeout") == 0) {
            args.timeout = o->option.argument;
        } else if (o->option.argument && strcmp(o->option.olong, "--baud") == 0) {
            args.baud = o->option.argument;
        }
        o = &options[++i];
    }
    return args;
}
