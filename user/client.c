#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#define PUT 134
#define GET 156
#define INVALIDATE 174

int main(){
    int z;
    int index;

    do{
        printf("1]PUT\n");
        printf("2]GET\n");
        printf("3]INVALIDATE\n");
        int scelta;
        scanf("%d",&scelta);
        while(scelta<1 || scelta>3){
            printf("Valore non valido\n");
            scanf("%d",&scelta); 
        };
       switch(scelta){
        case 1:
            printf("PUT\n");
            printf("inserire stringa:\n");
            char * buf1;
            scanf("%ms",&buf1);
            z=syscall(PUT,buf1,strlen(buf1));
            printf("Put restituisce come risposta: %d\n",z);
            break;
        case 2:
            printf("GET\n");
            printf("inserire indice di blocco la leggere:\n");
            scanf("%d",&index);
            char buf2[4096];
            z=syscall(GET,index,buf2,4096);
            printf("la dimensione dei dati letti e: %d",z);
            printf("nel blocco %d ho letto %s\n",index,buf2);
            break;
        case 3:
            printf("INVALIDATE\n");
            printf("inserire indice di blocco la leggere:\n");
            scanf("%d",&index);
            z=syscall(INVALIDATE,index);
            printf("la funzione ritorna: %d\n",z);
            break;
       } 
    }while( true );
    return 0;
}