#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char* fmtname(char *path)//fan hui zui hou yi ge dan ci
{
    char *temp=path;
    char *last=0;
    while(*temp!='\0'){
        if(*temp=='/'){
            last=temp;
        }
        temp++;
    }
    if(last==0){
        return path;
    }
    return last+1;
}

void find(char *path,char *re)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
        p=fmtname(path);
        if(strcmp(p,re)==0)
            printf("%s\n",path);
        break;
 

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      if(strcmp(de.name,".")==0||strcmp(de.name,"..")==0){
        continue;
      }
      find(buf,re);
    }
    break;
  }
  close(fd);
}

int main(int argc, char *argv[])
{
  if(argc <= 2){
    exit();
  }
  find(argv[1],argv[2]);
  exit();
}
