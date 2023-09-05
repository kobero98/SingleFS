#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define PUT 134
int main(int argc,char * argv[]){
    if(argc<2){
        printf("Eseguire ./put <stringa da aggiungere>\n");
        return 0;
    }
    int z=syscall(PUT,argv[1],strlen(argv[1]));
    if(z!=-1)   printf("index: %d\n",z);
    else    printf("Tutto lo spazio é occupato\n");
    return 0;
}