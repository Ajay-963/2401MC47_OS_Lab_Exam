#include "types.h"
#include "user.h"

int
main(void)
{
  char *page;
  int pid;

  page = sbrk(4096);
  if(page == (char*)-1){
    printf(1, "prottest: sbrk failed\n");
    exit();
  }
  *page = 'A';
  printf(1, "read before protection: %c\n", *page);
  if(mprotect(page, 4096) < 0){
    printf(1, "prottest: mprotect failed\n");
    exit();
  }
  printf(1, "read after protection: %c\n", *page);
  pid = fork();
  if(pid < 0){
    printf(1, "prottest: fork failed\n");
    exit();
  }
  if(pid == 0){
    *page = 'B';
    printf(1, "child write unexpectedly succeeded\n");
    exit();
  }
  wait();
  printf(1, "child was killed; parent reads: %c\n", *page);
  if(munprotect(page, 4096) < 0){
    printf(1, "prottest: munprotect failed\n");
    exit();
  }
  *page = 'C';
  printf(1, "after munprotect: %c\n", *page);
  printf(1, "unaligned: %d\n", mprotect(page + 1, 4096));
  printf(1, "zero length: %d\n", mprotect(page, 0));
  printf(1, "beyond end: %d\n", mprotect(page + 4096, 4096));
  exit();
}
