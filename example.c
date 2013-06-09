#include "docopt.c"

int main(int argc, char *argv[])
{
    DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ "2.0rc2");

    printf("Commands\n");
    printf("    mine == %s\n", args.mine ? "true" : "false");
    printf("    move == %s\n", args.move ? "true" : "false");
    printf("    create == %s\n", args.create ? "true" : "false");
    printf("    remove == %s\n", args.remove ? "true" : "false");
    printf("    set == %s\n", args.set ? "true" : "false");
    printf("    ship == %s\n", args.ship ? "true" : "false");
    printf("    shoot == %s\n", args.shoot ? "true" : "false");
    printf("Arguments\n");
    printf("    x == %s\n", args.x);
    printf("    y == %s\n", args.y);
    printf("Flags\n");
    printf("    --drifting == %s\n", args.drifting ? "true" : "false");
    printf("    --help == %s\n", args.help ? "true" : "false");
    printf("    --moored == %s\n", args.moored ? "true" : "false");
    printf("    --version == %s\n", args.version ? "true" : "false");
    printf("Options\n");
    printf("    --speed == %s\n", args.speed);
    return 0;
}
