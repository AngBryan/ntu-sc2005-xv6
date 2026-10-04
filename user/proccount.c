#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int count = getproccount();
  printf("Number of active processes: %d\n", count);
  exit(0);
}
