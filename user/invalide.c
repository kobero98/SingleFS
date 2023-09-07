#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#define INVALIDATE 174
int main(int argc,char * argv[]){
    if(argc!=2){
        printf("Eseguire ./invalide [index]\n");
        return 0;
    }
    int x=atoi(argv[1]);
    if(x==0 && errno!=0 ){
        printf("Valore non valido\n");
        printf("Eseguire ./invalide [index] ");
        printf("dove index é un numero\n");
        return 0;
    }
    int z=syscall(INVALIDATE,x);
    if(z!=-1)   printf("index: %d\n",z);
    else    printf("indice non valido\n");
    return 0;
}