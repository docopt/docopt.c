#include "docopt.h"

int main(int argc, char *argv[])
{
    struct DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ "2.0rc2");

    puts("Commands");
    printf("\tmine == %s\n", args.mine ? "true" : "false");
    printf("\tmove == %s\n", args.move ? "true" : "false");
    printf("\tcreate == %s\n", args.create ? "true" : "false");
    printf("\tremove == %s\n", args.remove ? "true" : "false");
    printf("\tset == %s\n", args.set ? "true" : "false");
    printf("\tship == %s\n", args.ship ? "true" : "false");
    printf("\tshoot == %s\n", args.shoot ? "true" : "false");
    puts("Arguments");
    printf("\tx == %s\n", args.x);
    printf("\ty == %s\n", args.y);
    puts("Flags");
    printf("\t--drifting == %s\n", args.drifting ? "true" : "false");
    printf("\t--help == %s\n", args.help ? "true" : "false");
    printf("\t--moored == %s\n", args.moored ? "true" : "false");
    printf("\t--version == %s\n", args.version ? "true" : "false");
    puts("Options");
    printf("\t--speed == %s\n", args.speed);

    return EXIT_SUCCESS;
}
