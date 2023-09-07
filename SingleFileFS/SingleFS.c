#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/mnt_idmapping.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include "SingleFileSystem.h"
#include "./../SystemCallFind/SystemCallFind.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matteo Federico");
MODULE_DESCRIPTION("un semplice modulo iniziale");
MODULE_VERSION("0.1");

metadati_block_element * testa;
metadati_block_element * coda;
atomic_register * info;
struct super_block * the_sb;
static int countFSmount = 0;

//driver da implementare!
static struct super_operations myFileSystem_super_ops = {};
static struct dentry_operations myFileSystem_dentry_ops={};


void inserimento_incoda(metadati_block_element * );
void init_metadata(struct super_block *,struct_sb_information* );
int trovaBit(void);
void printBitMask(void);

void printBitMask(void){
    int i;
    printk("Dim BitMask: %ld\n",DIMENSIONEBITMASK);
    for(i=0;i<DIMENSIONEBITMASK;i++) printk("Bitmask: %d\n",info->bitmask[i]);
}
void inserimento_incoda(metadati_block_element* elemento){
    if(likely(testa != NULL)){
        coda->next=elemento;
        coda = elemento;
    }else{ 
        testa=elemento;
        coda=testa;
    }
}

//imposta un bit a 1 della bit mask corrispondente
void setBitUP(int index){
    int i= index/DIMENSIONE_ELEMENTO_BITMASK;
    __sync_fetch_and_or(&(info->bitmask[i]),(1 << (index%DIMENSIONE_ELEMENTO_BITMASK)));    
    AUDIT printBitMask();
}
//imposta un bit a 0 della bit mask corrispondente
void setBitDown(int index){
    int i= index/DIMENSIONE_ELEMENTO_BITMASK;
    uint64_t x = ~(1 << (index%DIMENSIONE_ELEMENTO_BITMASK));
    __sync_fetch_and_and(&(info->bitmask[i]),x);
    AUDIT printBitMask();
}
//verifica il valore del bit del blocco
//torna 1 se nella bitmask il bit index é 1
//torna 0 se nella bitmask il bit index é 0 
int checkBit(int index){
    int i;
    uint64_t pos;
    i=index/DIMENSIONE_ELEMENTO_BITMASK;
    pos = 1 << (index%DIMENSIONE_ELEMENTO_BITMASK);
    return (info->bitmask[i] & pos) == pos;
}
//trova un blocco con bit pari a 0
int trovaBit(){
    int i,index;
    index=-1;
    AUDIT printBitMask();
    for(i=0;i<NBLOCK;i++){
        if(!checkBit(i)){
            index=i;
            break;
        }
    }
    return index;
}
void init_metadata(struct super_block * sb,struct_sb_information * mysb){
    int j,h;
    posizione i;
    //struct buffer_head * bh [mysb->nBlockMetadata];
    //struct_sb_metadata * sbdata[mysb->nBlockMetadata];
    struct buffer_head ** bh;
    struct_sb_metadata ** sbdata;
    registro_atomico* reg;


    bh =(struct buffer_head **) kmalloc(sizeof(struct buffer_head *)*mysb->nBlockMetadata,GFP_KERNEL);
    sbdata =(struct_sb_metadata **) kmalloc(sizeof(struct_sb_metadata *)*mysb->nBlockMetadata,GFP_KERNEL);
    
    testa=NULL;
    coda=testa;
    info = kmalloc(sizeof(atomic_register),GFP_KERNEL);
    for(j=0;j<DIMENSIONEBITMASK;j++){
        info->bitmask[j]=0;
    }
    //leggo i vari blocchi di metadati
    for(j=0;j<mysb->nBlockMetadata;j++){
        bh[j]=sb_bread(sb,j+1);
        sbdata[j]=(struct_sb_metadata *)bh[j]->b_data;
    }
    i = mysb->first;
    h=0;
    while(i.offset>=0 && h<NBLOCK ){
        AUDIT printk("indice %d, sb_i %d, Num %lld\n",i.offset,i.sb,mysb->nBlockMetadata);
        if(sbdata[i.sb]->vet[i.offset].valid==1){
            metadati_block_element* e=(metadati_block_element*)kmalloc(sizeof(metadati_block_element),GFP_KERNEL);
            e->block.index_block=i.offset+i.sb*DIMENSIONEVETTORE;
            e->block.time=getTime(); //potevo anche asseganre il tempo alla struttura del blocco ma supponenodi sia tutto ordinato funziona lo stesso
            e->next=NULL;
            setBitUP(e->block.index_block);
            inserimento_incoda(e);
        }
        i=sbdata[i.sb]->vet[i.offset].succ;
        h++;
    }
    AUDIT printk("h=%d",h);
    //qui rilascio i superblochi ma forse non serve
    for(j=0;j<mysb->nBlockMetadata;j++){
        brelse(bh[j]);
    }
    reg=(registro_atomico*)kmalloc(sizeof(registro_atomico),GFP_KERNEL);
    reg->num_entry=0;
    reg->num_exit=0;
    info->reg=reg;
    info->lockScrittore=0;
    info->lockInvalid=0;
    info->testa=testa;
    info->coda=coda;
    sb->s_fs_info=info;
    kfree(bh);
    kfree(sbdata);
}
//funzione per il riempimento del superblocco
int myFileSystem_fill_sb(struct super_block *sb,void*data,int silent){
    struct inode *root_inode;
    struct buffer_head *bh;  
    struct_sb_information *mysb;
    struct timespec64 curr_time;
    uint64_t magic;
    uint64_t nblock;

    sb->s_magic= MAGICNUMBER;
    bh = sb_bread(sb,0); //sostituire 0 con una macro lo 0 indica dove sta posizionato il super block 
    //qui forse ci sta un errore
    if(!sb) return -EIO;
    mysb = (struct_sb_information *)bh->b_data;
    //qui il prof ha messo un controllo sul meagic number che non ho ben compreso...
    magic = mysb->magic;
    nblock = mysb->nblock;
    brelse(bh);
    if(magic != sb->s_magic) return -EBADF;
    if(nblock != NBLOCK) return -EBADF; //questa da correggere
    sb->s_fs_info = NULL;
    sb->s_op = &myFileSystem_super_ops;
    //ora creo l'inode
    root_inode = iget_locked(sb,0);
    if(!root_inode) return  -ENOMEM;
    // 10 é l'id dell inode
    root_inode->i_ino = 10; //da sostituire con una macro 
    // questa istruzione é dipendendete dalla versione 6.3 la funzione inode_init_owner prende 4 parametri
    // FANFA: dalla 6.3 prendi mnt_idmap; dalla 5.12 alla 6.2 prendi user_namespace; prima di 5.12 il parametro manca
    #if LINUX_VERSION_CODE <= KERNEL_VERSION(5,12,0)
        inode_init_owner(root_inode, NULL, S_IFDIR);
    #elif LINUX_VERSION_CODE < KERNEL_VERSION(6,3,0)
        inode_init_owner(&init_user_ns,root_inode, NULL, S_IFDIR);
    #elif LINUX_VERSION_CODE >= KERNEL_VERSION(6,3,0)
        inode_init_owner(&nop_mnt_idmap,root_inode, NULL, S_IFDIR);
    #endif

    root_inode->i_sb = sb;
    root_inode->i_op = &onefilefs_inode_ops;
    root_inode->i_fop = &onefilefs_dir_operations;
    root_inode ->i_mode = S_IFDIR | S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IXUSR | S_IXGRP |  S_IXOTH;
    //DEVO LEGGERE ASSOLUTMANTE I_mode cosa gli sto dando...
    ktime_get_real_ts64(&curr_time);
    root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = curr_time;
    root_inode->i_private = NULL;
    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) return -ENOMEM;
    sb->s_root->d_op = &myFileSystem_dentry_ops;
    unlock_new_inode(root_inode);
    init_metadata(sb,mysb);
    the_sb = sb; //mi salvo il superblocco per passarlo alle systemcall, posso salvarmi qualcosa di più piccolo! 
    printk("moduloFS: Mount avvenuta con successo\n");
    return 0;
}

//funzione per il montaggio del filesystem
struct dentry * myFileSystem_mount(struct file_system_type * type, int flags, const char *dev_name,void *data){
    struct dentry * ret;
    if(!__sync_bool_compare_and_swap(&countFSmount,0,1)){
        printk("moduloFS: non é possibile montare il file system\n");
        return ERR_PTR(-EIO);
    }
    ret =  mount_bdev(type,flags,dev_name,data,myFileSystem_fill_sb); //forse bisogna utilizzare mount_nodev poi studio meglio.
    if (unlikely(IS_ERR(ret)))
        printk("moduloFS-Errore: Durante il montaggio del filesystem");
    else
        printk("moduloFS: Montaggio avvenuto con successo");
    return ret;
}
//funzione per lo smontaggio del filesystem
static void myFileSystem_kill_sb(struct super_block *sb){
    metadati_block_element *p,*q;
    if(! __sync_bool_compare_and_swap(&countFSmount,1,0)){
        printk("moduloFS-Errore: File system non montato\n");
        return;
    }
    //free list
    q = info->testa;
    while(q!=NULL){
        p=q->next;
        kfree(q);
        q=p;
    }
    kfree(info->reg);
    kfree(info);
    kill_block_super(sb);
    printk("moduloFS: File system smontato\n");
    return ;
}
//my file system type kernel 6.3 credo vada bene devo controllare nelle versioni più aggiornate
static struct file_system_type myFileSystemType = {
    .owner=THIS_MODULE,
    .name="MyFileSystem",
    .mount=myFileSystem_mount,//funzione di montaggio del file system
    .kill_sb=myFileSystem_kill_sb,//funzione di smontaggio del file system
    //.fs_flags= FS_NO_DCACHE, //controllare quali altri flag possono essere utili
};
//declare SYS_CALL
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
__SYSCALL_DEFINEx(2,_put_data, char *, A, size_t, B){
#else
asmlinkage int sys_put_data(char* A, size_t B){
#endif
        int ret;
        if(!countFSmount){
            printk("moduloFS-Errore: Filesystem non montato\n");
            return -ENODEV;
        }
        if(B>MAXBLOCKDATA || B<=0){
            return -EINVAL;
        }
        AUDIT printk("moduloFS: Put\n");
        ret=put_data(A,B);
        AUDIT printk("moduloFS: END Put\n");
        return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
__SYSCALL_DEFINEx(3,_get_data,int, A ,char *, B, size_t, C){
#else
asmlinkage int sys_get_data(int A, char* B, size_t C){
#endif
        int ret;
        if(!countFSmount){
            printk("moduloFS-Errore: Filesystem non montato\n");
            return -ENODEV;
        }
        if(A<0 || A>=NBLOCK){
            return -EINVAL;
        }
        AUDIT printk("moduloFS: Get\n");
        ret=get_data(A,B,C);
        AUDIT printk("moduloFS: End Get\n");
        return ret;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
__SYSCALL_DEFINEx(1,_invalidate_data, int, A){
#else
asmlinkage int sys_invalidate_data(int A){
#endif
        int ret;
        if(!countFSmount){
            printk("moduloFS-Errore: Filesystem non montato\n");
            return -ENODEV;
        }
        AUDIT printk("moduloFS: Invalide\n");
        ret=invalidate_data(A);
        AUDIT printk("moduloFS: End Invalide\n");
        return ret;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
static unsigned long sys_get_data = (unsigned long) __x64_sys_get_data;	
static unsigned long sys_put_data = (unsigned long) __x64_sys_put_data;
static unsigned long sys_invalidate_data = (unsigned long) __x64_sys_invalidate_data;	
#endif

unsigned long systemcall_table=0x0;
module_param(systemcall_table,ulong,0660);
int free_entries[15];
module_param_array(free_entries,int,NULL,0660);

unsigned long cr0;

static inline void write_cr0_forced(unsigned long val){
    unsigned long __force_order;

    /* __asm__ __volatile__( */
    asm volatile(
        "mov %0, %%cr0"
        : "+r"(val), "+m"(__force_order));
}

static inline void protect_memory(void){
    write_cr0_forced(cr0);
}

static inline void unprotect_memory(void){
    write_cr0_forced(cr0 & ~X86_CR0_WP);
}
unsigned long * nisyscall;
int init_func(void){
    //inserimento Systemcall
    int ret;
    unsigned long ** sys_call_table;
    if(systemcall_table!=0){
        cr0 = read_cr0();
        unprotect_memory();
        sys_call_table = (void*) systemcall_table; 
        nisyscall = sys_call_table[free_entries[0]];
        sys_call_table[free_entries[0]] = (unsigned long*)sys_put_data;
        sys_call_table[free_entries[1]] = (unsigned long*)sys_get_data;
        sys_call_table[free_entries[2]] = (unsigned long*)sys_invalidate_data;
        protect_memory();
        printk("moduloFS: put in: %d, get  in: %d, invalide in: %d\n",free_entries[0],free_entries[1],free_entries[2]);
    }else{
        printk("moduloFS-Errore: systemcall Table non trovata\n");
        return -1;
    }
    ret=register_filesystem(&myFileSystemType);
    if (likely(ret==0))
        printk("moduloFS:modulo inserito con successo\n");
    else   
        printk("moduloFS-Errore: Registrazione del File System %d\n",ret);
    return ret;
}
void cleanup_func(void){
    int  ret;
    unsigned long ** sys_call_table;
    cr0 = read_cr0();
    unprotect_memory();
    sys_call_table = (void*) systemcall_table; 
    sys_call_table[free_entries[0]] = nisyscall;
    sys_call_table[free_entries[1]] = nisyscall;
    sys_call_table[free_entries[2]] = nisyscall;
    protect_memory();  
    ret=unregister_filesystem(&myFileSystemType);
    if (likely(ret==0))
        printk("moduloFS: modulo smontato con successo\n");
    else    
        printk("moduloFS-Errore: de-registrazione del FS %d\n",ret);
    return;
}

module_init(init_func);
module_exit(cleanup_func);