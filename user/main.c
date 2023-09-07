#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#define NBLOCK 100
int bitmask[4];
void setBitUP(int index){
    int i= index/32;
    __sync_fetch_and_or(&(bitmask[i]),(1 << (index%32)));    
}
//imposta un bit a 0 della bit mask corrispondente
void setBitDown(int index){
    int i= index/32;
    uint64_t x = ~(1 << (index%32));
    __sync_fetch_and_and(&(bitmask[i]),x);
}
//verifica il valore del bit del blocco
//torna 1 se nella bitmask il bit index é 1
//torna 0 se nella bitmask il bit index é 0 
int checkBit(int index){
    int i;
    uint64_t pos;
    i=index/32;
    pos = 1 << (index%32);
    return (bitmask[i] & pos) == pos;
}
//trova un blocco con bit pari a 0
int trovaBit(){
    int i,index;
    index=-1;
    for(i=0;i<NBLOCK;i++){
        if(!checkBit(i)){
            index=i;
            break;
        }
    }
    return index;
}
int main(int argc,char*argv[]){
    bitmask[0]=0;
    bitmask[1]=0;
    for(int i=0;i<NBLOCK+1;i++){
        int index=trovaBit();
        printf("i=%d]index=%d\n",index,i);
        setBitUP(index);
        printf("[%lld][%lld]\n",bitmask[0],bitmask[1]);
    }
    for(int i=0;i<NBLOCK+1;i++){
        int index=trovaBit();
        printf("i=%d]index=%d\n",index,i);
        setBitDown(index);
        printf("[%lld][%lld]\n",bitmask[0],bitmask[1]);
    }
    return 0;
}