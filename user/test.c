#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>

#define PUT 134
#define GET 156
#define INVALIDATE 174
#define N 3

void *ThreadRead(void * param){
    int j=0;
    while(j<N){    
        int fd = open("../PointMount/the-file",O_RDONLY);
        if (fd <0){
             fd = open("./PointMount/the-file",O_RDONLY);
        }
        if (fd<0) return NULL;
        int x;
        do{
            char p[1024];
            x=read(fd,p,1024);
            printf("Lettore:fd=%d x=%d %s \n",fd,x,p);
        }while(x>0);
        close(fd);
        j++;
    };
    return NULL;
}
void *ThreadGet(void * param){
    int j=0;
    while(j<N){    
        int index=rand() % 100;
        char buf[4096];
        int ret=syscall(GET,index,buf,strlen(buf));
        printf("GET: index=%d ret=%d buf=%s\n",index,ret,buf);
        j++;
    };
    return NULL;
}
void *ThreadPut(void * param){
    int j=0;
    while(j<N){    
        int index=rand() % 1000;
        char frase[4096];
        sprintf(frase,"%d sotto la panca la capra panca",index);
        int ret=syscall(PUT,frase,strlen(frase));
        printf("PUT: index=%d \n",ret);
    j++;
    };
    return NULL;
}
void* ThreadInvalide(void * param){
    int j=0;
    while(j<N){    
        int index=rand() % 100;
        int ret=syscall(INVALIDATE,index);
        printf("INVALIDATE: index=%d ret=%d\n",index,ret);
        j++;
    };
    return NULL;
}
int main(){
    pthread_t tidPut[2];
    pthread_t tidGet[5];
    pthread_t tidInvalide[2];
    pthread_t tidread;
    int i;
    for(i=0;i<2;i++){
        pthread_create(&(tidPut[i]),NULL,&ThreadPut,NULL);
    }
    for(i=0;i<5;i++){
        pthread_create(&(tidGet[i]),NULL,&ThreadGet,NULL);
    }
    for(i=0;i<2;i++){
        pthread_create(&(tidInvalide[i]),NULL,&ThreadInvalide,NULL);
    }
    pthread_create(&tidread,NULL,&ThreadRead,NULL);
    for(i=0;i<2;i++){
        pthread_join(tidPut[i],NULL);
    }
    for(i=0;i<5;i++){
        pthread_join(tidGet[i],NULL);
    }
    for(i=0;i<2;i++){
        pthread_join(tidInvalide[i],NULL);
    }
    pthread_join(tidread,NULL); 
    return 0;
}