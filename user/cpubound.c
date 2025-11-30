#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("CPU-bound process starting (PID: %d)\n", getpid());
  
  int i, j;
  for(i = 0; i < 50; i++) {
    for(j = 0; j < 1000000; j++) {
      // Busy work
    }
    if(i % 10 == 0)
      printf("CPU-bound: iteration %d\n", i);
  }
  
  printf("CPU-bound process done\n");
  exit(0);
}
