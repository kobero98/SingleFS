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

metadati_block_ram * testa_valid;
metadati_block_ram * testa_invalid;
static int ticketToWrite = 0;
static int countFSmount = 0;

//driver da implementare!
static struct super_operations myFileSystem_super_ops = {};
static struct dentry_operations myFileSystem_dentry_ops={};
static struct inode_operations myFileSystem_inode_ops = {};
static struct file_operations myFileSystem_file_ops = {};
static struct file_operations myFileSystem_dir_ops = {};

void inserimento_incoda(metadati_block_ram ** ,metadati_block_ram*, int s);
void init_metadata(struct super_block * );
void stampa_invalid(void);
void stampa_valid(void);


void stampa_valid(){
    metadati_block_ram *q;
    int i=0;
    for(q=testa_valid;q!=NULL;q=q->next){
        printk("%d] %d %d %d \n",i,q->time,q->valid,q->dimension);
        i++;
    }
}
void stampa_invalid(){
    metadati_block_ram *q;
    int i=0;
    for(q=testa_invalid;q!=NULL;q=q->next){
        printk("%d] %d %d %d \n",i,q->time,q->valid,q->dimension);
        i++;
    }
}
void inserimento_incoda(metadati_block_ram ** doveInserire,metadati_block_ram* chiInserire,int s){
    if(likely(*doveInserire != NULL)){
        (*doveInserire)->next=chiInserire;
    }else{ 
        if(s==0){
            *doveInserire = chiInserire;
            testa_invalid = chiInserire;
        }
        else {
            *doveInserire = chiInserire;
            testa_valid = chiInserire;
        }   
    }

}
void init_metadata(struct super_block * sb){
    testa_valid=NULL;
    testa_invalid=NULL;
    metadati_block_ram* valid=testa_valid;
    metadati_block_ram* invalid=testa_invalid;
    for(int j=2;j<NBLOCK+2;j++){ 
        struct buffer_head *b = sb_bread(sb,j);
        metadati_block_ram *p= (metadati_block_ram*) kzalloc(sizeof(metadati_block_ram),0);
        if(p==NULL) return;
        metadati_block_struct data_read = ((block_file_struct*) b->b_data)->block_information;
        p->valid = data_read.valid;
        p->dimension = data_read.dimension;
        p->index_block = j;
        p->countLettore=0;
        p->countScrittore=0;
        p->next=NULL;
        if(p->valid){
            inserimento_incoda(&valid,p,1);
        }else{
            inserimento_incoda(&invalid,p,0);
        }
        brelse(b);
    }
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
    printk("%ld %ld\n",magic,nblock);
    if(magic != sb->s_magic) return -EBADF;
    printk("ciao1 %d\n",nblock);
    //if(nblock != NBLOCK) return -EBADF;
    sb->s_fs_info = NULL;
    sb->s_op = &myFileSystem_super_ops;
    //ora creo l'inode
    root_inode = iget_locked(sb,0);
    if(!root_inode) return  -ENOMEM;
    // 10 é l'id dell inode
    root_inode->i_ino = 10; //da sostituire con una macro 
    // questa istruzione é dipendendete dalla versione 6.3 la funzione inode_init_owner prende 4 parametri
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
    init_metadata(sb);
    stampa_invalid();
    stampa_valid();
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
    metadati_block_ram * q = testa_valid;
    metadati_block_ram * p = testa_valid;
    while(q!=NULL){
        p=p->next;
        kfree(q);
        q=p;
    }
    //free list invalid
    q = testa_invalid;
    p = testa_invalid;
    while(q!=NULL){
        p=p->next;
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
        put_data(A,B);
        return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
static unsigned long sys_put_data = (unsigned long) __x64_sys_put_data;	
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

int init_func(void){
    //TO-DO: da incrementare il contatore del modulo!
    //inserimento Systemcall
    if(systemcall_table!=0){
        cr0 = read_cr0();
        unprotect_memory();
        unsigned long * sys_call_table = (void*) systemcall_table; 
        sys_call_table[free_entries[0]] = (unsigned long*)sys_put_data;
        protect_memory();
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
    int  ret=unregister_filesystem(&myFileSystemType);
    if (likely(ret==0))
        printk("file system smontato con successo\n");
    else    
        printk("errore nel montaggio del file system %d\n",ret);
    return;
}

module_init(init_func);
module_exit(cleanup_func);