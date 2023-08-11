#include <linux/types.h>
#include <linux/fs.h>

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
#define DIMENSIONEBITMASK NBLOCK/64 + 1*((NBLOCK % 64)!=0)

typedef struct metadati_block{
    uint64_t time;
    int index_block;
    unsigned int valid:1;
}metadati_block;
typedef struct metadati_block_element{
    metadati_block block;
    struct metadati_block_element *next;
}metadati_block_element;
typedef struct atomic_register{
    int atomic_entry;
    int atomic_exit;
    int lockScrittore;
    int lockInvalid;
    uint64_t bitmask[DIMENSIONEBITMASK]; 
    metadati_block_element * testa;
    metadati_block_element * coda;
}atomic_register;
//struttura dei metadati di ogni blocco del fs
typedef struct myfiledata_struct{
    int num;
}myfiledata_struct;
#define MAXBLOCKDATA 4096 - sizeof(uint64_t)
//struttura di ogni blocco del fs
typedef struct block_file_struct{
    uint64_t size;
    char dati[MAXBLOCKDATA]; 
}block_file_struct;

//NB l'indice del blocco é ricavato tramite la posizione 
//come sb*DIMENSIONE+index
typedef struct posizione{
    int sb; //non serve
    int offset; //offset
}posizione;
typedef struct elementoVettoreMetadati{
    posizione succ;
    posizione prec;
    uint64_t valid:1;
}elementoVettoreMetadati;
//definizione dello specific superblock
//altri metadati vengono scritti su alti blocchi
typedef struct struct_sb_information{
    uint64_t magic;
    uint64_t version;
    uint64_t nblock;
    uint64_t nBlockMetadata;
    posizione first;
    posizione last;
    //padding da lasciare al superblock.
    char padding[4*1024-(4*sizeof(uint64_t)+2*sizeof(posizione))]; //potrei utilizzare questo come memoria ausiliaria
}struct_sb_information;

#define DIMENSIONEVETTORE (4*1024)/sizeof(elementoVettoreMetadati)
#define DIMENSIONEPADDING (4*1024 - DIMENSIONEVETTORE*sizeof(elementoVettoreMetadati))
#define NUMEROMETADATABLOCK (NBLOCK/DIMENSIONEVETTORE+1*((NBLOCK % 64)!=0))
typedef struct struct_sb_metadata{
    elementoVettoreMetadati vet[DIMENSIONEVETTORE]; //porta un pò di frammentazione interna
    char padding[DIMENSIONEPADDING];
}struct_sb_metadata;

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

extern struct super_block * the_sb;
//file.c
extern const struct inode_operations onefilefs_inode_ops;
extern const struct file_operations onefilefs_file_operations;
// dir.c
extern const struct file_operations onefilefs_dir_operations;
extern void stampa_invalid(void);
extern void stampa_valid(void);
extern int put_data(char * source,size_t size);
extern int get_data(int offset,char * destination ,size_t size);
extern int invalidate_data(int );
extern uint64_t getTime(void);
extern void setBit(int);
extern int checkBit(int);
extern void printBitMask(void);
extern int trovaBit(void);