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
void *ThreadRead(void * param){
    int fd = open("../PointMount/the-file",O_RDONLY);
    if (fd<0){
        fd = open("./PointMount/the-file",O_RDONLY);
    }
    if(fd<0) return NULL;
    printf("Lettore: fd=%d\n",fd);
    if(fd<0) return NULL;
    while(1){    
        int x;
        char p[1024]="";
        x=read(fd,p,1024);
        if(x>0) printf("Lettore: ret=%d %s \n",x,p);
        sleep(2);
    };
    close(fd);
    return NULL;
}

int main(){
    pthread_t tid;
    pthread_create(&tid,NULL,&ThreadRead,NULL);
    int ret;
    do{
        char frase[100];
        //getc(stdin);
        //fgets(frase, strlen(frase), stdin);
        scanf("%[^\n]",&frase);
        while(getc(stdin)==EOF);    
        ret=syscall(PUT,frase,strlen(frase));
        printf("PUT: index=%d \n",ret);
        sleep(2);
    }while(ret>=0);
    return 0;
}