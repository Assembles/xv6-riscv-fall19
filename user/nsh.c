#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

char params[100][100];

//仿照sh.c文件，用于读取指令
int getcmd(char *buf, int nbuf)
{
  fprintf(2, "@ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

//将输入中的参数拆分，读出其中的命令及符号存到params数组中，返回参数个数
int readcmd(char *buff)
{
  
  int index1 = 0;
  int index2 = 0;
  for(int i=0;buff[i]!='\0'&&buff[i]!='\n';i++)
  {
    if(buff[i]=='<'||buff[i]=='>'||buff[i]=='|')
    {
      if(buff[i-1]!=' ')
      {
        params[index1][index2]='\0';
        index1++;
      }
      index2=0;
      params[index1][index2++]=buff[i];
      params[index1][index2]='\0';
      index1++;
      index2=0;
      //printf("%d<\n",index1);
    }
    
    else if(buff[i]==' '&&buff[i-1]!=' ')
    {
      params[index1][index2]='\0';
      index2=0;
      if(!(buff[i-1]=='<'||buff[i-1]=='>'||buff[i-1]=='|'))
       index1++;
      //printf("%d !\n",index1);
    }
    else if(buff[i]==' '&&buff[i-1]==' ')
    {

    }
    else
    {
      params[index1][index2++]=buff[i];
      //printf("%d",index1);
    }
  }
  params[index1][index2+1]='\0';
  /*
  printf("%d",index1);
  for(int i=0;i<=index1;i++)
  {
    printf("%s%d\n",params[i],i);
  }
  */
  return index1;
 
}

//执行一条指令，按照规定格式打开对应文件并关闭标准流即可，按照标准形式将最后一个记为0
void runcmd(char *p[],int index)
{
  int i=0;
  for(i=0;i<=index;i++)
  {
    if(!strcmp(p[i],">"))
    {
      close(1);
      open(p[i+1],O_CREATE|O_WRONLY);
      p[i]=0;
    }
    if(!strcmp(p[i],"<"))
    {
      close(0);
      open(p[i+1],O_RDONLY);
      p[i]=0;
    }
  }
  p[i]=0;
  exec(p[0],p);
}

//执行管道指令
void runpipe(char *p[],int index)
{
  int i=0;
  //找到第一个‘|’符号
  for(i = 0;i<index;i++)
  {
    if(!strcmp(p[i],"|"))
    {
      p[i]=0;
      break;
    }
  }
  int fd[2];
  pipe(fd);
  //执行最左面的指令，再调用右边的指令
  //如果想递归调用可以把else里的runcmd改成runpipe再稍加修改
  //仿照实验指导书的代码
  if(fork()==0)
  {
    //子进程
    close(1);
    dup(fd[1]);
    close(fd[0]);
    close(fd[1]);
    runcmd(p,i);
  }
  else
  {
    //父进程
    close(0);
    dup(fd[0]);
    close(fd[0]);
    close(fd[1]);
    runcmd(p+i+1,index-i-1);
  }
  close(fd[0]);
  close(fd[1]);
  exit(0);
}

//执行命令，如果有管道相关指令则调用runpipe，否则调用runcmd
void myexec(char *p[],int index)
{
  int flag=0;
  for(int i=1;i<=index;i++)
  {
    if(!strcmp(p[i],"|"))
    {
      flag=1;
    }
  }
  if(flag==1)
  {
    runpipe(p,index);
  }
  else
  {
    runcmd(p,index);
  }
  exit(0);
}

int main(void)
{
  static char buf[100];
  int fd;
  //仿照sh.c读命令

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
    {
      int t=readcmd(buf);
      char * p[100];
      for(int i=0;i<100;i++)
      {
        p[i]=params[i];
      }
      myexec(p,t);
    }
    wait(0);
  }
  
  exit(0);
  
}