# Copyright (c) 2012 Vladimir Keleshev, <vladimir@keleshev.com>
# (see LICENSE-MIT file for copying)


#!/usr/bin/env python
#-*- coding:utf-8 -*-


"""\
usage: docopt_c.py [options] [DOCOPT]

Processes a docopt formatted string, from either stdin or a file, and
outputs the equivalent C code to parse a CLI, to either the stdout or a file.

Options:
  -o OUTNAME --output-name=OUTNAME
                Filename used to write the produced C file.
                If not present, the produced code is printed to stdout.
  -t TEMPLATE --template=TEMPLATE
                Filename used to read a TEMPLATE.
  -h,--help     Show this help message and exit

Arguments:
  DOCOPT        Input file describing your CLI in docopt language.

"""


OUT_C = r"""#ifdef __cplusplus
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
  * Tokens
  */

typedef struct Tokens Tokens;

struct Tokens {
    int i;
    int argc;
    char **argv;
    char *current;
};

Tokens tokens_create(int argc, char **argv)
{
    Tokens ts = { 0, argc, argv, argv[0] };
    return ts;
}

Tokens* tokens_move(Tokens *ts)
{
    if (ts->i < ts->argc) {
        ts->current = ts->argv[++ts->i];
    }
    if (ts->i == ts->argc) {
        ts->current = NULL;
    }
    return ts;
}

 /*
  * parse_shorts
  */

Tokens* parse_shorts(Tokens *ts, Element options[]) {
    char *raw = &ts->current[1];
    tokens_move(ts);
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
                if (ts->current == NULL) {
                    printf("%s requires argument", o->option.oshort);
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

 /*
  * parse_long
  */

Tokens* parse_long(Tokens *ts, Element options[]) {
    char *eq = strchr(ts->current, '=');
    char *argument = NULL;
    if (eq != NULL) {
        *eq = '\0'; // "--option=value\0" => "--option\0value\0"
        argument = eq + 1;
    }
    int i = 0;
    Element *o = &options[i];
    while (o->type != None) {
        if (o->type == Option &&
                strncmp(ts->current, o->option.olong, strlen(ts->current)) == 0) {
            break;
        }
        o = &options[++i];
    }
    if (o->type == None) {  // TODO '%s is not a unique prefix
        printf("%s is not recognized", ts->current);
        exit(1);
    }
    tokens_move(ts);
    if (o->option.argcount) {
        if (argument == NULL) {
            if (ts->current == NULL) {
                printf("%s requires argument", o->option.olong);
                exit(1);
            }
            o->option.argument = ts->current;
            tokens_move(ts);
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

/* This is how the generated struct may look like */
typedef struct {
    /* flag options */
    <<<flag_options>>>;
    /* options with arguments */
    <<<options_with_arguments>>>;
    /* special */
    const char *usage_pattern;
    const char *help_message;
} DocoptArgs;

const char help_message[] =
<<<help_message>>>;

const char usage_pattern[] =
<<<usage_pattern>>>;

DocoptArgs docopt(int argc, char *argv[], bool help, const char *version) {
    DocoptArgs args = {
        <<<defaults>>>,
        usage_pattern, help_message
    };
    Element options[] = {
        <<<options>>>,
        {None}
    };
    Tokens ts = tokens_create(argc, argv);
    parse_args(&ts, options);
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
import re
import docopt


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


def __parse_cli():
    arguments = docopt.docopt(__doc__)
    try:
        # If no filename is provided in the DOCOPT argument, then its assumed
        # the docopt string will be read from stdin. This allows the use of
        # pipes on the command line like it was intended by Vladimir in the
        # initial implementation of docopt_c.py.
        if arguments['DOCOPT'] is None:
            arguments['DOCOPT'] = sys.stdin.read()
        else:
            with open(arguments['DOCOPT'], 'r') as f:
                arguments['DOCOPT'] = f.read()
        return arguments
    except ValueError:
        sys.stderr.write('error: verbosity level must be an integer\n')
        exit(1)
    except KeyboardInterrupt:
        # If no filename is provided in the DOCOPT argument and the user
        # forgets to pipe in the DOCOPT string, then the program will hang,
        # waiting for something to be returned from stdin.read(). This
        # exception handles CTRL+C being press at this stage.
        sys.stderr.write('\n')
        exit(1)
    except IOError as e:
        sys.stderr.write(str(e) + '\n')
        exit(1)
    return arguments


if __name__ == '__main__':
    args = __parse_cli()

    doc = args['DOCOPT']
    usage_sections = docopt.parse_section('usage:', doc)

    if len(usage_sections) == 0:
        raise docopt.DocoptLanguageError('"usage:" (case-insensitive) not found.')
    if len(usage_sections) > 1:
        raise docopt.DocoptLanguageError('More than one "usage:" (case-insensitive).')
    usage = usage_sections[0]

    options = docopt.parse_defaults(doc)
    pattern = docopt.parse_pattern(docopt.formal_usage(usage), options)

    out = OUT_C if args['--template'] is None else args['--template']
    out = out.replace('<<<flag_options>>>',
                      ';\n    '.join('int %s' % c_name(o.long or o.short)
                                     for o in options if o.argcount == 0))
    out = out.replace('<<<options_with_arguments>>>',
                      ';\n    '.join('char *%s' % c_name(o.long or o.short)
                                     for o in options if o.argcount == 1))
    out = out.replace('<<<help_message>>>', to_c(doc))
    out = out.replace('<<<usage_pattern>>>', to_c(usage))

    defaults = ', '.join(to_c(o.value) for o in sorted(options, key=lambda o: o.argcount))
    defaults = re.sub(r'"(.*?)"', r'(char*) "\1"', defaults)
    out = out.replace('<<<defaults>>>', defaults)

    out = out.replace('<<<options>>>',
                      ',\n        '.join(c_option(o) for o in options))
    out = out.replace('<<<if_flag>>>',
            ''.join(c_if_flag(o) for o in options if o.argcount == 0))
    out = out.replace('<<<if_not_flag>>>',
            ''.join(c_if_not_flag(o) for o in options if o.argcount == 1))

    if args['--output-name'] is None:
        print(out.strip())
    else:
        try:
            with open(args['--output-name'], 'w') as f:
                f.write(out.strip())
        except IOError as e:
            sys.stderr.write(str(e) + '\n')
            exit(1)
