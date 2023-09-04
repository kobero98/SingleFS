#ifndef NBLOCK
#define NBLOCK 10
#endif
#define MAGICNUMBER 0x42424242
#define MAXBLOCKDATA 4096 - sizeof(uint64_t)
//definizione dello specific superblock
//altri metadati vengono scritti su alti blocchi
#define DIMENSIONEVETTORE (4*1024)/sizeof(elementoVettoreMetadati)
#define DIMENSIONEPADDING (4*1024 - DIMENSIONEVETTORE*sizeof(elementoVettoreMetadati))
#define NUMEROMETADATABLOCK (NBLOCK/DIMENSIONEVETTORE+1*((NBLOCK % 64)!=0))

//struttura di ogni blocco del fs
typedef struct block_file_struct{
    uint64_t size;
    char dati[MAXBLOCKDATA]; 
}block_file_struct;
//NB l'indice del blocco é ricavato tramite la posizione 
//come sb*DIMENSIONE+index
typedef struct posizione{
    int sb; //blocco di metadati di riferimento
    int offset; //offset
}posizione;

typedef struct elementoVettoreMetadati{
    posizione succ;
    posizione prec;
    uint64_t valid:1;
}elementoVettoreMetadati;

typedef struct struct_sb_metadata{
    elementoVettoreMetadati vet[DIMENSIONEVETTORE]; //porta un pò di frammentazione interna
    char padding[DIMENSIONEPADDING];
}struct_sb_metadata;

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
