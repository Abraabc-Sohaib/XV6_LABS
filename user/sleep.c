#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if (argc == 2) { // we need 2 args, one is ./sleep itself and one is ticks
        int ticks = atoi(argv[1]);
        if (ticks <= 0) {
            fprintf(2, "\nError: ticks must be positive integers\n");
            exit(1);
        }
        pause(ticks);
        exit(0);
    } else {
        fprintf(2, "\nError! Correct usage: sleep <ticks>\n");
        exit(1);
    }
}
