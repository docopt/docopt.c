#ifndef DOCOPT_$header_no_ext_H
#define DOCOPT_$header_no_ext_H

#include <stddef.h>

struct DocoptArgs {$commands$arguments$flags$options
    /* special */
    const char *usage_pattern;
    const char *help_message;
};

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

struct DocoptArgs docopt(size_t,char *[],bool,const char *);

#endif
