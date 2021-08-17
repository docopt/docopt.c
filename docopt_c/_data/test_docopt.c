#include "docopt.c"

#define assert(x) \
    if (x) \
        printf("."); \
    else \
        printf("\n[%s, line %d] test failed", __FILE__, __LINE__)

 /*
  * TokenStream
  */

int test_tokens(void) {
    char *argv[] = {"prog", "-o", "12"};
    Tokens ts = tokens_new(3, argv);

    assert(!strcmp(ts.current, "prog"));
    tokens_move(&ts);
    assert(!strcmp(ts.current, "-o"));
    tokens_move(&ts);
    assert(!strcmp(ts.current, "12"));
    tokens_move(&ts);
    assert(ts.current == NULL);
    return EXIT_SUCCESS;
}

 /*
  * parse_shorts
  */

int test_parse_shorts_1(void) {
    int ret = EXIT_SUCCESS;
    char *argv[] = {"-a"};
    struct Tokens ts = tokens_new(1, argv);
    struct Option options[] = {
        {"-a", NULL, false, false, NULL}
    };
    struct Option option;
    struct Elements elements = {0, 0, 1, NULL, NULL, options};

    ret = parse_shorts(&ts, &elements);
    option = options[0];
    assert(!ret);
    if (ret) return ret;
    assert(!strcmp(option.oshort, "-a"));
    assert(option.olong == NULL);
    assert(option.argcount == false);
    assert(option.value == true);
    assert(option.argument == NULL);
    return EXIT_SUCCESS;
}

int test_parse_shorts_2(void) {
    int ret = EXIT_SUCCESS;
    char *argv[] = {"-ab"};
    struct Tokens ts = tokens_new(1, argv);
    struct Option options[] = {
        {"-a", NULL, false, false, NULL},
        {"-b", NULL, false, false, NULL}
    };
    struct Option option1, option2;
    struct Elements elements = {0, 0, 2, NULL, NULL, options};

    ret = parse_shorts(&ts, &elements);
    option1 = options[0];
    option2 = options[1];
    assert(!ret);
    if (ret) return ret;
    assert(!strcmp(option1.oshort, "-a"));
    assert(option1.value == true);
    assert(!strcmp(option2.oshort, "-b"));
    assert(option2.value == true);
    return EXIT_SUCCESS;
}

int test_parse_shorts_3(void) {
    int ret = EXIT_SUCCESS;
    char *argv[] = {"-b"};
    struct Tokens ts = tokens_new(1, argv);
    struct Option options[] = {
        {"-a", NULL, false, false, NULL},
        {"-b", NULL, false, false, NULL}
    };
    struct Option option1, option2;
    struct Elements elements = {0, 0, 2, NULL, NULL, options};

    ret = parse_shorts(&ts, &elements);
    option1 = options[0];
    option2 = options[1];
    assert(!ret);
    if (ret) return ret;
    assert(!strcmp(option1.oshort, "-a"));
    assert(option1.value == false);
    assert(!strcmp(option2.oshort, "-b"));
    assert(option2.value == true);
    return EXIT_SUCCESS;
}

int test_parse_shorts_4(void) {
    int ret = EXIT_SUCCESS;
    char *argv[] = {"-aARG"};
    struct Tokens ts = tokens_new(1, argv);
    struct Option options[] = {
        {"-a", NULL, true, false, NULL}
    };
    struct Option option;
    struct Elements elements = {0, 0, 1, NULL, NULL, options};

    ret = parse_shorts(&ts, &elements);
    option = options[0];
    assert(!ret);
    if (ret) return ret;
    assert(!strcmp(option.oshort, "-a"));
    assert(option.value == false);
    assert(!strcmp(option.argument, "ARG"));
    return EXIT_SUCCESS;
}

int test_parse_shorts_5(void) {
    int ret = EXIT_SUCCESS;
    char *argv[] = {"-a", "ARG"};
    struct Tokens ts = tokens_new(2, argv);
    struct Option options[] = {
        {"-a", NULL, true, false, NULL}
    };
    struct Option option;
    struct Elements elements = {0, 0, 1, NULL, NULL, options};

    ret = parse_shorts(&ts, &elements);
    option = options[0];
    assert(!ret);
    if (ret) return ret;
    assert(!strcmp(option.oshort, "-a"));
    assert(option.value == false);
    assert(!strcmp(option.argument, "ARG"));
    return EXIT_SUCCESS;
}

 /*
  * parse_long
  */

int test_parse_long_1(void) {
    int ret = EXIT_SUCCESS;
    char *argv[] = {"--all"};
    struct Tokens ts = tokens_new(1, argv);
    struct Option options[] = {
        {NULL, "--all", false, false, NULL}
    };
    struct Option option;
    struct Elements elements = {0, 0, 1, NULL, NULL, options};

    ret = parse_long(&ts, &elements);
    option = options[0];
    assert(!ret);
    if (ret) return ret;
    assert(option.oshort == NULL);
    assert(!strcmp(option.olong, "--all"));
    assert(option.argcount == false);
    assert(option.value == true);
    assert(option.argument == NULL);
    return EXIT_SUCCESS;
}

int test_parse_long_2(void) {
    int ret = EXIT_SUCCESS;
    char *argv[] = {"--all"};
    struct Tokens ts = tokens_new(1, argv);
    struct Option options[] = {
        {NULL, "--all", false, false, NULL},
        {NULL, "--not", false, false, NULL}
    };
    struct Option option1;
    struct Option option2;
    struct Elements elements = {0, 0, 2, NULL, NULL, options};

    ret = parse_long(&ts, &elements);
    option1 = options[0];
    option2 = options[1];
    assert(!ret);
    if (ret) return ret;
    assert(option1.oshort == NULL);
    assert(!strcmp(option1.olong, "--all"));
    assert(option1.argcount == false);
    assert(option1.value == true);
    assert(option1.argument == NULL);
    assert(option2.oshort == NULL);
    assert(!strcmp(option2.olong, "--not"));
    assert(option2.argcount == false);
    assert(option2.value == false);
    assert(option2.argument == NULL);
    return EXIT_SUCCESS;
}

int test_parse_long_3(void) {
    int ret = EXIT_SUCCESS;
    char *argv[] = {"--all=ARG"};
    struct Tokens ts = tokens_new(1, argv);
    struct Option options[] = {
        {NULL, "--all", true, false, NULL}
    };
    struct Option option;
    struct Elements elements = {0, 0, 1, NULL, NULL, options};

    ret = parse_long(&ts, &elements);
    option = options[0];
    assert(!ret);
    if (ret) return ret;
    assert(option.oshort == NULL);
    assert(!strcmp(option.olong, "--all"));
    assert(option.argcount == true);
    assert(option.value == false);
    assert(!strcmp(option.argument, "ARG"));
    return EXIT_SUCCESS;
}

int test_parse_long_4(void) {
    int ret = EXIT_SUCCESS;
    char *argv[] = {"--all", "ARG"};
    struct Tokens ts = tokens_new(2, argv);
    struct Option options[] = {
        {NULL, "--all", true, false, NULL}
    };
    struct Option option;
    struct Elements elements = {0, 0, 1, NULL, NULL, options};

    ret = parse_long(&ts, &elements);
    option = options[0];
    assert(!ret);
    if (ret) return ret;
    assert(option.oshort == NULL);
    assert(!strcmp(option.olong, "--all"));
    assert(option.argcount == true);
    assert(option.value == false);
    assert(!strcmp(option.argument, "ARG"));
    return EXIT_SUCCESS;
}

 /*
  * parse_args
  */

int test_parse_args_1(void) {
    struct Command commands[] = {};
    struct Argument arguments[] = {};
    struct Option options[] = {
        {NULL, "--all", false, false, NULL},
        {"-b", NULL, false, false, NULL},
        {"-W", NULL, true, false, NULL}
    };
    struct Elements elements = {0, 0, 3, commands, arguments, options};
    char *argv[] = {"--all", "-b", "ARG"};
    struct Tokens ts = tokens_new(3, argv);
    int ret;

    ret = parse_args(&ts, &elements);
    assert(!ret);
    if (ret) return ret;
    assert(!strcmp(options[0].olong, "--all"));
    assert(options[0].value == true);
    assert(!strcmp(options[1].oshort, "-b"));
    assert(options[1].value == true);
    assert(!strcmp(options[2].oshort, "-W"));
    assert(options[2].argument == NULL);
    return EXIT_SUCCESS;
}

int test_parse_args_2(void) {
    struct Command commands[] = {};
    struct Argument arguments[] = {};
    struct Option options[] = {
        {NULL, "--all", false, false, NULL},
        {"-b", NULL, false, false, NULL},
        {"-W", NULL, true, false, NULL}
    };
    struct Elements elements = {0, 0, 3, commands, arguments, options};
    char *argv[] = {"ARG", "-Wall"};
    struct Tokens ts = tokens_new(2, argv);
    int ret;

    ret = parse_args(&ts, &elements);
    assert(!ret);
    if (ret) return ret;
    assert(!strcmp(options[0].olong, "--all"));
    assert(options[0].value == false);
    assert(!strcmp(options[1].oshort, "-b"));
    assert(options[1].value == false);
    assert(!strcmp(options[2].oshort, "-W"));
    assert(!strcmp(options[2].argument, "all"));
    return 0;
}

int main(int argc, char *argv[]) {
    int (*functions[])(void) = {test_tokens,
                                   test_parse_shorts_1,
                                   test_parse_shorts_2,
                                   test_parse_shorts_3,
                                   test_parse_shorts_4,
                                   test_parse_shorts_5,

                                   test_parse_long_1,
                                   test_parse_long_2,
                                   test_parse_long_3,
                                   test_parse_long_4,

                                   test_parse_args_1,
                                   test_parse_args_2,
                                   NULL};
    int (*function)(void);
    int i = -1;
    int ret;

    for (function = functions[++i];
         function != NULL;
         function = functions[++i]) {
        printf("%d", i);
        ret = (*function)();
        if (ret) {
            puts("\nFAILURE!");
            return EXIT_FAILURE;
        }
    }
    puts(" OK!");
    return EXIT_SUCCESS;
}
