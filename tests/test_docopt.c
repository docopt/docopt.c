#include "../docopt.c"

#define assert(x) printf("%c", x ? '.' : 'F')

 /*
  * TokenStream
  */

void test_token_stream(void) {
    char *argv[] = {"prog", "-o", "12"};
    TokenStream ts = TokenStream_create(3, argv);
    assert(strcmp(ts.current, "prog") == 0);

    ts = TokenStream_move(ts);
    assert(strcmp(ts.current, "-o") == 0);

    ts = TokenStream_move(ts);
    assert(strcmp(ts.current, "12") == 0);

    ts = TokenStream_move(ts);
    assert(ts.current == NULL);
}

 /*
  * parse_shorts
  */

void test_parse_shorts_1(void) {
    char *argv[] = {"-a"};
    TokenStream ts = TokenStream_create(1, argv);
    Element options[1] = {{Option, {"-a", NULL, false, false, NULL}}};
    parse_shorts(ts, options);
    Element o = options[0];
    assert(strcmp(o.option.oshort, "-a") == 0);
    assert(o.option.olong == NULL);
    assert(o.option.argcount == false);
    assert(o.option.value == true);
    assert(o.option.argument == NULL);
}

void test_parse_shorts_2(void) {
    char *argv[] = {"-ab"};
    TokenStream ts = TokenStream_create(1, argv);
    Element options[2] = {{Option, {"-a", NULL, false, false, NULL}},
                          {Option, {"-b", NULL, false, false, NULL}}};
    parse_shorts(ts, options);
    Element o1 = options[0];
    Element o2 = options[1];
    assert(strcmp(o1.option.oshort, "-a") == 0);
    assert(o1.option.value == true);
    assert(strcmp(o2.option.oshort, "-b") == 0);
    assert(o2.option.value == true);
}

void test_parse_shorts_3(void) {
    char *argv[] = {"-b"};
    TokenStream ts = TokenStream_create(1, argv);
    Element options[2] = {{Option, {"-a", NULL, false, false, NULL}},
                          {Option, {"-b", NULL, false, false, NULL}}};
    parse_shorts(ts, options);
    Element o1 = options[0];
    Element o2 = options[1];
    assert(strcmp(o1.option.oshort, "-a") == 0);
    assert(o1.option.value == false);
    assert(strcmp(o2.option.oshort, "-b") == 0);
    assert(o2.option.value == true);
}

void test_parse_shorts_4(void) {
    char *argv[] = {"-aARG"};
    TokenStream ts = TokenStream_create(1, argv);
    Element options[1] = {{Option, {"-a", NULL, true, false, NULL}}};
    parse_shorts(ts, options);
    Element o = options[0];
    assert(strcmp(o.option.oshort, "-a") == 0);
    assert(o.option.value == false);
    assert(strcmp(o.option.argument, "ARG") == 0);
}

void test_parse_shorts_5(void) {
    char *argv[] = {"-a", "ARG"};
    TokenStream ts = TokenStream_create(2, argv);
    Element options[1] = {{Option, {"-a", NULL, true, false, NULL}}};
    ts = parse_shorts(ts, options);
    assert(ts.current == NULL);
    Element o = options[0];
    assert(strcmp(o.option.oshort, "-a") == 0);
    assert(o.option.value == false);
    assert(strcmp(o.option.argument, "ARG") == 0);
}

 /*
  * parse_long
  */

void test_parse_long_1(void) {
    char *argv[] = {"--all"};
    TokenStream ts = TokenStream_create(1, argv);
    Element options[1] = {{Option, {NULL, "--all", false, false, NULL}}};
    parse_long(ts, options);
    Element o = options[0];
    assert(o.option.oshort == NULL);
    assert(strcmp(o.option.olong, "--all") == 0);
    assert(o.option.argcount == false);
    assert(o.option.value == true);
    assert(o.option.argument == NULL);
}

void test_parse_long_2(void) {
    char *argv[] = {"--all"};
    TokenStream ts = TokenStream_create(1, argv);
    Element options[2] = {{Option, {NULL, "--all", false, false, NULL}},
                          {Option, {NULL, "--not", false, false, NULL}}};
    parse_long(ts, options);
    Element o1 = options[0];
    Element o2 = options[1];

    assert(o1.option.oshort == NULL);
    assert(strcmp(o1.option.olong, "--all") == 0);
    assert(o1.option.argcount == false);
    assert(o1.option.value == true);
    assert(o1.option.argument == NULL);

    assert(o2.option.oshort == NULL);
    assert(strcmp(o2.option.olong, "--not") == 0);
    assert(o2.option.argcount == false);
    assert(o2.option.value == false);
    assert(o2.option.argument == NULL);
}

void test_parse_long_3(void) {
    char tmp[] = "--all=ARG";
    char *argv[] = {tmp};
    TokenStream ts = TokenStream_create(1, argv);
    Element options[1] = {{Option, {NULL, "--all", true, false, NULL}}};
    parse_long(ts, options);
    assert(memcmp(tmp, "--all\0ARG", 9) == 0);
    Element o = options[0];
    assert(o.option.oshort == NULL);
    assert(strcmp(o.option.olong, "--all") == 0);
    assert(o.option.argcount == true);
    assert(o.option.value == false);
    assert(strcmp(o.option.argument, "ARG") == 0);
}

void test_parse_long_4(void) {
    //char tmp[] = "--all ARG";
    char *argv[] = {"--all", "ARG"};
    TokenStream ts = TokenStream_create(2, argv);
    Element options[1] = {{Option, {NULL, "--all", true, false, NULL}}};
    parse_long(ts, options);
    //assert(memcmp(tmp, "--all\0ARG", 9) == 0);
    Element o = options[0];
    assert(o.option.oshort == NULL);
    assert(strcmp(o.option.olong, "--all") == 0);
    assert(o.option.argcount == true);
    assert(o.option.value == false);
    assert(strcmp(o.option.argument, "ARG") == 0);
}

 /*
  * parse_args
  */

void test_parse_args_1(void) {
    Element options[4] = {{Option, {NULL, "--all", false, false, NULL}},
                          {Option, {"-b", NULL, false, false, NULL}},
                          {Option, {"-W", NULL, true, false, NULL}},
                          {None}};
    char *argv[] = {"--all", "-b", "ARG"};
    TokenStream ts = TokenStream_create(3, argv);
    parse_args(ts, options);
    assert(strcmp(options[0].option.olong, "--all") == 0);
    assert(options[0].option.value == true);
    assert(strcmp(options[1].option.oshort, "-b") == 0);
    assert(options[1].option.value == true);
    assert(strcmp(options[2].option.oshort, "-W") == 0);
    assert(options[2].option.argument == NULL);
}

void test_parse_args_2(void) {
    Element options[4] = {{Option, {NULL, "--all", false, false, NULL}},
                          {Option, {"-b", NULL, false, false, NULL}},
                          {Option, {"-W", NULL, true, false, NULL}},
                          {None}};
    char *argv[] = {"ARG", "-Wall"};
    TokenStream ts = TokenStream_create(2, argv);
    parse_args(ts, options);
    assert(strcmp(options[0].option.olong, "--all") == 0);
    assert(options[0].option.value == false);
    assert(strcmp(options[1].option.oshort, "-b") == 0);
    assert(options[1].option.value == false);
    assert(strcmp(options[2].option.oshort, "-W") == 0);
    assert(strcmp(options[2].option.argument, "all") == 0);
}

int main(int argc, char *argv[]) {

    test_token_stream();

    test_parse_shorts_1();
    test_parse_shorts_2();
    test_parse_shorts_3();
    test_parse_shorts_4();
    test_parse_shorts_5();

    test_parse_long_1();
    test_parse_long_2();
    test_parse_long_3();
    test_parse_long_4();

    test_parse_args_1();
    test_parse_args_2();

    puts("OK!");
    exit(0);
}
