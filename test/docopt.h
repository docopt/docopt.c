#ifndef DOCOPT_DOCOPT_H
#define DOCOPT_DOCOPT_H

#include <stddef.h>

#if defined(__STDC__) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L

#include <stdbool.h>

#elif !defined(_STDBOOL_H)
#define _STDBOOL_H

#include <stdlib.h>

#ifdef true
#undef true
#endif
#ifdef false
#undef false
#endif
#ifdef bool
#undef bool
#endif

#define true 1
#define false (!true)
typedef size_t bool;

#endif

#if defined(_AIX)

#include <sys/limits.h>

#elif defined(__FreeBSD__) || defined(__NetBSD__)
|| defined(__OpenBSD__) || defined(__bsdi__)
|| defined(__DragonFly__) || defined(macintosh)
|| defined(__APPLE__) || defined(__APPLE_CC__)

#include <sys/syslimits.h>

#elif defined(__HAIKU__)

#include <system/user_runtime.h>

#elif defined(__linux__) || defined(linux) || defined(__linux)

#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,22)

#include <linux/limits.h>

#else

#define ARG_MAX       131072    /* # bytes of args + environ for exec() */
/* it's no longer defined, see this example and more at https://unix.stackexchange.com/q/120642 */

#endif

#elif (defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__)  || defined(__DragonFly__) || defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__))

#include <sys/param.h>

#if defined(__APPLE__) || defined(__APPLE_CC__)
/* ARG_MAX gives a segfault on macOS when used for array size below */
#undef ARG_MAX
#undef NCARGS
#endif

#else

#include <limits.h>

#endif

#ifndef ARG_MAX
#ifdef NCARGS
#define ARG_MAX NCARGS
#else
#define ARG_MAX 131072
#endif
#endif

struct DocoptArgs {
    
    /* commands */
    size_t create;
    size_t mine;
    size_t move;
    size_t remove;
    size_t set;
    size_t ship;
    size_t shoot;
    /* arguments */
    char *name;
{indent}char *x;
{indent}char *y;
    /* options without arguments */
    size_t drifting;
    size_t help;
    size_t moored;
    size_t version;
    /* options with arguments */
    char *speed;
    /* special */
    const char *usage_pattern;
    const char *help_message[17];
};

struct DocoptArgs docopt(int, char *[], bool, const char *);

#endif
