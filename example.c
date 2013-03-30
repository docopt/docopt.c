#include "docopt.c"

int main(int argc, char *argv[])
{
    DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ "2.0rc2");

    printf("--help == %s\n", args.help ? "true" : "false");
    printf("--version == %s\n", args.version ? "true" : "false");
    printf("--tcp == %s\n", args.tcp ? "true" : "false");
    printf("--serial == %s\n", args.serial ? "true" : "false");
    printf("--host == %s\n", args.host);
    printf("--port == %s\n", args.port);
    printf("--timeout == %s\n", args.timeout);
    printf("--baud == %s\n", args.baud);
}
