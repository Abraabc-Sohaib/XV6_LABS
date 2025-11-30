#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("I/O-bound process starting (PID: %d)\n", getpid());
  
  int i;
  for(i = 0; i < 30; i++) {
    sleep(2);  // Simulate I/O
    printf("I/O-bound: iteration %d\n", i);
  }
  
  printf("I/O-bound process done\n");
  exit(0);
}
