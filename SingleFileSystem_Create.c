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
    char * file_body= "volsi cosi cola come si pote dove si vuole";

    if(argc != 2){
        return -1;
    }
    fd= open(argv[1],O_RDWR);
    if(fd == -1) return -1;

    sb.magic = MAGICNUMBER;
    sb.version = 0.1;
    sb.nblock = 3;

    ret = write(fd,(char * )&sb, sizeof(sb));
    if(ret != 4096) {
        printf("%d \n",ret);
        printf("ciao1\n");
        close(fd);
        return -ret;
    }
    file_inode.mode = S_IFREG;
    file_inode.inode_number = 1;
    file_inode.file_size = strlen(file_body);
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
    nbytes = strlen(file_body);
    ret = write(fd,file_body,nbytes);
    if (ret != nbytes){
        close(fd);
        return -1;
    }
    printf("tutto avvenuto con successo\n");
    
    
    return 0;
}
