#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("Testing getprocinfo syscall:\n");
  getprocinfo();
  exit(0);
}
