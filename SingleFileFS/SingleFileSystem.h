#include <linux/types.h>
#ifndef SYNCRONUS
#define SYNCRONUS 1
#endif

#ifndef NBLOCK
#define NBLOCK 10
#endif

#define MOD_NAME "modulo_File_System"
#define MAGICNUMBER 0x42424242
#define DEFAULT_BLOCK_SIZE 4096
#define UNIQUE_FILE_NAME "the-file"

#define FILENAME_MAXLEN 255
#define SINGLEFILEFS_FILE_INODE_NUMBER 1
#define SINGLEFILEFS_ROOT_INODE_NUMBER 10
#define SINGLEFILEFS_INODES_BLOCK_NUMBER 1
//struttura dei metadati di ogni blocco del fs
typedef struct metadati_block_ram{
    int time;
    unsigned int valid:1;
    size_t dimension;
    int index_block;
    int countLettore;
    int countScrittore;
    struct metadati_block_ram *next;
}metadati_block_ram;

typedef struct metadati_block_struct{
    int time;
    unsigned int valid:1;
    size_t dimension;
    //int index_block; me sa non serve
}metadati_block_struct;

typedef struct myfiledata_struct{
    int num;
}myfiledata_struct;
#define MAXBLOCKDATA 4096 - sizeof(metadati_block_struct)

//struttura di ogni blocco del fs
typedef struct block_file_struct{
    metadati_block_struct block_information;
    char dati[MAXBLOCKDATA]; 
}block_file_struct;


//definizione dello specific superblock
typedef struct struct_sb_information{
    uint64_t magic;
    uint64_t version;
    uint64_t nblock;
    uint64_t nblock_in_use;
    //padding da lasciare al superblock.
    char padding[(4*1024)-(4*sizeof(uint64_t))];
}struct_sb_information;

//definizione myInode
typedef struct struct_MyInode{
    mode_t mode;
    uint64_t inode_number;
    uint64_t data_block_number;
    union {
        uint64_t file_size;
        uint64_t dir_children_count;
    };
}struct_MyInode;



extern metadati_block_ram *testa_valid;
extern metadati_block_ram *testa_invalid;
//file.c
extern const struct inode_operations onefilefs_inode_ops;
extern const struct file_operations onefilefs_file_operations;
// dir.c
extern const struct file_operations onefilefs_dir_operations;
extern void stampa_invalid(void);
extern void stampa_valid(void);
extern int put_data(char * source,size_t size);
extern int get_data(int ,char * ,size_t );
extern int invalidate_data(int );