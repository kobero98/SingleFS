#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>

#include "SingleFileSystem.h"



int main(int argc,char *argv[]){
    int fd, nbytes;
    ssize_t ret;
    struct_sb_information sb;
    struct_MyInode root_inode;
    struct_MyInode file_inode;
    char * file_body= "volsi cosi cola come si pote dove si vuole\n";

    if(argc != 2){
        return -1;
    }
    fd= open(argv[1],O_RDWR);
    if(fd == -1) return -1;

    sb.magic = MAGICNUMBER;
    sb.version = 0.1;
    sb.nblock = 10;

    ret = write(fd,(char * )&sb, sizeof(sb));
    if(ret != 4096) {
        close(fd);
        return -ret;
    }
    file_inode.mode = S_IFREG;
    file_inode.inode_number = 1;
    file_inode.file_size = strlen(file_body)+1;
    fflush(stdout);
    ret = write(fd, (char*)&file_inode,sizeof(file_inode));
    if (ret != sizeof(file_inode)){
        close(fd);
        return -1;
    }
    nbytes = 4096 - sizeof(file_inode);
    char * block_padding = malloc(nbytes);
    ret = write(fd, block_padding, nbytes);
    if (ret != nbytes){
        close(fd);
        return -1;
    }
    block_file_struct p;
    p.block_information.time = 1;
    p.block_information.valid = 0x1;
    p.block_information.dimension = strlen(file_body)+1;
    strncpy(p.dati,file_body,strlen(file_body));
    ret = write(fd,(char*)&p,sizeof(p));
    printf("quanto ha scritto:%d %d\n",ret,sizeof(p));
    printf("%d %d\n",strlen(file_body),p.block_information.valid);
    
    p.block_information.time = 2;
    p.block_information.valid = 0x1;
    p.block_information.dimension = strlen(file_body)+1;
    strncpy(p.dati,file_body,strlen(file_body));
    ret = write(fd,(char*)&p,sizeof(p));
    printf("quanto ha scritto:%d %d\n",ret,sizeof(p));
    printf("%d %d\n",strlen(file_body),p.block_information.valid); 

    p.block_information.time = 3;
    p.block_information.valid = 0x0;
    p.block_information.dimension = strlen(file_body)+1;
    strncpy(p.dati,file_body,strlen(file_body));
    ret = write(fd,(char*)&p,sizeof(p));
    printf("quanto ha scritto:%d %d\n",ret,sizeof(p));
    printf("%d %d\n",strlen(file_body),p.block_information.valid);      
   
    p.block_information.time = 4;
    p.block_information.valid = 0x1;
    p.block_information.dimension = strlen(file_body)+1;
    strncpy(p.dati,file_body,strlen(file_body));
    ret = write(fd,(char*)&p,sizeof(p));
    printf("quanto ha scritto:%d %d\n",ret,sizeof(p));
    printf("%d %d\n",strlen(file_body),p.block_information.valid);
    printf("tutto avvenuto con successo\n");
    return 0;
}
