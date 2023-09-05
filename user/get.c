#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#define GET 156
int main(int argc,char * argv[]){
    if(argc!=2){
        printf("Eseguire ./get [index]\n");
        return 0;
    }
    int x=atoi(argv[1]);
    if(x==0 && errno!=0 ){
        printf("Valore non valido\n");
        printf("Eseguire ./get [index] ");
        printf("dove index é un numero\n");
        return 0;
    }
    char buffer [4096];
    int z=syscall(GET,x,buffer,4096);
    if(z!=-1) printf("blocco:%d\nil messaggio trovato è: %s\n",x,buffer);
    else printf("indice già non eliminabile\n");
    return 0;
}