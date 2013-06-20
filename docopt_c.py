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


def c_command(o):
    return '{%s}' % ', '.join(to_c(v) for v in (o.name, o.value))


def c_argument(o):
    return '{%s}' % ', '.join(to_c(v) for v in (o.name, o.value, None))


def c_option(o):
    return '{%s}' % ', '.join(to_c(v) for v in (o.short, o.long, o.argcount,
                                                False, None))


def c_name(s):
    return ''.join(c if c.isalnum() else '_' for c in s).strip('_')


def c_if_command(cmd):
    t = """ else if (o->type == Command &&
                   strcmp(o->command.name, %s) == 0) {
            args.%s = o->command.value;\n        }"""
    return t % (to_c(cmd.name), c_name(cmd.name))


def c_if_argument(arg):
    t = """ else if (o->type == Argument &&
                   strcmp(o->argument.name, %s) == 0) {
            args.%s = o->argument.value;\n        }"""
    return t % (to_c(arg.name), c_name(arg.name))


def c_if_flag(o):
    t = """ else if (o->type == Option &&
                   strcmp(o->option.o%s, %s) == 0) {
            args.%s = o->option.value;\n        }"""
    return t % (('long' if o.long else 'short'),
                to_c(o.long or o.short),
                c_name(o.long or o.short))


def c_if_option(o):
    t = """ else if (o->option.argument &&
                   strcmp(o->option.o%s, %s) == 0) {
            args.%s = o->option.argument;\n        }"""
    return t % (('long' if o.long else 'short'),
                to_c(o.long or o.short),
                c_name(o.long or o.short))


def parse_leafs(pattern):
    leafs = []
    queue = [(0, pattern)]
    while queue:
        level, node = queue.pop(-1)  # depth-first search
        if hasattr(node, 'children'):
            children = [((level+1), child) for child in node.children]
            children.reverse()
            queue.extend(children)
        else:
            if node not in leafs:
                leafs.append(node)
    leafs.sort(key=lambda e: e.name)
    commands = [leaf for leaf in leafs if type(leaf) == docopt.Command]
    arguments = [leaf for leaf in leafs if type(leaf) == docopt.Argument]
    flags = [leaf for leaf in leafs
                  if type(leaf) == docopt.Option and leaf.argcount == 0]
    options = [leaf for leaf in leafs
                    if type(leaf) == docopt.Option and leaf.argcount > 0]
    leafs = [i for sl in [commands, arguments, flags, options] for i in sl]
    return leafs, commands, arguments, flags, options


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
    leafs, commands, arguments, flags, options = parse_leafs(pattern)

    t_commands = ';\n    '.join('int %s' % c_name(cmd.name)
                                for cmd in commands)
    t_commands = (('\n    /* commands */\n    ' + t_commands + ';')
                  if t_commands != '' else '')
    t_arguments = ';\n    '.join('char *%s' % c_name(arg.name)
                                 for arg in arguments)
    t_arguments = (('\n    /* arguments */\n    ' + t_arguments + ';')
                   if t_arguments != '' else '')
    t_flags = ';\n    '.join('int %s' % c_name(flag.long or flag.short)
                             for flag in flags)
    t_flags = (('\n    /* options without arguments */\n    ' + t_flags + ';')
               if t_flags != '' else '')
    t_options = ';\n    '.join('char *%s' % c_name(opt.long or opt.short)
                               for opt in options)
    t_options = (('\n    /* options with arguments */\n    ' + t_options + ';')
                 if t_options != '' else '')
    t_defaults = ', '.join(to_c(leaf.value) for leaf in leafs)
    t_defaults = re.sub(r'"(.*?)"', r'(char*) "\1"', t_defaults)
    t_defaults = ('\n        ' + t_defaults + ',') if t_defaults != '' else ''

    t_elements = ',\n        '.join([
            '/* commands */\n        {%s}' % ',\n         '.join([c_command(cmd) for cmd in (commands)]),
            '/* arguments */\n        {%s}' % ',\n         '.join([c_argument(arg) for arg in (arguments)]),
            '/* options */\n        {%s}' % ',\n         '.join([c_option(o) for o in (flags+options)])])
    t_elements = ('\n        ' + t_elements) if t_elements != '' else ''

    # t_elements = ',\n        '.join([c_command(cmd) for cmd in (commands)] +
    #                                 [c_argument(arg) for arg in (arguments)] +
    #                                 [c_option(o) for o in (flags+options)])
    # t_elements = ('\n        ' + t_elements + ',') if t_elements != '' else ''


    t_if_command = ''.join(c_if_command(command) for command in commands)
    t_if_argument = ''.join(c_if_argument(arg) for arg in arguments)
    t_if_flag = ''.join(c_if_flag(flag) for flag in flags)
    t_if_option = ''.join(c_if_option(opt) for opt in options)

    out = Template(args['--template']).safe_substitute(
            commands=t_commands,
            arguments=t_arguments,
            flags=t_flags,
            options=t_options,
            help_message=to_c(doc),
            usage_pattern=to_c(usage),
            defaults=t_defaults,
            elements=t_elements,
            if_command=t_if_command,
            if_argument=t_if_argument,
            if_flag=t_if_flag,
            if_option=t_if_option)

    if args['--output-name'] is None:
        print(out.strip() + '\n')
    else:
        try:
            with open(args['--output-name'], 'w') as f:
                f.write(out.strip() + '\n')
        except IOError as e:
            sys.exit(str(e))
