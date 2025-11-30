#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("=== MLFQ Scheduler Test ===\n\n");
  
  printf("Starting mixed workload test...\n");
  printf("2 CPU-bound processes + 2 I/O-bound processes\n\n");
  
  int pid1 = fork();
  if(pid1 == 0) {
    // CPU-bound 1
    printf("[CPU-1] Starting (PID: %d)\n", getpid());
    for(int i = 0; i < 100; i++) {
      for(int j = 0; j < 100000; j++);
      if(i % 25 == 0)
        printf("[CPU-1] Progress: %d%%\n", i);
    }
    printf("[CPU-1] Done\n");
    exit(0);
  }
  
  int pid2 = fork();
  if(pid2 == 0) {
    // I/O-bound 1
    printf("[IO-1] Starting (PID: %d)\n", getpid());
    for(int i = 0; i < 20; i++) {
      sleep(3);
      printf("[IO-1] Tick %d\n", i);
    }
    printf("[IO-1] Done\n");
    exit(0);
  }
  
  int pid3 = fork();
  if(pid3 == 0) {
    // CPU-bound 2
    printf("[CPU-2] Starting (PID: %d)\n", getpid());
    for(int i = 0; i < 100; i++) {
      for(int j = 0; j < 100000; j++);
      if(i % 25 == 0)
        printf("[CPU-2] Progress: %d%%\n", i);
    }
    printf("[CPU-2] Done\n");
    exit(0);
  }
  
  int pid4 = fork();
  if(pid4 == 0) {
    // I/O-bound 2
    printf("[IO-2] Starting (PID: %d)\n", getpid());
    for(int i = 0; i < 20; i++) {
      sleep(3);
      printf("[IO-2] Tick %d\n", i);
    }
    printf("[IO-2] Done\n");
    exit(0);
  }
  
  // Parent waits for all children
  wait(0);
  wait(0);
  wait(0);
  wait(0);
  
  printf("\n=== Test Complete ===\n");
  printf("Final process state:\n\n");
  getprocinfo();
  
  exit(0);
}
