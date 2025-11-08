#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/syscall.h"
#include "user/user.h"

void usage(char *s) {
  fprintf(2, "Usage: %s <mask> <path> <command>\n", s);
  exit(1);
}

// Sandbox a command by disallowing system calls in mask and
// system calls that are using path
int
main(int argc, char *argv[])
{
    if(argc < 4){
        printf("Usage: sandbox mask path command...\n");
        exit(1);
    }

    int mask = atoi(argv[1]);
    char *path = argv[2];

    if(fork() == 0){
        // child
        interpose(mask, path);
        exec(argv[3], &argv[3]);
        printf("sandbox: exec %s failed\n", argv[3]);
        exit(1);
    } else {
        wait(0);
    }

    exit(0);
}
