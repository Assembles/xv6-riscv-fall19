#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
 
#define LINE 128
#define PARAMS 10

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    exit();
  }
  char line[LINE];
  char *params[PARAMS];
  int index = 0;
  params[index++] = argv[1];
  for (int i = 2; i < argc; i++)
  {
    params[index++] = argv[i];
  }
  int n = 0;
  while ((n = read(0, line, LINE)) > 0)
  {
    if (fork() == 0)
    {
      char *temp = (char *)malloc(sizeof(char) * LINE);
      int count = 0;
      for (int i=0; i < n; i++)
      {
        if (line[i] == '\n' || line[i] == ' ')
        {
          temp[count]='\0';
          params[index++] = temp;
          count=0;
          temp = (char *)malloc(sizeof(char) * LINE);
        }else
          temp[count++] = line[i];
      }
      temp[count] = '\0';
      params[index]=0;
      exec(params[0], params);
      exit();
    }
    else
    {
      wait();
    }
  }
  exit();
}