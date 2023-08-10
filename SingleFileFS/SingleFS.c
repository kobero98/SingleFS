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
/*
    #ifdef MY_DEBUG
    #define AUDIT if(1)
    #else
    #define AUDIT if(0)
    #endif
*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matteo Federico");
MODULE_DESCRIPTION("un semplice modulo iniziale");
MODULE_VERSION("0.1");

metadati_block_element * testa;
metadati_block_element * coda;
atomic_register * info;
struct super_block * the_sb;
static int ticketToWrite = 0;
static int countFSmount = 0;

//driver da implementare!
static struct super_operations myFileSystem_super_ops = {};
static struct dentry_operations myFileSystem_dentry_ops={};
static struct inode_operations myFileSystem_inode_ops = {};
static struct file_operations myFileSystem_file_ops = {};
static struct file_operations myFileSystem_dir_ops = {};

void inserimento_incoda(metadati_block_element * );
void init_metadata(struct super_block *,struct_sb_information* );
void printBitMask(void);
int trovaBit(void);
void inserimento_incoda(metadati_block_element* elemento){
    if(likely(testa != NULL)){
        coda->next=elemento;
        coda = elemento;
    }else{ 
        testa=elemento;
        coda=testa;
    }
}
void setBit(int index){
    int i= index/64;
    __sync_fetch_and_or(&(info->bitmask[i]),(1 << (index%64)));
    printBitMask();    

}
//verifica il valore del bit del blocco
int checkBit(int index){
    int i,pos;
    i=index/64;
    pos = 1 << (index%64);
    return (info->bitmask[i] & pos) == pos;

}
//trova un blocco con bit pari a 0
int trovaBit(){
    int i;
    int index=-1;
    for(i=0;i<NBLOCK;i++){
        if(!checkBit(i)){
            index=i;
            break;
        }
    }
    return index;
}
void printBitMask(){
    int i;
    printk("BitMask\n");
    for(i=0;i<DIMENSIONEBITMASK;i++){
        printk("%d] %d",i,info->bitmask[i]);
    }
}
void init_metadata(struct super_block * sb,struct_sb_information * mysb){
    int j;
    testa=NULL;
    coda=testa;
    info = kmalloc(sizeof(atomic_register),GFP_KERNEL);
    for(j=0;j<DIMENSIONEBITMASK;j++){
        info->bitmask[j]=0;
    }
    //TO-DO: da inserire l'inserimento iniziale dei blocchi!
    //leggo i vari blocchi di metadati
    struct buffer_head * bh [mysb->nBlockMetadata];
    struct_sb_metadata * sbdata[mysb->nBlockMetadata];
    for(j=0;j<mysb->nBlockMetadata;j++){
        bh[j]=sb_bread(sb,j+1);
        sbdata[j]=(struct_sb_metadata *)bh[j]->b_data;
    }
    
    posizione i = mysb->first;
    while(i.index>=0){
        printk("indice %d",i.index);
        if(sbdata[i.index/NUMEROMETADATABLOCK]->vet[i.index%NUMEROMETADATABLOCK].valid==1){
            metadati_block_element* e=(metadati_block_element*)kmalloc(sizeof(metadati_block_element),GFP_KERNEL);
            e->block.index_block=i.index; //+i.sb*DIMENSIONEVETTORE;
            e->block.time=getTime(); //potevo anche asseganre il tempo alla struttura del blocco ma supponenodi sia tutto ordinato funziona lo stesso
            e->block.valid=1;
            e->next=NULL;
            setBit(i.index);
            inserimento_incoda(e);
        }
        i=(sbdata[i.index/NUMEROMETADATABLOCK])->vet[i.index].succ;
    }
    printk("exit from cicle\n");
    //qui rilascio i superblochi ma forse non serve
    for(j=0;j<mysb->nBlockMetadata;j++){
        brelse(bh[j]);
    }
    info->atomic_entry=0;
    info->atomic_exit=0;
    info->lockScrittore=0;
    info->lockInvalid=0;
    info->testa=testa;
    info->coda=coda;
    sb->s_fs_info=info;
}
//funzione per il riempimento del superblocco
int myFileSystem_fill_sb(struct super_block *sb,void*data,int silent){
    struct inode *root_inode;
    struct buffer_head *bh;  
    struct buffer_head * b;
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
    printk("magic:%ld, nblock:%ld\n",magic,nblock);
    if(magic != sb->s_magic) return -EBADF;
    //if(nblock != NBLOCK) return -EBADF; //questa da correggere
    sb->s_fs_info = NULL;
    sb->s_op = &myFileSystem_super_ops;
    //ora creo l'inode
    root_inode = iget_locked(sb,0);
    if(!root_inode) return  -ENOMEM;
    // 10 é l'id dell inode
    root_inode->i_ino = 10; //da sostituire con una macro 
    // questa istruzione é dipendendete dalla versione 6.3 la funzione inode_init_owner prende 4 parametri
    // FANFA: dalla 6.3 prendi mnt_idmap; dalla 5.12 alla 6.2 prendi user_namespace; prima di 5.12 il parametro manca
    inode_init_owner(&nop_mnt_idmap,root_inode, NULL, S_IFDIR);
    root_inode->i_sb = sb;
    //root_inode->i_op = &myFileSystem_inode_ops;//&onefilefs_inode_ops;     
    //root_inode->i_fop = &myFileSystem_dir_ops; //&onefilefs_dir_operations; credo che qui il prof possa essersi sbagliato
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
    printk("mount avvenuta con successo\n");
    return 0;
}
//funzione per il montaggio del filesystem
struct dentry * myFileSystem_mount(struct file_system_type * type, int flags, const char *dev_name,void *data){
    if(! __sync_bool_compare_and_swap(&countFSmount,0,1)){
        printk("non é possibile montare il file system\n");
        return ERR_PTR(-EIO);
    }
    struct dentry * ret =  mount_bdev(type,flags,dev_name,data,myFileSystem_fill_sb); //forse bisogna utilizzare mount_nodev poi studio meglio.
    if (unlikely(IS_ERR(ret)))
        printk("Errore durante il montaggio del filesystem");
    else
        printk("montaggio avvenuto con successo");
    return ret;
}
//funzione per lo smontaggio del filesystem
static void myFileSystem_kill_sb(struct super_block *sb){
    if(! __sync_bool_compare_and_swap(&countFSmount,1,0)){
        printk("non é possibile smontare il file system\n");
        return ;
    }
    //free list valid
    metadati_block_element *p;
    metadati_block_element * q = testa;
    while(q!=NULL){
        p=q->next;
        kfree(q);
        q=p;
    }
    kill_block_super(sb);
    printk("file system smontato\n");
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
        printk("put\n");
        ret=put_data(A,B);
        return ret;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
__SYSCALL_DEFINEx(3,_get_data,int, A ,char *, B, size_t, C){
#else
asmlinkage int sys_get_data(int A, char* B, size_t C){
#endif
        int ret;
        printk("get\n");
        ret=get_data(A,B,C);
        return ret;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
__SYSCALL_DEFINEx(1,_invalidate_data, int, A){
#else
asmlinkage int sys_invalidate_data(int A){
#endif
        int ret;
        printk("invalidate\n");
        ret=invalidate_data(A);
        return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
static unsigned long sys_get_data = (unsigned long) __x64_sys_get_data;	
static unsigned long sys_put_data = (unsigned long) __x64_sys_put_data;
static unsigned long sys_invalidate_data = (unsigned long) __x64_sys_invalidate_data;	
#else
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
    //TO-DO: da incrementare il contatore del modulo!
    //inserimento Systemcall
    if(systemcall_table!=0){
        cr0 = read_cr0();
        unprotect_memory();
        unsigned long * sys_call_table = (void*) systemcall_table; 
        nisyscall = sys_call_table[free_entries[0]];
        sys_call_table[free_entries[0]] = (unsigned long*)sys_put_data;
        sys_call_table[free_entries[1]] = (unsigned long*)sys_get_data;
        sys_call_table[free_entries[2]] = (unsigned long*)sys_invalidate_data;
        protect_memory();
        printk("put in: %d,get  in: %d, invalide in: %d",free_entries[0],free_entries[1],free_entries[2]);
	    printk("%s: a sys_call with 2 parameters has been installed as a trial on the sys_call_table at displacement\n",MOD_NAME);	
    }
    int  ret=register_filesystem(&myFileSystemType);
    printk("il valore della macro é %d\n",NBLOCK);
    if (likely(ret==0))
        printk("file system inserito con successo\n");
    else   
        printk("errore nel montaggio del file system %d\n",ret);
    return ret;
}

void cleanup_func(void){
    cr0 = read_cr0();
    unprotect_memory();
    unsigned long * sys_call_table = (void*) systemcall_table; 
    sys_call_table[free_entries[0]] = nisyscall;
    sys_call_table[free_entries[1]] = nisyscall;
    sys_call_table[free_entries[2]] = nisyscall;
    protect_memory();
    printk("Systemcall eliminate\n");    
    int  ret=unregister_filesystem(&myFileSystemType);
    if (likely(ret==0))
        printk("file system smontato con successo\n");
    else    
        printk("errore nel montaggio del file system %d\n",ret);
    return;
}

module_init(init_func);
module_exit(cleanup_func);
