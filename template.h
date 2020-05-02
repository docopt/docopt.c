#ifndef DOCOPT_$header_no_ext_H
#define DOCOPT_$header_no_ext_H

#include <stddef.h>

struct DocoptArgs {$commands$arguments$flags$options
    /* special */
    const char *usage_pattern;
    const char *help_message;
};

struct DocoptArgs docopt(size_t,char *[],bool,const char *);

#endif
