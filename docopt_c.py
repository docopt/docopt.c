r"""#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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
    <<<flag_options>>>;
    /* options with arguments */
    <<<options_with_arguments>>>;
    /* special */
    char *usage_pattern;
    char *help_message;
} DocoptArgs;

char *help_message =
<<<help_message>>>;

char *usage_pattern =
<<<usage_pattern>>>;

DocoptArgs docopt(int argc, char *argv[], bool help, char *version) {
    DocoptArgs args = {
        <<<defaults>>>,
        usage_pattern, help_message
    };
    Element options[] = {
        <<<options>>>,
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
        }<<<if_flag>>> <<<if_not_flag>>>
        o = &options[++i];
    }
    return args;
}

"""
import sys
from docopt import (
        printable_usage, parse_doc_options, parse_pattern, formal_usage,
)


def to_c(s):
    if type(s) is str:
        return ('"%s"' % s.replace('\\', r'\\')\
                          .replace('"', r'\"')\
                          .replace('\n', '\\n"\n"'))
    if type(s) in [int, long, float]:
        return str(s)
    if s is True:
        return '1'
    if s is False:
        return '0'
    if s is None:
        return 'NULL'
    raise ValueError("can't convert to c type: %r" % s)


def c_option(o):
    return '{Option, {%s}}' % ', '.join(to_c(v) for v in
             (o.short, o.long, o.argcount, False, None))


def c_name(s):
    return ''.join(c if c.isalnum() else '_' for c in s).strip('_')


def c_if_flag(o):
    t = """ else if (strcmp(o->option.o%s, %s) == 0) {
            args.%s = o->option.value;\n        }"""
    return t % (('long' if o.long else 'short'),
                to_c(o.long or o.short),
                c_name(o.long or o.short))


def c_if_not_flag(o):
    t = """ else if (o->option.argument && strcmp(o->option.o%s, %s) == 0) {
            args.%s = o->option.argument;\n        }"""
    return t % (('long' if o.long else 'short'),
                to_c(o.long or o.short),
                c_name(o.long or o.short))


if __name__ == '__main__':
    help_message=sys.stdin.read()
    usage_pattern = printable_usage(help_message)
    options = parse_doc_options(help_message)
    formal_pattern = parse_pattern(formal_usage(usage_pattern), options=options)
    formal_pattern.fix()

    out = __doc__
    out = out.replace('<<<flag_options>>>',
                      ';\n    '.join('int %s' % c_name(o.long or o.short)
                                     for o in options if o.argcount == 0))
    out = out.replace('<<<options_with_arguments>>>',
                      ';\n    '.join('char *%s' % c_name(o.long or o.short)
                                     for o in options if o.argcount == 1))
    out = out.replace('<<<help_message>>>', to_c(help_message))
    out = out.replace('<<<usage_pattern>>>', to_c(usage_pattern))
    out = out.replace('<<<defaults>>>',
                      ', '.join(to_c(o.value) for o in
                                sorted(options, key=lambda o: o.argcount)))
    out = out.replace('<<<options>>>',
                      ',\n        '.join(c_option(o) for o in options))
    out = out.replace('<<<if_flag>>>',
            ''.join(c_if_flag(o) for o in options if o.argcount == 0))
    out = out.replace('<<<if_not_flag>>>',
            ''.join(c_if_not_flag(o) for o in options if o.argcount == 1))
    print(out.strip())
