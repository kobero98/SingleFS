#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include "Create_FS.h"
#include <malloc.h>


int main(int argc,char *argv[]){
    int fd, nbytes;
    ssize_t ret;
    struct_sb_information sb;
    struct_MyInode root_inode;
    struct_MyInode file_inode;
    char * file_body= "volsi cosi cola come si pote cio che si vuole";
    if(argc != 2){
        return -1;
    }
    fd= open(argv[1],O_RDWR);
    if(fd == -1) return -1;
    sb.magic = MAGICNUMBER;
    sb.version = 0.1;
    sb.nblock = 100;
    sb.nBlockMetadata=1; // devo cambiare un po di cose
    sb.first.offset=0;
    sb.first.sb=0;
    sb.last.offset=7;
    sb.last.sb=0;
    ret = write(fd,(char * )&sb, sizeof(sb));
    if(ret != 4096) {
        close(fd);
        return -ret;
    }
    struct_sb_metadata * metadata=calloc(1,sizeof(struct_sb_metadata));
    
    //modificare il client in modo da poter fare più inizializzazioni
    metadata->vet[0].succ.sb=0;
    metadata->vet[0].succ.offset=1;
    metadata->vet[0].prec.sb=0;
    metadata->vet[0].prec.offset=-1;
    metadata->vet[0].valid=1;
    
    metadata->vet[1].succ.sb=0;
    metadata->vet[1].succ.offset=4;
    metadata->vet[1].prec.sb=0;
    metadata->vet[1].prec.offset=0;
    metadata->vet[1].valid=1;
    
    metadata->vet[2].succ.sb=0;
    metadata->vet[2].succ.offset=0;
    metadata->vet[2].prec.sb=0;
    metadata->vet[2].prec.offset=0;
    metadata->vet[2].valid=0;

    metadata->vet[3].succ.sb=0;
    metadata->vet[3].succ.offset=8;
    metadata->vet[3].prec.sb=0;
    metadata->vet[3].prec.offset=5;
    metadata->vet[3].valid=1;
    
    metadata->vet[4].succ.sb=0;
    metadata->vet[4].succ.offset=5;
    metadata->vet[4].prec.sb=0;
    metadata->vet[4].prec.offset=1;
    metadata->vet[4].valid=0;
    
    metadata->vet[5].succ.sb=0;
    metadata->vet[5].succ.offset=3;
    metadata->vet[5].prec.sb=0;
    metadata->vet[5].prec.offset=4;
    metadata->vet[5].valid=1;

    metadata->vet[6].succ.sb=0;
    metadata->vet[6].succ.offset=7;
    metadata->vet[6].prec.sb=0;
    metadata->vet[6].prec.offset=8;
    metadata->vet[6].valid=1;
    
    metadata->vet[7].succ.sb=0;
    metadata->vet[7].succ.offset=-1;
    metadata->vet[7].prec.sb=0;
    metadata->vet[7].prec.offset=6;
    metadata->vet[7].valid=0;

    metadata->vet[8].succ.sb=0;
    metadata->vet[8].succ.offset=6;
    metadata->vet[8].prec.sb=0;
    metadata->vet[8].prec.offset=3;
    metadata->vet[8].valid=0;
    ret = write(fd,(char * )metadata, sizeof(struct_sb_metadata));
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
    int i;
    for(i=0;i<8;i++){
        block_file_struct p;
        p.size = strlen(file_body);
        strncpy(p.dati,file_body,strlen(file_body));
        ret = write(fd,(char*)&p,sizeof(p));
    }
    return 0;
}
