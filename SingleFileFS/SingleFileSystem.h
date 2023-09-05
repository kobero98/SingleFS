#include <linux/types.h>
#include <linux/fs.h>
#include "Create_FS.h"

#ifndef SYNC
#define SYNCRONUS if(0)
#else
#define SYNCRONUS if(1)
#endif

#ifdef DEBUG
#define AUDIT if(1)
#else
#define AUDIT if(0)
#endif

#define MOD_NAME "modulo_File_System"
#define DEFAULT_BLOCK_SIZE 4096
#define UNIQUE_FILE_NAME "the-file"

#define FILENAME_MAXLEN 255
#define SINGLEFILEFS_FILE_INODE_NUMBER 1
#define SINGLEFILEFS_ROOT_INODE_NUMBER 10
#define SINGLEFILEFS_INODES_BLOCK_NUMBER 1
#define DIMENSIONEBITMASK (NBLOCK/(sizeof(uint64_t)*8) + 1*((NBLOCK % 64)!=0))


typedef struct metadati_block{
    uint64_t time;
    int index_block;
}metadati_block;

typedef struct metadati_block_element{
    metadati_block block;
    struct metadati_block_element *next;
}metadati_block_element;

typedef struct registro_atomico{
    int num_entry;
    int num_exit;
}registro_atomico;

typedef struct atomic_register{
    registro_atomico * reg;
    int lockScrittore;
    int lockInvalid;
    uint64_t bitmask[DIMENSIONEBITMASK];
    metadati_block_element * testa;
    metadati_block_element * coda;
}atomic_register;
//struttura dei metadati di ogni blocco del fs
typedef struct myfiledata_struct{
    uint64_t time;
}myfiledata_struct;

extern struct super_block * the_sb;
//file.c
extern const struct inode_operations onefilefs_inode_ops;
extern const struct file_operations onefilefs_file_operations;
// dir.c
extern const struct file_operations onefilefs_dir_operations;

extern int put_data(char * source,size_t size);
extern int get_data(int offset,char * destination ,size_t size);
extern int invalidate_data(int );
extern uint64_t getTime(void);
extern void setBitUP(int);
extern void setBitDown(int);
extern int checkBit(int);
extern int trovaBit(void);