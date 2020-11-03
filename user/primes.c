#include "kernel/types.h"
#include "user/user.h"

void function(int *num, int numsize)
{
    int pipe_1[2];
    pipe(pipe_1);
    int count=0;
    printf("prime %d\n",num[0]);
    int nextnum[34];
    
    if(fork())
    {
        close(pipe_1[0]);
        for(int i=0;i<numsize;i++)
        {
            if(num[i]%num[0]!=0)
            {
                write(pipe_1[1],&num[i],sizeof(num[i]));
            }
        }
        close(pipe_1[1]);
        wait();
    }
    else
    {
        close(pipe_1[1]);
        while(read(pipe_1[0],&nextnum[count],sizeof(nextnum[count]))) 
        {
            count++;
            //printf("%d,%d\n",nextnum[count],count);
        }
     
        close(pipe_1[0]);
        if(count>=1)
        {
            function(nextnum,count);
        }    
    }
    exit();
}
int main(int argc, char *argv[]) 
{
    int buffer[34];
    for(int i=0;i<34;i++)
    {
        buffer[i]=i+2;
    }
    function(buffer,34);
    
    exit();
}