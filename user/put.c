#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define PUT 134
int main(int argc,char * argv[]){
    if(argc<2){
        printf("Eseguire ./put <stringa da aggiungere>\n");
        return 0;
    }
    char buffer[4098];
    int i=1;
    int len=0;
    for(;i<argc;i++){
        if(len+strlen(argv[i])>=4097)break;
        strcpy(buffer+len,argv[i]);
        len=len+strlen(argv[i]);
        char *spazio=" ";
        printf("buffer\n");
        strcpy(buffer+len,spazio);
        len=len+1;
    }
    printf("buffer=%s\n",buffer);
    int z=syscall(PUT,buffer,strlen(buffer));
    if(z!=-1)   printf("index: %d\n",z);
    else    printf("Tutto lo spazio é occupato\n");
    return 0;
}