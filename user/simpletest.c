#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("Test starting, PID: %d\n", getpid());
  
  for(int i = 0; i < 5; i++) {
    printf("Iteration %d\n", i);
    for(int j = 0; j < 100000; j++);
  }
  
  printf("Test done\n");
  exit(0);
}
