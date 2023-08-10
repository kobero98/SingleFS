#include <unistd.h>
#include <stdio.h>
unsigned long getTime(){
    unsigned long Uptime=0;
    unsigned long LowTime=0;
    __asm__ volatile (
    "rdtsc  \n"
    : "=a" (LowTime), "=d" (Uptime)
    :
    :  
    );
    unsigned long timetotal = Uptime<<32 | LowTime;
    return timetotal;
}
int main(){
    // char *a="ciao";
    // int n=4; 
    // int i=syscall(134,a,n);
    // printf("%d\n",i);
    // char b[5];
    for(int i=0;i<5;i++){
        char b[100]="";
        int z=syscall(156,i,b,100);
        printf("%d\n",z);
        printf("%s\n",b);
    }
    return 0;
}