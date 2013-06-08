#!/usr/bin/env python
#-*- coding:utf-8 -*-


# Copyright (c) 2012 Vladimir Keleshev, <vladimir@keleshev.com>
# (see LICENSE-MIT file for copying)


"""Usage: docopt_c.py [options] [<docopt>]

Processes a docopt formatted string, from either stdin or a file, and
outputs the equivalent C code to parse a CLI, to either the stdout or a file.

Options:
  -o, --output-name=<outname>
                Filename used to write the produced C file.
                If not present, the produced code is printed to stdout.
  -t, --template=<template>
                Filename used to read a C template.
  -h,--help     Show this help message and exit.

Arguments:
  <docopt>      Input file describing your CLI in docopt language.

"""


import sys
import os.path
import re
import docopt
from string import Template


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
    args = docopt.docopt(__doc__)

    try:
        if args['<docopt>'] is not None:
            with open(args['<docopt>'], 'r') as f:
                args['<docopt>'] = f.read()
        elif args['<docopt>'] is None and sys.stdin.isatty():
            print(__doc__.strip("\n"))
            sys.exit("")
        else:
            args['<docopt>'] = sys.stdin.read()
        if args['--template'] is None:
            args['--template'] = os.path.join(
                    os.path.dirname(os.path.realpath(__file__)), "template.c")
        with open(args['--template'], 'r') as f:
                args['--template'] = f.read()
    except IOError as e:
        sys.exit(e)

    doc = args['<docopt>']
    usage_sections = docopt.parse_section('usage:', doc)

    if len(usage_sections) == 0:
        raise docopt.DocoptLanguageError('"usage:" (case-insensitive) not found.')
    if len(usage_sections) > 1:
        raise docopt.DocoptLanguageError('More than one "usage:" (case-insensitive).')
    usage = usage_sections[0]

    options = docopt.parse_defaults(doc)
    pattern = docopt.parse_pattern(docopt.formal_usage(usage), options)
    defaults = ', '.join(to_c(o.value) for o in sorted(options, key=lambda o: o.argcount))
    defaults = re.sub(r'"(.*?)"', r'(char*) "\1"', defaults)

    out = Template(args['--template']).safe_substitute(
            flag_options=';\n    '.join(
                    'int %s' % c_name(o.long or o.short)
                    for o in options if o.argcount == 0),
            options_with_arguments=';\n    '.join(
                    'char *%s' % c_name(o.long or o.short)
                    for o in options if o.argcount == 1),
            help_message=to_c(doc),
            usage_pattern=to_c(usage),
            defaults=defaults,
            options=',\n        '.join(c_option(o) for o in options),
            if_flag=''.join(c_if_flag(o) for o in options if o.argcount == 0),
            if_not_flag=''.join(
                    c_if_not_flag(o) for o in options if o.argcount == 1))

    if args['--output-name'] is None:
        print(out.strip() + '\n')
    else:
        try:
            with open(args['--output-name'], 'w') as f:
                f.write(out.strip() + '\n')
        except IOError as e:
            sys.exit(str(e))
