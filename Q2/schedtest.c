#include "types.h"
#include "user.h"

static void
work(int priority)
{
  volatile uint i;
  for(i = 0; i < 30000000; i++)
    ;
  printf(1, "priority %d finished\n", priority);
  exit();
}

int
main(void)
{
  int pid, i;
  int priorities[3] = {5, 10, 15};

  if(set_priority(getpid(), 1) < 0){
    printf(1, "schedtest: failed to set parent priority\n");
    exit();
  }
  for(i = 0; i < 3; i++){
    pid = fork();
    if(pid < 0){
      printf(1, "schedtest: fork failed\n");
      exit();
    }
    if(pid == 0){
      if(set_priority(getpid(), priorities[i]) < 0)
        exit();
      work(priorities[i]);
    }
  }
  for(i = 0; i < 3; i++)
    wait();
  printf(1, "Done\n");
  exit();
}
