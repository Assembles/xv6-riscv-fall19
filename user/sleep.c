  
#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if (argc != 2)
    write(2, "Error message", strlen("Error message"));

  int sleep_time = atoi(argv[1]);

  sleep(sleep_time);
  printf("Sleep %d\n",sleep_time);
  exit();
}