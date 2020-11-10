#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
 
#define LINE 128
#define PARAMS 10

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    exit();
  }

  char *cmd = argv[1];
  char line[LINE];
  char *params[PARAMS];
  int index = 0;
  params[index++] = cmd;
  for (int i = 2; i < argc; i++)
  {
    params[index++] = argv[i];
  }
 
  int n = 0;
  while ((n = read(0, line, LINE)) > 0)
  {
    if (fork() == 0)
    {
      char *t = (char *)malloc(sizeof(char) * LINE);
      int c = 0;
      for (int i=0; i < n; i++)
      {
        if (line[i] == '\n' || line[i] == ' ')
        {
          t[c]='\0';
          params[index++] = t;
          c=0;
          t = (char *)malloc(sizeof(char) * LINE);
        }else
          t[c++] = line[i];
      }
      t[c] = '\0';
      params[index]=0;
      exec(cmd, params);
      //printf("exec fail!\n");
      exit();
    }
    else
    {
      wait();
    }
  }
  exit();
}