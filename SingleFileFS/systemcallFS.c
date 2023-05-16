#include <linux/module.h>
#include <linux/kernel.h>

#include "SingleFileSystem.h"


int put_data(char * source,size_t size){
    printk("sono la systemcall put_data\n");
    return 0;
}
int get_data(int offset,char * destination,size_t size){
    printk("sono la systemcall get_data\n");
    return 0;
}
int invalidate_data(int offset){
    printk("sono la systemcall inv_data\n");
    return 0;
}