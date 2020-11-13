#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
int
getcmd(char *buf, int nbuf)
{
  fprintf(2, "@ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}
void
runcmd(char *s)
{
    printf("!");
}

int
main(void)
{
  static char buf[100];
  int fd;

  // Ensure that three file descriptors are open.
  while((fd = open("console", O_RDWR)) >= 0)
  {
    if(fd >= 3){
      close(fd);
      break;
    }
  }

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0)
  {
    if(fork() == 0)
      runcmd(buf);
    wait(0);
  }
  exit(0);
}