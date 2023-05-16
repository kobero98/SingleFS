#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/mnt_idmapping.h>
#include <linux/types.h>
#include <linux/string.h>

#include "SingleFileSystem.h"

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

metadati_block_ram metadata_vector[NBLOCK];
static int countFSmount = 0;
//driver da implementare!
static struct super_operations myFileSystem_super_ops = {};
static struct dentry_operations myFileSystem_dentry_ops={};
static struct inode_operations myFileSystem_inode_ops = {};
static struct file_operations myFileSystem_file_ops = {};
static struct file_operations myFileSystem_dir_ops = {};
void init_struct_block(struct buffer_head *,int );
void stampa_mvector(void);

void stampa_mvector(){
    printk("il ponter vector sta a: %p\n",metadata_vector);
    for(int i=0;i<NBLOCK;i++){
        printk("%d] %d %d %d \n",i,metadata_vector[i].time,metadata_vector[i].valid,metadata_vector[i].dimension);
    }
}

void init_struct_block(struct buffer_head *b,int j){
    printk("qua va 1.j?\n");
 /*   metadati_block_struct data_read = ((block_file_struct*) b->b_data)->block_information;
    (data+j)->time = data_read.time;
    printk("qua va 3.j?\n");
    (data+j)->valid = data_read.valid;
    (data+j)->dimension = data_read.dimension;
    printk("qua va 3.j?\n");
    (data+j)->index_block = j;
    (data+j)->countLettore = 0;
    (data+j)->countScrittore= 0;
    printk("qua va 4.j?\n");
    brelse(b);
    */
    return;
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
    //metadata_vector = (metadati_block_ram *) kmalloc(sizeof(metadati_block_ram)*NBLOCK,0);
    if(metadata_vector == NULL) printk("errno \n");
    for(int j=0;j<NBLOCK;j++){
        //init_struct_block(sb_bread(sb,j+2),j);
        b = sb_bread(sb,j+2);
        metadati_block_struct data_read = ((block_file_struct*) b->b_data)->block_information;
        metadata_vector[j].time = data_read.time;
        metadata_vector[j].valid = data_read.valid;
        metadata_vector[j].dimension = data_read.dimension;
        metadata_vector[j].index_block = j+2;
        metadata_vector[j].countLettore = 0;
        metadata_vector[j].countScrittore= 0;
        brelse(b);
    }
    stampa_mvector();
    printk("metadatapointer: %p",metadata_vector);
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
__SYSCALL_DEFINEx(2, _put_data, char *, A, size_t, B){
#else
asmlinkage long sys_put_data(char* A, size_t B){
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


int init_func(void){
    //TO-DO: da incrementare il contatore del modulo!
    //inserimento Systemcall
    if(systemcall_table!=0){
        cr0 = read_cr0();
        unprotect_memory();
        hacked_syscall_tbl[free_entries[0]] = (unsigned long*)sys_put_data;
        protect_memory();
	    printk("%s: a sys_call with 2 parameters has been installed as a trial on the sys_call_table at displacement %d\n",MODNAME,FIRST_NI_SYSCALL);	
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